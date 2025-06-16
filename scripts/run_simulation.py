#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO Simulation Runner
========================

Command-line tool for running SemiPRO simulations with various configurations.
Supports batch processing, parameter sweeps, and result analysis.
"""

import sys
import os
import argparse
import asyncio
import yaml
import json
import time
from pathlib import Path
from typing import Dict, List, Any, Optional
import logging

# Add SemiPRO to path
sys.path.append(str(Path(__file__).parent.parent / "src" / "python"))

from api.client import SemiPROClient, SemiPROClientError
from api.serializers import serialize_to_json

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

class SimulationRunner:
    """SemiPRO simulation runner"""
    
    def __init__(self, api_url: str = "http://localhost:5000", output_dir: str = "output"):
        self.client = SemiPROClient(base_url=api_url)
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        
        # Results storage
        self.results = {}
        self.simulation_history = []
    
    def load_config(self, config_file: str) -> Dict[str, Any]:
        """Load simulation configuration from file"""
        config_path = Path(config_file)
        
        if not config_path.exists():
            raise FileNotFoundError(f"Configuration file not found: {config_file}")
        
        if config_path.suffix.lower() in ['.yaml', '.yml']:
            with open(config_path, 'r') as f:
                return yaml.safe_load(f)
        elif config_path.suffix.lower() == '.json':
            with open(config_path, 'r') as f:
                return json.load(f)
        else:
            raise ValueError(f"Unsupported configuration format: {config_path.suffix}")
    
    def save_results(self, results: Dict[str, Any], filename: str):
        """Save simulation results to file"""
        output_file = self.output_dir / filename
        
        if output_file.suffix.lower() == '.json':
            with open(output_file, 'w') as f:
                f.write(serialize_to_json(results))
        elif output_file.suffix.lower() in ['.yaml', '.yml']:
            with open(output_file, 'w') as f:
                yaml.dump(results, f, default_flow_style=False)
        else:
            # Default to JSON
            with open(output_file.with_suffix('.json'), 'w') as f:
                f.write(serialize_to_json(results))
        
        logger.info(f"Results saved to: {output_file}")
    
    async def run_single_simulation(self, config: Dict[str, Any], 
                                  simulation_name: str = "simulation") -> Dict[str, Any]:
        """Run a single simulation"""
        logger.info(f"Starting simulation: {simulation_name}")
        
        try:
            # Extract configuration sections
            wafer_config = config.get('wafer', {})
            process_steps = config.get('process_steps', [])
            simulation_config = config.get('simulation', {})
            
            # Validate configuration
            if not wafer_config:
                raise ValueError("Wafer configuration is required")
            if not process_steps:
                raise ValueError("Process steps are required")
            
            # Create simulator
            simulator_config = {
                'input_directory': str(self.output_dir / "input"),
                'output_directory': str(self.output_dir / "output")
            }
            
            # Create directories
            Path(simulator_config['input_directory']).mkdir(parents=True, exist_ok=True)
            Path(simulator_config['output_directory']).mkdir(parents=True, exist_ok=True)
            
            simulator_id = self.client.create_simulator(simulator_config)
            logger.info(f"Created simulator: {simulator_id}")
            
            try:
                # Create wafer
                self.client.create_wafer(simulator_id, wafer_config)
                logger.info(f"Created wafer: {wafer_config.get('material', 'unknown')} "
                          f"{wafer_config.get('diameter', 0)}mm")
                
                # Add process steps
                for i, step in enumerate(process_steps):
                    self.client.add_process_step(simulator_id, step)
                    logger.info(f"Added step {i+1}/{len(process_steps)}: {step.get('name', 'unnamed')}")
                
                # Start simulation
                wafer_name = simulation_config.get('wafer_name', simulation_name)
                task_id = self.client.start_simulation(simulator_id, wafer_name)
                logger.info(f"Started simulation with task ID: {task_id}")
                
                # Monitor progress
                start_time = time.time()
                last_progress = -1
                
                while True:
                    status = self.client.get_simulation_status(task_id)
                    current_progress = status.get('progress', {}).get('progress_percentage', 0)
                    
                    if current_progress != last_progress:
                        logger.info(f"Progress: {current_progress:.1f}% - "
                                  f"{status.get('progress', {}).get('current_operation', 'Processing...')}")
                        last_progress = current_progress
                    
                    if status['status'] in ['completed', 'failed', 'cancelled']:
                        break
                    
                    await asyncio.sleep(2)
                
                elapsed_time = time.time() - start_time
                logger.info(f"Simulation completed in {elapsed_time:.1f} seconds")
                
                if status['status'] == 'completed':
                    # Get results
                    results = self.client.get_results(simulator_id)
                    
                    # Add metadata
                    results['metadata'] = {
                        'simulation_name': simulation_name,
                        'elapsed_time': elapsed_time,
                        'timestamp': time.time(),
                        'config': config
                    }
                    
                    logger.info(f"✓ Simulation '{simulation_name}' completed successfully")
                    return results
                else:
                    error_msg = f"Simulation failed with status: {status['status']}"
                    logger.error(error_msg)
                    raise RuntimeError(error_msg)
            
            finally:
                # Cleanup simulator
                self.client.delete_simulator(simulator_id)
        
        except Exception as e:
            logger.error(f"Simulation '{simulation_name}' failed: {e}")
            raise
    
    async def run_batch_simulations(self, config_files: List[str]) -> Dict[str, Any]:
        """Run multiple simulations in batch"""
        logger.info(f"Starting batch simulation with {len(config_files)} configurations")
        
        batch_results = {}
        failed_simulations = []
        
        for i, config_file in enumerate(config_files):
            simulation_name = f"batch_{i+1}_{Path(config_file).stem}"
            
            try:
                config = self.load_config(config_file)
                results = await self.run_single_simulation(config, simulation_name)
                batch_results[simulation_name] = results
                
                # Save individual results
                self.save_results(results, f"{simulation_name}_results.json")
                
            except Exception as e:
                logger.error(f"Batch simulation {simulation_name} failed: {e}")
                failed_simulations.append((simulation_name, str(e)))
        
        # Create batch summary
        batch_summary = {
            'total_simulations': len(config_files),
            'successful_simulations': len(batch_results),
            'failed_simulations': len(failed_simulations),
            'failed_list': failed_simulations,
            'results': batch_results
        }
        
        # Save batch summary
        self.save_results(batch_summary, "batch_summary.json")
        
        logger.info(f"Batch simulation completed: {len(batch_results)}/{len(config_files)} successful")
        return batch_summary
    
    async def run_parameter_sweep(self, base_config: Dict[str, Any], 
                                sweep_params: Dict[str, List[Any]]) -> Dict[str, Any]:
        """Run parameter sweep simulation"""
        logger.info(f"Starting parameter sweep with {len(sweep_params)} parameters")
        
        # Generate all parameter combinations
        import itertools
        
        param_names = list(sweep_params.keys())
        param_values = list(sweep_params.values())
        combinations = list(itertools.product(*param_values))
        
        logger.info(f"Generated {len(combinations)} parameter combinations")
        
        sweep_results = {}
        
        for i, combination in enumerate(combinations):
            # Create configuration for this combination
            config = base_config.copy()
            param_dict = dict(zip(param_names, combination))
            
            # Apply parameters to configuration
            for param_path, value in param_dict.items():
                self._set_nested_param(config, param_path, value)
            
            simulation_name = f"sweep_{i+1}_" + "_".join(f"{k}_{v}" for k, v in param_dict.items())
            
            try:
                results = await self.run_single_simulation(config, simulation_name)
                sweep_results[simulation_name] = {
                    'parameters': param_dict,
                    'results': results
                }
                
                # Save individual results
                self.save_results(results, f"{simulation_name}_results.json")
                
            except Exception as e:
                logger.error(f"Parameter sweep simulation {simulation_name} failed: {e}")
        
        # Create sweep summary
        sweep_summary = {
            'sweep_parameters': sweep_params,
            'total_combinations': len(combinations),
            'successful_simulations': len(sweep_results),
            'results': sweep_results
        }
        
        # Save sweep summary
        self.save_results(sweep_summary, "parameter_sweep_summary.json")
        
        logger.info(f"Parameter sweep completed: {len(sweep_results)}/{len(combinations)} successful")
        return sweep_summary
    
    def _set_nested_param(self, config: Dict[str, Any], param_path: str, value: Any):
        """Set nested parameter in configuration"""
        keys = param_path.split('.')
        current = config
        
        for key in keys[:-1]:
            if key not in current:
                current[key] = {}
            current = current[key]
        
        current[keys[-1]] = value
    
    def create_default_config(self, output_file: str = "default_config.yaml"):
        """Create a default simulation configuration file"""
        default_config = {
            'wafer': {
                'diameter': 300.0,
                'thickness': 775.0,
                'material': 'silicon',
                'orientation': '(100)',
                'resistivity': 10.0,
                'doping_type': 'p-type',
                'width': 100,
                'height': 100
            },
            'process_steps': [
                {
                    'type': 'oxidation',
                    'name': 'gate_oxidation',
                    'parameters': {
                        'temperature': 1000.0,
                        'time': 0.5,
                        'atmosphere': 'dry'
                    }
                },
                {
                    'type': 'deposition',
                    'name': 'polysilicon_gate',
                    'parameters': {
                        'material': 'polysilicon',
                        'thickness': 0.3,
                        'temperature': 620.0
                    }
                },
                {
                    'type': 'doping',
                    'name': 'source_drain_implant',
                    'parameters': {
                        'dopant': 'phosphorus',
                        'concentration': 1e20,
                        'energy': 80.0,
                        'temperature': 1000.0
                    }
                }
            ],
            'simulation': {
                'wafer_name': 'default_wafer',
                'solver': {
                    'tolerance': 1e-8,
                    'max_iterations': 1000
                }
            }
        }
        
        output_path = self.output_dir / output_file
        with open(output_path, 'w') as f:
            yaml.dump(default_config, f, default_flow_style=False)
        
        logger.info(f"Created default configuration: {output_path}")
        return output_path

async def main():
    """Main function"""
    parser = argparse.ArgumentParser(description="SemiPRO Simulation Runner")
    parser.add_argument("--config", "-c", type=str,
                       help="Configuration file for single simulation")
    parser.add_argument("--batch", "-b", nargs='+',
                       help="Configuration files for batch simulation")
    parser.add_argument("--sweep", "-s", type=str,
                       help="Parameter sweep configuration file")
    parser.add_argument("--api-url", default="http://localhost:5000",
                       help="SemiPRO API server URL")
    parser.add_argument("--output-dir", "-o", default="output",
                       help="Output directory for results")
    parser.add_argument("--create-config", action="store_true",
                       help="Create default configuration file")
    parser.add_argument("--verbose", "-v", action="store_true",
                       help="Enable verbose logging")
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    runner = SimulationRunner(args.api_url, args.output_dir)
    
    try:
        if args.create_config:
            config_file = runner.create_default_config()
            print(f"Created default configuration: {config_file}")
            return
        
        if args.config:
            # Single simulation
            config = runner.load_config(args.config)
            results = await runner.run_single_simulation(config)
            runner.save_results(results, "simulation_results.json")
            print(f"✓ Simulation completed successfully")
        
        elif args.batch:
            # Batch simulation
            results = await runner.run_batch_simulations(args.batch)
            print(f"✓ Batch simulation completed: {results['successful_simulations']}/{results['total_simulations']} successful")
        
        elif args.sweep:
            # Parameter sweep
            sweep_config = runner.load_config(args.sweep)
            base_config = sweep_config.get('base_config', {})
            sweep_params = sweep_config.get('sweep_parameters', {})
            
            if not base_config or not sweep_params:
                raise ValueError("Sweep configuration must contain 'base_config' and 'sweep_parameters'")
            
            results = await runner.run_parameter_sweep(base_config, sweep_params)
            print(f"✓ Parameter sweep completed: {results['successful_simulations']}/{results['total_combinations']} successful")
        
        else:
            parser.print_help()
            print("\nExample usage:")
            print("  # Create default configuration")
            print("  python run_simulation.py --create-config")
            print("  # Run single simulation")
            print("  python run_simulation.py --config config.yaml")
            print("  # Run batch simulations")
            print("  python run_simulation.py --batch config1.yaml config2.yaml")
    
    except KeyboardInterrupt:
        logger.info("Simulation interrupted by user")
        sys.exit(1)
    except Exception as e:
        logger.error(f"Simulation failed: {e}")
        sys.exit(1)

if __name__ == "__main__":
    asyncio.run(main())

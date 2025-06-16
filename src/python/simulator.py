# Author: Dr. Mazharuddin Mohammed
from .geometry import PyGeometryManager, PyWafer
from .oxidation import PyOxidationModel
from .doping import PyDopingManager
from .photolithography import PyLithographyModel
from .deposition import PyDepositionModel
from .etching import PyEtchingModel
from .metallization import PyMetallizationModel
from .packaging import PyPackagingModel
from .thermal import PyThermalSimulationModel
from .reliability import PyReliabilityModel
from .renderer import PyVulkanRenderer
from .multi_die import PyMultiDieModel, PyDie, PyInterconnect
from .drc import PyDRCModel, PyDRCRule
from .advanced_visualization import PyAdvancedVisualizationModel

# Import orchestration components
try:
    from .simulation_orchestrator import (
        PySimulationOrchestrator, PyProcessStepDefinition,
        create_process_step, get_orchestrator,
        STEP_TYPE_OXIDATION, STEP_TYPE_DOPING, STEP_TYPE_LITHOGRAPHY,
        STEP_TYPE_DEPOSITION, STEP_TYPE_ETCHING, STEP_TYPE_METALLIZATION,
        STEP_TYPE_ANNEALING, STEP_TYPE_CMP, STEP_TYPE_INSPECTION,
        EXECUTION_MODE_SEQUENTIAL, EXECUTION_MODE_PARALLEL
    )
    ORCHESTRATION_AVAILABLE = True
except ImportError:
    ORCHESTRATION_AVAILABLE = False
    print("Warning: Simulation orchestration not available")

import numpy as np
import matplotlib.pyplot as plt
from typing import Dict, List, Optional, Tuple, Any, Callable
import json
import yaml
import asyncio
import logging
from pathlib import Path
import time

class Simulator:
    """Enhanced semiconductor process simulator with orchestration capabilities"""

    def __init__(self, config_file: Optional[str] = None):
        # Initialize core modules
        self.geometry_manager = PyGeometryManager()
        self.oxidation_model = PyOxidationModel()
        self.doping_manager = PyDopingManager()
        self.lithography_model = PyLithographyModel()
        self.deposition_model = PyDepositionModel()
        self.etching_model = PyEtchingModel()
        self.metallization_model = PyMetallizationModel()
        self.packaging_model = PyPackagingModel()
        self.thermal_model = PyThermalSimulationModel()
        self.reliability_model = PyReliabilityModel()
        self.multi_die_model = PyMultiDieModel()
        self.drc_model = PyDRCModel()
        self.advanced_visualization = PyAdvancedVisualizationModel()
        self.wafer = PyWafer(300.0, 775.0, "silicon")
        self.renderer = None

        # Initialize orchestration
        if ORCHESTRATION_AVAILABLE:
            self.orchestrator = get_orchestrator()
            if config_file:
                self.orchestrator.load_configuration(config_file)
        else:
            self.orchestrator = None

        # Process flow management
        self.current_flow = []
        self.process_history = []
        self.simulation_results = {}

        # Configuration
        self.input_directory = "input"
        self.output_directory = "output"
        self.enable_logging = True

        # Setup logging
        if self.enable_logging:
            self._setup_logging()

        # Callbacks
        self.progress_callback = None
        self.completion_callback = None
        self.error_callback = None

    def initialize_geometry(self, x_dim: int, y_dim: int):
        self.geometry_manager.initialize_grid(self.wafer, x_dim, y_dim)

    def apply_layer(self, thickness: float, material_id: str):
        self.geometry_manager.apply_layer(self.wafer, thickness, material_id)

    def run_oxidation(self, temperature: float, time: float):
        self.oxidation_model.simulate_oxidation(self.wafer, temperature, time)

    def run_ion_implantation(self, energy: float, dose: float):
        self.doping_manager.simulate_ion_implantation(self.wafer, energy, dose)

    def run_diffusion(self, temperature: float, time: float):
        self.doping_manager.simulate_diffusion(self.wafer, temperature, time)

    def run_exposure(self, wavelength: float, na: float, mask: list):
        self.lithography_model.simulate_exposure(self.wafer, wavelength, na, mask)

    def run_multi_patterning(self, wavelength: float, na: float, masks: list):
        self.lithography_model.simulate_multi_patterning(self.wafer, wavelength, na, masks)

    def run_deposition(self, thickness: float, material: str, type: str):
        self.deposition_model.simulate_deposition(self.wafer, thickness, material, type)

    def run_etching(self, depth: float, type: str):
        self.etching_model.simulate_etching(self.wafer, depth, type)

    def run_metallization(self, thickness: float, metal: str, method: str):
        self.metallization_model.simulate_metallization(self.wafer, thickness, metal, method)

    def simulate_packaging(self, substrate_thickness: float, substrate_material: str, num_wires: int):
        self.packaging_model.simulate_packaging(self.wafer, substrate_thickness, substrate_material, num_wires)

    def perform_electrical_test(self, test_type: str):
        self.packaging_model.perform_electrical_test(self.wafer, test_type)

    def simulate_thermal(self, ambient_temperature: float, current: float):
        self.thermal_model.simulate_thermal(self.wafer, ambient_temperature, current)

    def perform_reliability_test(self, current: float, voltage: float):
        self.reliability_model.perform_reliability_test(self.wafer, current, voltage)

    def initialize_renderer(self, width: int, height: int):
        self.renderer = PyVulkanRenderer(width, height)
        self.renderer.initialize()

    # Multi-die integration methods
    def add_die(self, die_id: str, die_type: int, width: float, height: float, thickness: float):
        die = PyDie(die_id, die_type, width, height, thickness)
        self.multi_die_model.add_die(die)
        return die

    def position_die(self, die_id: str, x: float, y: float):
        self.multi_die_model.position_die(die_id, x, y)

    def perform_wire_bonding(self, die1: str, die2: str, bonds: list):
        self.multi_die_model.perform_wire_bonding(self.wafer, die1, die2, bonds)

    def perform_flip_chip_bonding(self, die1: str, die2: str, bump_pitch: float, bump_diameter: float):
        self.multi_die_model.perform_flip_chip_bonding(self.wafer, die1, die2, bump_pitch, bump_diameter)

    def perform_tsv_integration(self, die_id: str, tsv_positions: list, tsv_diameter: float, tsv_depth: float):
        self.multi_die_model.perform_tsv_integration(self.wafer, die_id, tsv_positions, tsv_diameter, tsv_depth)

    def analyze_electrical_performance(self):
        self.multi_die_model.analyze_electrical_performance(self.wafer)

    def analyze_thermal_performance(self):
        self.multi_die_model.analyze_thermal_performance(self.wafer)

    def get_system_metrics(self):
        return self.multi_die_model.get_system_metrics()

    # Design Rule Check methods
    def add_drc_rule(self, rule_name: str, rule_type: int, layer: str, min_value: float, max_value: float = -1.0):
        rule = PyDRCRule(rule_name, rule_type, layer, min_value, max_value)
        self.drc_model.add_rule(rule)
        return rule

    def load_technology_rules(self, technology_node: str):
        self.drc_model.load_technology_rules(technology_node)

    def run_full_drc(self):
        self.drc_model.run_full_drc(self.wafer)

    def run_layer_drc(self, layer: str):
        self.drc_model.run_layer_drc(self.wafer, layer)

    def get_violation_count(self):
        return self.drc_model.get_violation_count()

    def get_critical_violation_count(self):
        return self.drc_model.get_critical_violation_count()

    def generate_drc_report(self, filename: str):
        self.drc_model.generate_drc_report(filename)

    def get_violations_by_layer(self):
        return self.drc_model.get_violations_by_layer()

    # Advanced visualization methods
    def set_rendering_mode(self, mode: int):
        self.advanced_visualization.set_rendering_mode(mode)

    def enable_layer(self, layer: int, enabled: bool):
        self.advanced_visualization.enable_layer(layer, enabled)

    def set_camera_position(self, x: float, y: float, z: float):
        self.advanced_visualization.set_camera_position(x, y, z)

    def set_camera_target(self, x: float, y: float, z: float):
        self.advanced_visualization.set_camera_target(x, y, z)

    def orbit_camera(self, theta: float, phi: float, radius: float):
        self.advanced_visualization.orbit_camera(theta, phi, radius)

    def add_light(self, position: list, color: list, intensity: float):
        self.advanced_visualization.add_light(position, color, intensity)

    def start_animation(self):
        self.advanced_visualization.start_animation()

    def stop_animation(self):
        self.advanced_visualization.stop_animation()

    def enable_volumetric_rendering(self, enabled: bool):
        self.advanced_visualization.enable_volumetric_rendering(enabled)

    def render_temperature_field(self):
        self.advanced_visualization.render_temperature_field(self.wafer)

    def render_dopant_distribution(self):
        self.advanced_visualization.render_dopant_distribution(self.wafer)

    def export_image(self, filename: str, width: int, height: int):
        self.advanced_visualization.export_image(filename, width, height)

    def export_stl(self, filename: str):
        self.advanced_visualization.export_stl(self.wafer, filename)

    def enable_bloom(self, enabled: bool, threshold: float = 1.0, intensity: float = 1.0):
        self.advanced_visualization.enable_bloom(enabled, threshold, intensity)

    def set_rendering_quality(self, quality: float):
        self.advanced_visualization.set_rendering_quality(quality)

    def render_advanced(self):
        self.advanced_visualization.render(self.wafer)

    def get_frame_rate(self):
        return self.advanced_visualization.get_frame_rate()

    def get_render_time(self):
        return self.advanced_visualization.get_render_time()

    def render(self, wafer):
        if self.renderer:
            self.renderer.render(wafer)

    def get_wafer(self):
        return self.wafer

    def get_renderer_window(self):
        return self.renderer.get_window()

    # ========== ORCHESTRATION METHODS ==========

    def _setup_logging(self):
        """Setup logging for simulation tracking"""
        logging.basicConfig(
            level=logging.INFO,
            format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
            handlers=[
                logging.FileHandler(f"{self.output_directory}/simulation.log"),
                logging.StreamHandler()
            ]
        )
        self.logger = logging.getLogger("SemiPRO.Simulator")

    # Configuration Management
    def load_configuration(self, config_file: str):
        """Load simulation configuration from file"""
        if self.orchestrator:
            self.orchestrator.load_configuration(config_file)
            self.logger.info(f"Loaded configuration from {config_file}")
        else:
            self.logger.warning("Orchestration not available")

    def save_configuration(self, config_file: str):
        """Save current configuration to file"""
        if self.orchestrator:
            self.orchestrator.save_configuration(config_file)
            self.logger.info(f"Saved configuration to {config_file}")

    def set_parameter(self, key: str, value: str):
        """Set a configuration parameter"""
        if self.orchestrator:
            self.orchestrator.set_parameter(key, value)
        else:
            # Store locally if orchestrator not available
            if not hasattr(self, '_local_config'):
                self._local_config = {}
            self._local_config[key] = value

    def get_parameter(self, key: str) -> str:
        """Get a configuration parameter"""
        if self.orchestrator:
            return self.orchestrator.get_parameter(key)
        else:
            return getattr(self, '_local_config', {}).get(key, "")

    # Process Flow Management
    def create_process_step(self, step_type: str, name: str, **kwargs) -> Optional[Any]:
        """Create a process step definition"""
        if ORCHESTRATION_AVAILABLE:
            return create_process_step(step_type, name, **kwargs)
        else:
            # Create local step representation
            step = {
                'type': step_type,
                'name': name,
                'parameters': kwargs.get('parameters', {}),
                'input_files': kwargs.get('input_files', []),
                'output_files': kwargs.get('output_files', []),
                'dependencies': kwargs.get('dependencies', [])
            }
            return step

    def add_process_step(self, step):
        """Add a process step to the current flow"""
        if self.orchestrator and hasattr(step, '_step'):
            self.orchestrator.add_process_step(step)
        else:
            self.current_flow.append(step)
        self.logger.info(f"Added process step: {step.name if hasattr(step, 'name') else step.get('name', 'Unknown')}")

    def remove_process_step(self, step_name: str):
        """Remove a process step from the current flow"""
        if self.orchestrator:
            self.orchestrator.remove_process_step(step_name)
        else:
            self.current_flow = [s for s in self.current_flow if s.get('name') != step_name]
        self.logger.info(f"Removed process step: {step_name}")

    def load_simulation_flow(self, flow_file: str):
        """Load simulation flow from file"""
        if self.orchestrator:
            self.orchestrator.load_simulation_flow(flow_file)
            self.logger.info(f"Loaded simulation flow from {flow_file}")
        else:
            # Load YAML flow file manually
            try:
                with open(flow_file, 'r') as f:
                    flow_data = yaml.safe_load(f)
                self.current_flow = flow_data.get('steps', [])
                self.logger.info(f"Loaded flow with {len(self.current_flow)} steps")
            except Exception as e:
                self.logger.error(f"Failed to load flow file: {e}")

    def save_simulation_flow(self, flow_file: str):
        """Save current simulation flow to file"""
        if self.orchestrator:
            self.orchestrator.save_simulation_flow(flow_file)
        else:
            # Save flow manually
            try:
                flow_data = {
                    'name': 'Custom Flow',
                    'description': 'User-defined simulation flow',
                    'steps': self.current_flow
                }
                with open(flow_file, 'w') as f:
                    yaml.dump(flow_data, f, default_flow_style=False)
                self.logger.info(f"Saved simulation flow to {flow_file}")
            except Exception as e:
                self.logger.error(f"Failed to save flow file: {e}")

    # Execution Control
    async def execute_simulation_flow(self, wafer_name: str = "default") -> bool:
        """Execute the current simulation flow asynchronously"""
        if self.orchestrator:
            try:
                result = await self.orchestrator.execute_simulation(wafer_name)
                self.logger.info(f"Simulation flow completed with result: {result}")
                return result
            except Exception as e:
                self.logger.error(f"Simulation flow failed: {e}")
                return False
        else:
            # Execute flow manually
            return await self._execute_flow_manually(wafer_name)

    async def _execute_flow_manually(self, wafer_name: str) -> bool:
        """Execute flow manually when orchestrator is not available"""
        try:
            self.logger.info(f"Starting manual execution of {len(self.current_flow)} steps")

            for i, step in enumerate(self.current_flow):
                step_name = step.get('name', f'Step_{i}')
                step_type = step.get('type', 'unknown')
                parameters = step.get('parameters', {})

                self.logger.info(f"Executing step {i+1}/{len(self.current_flow)}: {step_name}")

                # Execute step based on type
                success = await self._execute_step_manually(step_type, parameters)

                if not success:
                    self.logger.error(f"Step {step_name} failed")
                    return False

                self.process_history.append({
                    'step': step_name,
                    'type': step_type,
                    'parameters': parameters,
                    'timestamp': time.time(),
                    'success': True
                })

                # Call progress callback if set
                if self.progress_callback:
                    progress = (i + 1) / len(self.current_flow) * 100
                    self.progress_callback(progress, step_name)

            self.logger.info("Manual simulation flow completed successfully")
            if self.completion_callback:
                self.completion_callback(True, "Simulation completed successfully")
            return True

        except Exception as e:
            self.logger.error(f"Manual simulation flow failed: {e}")
            if self.error_callback:
                self.error_callback(str(e))
            return False

    async def _execute_step_manually(self, step_type: str, parameters: Dict[str, Any]) -> bool:
        """Execute a single step manually"""
        try:
            if step_type == STEP_TYPE_OXIDATION or step_type == "oxidation":
                temp = parameters.get('temperature', 1000.0)
                time_val = parameters.get('time', 1.0)
                atmosphere = parameters.get('atmosphere', 'dry')
                return self.simulate_oxidation(temp, time_val, atmosphere)

            elif step_type == STEP_TYPE_DOPING or step_type == "doping":
                dopant = parameters.get('dopant', 'boron')
                concentration = parameters.get('concentration', 1e16)
                energy = parameters.get('energy', 50.0)
                temp = parameters.get('temperature', 1000.0)
                return self.simulate_doping(dopant, concentration, energy, temp)

            elif step_type == STEP_TYPE_LITHOGRAPHY or step_type == "lithography":
                wavelength = parameters.get('wavelength', 193.0)
                dose = parameters.get('dose', 25.0)
                mask_file = parameters.get('mask_file', '')
                return self.simulate_lithography(wavelength, dose, mask_file)

            elif step_type == STEP_TYPE_DEPOSITION or step_type == "deposition":
                material = parameters.get('material', 'silicon')
                thickness = parameters.get('thickness', 0.1)
                temp = parameters.get('temperature', 300.0)
                return self.simulate_deposition(material, thickness, temp)

            elif step_type == STEP_TYPE_ETCHING or step_type == "etching":
                depth = parameters.get('depth', 0.1)
                etch_type = parameters.get('type', 'anisotropic')
                return self.simulate_etching(depth, etch_type)

            elif step_type == STEP_TYPE_ANNEALING or step_type == "annealing":
                temp = parameters.get('temperature', 1000.0)
                time_val = parameters.get('time', 0.5)
                atmosphere = parameters.get('atmosphere', 'N2')
                return self.simulate_annealing(temp, time_val, atmosphere)

            else:
                self.logger.warning(f"Unknown step type: {step_type}")
                return True  # Don't fail for unknown steps

        except Exception as e:
            self.logger.error(f"Failed to execute step {step_type}: {e}")
            return False

    def pause_simulation(self):
        """Pause the current simulation"""
        if self.orchestrator:
            self.orchestrator.pause_simulation()
        self.logger.info("Simulation paused")

    def resume_simulation(self):
        """Resume the paused simulation"""
        if self.orchestrator:
            self.orchestrator.resume_simulation()
        self.logger.info("Simulation resumed")

    def cancel_simulation(self):
        """Cancel the current simulation"""
        if self.orchestrator:
            self.orchestrator.cancel_simulation()
        self.logger.info("Simulation cancelled")

    def get_simulation_progress(self) -> Dict[str, Any]:
        """Get current simulation progress"""
        if self.orchestrator:
            progress = self.orchestrator.get_progress()
            return {
                'state': progress.state,
                'current_step': progress.current_step,
                'total_steps': progress.total_steps,
                'progress_percentage': progress.progress_percentage,
                'current_operation': progress.current_operation,
                'completed_steps': progress.completed_steps,
                'errors': progress.errors
            }
        else:
            # Return manual progress
            return {
                'state': 'idle',
                'current_step': 0,
                'total_steps': len(self.current_flow),
                'progress_percentage': 0.0,
                'current_operation': 'Ready',
                'completed_steps': [h['step'] for h in self.process_history],
                'errors': []
            }

    # Input/Output Management
    def set_input_directory(self, directory: str):
        """Set input directory for simulation files"""
        self.input_directory = directory
        if self.orchestrator:
            self.orchestrator.set_input_directory(directory)
        Path(directory).mkdir(parents=True, exist_ok=True)
        self.logger.info(f"Input directory set to: {directory}")

    def set_output_directory(self, directory: str):
        """Set output directory for simulation results"""
        self.output_directory = directory
        if self.orchestrator:
            self.orchestrator.set_output_directory(directory)
        Path(directory).mkdir(parents=True, exist_ok=True)
        self.logger.info(f"Output directory set to: {directory}")

    def get_input_directory(self) -> str:
        """Get current input directory"""
        if self.orchestrator:
            return self.orchestrator.get_input_directory()
        return self.input_directory

    def get_output_directory(self) -> str:
        """Get current output directory"""
        if self.orchestrator:
            return self.orchestrator.get_output_directory()
        return self.output_directory

    # Callback Management
    def set_progress_callback(self, callback: Callable[[float, str], None]):
        """Set progress callback function"""
        self.progress_callback = callback

    def set_completion_callback(self, callback: Callable[[bool, str], None]):
        """Set completion callback function"""
        self.completion_callback = callback

    def set_error_callback(self, callback: Callable[[str], None]):
        """Set error callback function"""
        self.error_callback = callback

    # Enhanced simulation methods with orchestration
    def simulate_oxidation(self, temperature: float, time: float, atmosphere: str = "dry") -> bool:
        """Enhanced oxidation simulation with logging"""
        try:
            self.run_oxidation(temperature, time)
            self.logger.info(f"Oxidation: {temperature}°C, {time}h, {atmosphere}")
            return True
        except Exception as e:
            self.logger.error(f"Oxidation failed: {e}")
            return False

    def simulate_doping(self, dopant: str, concentration: float, energy: float, temperature: float) -> bool:
        """Enhanced doping simulation with logging"""
        try:
            self.run_ion_implantation(energy, concentration)
            self.run_diffusion(temperature, 0.5)  # Default diffusion time
            self.logger.info(f"Doping: {dopant}, {concentration:.2e} cm⁻³, {energy} keV")
            return True
        except Exception as e:
            self.logger.error(f"Doping failed: {e}")
            return False

    def simulate_lithography(self, wavelength: float, dose: float, mask_file: str = "") -> bool:
        """Enhanced lithography simulation with logging"""
        try:
            # Create dummy mask if none provided
            mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]] if not mask_file else self._load_mask(mask_file)
            self.run_exposure(wavelength, 0.75, mask)  # Default NA = 0.75
            self.logger.info(f"Lithography: {wavelength}nm, {dose} mJ/cm²")
            return True
        except Exception as e:
            self.logger.error(f"Lithography failed: {e}")
            return False

    def simulate_deposition(self, material: str, thickness: float, temperature: float) -> bool:
        """Enhanced deposition simulation with logging"""
        try:
            self.run_deposition(thickness, material, "CVD")  # Default CVD
            self.logger.info(f"Deposition: {material}, {thickness}μm, {temperature}°C")
            return True
        except Exception as e:
            self.logger.error(f"Deposition failed: {e}")
            return False

    def simulate_etching(self, depth: float, etch_type: str = "anisotropic") -> bool:
        """Enhanced etching simulation with logging"""
        try:
            self.run_etching(depth, etch_type)
            self.logger.info(f"Etching: {depth}μm, {etch_type}")
            return True
        except Exception as e:
            self.logger.error(f"Etching failed: {e}")
            return False

    def simulate_annealing(self, temperature: float, time: float, atmosphere: str = "N2") -> bool:
        """Enhanced annealing simulation with logging"""
        try:
            # Use thermal simulation for annealing
            self.simulate_thermal(temperature, 0.0)  # No current for annealing
            self.logger.info(f"Annealing: {temperature}°C, {time}h, {atmosphere}")
            return True
        except Exception as e:
            self.logger.error(f"Annealing failed: {e}")
            return False

    def _load_mask(self, mask_file: str) -> List[List[int]]:
        """Load mask pattern from file"""
        try:
            full_path = Path(self.input_directory) / mask_file
            if full_path.suffix.lower() == '.json':
                with open(full_path, 'r') as f:
                    return json.load(f)
            elif full_path.suffix.lower() in ['.yaml', '.yml']:
                with open(full_path, 'r') as f:
                    return yaml.safe_load(f)
            else:
                # Default mask pattern
                return [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
        except Exception as e:
            self.logger.warning(f"Failed to load mask {mask_file}: {e}")
            return [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
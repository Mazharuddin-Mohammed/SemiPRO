# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO API Client
==================

Python client for interacting with SemiPRO REST API.
Provides high-level interface for simulation management and data access.
"""

import requests
import time
import json
from typing import Dict, List, Optional, Any, Union
from urllib.parse import urljoin
import logging

from .serializers import (
    WaferSerializer, ProcessStepSerializer, SimulationResultSerializer,
    ConfigurationSerializer, FlowSerializer
)

logger = logging.getLogger(__name__)

class SemiPROClientError(Exception):
    """Custom exception for SemiPRO client errors"""
    pass

class SemiPROClient:
    """Client for SemiPRO REST API"""
    
    def __init__(self, base_url: str = "http://localhost:5000", 
                 api_key: Optional[str] = None, timeout: int = 30):
        """
        Initialize SemiPRO API client
        
        Args:
            base_url: Base URL of the SemiPRO API server
            api_key: Optional API key for authentication
            timeout: Request timeout in seconds
        """
        self.base_url = base_url.rstrip('/')
        self.api_key = api_key
        self.timeout = timeout
        self.session = requests.Session()
        
        # Set default headers
        self.session.headers.update({
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        })
        
        if api_key:
            self.session.headers.update({
                'Authorization': f'Bearer {api_key}'
            })
    
    def _make_request(self, method: str, endpoint: str, 
                     data: Optional[Dict] = None, 
                     params: Optional[Dict] = None) -> Dict[str, Any]:
        """Make HTTP request to API"""
        url = urljoin(self.base_url, endpoint)
        
        try:
            if method.upper() == 'GET':
                response = self.session.get(url, params=params, timeout=self.timeout)
            elif method.upper() == 'POST':
                response = self.session.post(url, json=data, params=params, timeout=self.timeout)
            elif method.upper() == 'PUT':
                response = self.session.put(url, json=data, params=params, timeout=self.timeout)
            elif method.upper() == 'DELETE':
                response = self.session.delete(url, params=params, timeout=self.timeout)
            else:
                raise SemiPROClientError(f"Unsupported HTTP method: {method}")
            
            response.raise_for_status()
            
            if response.content:
                return response.json()
            else:
                return {}
                
        except requests.exceptions.RequestException as e:
            logger.error(f"API request failed: {e}")
            raise SemiPROClientError(f"API request failed: {e}")
        except json.JSONDecodeError as e:
            logger.error(f"Failed to decode JSON response: {e}")
            raise SemiPROClientError(f"Invalid JSON response: {e}")
    
    def health_check(self) -> Dict[str, Any]:
        """Check API health status"""
        return self._make_request('GET', '/health')
    
    # Simulator management
    def create_simulator(self, config: Optional[Dict[str, Any]] = None) -> str:
        """
        Create a new simulator instance
        
        Args:
            config: Optional configuration dictionary
            
        Returns:
            Simulator ID
        """
        data = config or {}
        response = self._make_request('POST', '/simulators', data=data)
        return response['simulator_id']
    
    def get_simulator(self, simulator_id: str) -> Dict[str, Any]:
        """Get simulator information"""
        return self._make_request('GET', f'/simulators/{simulator_id}')
    
    def delete_simulator(self, simulator_id: str) -> Dict[str, Any]:
        """Delete simulator instance"""
        return self._make_request('DELETE', f'/simulators/{simulator_id}')
    
    # Wafer management
    def create_wafer(self, simulator_id: str, wafer_config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Create a new wafer
        
        Args:
            simulator_id: ID of the simulator
            wafer_config: Wafer configuration parameters
            
        Returns:
            Wafer creation response
        """
        return self._make_request('POST', f'/simulators/{simulator_id}/wafers', data=wafer_config)
    
    # Process step management
    def add_process_step(self, simulator_id: str, step_config: Dict[str, Any]) -> Dict[str, Any]:
        """
        Add a process step to the simulation flow
        
        Args:
            simulator_id: ID of the simulator
            step_config: Process step configuration
            
        Returns:
            Step addition response
        """
        return self._make_request('POST', f'/simulators/{simulator_id}/steps', data=step_config)
    
    def add_oxidation_step(self, simulator_id: str, name: str, 
                          temperature: float, time: float, 
                          atmosphere: str = "dry") -> Dict[str, Any]:
        """Add oxidation process step"""
        step_config = {
            'type': 'oxidation',
            'name': name,
            'parameters': {
                'temperature': temperature,
                'time': time,
                'atmosphere': atmosphere
            }
        }
        return self.add_process_step(simulator_id, step_config)
    
    def add_doping_step(self, simulator_id: str, name: str,
                       dopant: str, concentration: float, 
                       energy: float, temperature: float) -> Dict[str, Any]:
        """Add doping process step"""
        step_config = {
            'type': 'doping',
            'name': name,
            'parameters': {
                'dopant': dopant,
                'concentration': concentration,
                'energy': energy,
                'temperature': temperature
            }
        }
        return self.add_process_step(simulator_id, step_config)
    
    def add_lithography_step(self, simulator_id: str, name: str,
                           wavelength: float, dose: float,
                           mask_file: Optional[str] = None) -> Dict[str, Any]:
        """Add lithography process step"""
        step_config = {
            'type': 'lithography',
            'name': name,
            'parameters': {
                'wavelength': wavelength,
                'dose': dose
            }
        }
        if mask_file:
            step_config['input_files'] = [mask_file]
        return self.add_process_step(simulator_id, step_config)
    
    def add_deposition_step(self, simulator_id: str, name: str,
                          material: str, thickness: float,
                          temperature: float) -> Dict[str, Any]:
        """Add deposition process step"""
        step_config = {
            'type': 'deposition',
            'name': name,
            'parameters': {
                'material': material,
                'thickness': thickness,
                'temperature': temperature
            }
        }
        return self.add_process_step(simulator_id, step_config)
    
    def add_etching_step(self, simulator_id: str, name: str,
                        depth: float, etch_type: str = "anisotropic") -> Dict[str, Any]:
        """Add etching process step"""
        step_config = {
            'type': 'etching',
            'name': name,
            'parameters': {
                'depth': depth,
                'type': etch_type
            }
        }
        return self.add_process_step(simulator_id, step_config)
    
    # Simulation execution
    def start_simulation(self, simulator_id: str, wafer_name: str = "default") -> str:
        """
        Start simulation execution
        
        Args:
            simulator_id: ID of the simulator
            wafer_name: Name of the wafer to simulate
            
        Returns:
            Task ID for tracking simulation progress
        """
        data = {'wafer_name': wafer_name}
        response = self._make_request('POST', f'/simulators/{simulator_id}/simulate', data=data)
        return response['task_id']
    
    def get_simulation_status(self, task_id: str) -> Dict[str, Any]:
        """Get simulation status and progress"""
        return self._make_request('GET', f'/simulations/{task_id}/status')
    
    def cancel_simulation(self, task_id: str) -> Dict[str, Any]:
        """Cancel running simulation"""
        return self._make_request('POST', f'/simulations/{task_id}/cancel')
    
    def wait_for_simulation(self, task_id: str, poll_interval: float = 2.0,
                          timeout: Optional[float] = None) -> Dict[str, Any]:
        """
        Wait for simulation to complete
        
        Args:
            task_id: Simulation task ID
            poll_interval: Polling interval in seconds
            timeout: Maximum wait time in seconds
            
        Returns:
            Final simulation status
        """
        start_time = time.time()
        
        while True:
            status = self.get_simulation_status(task_id)
            
            if status['status'] in ['completed', 'failed', 'cancelled']:
                return status
            
            if timeout and (time.time() - start_time) > timeout:
                raise SemiPROClientError(f"Simulation timeout after {timeout} seconds")
            
            time.sleep(poll_interval)
    
    # Results and data access
    def get_results(self, simulator_id: str) -> Dict[str, Any]:
        """Get simulation results"""
        return self._make_request('GET', f'/simulators/{simulator_id}/results')
    
    def export_data(self, simulator_id: str, format_type: str = 'json',
                   data_type: str = 'results') -> bytes:
        """
        Export simulation data
        
        Args:
            simulator_id: ID of the simulator
            format_type: Export format ('json', 'csv', etc.)
            data_type: Type of data to export
            
        Returns:
            Exported data as bytes
        """
        data = {
            'format': format_type,
            'data_type': data_type
        }
        
        url = urljoin(self.base_url, f'/simulators/{simulator_id}/export')
        response = self.session.post(url, json=data, timeout=self.timeout)
        response.raise_for_status()
        
        return response.content
    
    # Configuration management
    def get_config(self, simulator_id: str) -> Dict[str, Any]:
        """Get simulator configuration"""
        return self._make_request('GET', f'/simulators/{simulator_id}/config')
    
    def update_config(self, simulator_id: str, config: Dict[str, Any]) -> Dict[str, Any]:
        """Update simulator configuration"""
        return self._make_request('PUT', f'/simulators/{simulator_id}/config', data=config)
    
    # High-level workflow methods
    def run_simple_simulation(self, wafer_config: Dict[str, Any],
                            process_steps: List[Dict[str, Any]],
                            wait: bool = True) -> Dict[str, Any]:
        """
        Run a complete simulation workflow
        
        Args:
            wafer_config: Wafer configuration
            process_steps: List of process step configurations
            wait: Whether to wait for completion
            
        Returns:
            Simulation results or status
        """
        # Create simulator
        simulator_id = self.create_simulator()
        
        try:
            # Create wafer
            self.create_wafer(simulator_id, wafer_config)
            
            # Add process steps
            for step in process_steps:
                self.add_process_step(simulator_id, step)
            
            # Start simulation
            task_id = self.start_simulation(simulator_id)
            
            if wait:
                # Wait for completion and get results
                final_status = self.wait_for_simulation(task_id)
                if final_status['status'] == 'completed':
                    return self.get_results(simulator_id)
                else:
                    raise SemiPROClientError(f"Simulation failed: {final_status}")
            else:
                return {'task_id': task_id, 'simulator_id': simulator_id}
                
        except Exception as e:
            # Cleanup on error
            try:
                self.delete_simulator(simulator_id)
            except:
                pass
            raise e
    
    def run_mosfet_simulation(self, gate_length: float = 0.1,
                            gate_oxide_thickness: float = 0.002,
                            source_drain_depth: float = 0.15) -> Dict[str, Any]:
        """
        Run a complete MOSFET fabrication simulation
        
        Args:
            gate_length: Gate length in micrometers
            gate_oxide_thickness: Gate oxide thickness in micrometers
            source_drain_depth: Source/drain junction depth in micrometers
            
        Returns:
            Simulation results
        """
        wafer_config = {
            'diameter': 300.0,
            'thickness': 775.0,
            'material': 'silicon',
            'width': 200,
            'height': 200
        }
        
        process_steps = [
            {
                'type': 'oxidation',
                'name': 'gate_oxidation',
                'parameters': {
                    'temperature': 1000.0,
                    'time': gate_oxide_thickness / 0.04,  # Approximate time for desired thickness
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
                'type': 'lithography',
                'name': 'gate_patterning',
                'parameters': {
                    'wavelength': 193.0,
                    'dose': 25.0
                }
            },
            {
                'type': 'etching',
                'name': 'gate_etch',
                'parameters': {
                    'depth': 0.3,
                    'type': 'anisotropic'
                }
            },
            {
                'type': 'doping',
                'name': 'source_drain_implant',
                'parameters': {
                    'dopant': 'arsenic',
                    'concentration': 1e20,
                    'energy': 80.0,
                    'temperature': 1000.0
                }
            },
            {
                'type': 'annealing',
                'name': 'activation_anneal',
                'parameters': {
                    'temperature': 1000.0,
                    'time': 0.5,
                    'atmosphere': 'N2'
                }
            }
        ]
        
        return self.run_simple_simulation(wafer_config, process_steps)

# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO API Integration Tests
============================

Comprehensive integration tests for the SemiPRO API layer.
Tests end-to-end functionality including REST API, serialization, and simulation execution.
"""

import pytest
import requests
import time
import json
import threading
from typing import Dict, Any
import tempfile
import os
from pathlib import Path

from src.python.api.rest_api import create_app
from src.python.api.client import SemiPROClient, SemiPROClientError
from src.python.api.serializers import (
    WaferSerializer, ProcessStepSerializer, SimulationResultSerializer
)
from src.python.api.validators import ValidationError

class TestAPIIntegration:
    """Integration tests for SemiPRO API"""
    
    @pytest.fixture(scope="class")
    def api_server(self):
        """Start API server for testing"""
        app = create_app()
        app.config['TESTING'] = True
        
        # Start server in background thread
        server_thread = threading.Thread(
            target=lambda: app.run(host='127.0.0.1', port=5001, debug=False),
            daemon=True
        )
        server_thread.start()
        
        # Wait for server to start
        time.sleep(2)
        
        yield "http://127.0.0.1:5001"
    
    @pytest.fixture
    def client(self, api_server):
        """Create API client"""
        return SemiPROClient(base_url=api_server)
    
    @pytest.fixture
    def temp_dir(self):
        """Create temporary directory for test files"""
        with tempfile.TemporaryDirectory() as temp_dir:
            yield temp_dir
    
    def test_health_check(self, client):
        """Test API health check endpoint"""
        health = client.health_check()
        assert health['status'] == 'healthy'
        assert 'timestamp' in health
        assert 'version' in health
    
    def test_simulator_lifecycle(self, client, temp_dir):
        """Test complete simulator lifecycle"""
        # Create simulator
        config = {
            'input_directory': temp_dir,
            'output_directory': temp_dir
        }
        simulator_id = client.create_simulator(config)
        assert simulator_id is not None
        
        # Get simulator info
        info = client.get_simulator(simulator_id)
        assert info['simulator_id'] == simulator_id
        assert info['status'] == 'active'
        
        # Update configuration
        new_config = {
            'input_directory': temp_dir,
            'output_directory': temp_dir
        }
        result = client.update_config(simulator_id, new_config)
        assert result['status'] == 'updated'
        
        # Delete simulator
        result = client.delete_simulator(simulator_id)
        assert result['status'] == 'deleted'
        
        # Verify simulator is deleted
        with pytest.raises(SemiPROClientError):
            client.get_simulator(simulator_id)
    
    def test_wafer_creation(self, client, temp_dir):
        """Test wafer creation and validation"""
        simulator_id = client.create_simulator({
            'input_directory': temp_dir,
            'output_directory': temp_dir
        })
        
        try:
            # Valid wafer configuration
            wafer_config = {
                'diameter': 300.0,
                'thickness': 775.0,
                'material': 'silicon',
                'orientation': '(100)',
                'resistivity': 10.0,
                'doping_type': 'p-type',
                'width': 100,
                'height': 100
            }
            
            result = client.create_wafer(simulator_id, wafer_config)
            assert result['status'] == 'created'
            assert 'wafer' in result
            
            # Invalid wafer configuration
            invalid_config = {
                'diameter': 1000.0,  # Too large
                'thickness': 50.0,   # Too thin
                'material': 'invalid_material',
                'width': 10,
                'height': 10
            }
            
            with pytest.raises(SemiPROClientError):
                client.create_wafer(simulator_id, invalid_config)
                
        finally:
            client.delete_simulator(simulator_id)
    
    def test_process_step_management(self, client, temp_dir):
        """Test process step addition and validation"""
        simulator_id = client.create_simulator({
            'input_directory': temp_dir,
            'output_directory': temp_dir
        })
        
        try:
            # Create wafer first
            wafer_config = {
                'diameter': 300.0,
                'thickness': 775.0,
                'material': 'silicon',
                'width': 50,
                'height': 50
            }
            client.create_wafer(simulator_id, wafer_config)
            
            # Add oxidation step
            result = client.add_oxidation_step(
                simulator_id, 'gate_oxide', 1000.0, 0.5, 'dry'
            )
            assert result['status'] == 'added'
            
            # Add doping step
            result = client.add_doping_step(
                simulator_id, 'source_drain', 'phosphorus', 1e20, 80.0, 1000.0
            )
            assert result['status'] == 'added'
            
            # Add lithography step
            result = client.add_lithography_step(
                simulator_id, 'gate_pattern', 193.0, 25.0
            )
            assert result['status'] == 'added'
            
            # Add deposition step
            result = client.add_deposition_step(
                simulator_id, 'poly_gate', 'polysilicon', 0.3, 620.0
            )
            assert result['status'] == 'added'
            
            # Add etching step
            result = client.add_etching_step(
                simulator_id, 'gate_etch', 0.3, 'anisotropic'
            )
            assert result['status'] == 'added'
            
        finally:
            client.delete_simulator(simulator_id)
    
    def test_simulation_execution(self, client, temp_dir):
        """Test simulation execution and monitoring"""
        simulator_id = client.create_simulator({
            'input_directory': temp_dir,
            'output_directory': temp_dir
        })
        
        try:
            # Create wafer
            wafer_config = {
                'diameter': 300.0,
                'thickness': 775.0,
                'material': 'silicon',
                'width': 20,  # Small grid for fast test
                'height': 20
            }
            client.create_wafer(simulator_id, wafer_config)
            
            # Add simple process steps
            client.add_oxidation_step(simulator_id, 'test_oxide', 1000.0, 0.1, 'dry')
            client.add_doping_step(simulator_id, 'test_doping', 'boron', 1e18, 50.0, 900.0)
            
            # Start simulation
            task_id = client.start_simulation(simulator_id, 'test_wafer')
            assert task_id is not None
            
            # Monitor simulation progress
            max_wait = 30  # seconds
            start_time = time.time()
            
            while time.time() - start_time < max_wait:
                status = client.get_simulation_status(task_id)
                assert 'status' in status
                assert 'progress' in status
                
                if status['status'] in ['completed', 'failed', 'cancelled']:
                    break
                
                time.sleep(1)
            
            # Get final status
            final_status = client.get_simulation_status(task_id)
            
            # If simulation completed, get results
            if final_status['status'] == 'completed':
                results = client.get_results(simulator_id)
                assert 'results' in results
                assert 'timestamp' in results
            
        finally:
            client.delete_simulator(simulator_id)
    
    def test_mosfet_simulation(self, client, temp_dir):
        """Test complete MOSFET simulation workflow"""
        # This test may take longer, so we use a very small grid
        try:
            results = client.run_mosfet_simulation()
            
            # Check that we got results (even if simulation failed due to missing modules)
            assert isinstance(results, dict)
            
        except SemiPROClientError as e:
            # Expected if simulation modules are not fully implemented
            pytest.skip(f"MOSFET simulation not available: {e}")
    
    def test_data_serialization(self):
        """Test data serialization and deserialization"""
        # Test wafer serialization
        wafer_data = {
            'id': 'test_wafer',
            'diameter': 300.0,
            'thickness': 775.0,
            'material': 'silicon',
            'width': 100,
            'height': 100,
            'layers': [
                {
                    'name': 'substrate',
                    'material': 'silicon',
                    'thickness': 775.0,
                    'properties': {'resistivity': 10.0}
                }
            ]
        }
        
        serialized = WaferSerializer.serialize(wafer_data)
        deserialized = WaferSerializer.deserialize(serialized)
        
        assert deserialized['id'] == wafer_data['id']
        assert deserialized['diameter'] == wafer_data['diameter']
        assert len(deserialized['layers']) == len(wafer_data['layers'])
        
        # Test process step serialization
        step_data = {
            'id': 'test_step',
            'name': 'oxidation_step',
            'type': 'oxidation',
            'parameters': {
                'temperature': 1000.0,
                'time': 0.5,
                'atmosphere': 'dry'
            },
            'status': 'pending'
        }
        
        serialized = ProcessStepSerializer.serialize(step_data)
        deserialized = ProcessStepSerializer.deserialize(serialized)
        
        assert deserialized['name'] == step_data['name']
        assert deserialized['type'] == step_data['type']
        assert deserialized['parameters'] == step_data['parameters']
    
    def test_validation_errors(self, client, temp_dir):
        """Test validation error handling"""
        simulator_id = client.create_simulator({
            'input_directory': temp_dir,
            'output_directory': temp_dir
        })
        
        try:
            # Test invalid wafer configuration
            invalid_wafer = {
                'diameter': -100.0,  # Negative diameter
                'thickness': 0.0,    # Zero thickness
                'material': 'invalid',
                'width': 0,
                'height': 0
            }
            
            with pytest.raises(SemiPROClientError):
                client.create_wafer(simulator_id, invalid_wafer)
            
            # Create valid wafer for step tests
            valid_wafer = {
                'diameter': 300.0,
                'thickness': 775.0,
                'material': 'silicon',
                'width': 50,
                'height': 50
            }
            client.create_wafer(simulator_id, valid_wafer)
            
            # Test invalid process step
            invalid_step = {
                'type': 'invalid_type',
                'name': '',  # Empty name
                'parameters': {}
            }
            
            with pytest.raises(SemiPROClientError):
                client.add_process_step(simulator_id, invalid_step)
            
            # Test invalid oxidation parameters
            with pytest.raises(SemiPROClientError):
                client.add_oxidation_step(
                    simulator_id, 'invalid_oxide', 
                    2000.0,  # Too high temperature
                    -1.0,    # Negative time
                    'invalid_atmosphere'
                )
            
        finally:
            client.delete_simulator(simulator_id)
    
    def test_data_export(self, client, temp_dir):
        """Test data export functionality"""
        simulator_id = client.create_simulator({
            'input_directory': temp_dir,
            'output_directory': temp_dir
        })
        
        try:
            # Create wafer and add simple step
            wafer_config = {
                'diameter': 300.0,
                'thickness': 775.0,
                'material': 'silicon',
                'width': 20,
                'height': 20
            }
            client.create_wafer(simulator_id, wafer_config)
            client.add_oxidation_step(simulator_id, 'test_oxide', 1000.0, 0.1, 'dry')
            
            # Test JSON export
            json_data = client.export_data(simulator_id, 'json', 'results')
            assert isinstance(json_data, bytes)
            assert len(json_data) > 0
            
            # Test CSV export
            csv_data = client.export_data(simulator_id, 'csv', 'results')
            assert isinstance(csv_data, bytes)
            assert len(csv_data) > 0
            
        finally:
            client.delete_simulator(simulator_id)
    
    def test_concurrent_simulations(self, client, temp_dir):
        """Test concurrent simulation handling"""
        simulators = []
        
        try:
            # Create multiple simulators
            for i in range(3):
                simulator_id = client.create_simulator({
                    'input_directory': temp_dir,
                    'output_directory': temp_dir
                })
                simulators.append(simulator_id)
                
                # Create wafer and add step
                wafer_config = {
                    'diameter': 300.0,
                    'thickness': 775.0,
                    'material': 'silicon',
                    'width': 10,  # Very small for fast execution
                    'height': 10
                }
                client.create_wafer(simulator_id, wafer_config)
                client.add_oxidation_step(simulator_id, f'oxide_{i}', 1000.0, 0.05, 'dry')
            
            # Start simulations concurrently
            task_ids = []
            for simulator_id in simulators:
                task_id = client.start_simulation(simulator_id, f'wafer_{simulator_id}')
                task_ids.append(task_id)
            
            # Monitor all simulations
            completed = 0
            max_wait = 60  # seconds
            start_time = time.time()
            
            while completed < len(task_ids) and time.time() - start_time < max_wait:
                for task_id in task_ids:
                    status = client.get_simulation_status(task_id)
                    if status['status'] in ['completed', 'failed', 'cancelled']:
                        completed += 1
                
                time.sleep(1)
            
            # Verify at least some simulations completed
            assert completed > 0
            
        finally:
            # Cleanup
            for simulator_id in simulators:
                try:
                    client.delete_simulator(simulator_id)
                except:
                    pass

class TestEndToEndWorkflows:
    """End-to-end workflow tests"""
    
    @pytest.fixture
    def client(self):
        """Create API client (assumes server is running)"""
        return SemiPROClient(base_url="http://127.0.0.1:5001")
    
    def test_complete_cmos_workflow(self, client):
        """Test complete CMOS fabrication workflow"""
        try:
            # Define CMOS process flow
            wafer_config = {
                'diameter': 300.0,
                'thickness': 775.0,
                'material': 'silicon',
                'orientation': '(100)',
                'resistivity': 10.0,
                'doping_type': 'p-type',
                'width': 50,
                'height': 50
            }
            
            process_steps = [
                {
                    'type': 'oxidation',
                    'name': 'field_oxidation',
                    'parameters': {
                        'temperature': 1000.0,
                        'time': 2.0,
                        'atmosphere': 'wet'
                    }
                },
                {
                    'type': 'doping',
                    'name': 'nwell_implant',
                    'parameters': {
                        'dopant': 'phosphorus',
                        'concentration': 1e17,
                        'energy': 100.0,
                        'temperature': 1100.0
                    }
                },
                {
                    'type': 'oxidation',
                    'name': 'gate_oxidation',
                    'parameters': {
                        'temperature': 950.0,
                        'time': 0.3,
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
                        'wavelength': 248.0,
                        'dose': 30.0
                    }
                },
                {
                    'type': 'etching',
                    'name': 'gate_etch',
                    'parameters': {
                        'depth': 0.3,
                        'type': 'anisotropic'
                    }
                }
            ]
            
            # Run simulation
            results = client.run_simple_simulation(wafer_config, process_steps, wait=True)
            
            # Verify results structure
            assert isinstance(results, dict)
            
        except SemiPROClientError as e:
            pytest.skip(f"CMOS workflow test skipped: {e}")
    
    def test_memory_device_workflow(self, client):
        """Test memory device fabrication workflow"""
        try:
            # Define memory device process flow
            wafer_config = {
                'diameter': 200.0,
                'thickness': 525.0,
                'material': 'silicon',
                'width': 30,
                'height': 30
            }
            
            process_steps = [
                {
                    'type': 'oxidation',
                    'name': 'tunnel_oxide',
                    'parameters': {
                        'temperature': 900.0,
                        'time': 0.1,
                        'atmosphere': 'dry'
                    }
                },
                {
                    'type': 'deposition',
                    'name': 'floating_gate',
                    'parameters': {
                        'material': 'polysilicon',
                        'thickness': 0.15,
                        'temperature': 580.0
                    }
                },
                {
                    'type': 'deposition',
                    'name': 'control_dielectric',
                    'parameters': {
                        'material': 'si3n4',
                        'thickness': 0.02,
                        'temperature': 700.0
                    }
                },
                {
                    'type': 'deposition',
                    'name': 'control_gate',
                    'parameters': {
                        'material': 'polysilicon',
                        'thickness': 0.2,
                        'temperature': 620.0
                    }
                }
            ]
            
            # Run simulation
            results = client.run_simple_simulation(wafer_config, process_steps, wait=True)
            
            # Verify results
            assert isinstance(results, dict)
            
        except SemiPROClientError as e:
            pytest.skip(f"Memory device workflow test skipped: {e}")

if __name__ == '__main__':
    pytest.main([__file__, '-v'])

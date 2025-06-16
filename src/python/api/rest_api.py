# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO REST API
===============

Comprehensive REST API for SemiPRO semiconductor simulation platform.
Provides endpoints for simulation management, process control, and data access.
"""

from flask import Flask, request, jsonify, send_file
from flask_cors import CORS
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
import asyncio
import threading
import uuid
import time
from datetime import datetime
from typing import Dict, List, Optional, Any
import logging
import os
from pathlib import Path

from ..simulator import Simulator
from .serializers import (
    WaferSerializer, ProcessStepSerializer, SimulationResultSerializer,
    ConfigurationSerializer, FlowSerializer
)
from .validators import (
    validate_process_parameters, validate_wafer_config,
    validate_simulation_flow, ValidationError
)

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class SemiPROAPI:
    """Main API class for SemiPRO REST endpoints"""
    
    def __init__(self, config: Optional[Dict[str, Any]] = None):
        self.config = config or {}
        self.simulators: Dict[str, Simulator] = {}
        self.simulation_tasks: Dict[str, Dict[str, Any]] = {}
        self.results_cache: Dict[str, Any] = {}
        
        # Initialize Flask app
        self.app = Flask(__name__)
        CORS(self.app)
        
        # Rate limiting
        self.limiter = Limiter(
            app=self.app,
            key_func=get_remote_address,
            default_limits=["200 per day", "50 per hour"]
        )
        
        # Setup routes
        self._setup_routes()
        
        # Background task management
        self.loop = None
        self.thread = None
        self._start_background_loop()
    
    def _start_background_loop(self):
        """Start background event loop for async operations"""
        def run_loop():
            self.loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self.loop)
            self.loop.run_forever()
        
        self.thread = threading.Thread(target=run_loop, daemon=True)
        self.thread.start()
    
    def _setup_routes(self):
        """Setup all API routes"""
        
        # Health check
        @self.app.route('/health', methods=['GET'])
        def health_check():
            return jsonify({
                'status': 'healthy',
                'timestamp': datetime.utcnow().isoformat(),
                'version': '1.0.0'
            })
        
        # Simulator management
        @self.app.route('/simulators', methods=['POST'])
        @self.limiter.limit("10 per minute")
        def create_simulator():
            try:
                data = request.get_json()
                simulator_id = str(uuid.uuid4())
                
                config_file = data.get('config_file')
                simulator = Simulator(config_file)
                
                if 'input_directory' in data:
                    simulator.set_input_directory(data['input_directory'])
                if 'output_directory' in data:
                    simulator.set_output_directory(data['output_directory'])
                
                self.simulators[simulator_id] = simulator
                
                return jsonify({
                    'simulator_id': simulator_id,
                    'status': 'created',
                    'config': data
                }), 201
                
            except Exception as e:
                logger.error(f"Failed to create simulator: {e}")
                return jsonify({'error': str(e)}), 400
        
        @self.app.route('/simulators/<simulator_id>', methods=['GET'])
        def get_simulator(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            simulator = self.simulators[simulator_id]
            return jsonify({
                'simulator_id': simulator_id,
                'input_directory': simulator.get_input_directory(),
                'output_directory': simulator.get_output_directory(),
                'status': 'active'
            })
        
        @self.app.route('/simulators/<simulator_id>', methods=['DELETE'])
        def delete_simulator(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            # Cancel any running simulations
            if simulator_id in self.simulation_tasks:
                del self.simulation_tasks[simulator_id]
            
            del self.simulators[simulator_id]
            return jsonify({'status': 'deleted'})
        
        # Wafer management
        @self.app.route('/simulators/<simulator_id>/wafers', methods=['POST'])
        def create_wafer(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            try:
                data = request.get_json()
                validate_wafer_config(data)
                
                simulator = self.simulators[simulator_id]
                
                width = data.get('width', 100)
                height = data.get('height', 100)
                diameter = data.get('diameter', 300.0)
                thickness = data.get('thickness', 775.0)
                material = data.get('material', 'silicon')
                
                simulator.initialize_geometry(width, height)
                
                wafer_data = WaferSerializer.serialize({
                    'width': width,
                    'height': height,
                    'diameter': diameter,
                    'thickness': thickness,
                    'material': material
                })
                
                return jsonify({
                    'wafer': wafer_data,
                    'status': 'created'
                }), 201
                
            except ValidationError as e:
                return jsonify({'error': str(e)}), 400
            except Exception as e:
                logger.error(f"Failed to create wafer: {e}")
                return jsonify({'error': str(e)}), 500
        
        # Process steps
        @self.app.route('/simulators/<simulator_id>/steps', methods=['POST'])
        def add_process_step(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            try:
                data = request.get_json()
                validate_process_parameters(data)
                
                simulator = self.simulators[simulator_id]
                step = simulator.create_process_step(
                    data['type'],
                    data['name'],
                    parameters=data.get('parameters', {}),
                    input_files=data.get('input_files', []),
                    output_files=data.get('output_files', [])
                )
                
                simulator.add_process_step(step)
                
                step_data = ProcessStepSerializer.serialize(data)
                return jsonify({
                    'step': step_data,
                    'status': 'added'
                }), 201
                
            except ValidationError as e:
                return jsonify({'error': str(e)}), 400
            except Exception as e:
                logger.error(f"Failed to add process step: {e}")
                return jsonify({'error': str(e)}), 500
        
        # Simulation execution
        @self.app.route('/simulators/<simulator_id>/simulate', methods=['POST'])
        @self.limiter.limit("5 per minute")
        def start_simulation(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            try:
                data = request.get_json()
                wafer_name = data.get('wafer_name', 'default')
                
                simulator = self.simulators[simulator_id]
                task_id = str(uuid.uuid4())
                
                # Start simulation in background
                future = asyncio.run_coroutine_threadsafe(
                    simulator.execute_simulation_flow(wafer_name),
                    self.loop
                )
                
                self.simulation_tasks[task_id] = {
                    'simulator_id': simulator_id,
                    'wafer_name': wafer_name,
                    'future': future,
                    'start_time': time.time(),
                    'status': 'running'
                }
                
                return jsonify({
                    'task_id': task_id,
                    'status': 'started',
                    'wafer_name': wafer_name
                }), 202
                
            except Exception as e:
                logger.error(f"Failed to start simulation: {e}")
                return jsonify({'error': str(e)}), 500
        
        @self.app.route('/simulations/<task_id>/status', methods=['GET'])
        def get_simulation_status(task_id):
            if task_id not in self.simulation_tasks:
                return jsonify({'error': 'Simulation task not found'}), 404
            
            task = self.simulation_tasks[task_id]
            simulator_id = task['simulator_id']
            
            if simulator_id in self.simulators:
                simulator = self.simulators[simulator_id]
                progress = simulator.get_simulation_progress()
                
                # Check if simulation is complete
                if task['future'].done():
                    try:
                        result = task['future'].result()
                        task['status'] = 'completed' if result else 'failed'
                        task['result'] = result
                    except Exception as e:
                        task['status'] = 'failed'
                        task['error'] = str(e)
                
                return jsonify({
                    'task_id': task_id,
                    'status': task['status'],
                    'progress': progress,
                    'elapsed_time': time.time() - task['start_time']
                })
            
            return jsonify({'error': 'Simulator not found'}), 404
        
        @self.app.route('/simulations/<task_id>/cancel', methods=['POST'])
        def cancel_simulation(task_id):
            if task_id not in self.simulation_tasks:
                return jsonify({'error': 'Simulation task not found'}), 404
            
            task = self.simulation_tasks[task_id]
            simulator_id = task['simulator_id']
            
            if simulator_id in self.simulators:
                simulator = self.simulators[simulator_id]
                simulator.cancel_simulation()
                task['future'].cancel()
                task['status'] = 'cancelled'
                
                return jsonify({
                    'task_id': task_id,
                    'status': 'cancelled'
                })
            
            return jsonify({'error': 'Simulator not found'}), 404
        
        # Results and data export
        @self.app.route('/simulators/<simulator_id>/results', methods=['GET'])
        def get_results(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            try:
                simulator = self.simulators[simulator_id]
                results = simulator.get_simulation_results()
                
                serialized_results = SimulationResultSerializer.serialize(results)
                return jsonify({
                    'results': serialized_results,
                    'timestamp': datetime.utcnow().isoformat()
                })
                
            except Exception as e:
                logger.error(f"Failed to get results: {e}")
                return jsonify({'error': str(e)}), 500
        
        @self.app.route('/simulators/<simulator_id>/export', methods=['POST'])
        def export_data(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            try:
                data = request.get_json()
                format_type = data.get('format', 'json')
                data_type = data.get('data_type', 'results')
                
                simulator = self.simulators[simulator_id]
                output_dir = simulator.get_output_directory()
                
                filename = f"export_{int(time.time())}.{format_type}"
                filepath = os.path.join(output_dir, filename)
                
                # Export data based on type and format
                if data_type == 'results':
                    results = simulator.get_simulation_results()
                    if format_type == 'json':
                        import json
                        with open(filepath, 'w') as f:
                            json.dump(results, f, indent=2)
                    elif format_type == 'csv':
                        import pandas as pd
                        df = pd.DataFrame([results])
                        df.to_csv(filepath, index=False)
                
                return send_file(filepath, as_attachment=True)
                
            except Exception as e:
                logger.error(f"Failed to export data: {e}")
                return jsonify({'error': str(e)}), 500
        
        # Configuration management
        @self.app.route('/simulators/<simulator_id>/config', methods=['GET'])
        def get_config(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            simulator = self.simulators[simulator_id]
            # Get configuration from simulator
            config = {
                'input_directory': simulator.get_input_directory(),
                'output_directory': simulator.get_output_directory(),
                # Add more configuration parameters as needed
            }
            
            return jsonify({
                'config': ConfigurationSerializer.serialize(config)
            })
        
        @self.app.route('/simulators/<simulator_id>/config', methods=['PUT'])
        def update_config(simulator_id):
            if simulator_id not in self.simulators:
                return jsonify({'error': 'Simulator not found'}), 404
            
            try:
                data = request.get_json()
                simulator = self.simulators[simulator_id]
                
                if 'input_directory' in data:
                    simulator.set_input_directory(data['input_directory'])
                if 'output_directory' in data:
                    simulator.set_output_directory(data['output_directory'])
                
                return jsonify({
                    'status': 'updated',
                    'config': data
                })
                
            except Exception as e:
                logger.error(f"Failed to update config: {e}")
                return jsonify({'error': str(e)}), 500

def create_app(config: Optional[Dict[str, Any]] = None) -> Flask:
    """Factory function to create Flask app with SemiPRO API"""
    api = SemiPROAPI(config)
    return api.app

if __name__ == '__main__':
    app = create_app()
    app.run(debug=True, host='0.0.0.0', port=5000)

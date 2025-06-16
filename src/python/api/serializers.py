# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO Data Serializers
========================

Comprehensive data serialization and deserialization for SemiPRO API.
Handles conversion between Python objects and JSON/other formats.
"""

import json
import numpy as np
from datetime import datetime
from typing import Dict, List, Any, Optional, Union
import base64
import pickle

class BaseSerializer:
    """Base serializer class with common functionality"""
    
    @staticmethod
    def serialize_numpy(obj: np.ndarray) -> Dict[str, Any]:
        """Serialize numpy array to JSON-compatible format"""
        return {
            'type': 'numpy_array',
            'dtype': str(obj.dtype),
            'shape': obj.shape,
            'data': base64.b64encode(obj.tobytes()).decode('utf-8')
        }
    
    @staticmethod
    def deserialize_numpy(data: Dict[str, Any]) -> np.ndarray:
        """Deserialize numpy array from JSON format"""
        if data.get('type') != 'numpy_array':
            raise ValueError("Invalid numpy array format")
        
        bytes_data = base64.b64decode(data['data'])
        dtype = np.dtype(data['dtype'])
        shape = tuple(data['shape'])
        
        return np.frombuffer(bytes_data, dtype=dtype).reshape(shape)
    
    @staticmethod
    def serialize_datetime(dt: datetime) -> str:
        """Serialize datetime to ISO format"""
        return dt.isoformat()
    
    @staticmethod
    def deserialize_datetime(dt_str: str) -> datetime:
        """Deserialize datetime from ISO format"""
        return datetime.fromisoformat(dt_str)

class WaferSerializer(BaseSerializer):
    """Serializer for wafer data"""
    
    @classmethod
    def serialize(cls, wafer_data: Dict[str, Any]) -> Dict[str, Any]:
        """Serialize wafer data to JSON-compatible format"""
        serialized = {
            'id': wafer_data.get('id'),
            'diameter': wafer_data.get('diameter'),
            'thickness': wafer_data.get('thickness'),
            'material': wafer_data.get('material'),
            'orientation': wafer_data.get('orientation'),
            'resistivity': wafer_data.get('resistivity'),
            'doping_type': wafer_data.get('doping_type'),
            'geometry': {
                'width': wafer_data.get('width'),
                'height': wafer_data.get('height'),
                'grid_spacing': wafer_data.get('grid_spacing')
            },
            'layers': [],
            'metadata': wafer_data.get('metadata', {})
        }
        
        # Serialize layer data
        if 'layers' in wafer_data:
            for layer in wafer_data['layers']:
                layer_data = {
                    'name': layer.get('name'),
                    'material': layer.get('material'),
                    'thickness': layer.get('thickness'),
                    'properties': layer.get('properties', {})
                }
                
                # Handle numpy arrays in layer data
                if 'concentration_profile' in layer:
                    profile = layer['concentration_profile']
                    if isinstance(profile, np.ndarray):
                        layer_data['concentration_profile'] = cls.serialize_numpy(profile)
                    else:
                        layer_data['concentration_profile'] = profile
                
                serialized['layers'].append(layer_data)
        
        return serialized
    
    @classmethod
    def deserialize(cls, data: Dict[str, Any]) -> Dict[str, Any]:
        """Deserialize wafer data from JSON format"""
        wafer_data = {
            'id': data.get('id'),
            'diameter': data.get('diameter'),
            'thickness': data.get('thickness'),
            'material': data.get('material'),
            'orientation': data.get('orientation'),
            'resistivity': data.get('resistivity'),
            'doping_type': data.get('doping_type'),
            'width': data.get('geometry', {}).get('width'),
            'height': data.get('geometry', {}).get('height'),
            'grid_spacing': data.get('geometry', {}).get('grid_spacing'),
            'layers': [],
            'metadata': data.get('metadata', {})
        }
        
        # Deserialize layer data
        for layer_data in data.get('layers', []):
            layer = {
                'name': layer_data.get('name'),
                'material': layer_data.get('material'),
                'thickness': layer_data.get('thickness'),
                'properties': layer_data.get('properties', {})
            }
            
            # Handle numpy arrays
            if 'concentration_profile' in layer_data:
                profile_data = layer_data['concentration_profile']
                if isinstance(profile_data, dict) and profile_data.get('type') == 'numpy_array':
                    layer['concentration_profile'] = cls.deserialize_numpy(profile_data)
                else:
                    layer['concentration_profile'] = profile_data
            
            wafer_data['layers'].append(layer)
        
        return wafer_data

class ProcessStepSerializer(BaseSerializer):
    """Serializer for process step data"""
    
    @classmethod
    def serialize(cls, step_data: Dict[str, Any]) -> Dict[str, Any]:
        """Serialize process step data"""
        return {
            'id': step_data.get('id'),
            'name': step_data.get('name'),
            'type': step_data.get('type'),
            'parameters': step_data.get('parameters', {}),
            'input_files': step_data.get('input_files', []),
            'output_files': step_data.get('output_files', []),
            'dependencies': step_data.get('dependencies', []),
            'estimated_duration': step_data.get('estimated_duration'),
            'priority': step_data.get('priority', 0),
            'parallel_compatible': step_data.get('parallel_compatible', False),
            'status': step_data.get('status', 'pending'),
            'start_time': cls.serialize_datetime(step_data['start_time']) if 'start_time' in step_data else None,
            'end_time': cls.serialize_datetime(step_data['end_time']) if 'end_time' in step_data else None,
            'metadata': step_data.get('metadata', {})
        }
    
    @classmethod
    def deserialize(cls, data: Dict[str, Any]) -> Dict[str, Any]:
        """Deserialize process step data"""
        step_data = {
            'id': data.get('id'),
            'name': data.get('name'),
            'type': data.get('type'),
            'parameters': data.get('parameters', {}),
            'input_files': data.get('input_files', []),
            'output_files': data.get('output_files', []),
            'dependencies': data.get('dependencies', []),
            'estimated_duration': data.get('estimated_duration'),
            'priority': data.get('priority', 0),
            'parallel_compatible': data.get('parallel_compatible', False),
            'status': data.get('status', 'pending'),
            'metadata': data.get('metadata', {})
        }
        
        if data.get('start_time'):
            step_data['start_time'] = cls.deserialize_datetime(data['start_time'])
        if data.get('end_time'):
            step_data['end_time'] = cls.deserialize_datetime(data['end_time'])
        
        return step_data

class SimulationResultSerializer(BaseSerializer):
    """Serializer for simulation results"""
    
    @classmethod
    def serialize(cls, results: Dict[str, Any]) -> Dict[str, Any]:
        """Serialize simulation results"""
        serialized = {
            'simulation_id': results.get('simulation_id'),
            'wafer_id': results.get('wafer_id'),
            'flow_name': results.get('flow_name'),
            'start_time': cls.serialize_datetime(results['start_time']) if 'start_time' in results else None,
            'end_time': cls.serialize_datetime(results['end_time']) if 'end_time' in results else None,
            'duration': results.get('duration'),
            'status': results.get('status'),
            'steps_completed': results.get('steps_completed', []),
            'steps_failed': results.get('steps_failed', []),
            'metrics': results.get('metrics', {}),
            'outputs': {},
            'errors': results.get('errors', []),
            'warnings': results.get('warnings', []),
            'metadata': results.get('metadata', {})
        }
        
        # Serialize output data
        for key, value in results.get('outputs', {}).items():
            if isinstance(value, np.ndarray):
                serialized['outputs'][key] = cls.serialize_numpy(value)
            elif isinstance(value, dict):
                # Recursively handle nested dictionaries
                serialized['outputs'][key] = cls._serialize_nested_dict(value)
            else:
                serialized['outputs'][key] = value
        
        return serialized
    
    @classmethod
    def _serialize_nested_dict(cls, data: Dict[str, Any]) -> Dict[str, Any]:
        """Recursively serialize nested dictionary"""
        result = {}
        for key, value in data.items():
            if isinstance(value, np.ndarray):
                result[key] = cls.serialize_numpy(value)
            elif isinstance(value, dict):
                result[key] = cls._serialize_nested_dict(value)
            elif isinstance(value, datetime):
                result[key] = cls.serialize_datetime(value)
            else:
                result[key] = value
        return result
    
    @classmethod
    def deserialize(cls, data: Dict[str, Any]) -> Dict[str, Any]:
        """Deserialize simulation results"""
        results = {
            'simulation_id': data.get('simulation_id'),
            'wafer_id': data.get('wafer_id'),
            'flow_name': data.get('flow_name'),
            'duration': data.get('duration'),
            'status': data.get('status'),
            'steps_completed': data.get('steps_completed', []),
            'steps_failed': data.get('steps_failed', []),
            'metrics': data.get('metrics', {}),
            'outputs': {},
            'errors': data.get('errors', []),
            'warnings': data.get('warnings', []),
            'metadata': data.get('metadata', {})
        }
        
        if data.get('start_time'):
            results['start_time'] = cls.deserialize_datetime(data['start_time'])
        if data.get('end_time'):
            results['end_time'] = cls.deserialize_datetime(data['end_time'])
        
        # Deserialize output data
        for key, value in data.get('outputs', {}).items():
            if isinstance(value, dict) and value.get('type') == 'numpy_array':
                results['outputs'][key] = cls.deserialize_numpy(value)
            elif isinstance(value, dict):
                results['outputs'][key] = cls._deserialize_nested_dict(value)
            else:
                results['outputs'][key] = value
        
        return results
    
    @classmethod
    def _deserialize_nested_dict(cls, data: Dict[str, Any]) -> Dict[str, Any]:
        """Recursively deserialize nested dictionary"""
        result = {}
        for key, value in data.items():
            if isinstance(value, dict) and value.get('type') == 'numpy_array':
                result[key] = cls.deserialize_numpy(value)
            elif isinstance(value, dict):
                result[key] = cls._deserialize_nested_dict(value)
            elif isinstance(value, str):
                try:
                    # Try to parse as datetime
                    result[key] = cls.deserialize_datetime(value)
                except:
                    result[key] = value
            else:
                result[key] = value
        return result

class ConfigurationSerializer(BaseSerializer):
    """Serializer for configuration data"""
    
    @classmethod
    def serialize(cls, config: Dict[str, Any]) -> Dict[str, Any]:
        """Serialize configuration data"""
        return {
            'version': config.get('version', '1.0.0'),
            'simulation': config.get('simulation', {}),
            'solver': config.get('solver', {}),
            'physics': config.get('physics', {}),
            'materials': config.get('materials', {}),
            'technology': config.get('technology', {}),
            'paths': config.get('paths', {}),
            'performance': config.get('performance', {}),
            'logging': config.get('logging', {}),
            'metadata': config.get('metadata', {})
        }
    
    @classmethod
    def deserialize(cls, data: Dict[str, Any]) -> Dict[str, Any]:
        """Deserialize configuration data"""
        return {
            'version': data.get('version', '1.0.0'),
            'simulation': data.get('simulation', {}),
            'solver': data.get('solver', {}),
            'physics': data.get('physics', {}),
            'materials': data.get('materials', {}),
            'technology': data.get('technology', {}),
            'paths': data.get('paths', {}),
            'performance': data.get('performance', {}),
            'logging': data.get('logging', {}),
            'metadata': data.get('metadata', {})
        }

class FlowSerializer(BaseSerializer):
    """Serializer for simulation flow data"""
    
    @classmethod
    def serialize(cls, flow: Dict[str, Any]) -> Dict[str, Any]:
        """Serialize simulation flow"""
        serialized = {
            'name': flow.get('name'),
            'description': flow.get('description'),
            'version': flow.get('version', '1.0.0'),
            'author': flow.get('author'),
            'created_date': cls.serialize_datetime(flow['created_date']) if 'created_date' in flow else None,
            'modified_date': cls.serialize_datetime(flow['modified_date']) if 'modified_date' in flow else None,
            'execution_mode': flow.get('execution_mode', 'sequential'),
            'global_parameters': flow.get('global_parameters', {}),
            'steps': [],
            'metadata': flow.get('metadata', {})
        }
        
        # Serialize steps
        for step in flow.get('steps', []):
            serialized['steps'].append(ProcessStepSerializer.serialize(step))
        
        return serialized
    
    @classmethod
    def deserialize(cls, data: Dict[str, Any]) -> Dict[str, Any]:
        """Deserialize simulation flow"""
        flow = {
            'name': data.get('name'),
            'description': data.get('description'),
            'version': data.get('version', '1.0.0'),
            'author': data.get('author'),
            'execution_mode': data.get('execution_mode', 'sequential'),
            'global_parameters': data.get('global_parameters', {}),
            'steps': [],
            'metadata': data.get('metadata', {})
        }
        
        if data.get('created_date'):
            flow['created_date'] = cls.deserialize_datetime(data['created_date'])
        if data.get('modified_date'):
            flow['modified_date'] = cls.deserialize_datetime(data['modified_date'])
        
        # Deserialize steps
        for step_data in data.get('steps', []):
            flow['steps'].append(ProcessStepSerializer.deserialize(step_data))
        
        return flow

# Utility functions for common serialization tasks
def serialize_to_json(obj: Any, serializer_class: BaseSerializer = None) -> str:
    """Serialize object to JSON string"""
    if serializer_class:
        serialized = serializer_class.serialize(obj)
    else:
        serialized = obj
    
    return json.dumps(serialized, indent=2, default=str)

def deserialize_from_json(json_str: str, serializer_class: BaseSerializer = None) -> Any:
    """Deserialize object from JSON string"""
    data = json.loads(json_str)
    
    if serializer_class:
        return serializer_class.deserialize(data)
    else:
        return data

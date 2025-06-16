# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO API Layer
================

This module provides a comprehensive API layer for external interaction with SemiPRO.
It includes REST API endpoints, data serialization, and integration interfaces.
"""

try:
    from .rest_api import SemiPROAPI, create_app
    from .serializers import (
        WaferSerializer, ProcessStepSerializer, SimulationResultSerializer,
        ConfigurationSerializer, FlowSerializer
    )
    from .client import SemiPROClient
    from .websocket_api import WebSocketAPI
    from .validators import (
        validate_process_parameters, validate_wafer_config,
        validate_simulation_flow, ValidationError
    )
except ImportError:
    # For documentation building
    pass

__all__ = [
    'SemiPROAPI',
    'create_app',
    'SemiPROClient',
    'WebSocketAPI',
    'WaferSerializer',
    'ProcessStepSerializer',
    'SimulationResultSerializer',
    'ConfigurationSerializer',
    'FlowSerializer',
    'validate_process_parameters',
    'validate_wafer_config',
    'validate_simulation_flow',
    'ValidationError'
]

__version__ = "1.0.0"
__author__ = "Dr. Mazharuddin Mohammed"

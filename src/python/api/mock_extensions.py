# Author: Dr. Mazharuddin Mohammed
"""
Mock Extensions for Documentation Building
==========================================

This module provides mock implementations of Cython extensions
for use during documentation building when C++ extensions are not available.
"""

import sys
import os
from unittest.mock import MagicMock

class MockCythonModule:
    """Mock Cython module for documentation"""
    
    def __init__(self, name):
        self.name = name
    
    def __getattr__(self, name):
        return MagicMock()
    
    def __call__(self, *args, **kwargs):
        return MagicMock()

# Mock classes for Cython extensions
class PyWafer(MockCythonModule):
    """Mock PyWafer class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyWafer")

class PyGeometryManager(MockCythonModule):
    """Mock PyGeometryManager class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyGeometryManager")

class PyOxidationModel(MockCythonModule):
    """Mock PyOxidationModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyOxidationModel")

class PyDopingManager(MockCythonModule):
    """Mock PyDopingManager class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyDopingManager")

class PyLithographyModel(MockCythonModule):
    """Mock PyLithographyModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyLithographyModel")

class PyDepositionModel(MockCythonModule):
    """Mock PyDepositionModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyDepositionModel")

class PyEtchingModel(MockCythonModule):
    """Mock PyEtchingModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyEtchingModel")

class PyMetallizationModel(MockCythonModule):
    """Mock PyMetallizationModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyMetallizationModel")

class PyPackagingModel(MockCythonModule):
    """Mock PyPackagingModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyPackagingModel")

class PyThermalSimulationModel(MockCythonModule):
    """Mock PyThermalSimulationModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyThermalSimulationModel")

class PyReliabilityModel(MockCythonModule):
    """Mock PyReliabilityModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyReliabilityModel")

class PyVulkanRenderer(MockCythonModule):
    """Mock PyVulkanRenderer class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyVulkanRenderer")

class PyMultiDieModel(MockCythonModule):
    """Mock PyMultiDieModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyMultiDieModel")

class PyDie(MockCythonModule):
    """Mock PyDie class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyDie")

class PyInterconnect(MockCythonModule):
    """Mock PyInterconnect class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyInterconnect")

class PyDRCModel(MockCythonModule):
    """Mock PyDRCModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyDRCModel")

class PyDRCRule(MockCythonModule):
    """Mock PyDRCRule class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyDRCRule")

class PyAdvancedVisualizationModel(MockCythonModule):
    """Mock PyAdvancedVisualizationModel class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyAdvancedVisualizationModel")

class PySimulationOrchestrator(MockCythonModule):
    """Mock PySimulationOrchestrator class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PySimulationOrchestrator")

class PyProcessStepDefinition(MockCythonModule):
    """Mock PyProcessStepDefinition class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyProcessStepDefinition")

class PySimulationProgress(MockCythonModule):
    """Mock PySimulationProgress class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PySimulationProgress")

class PyExecutionStatistics(MockCythonModule):
    """Mock PyExecutionStatistics class"""
    def __init__(self, *args, **kwargs):
        super().__init__("PyExecutionStatistics")

# Mock functions
def create_process_step(*args, **kwargs):
    """Mock create_process_step function"""
    return PyProcessStepDefinition()

def get_orchestrator():
    """Mock get_orchestrator function"""
    return PySimulationOrchestrator()

# Constants
STEP_TYPE_OXIDATION = "oxidation"
STEP_TYPE_DOPING = "doping"
STEP_TYPE_LITHOGRAPHY = "lithography"
STEP_TYPE_DEPOSITION = "deposition"
STEP_TYPE_ETCHING = "etching"
STEP_TYPE_METALLIZATION = "metallization"
STEP_TYPE_ANNEALING = "annealing"
STEP_TYPE_CMP = "cmp"
STEP_TYPE_INSPECTION = "inspection"
STEP_TYPE_CUSTOM = "custom"

EXECUTION_MODE_SEQUENTIAL = "sequential"
EXECUTION_MODE_PARALLEL = "parallel"
EXECUTION_MODE_PIPELINE = "pipeline"
EXECUTION_MODE_BATCH = "batch"

def install_mock_modules():
    """Install mock modules in sys.modules for documentation building"""
    
    # Mock module mappings
    mock_modules = {
        'geometry': {
            'PyGeometryManager': PyGeometryManager,
            'PyWafer': PyWafer
        },
        'oxidation': {
            'PyOxidationModel': PyOxidationModel
        },
        'doping': {
            'PyDopingManager': PyDopingManager
        },
        'photolithography': {
            'PyLithographyModel': PyLithographyModel
        },
        'deposition': {
            'PyDepositionModel': PyDepositionModel
        },
        'etching': {
            'PyEtchingModel': PyEtchingModel
        },
        'metallization': {
            'PyMetallizationModel': PyMetallizationModel
        },
        'packaging': {
            'PyPackagingModel': PyPackagingModel
        },
        'thermal': {
            'PyThermalSimulationModel': PyThermalSimulationModel
        },
        'reliability': {
            'PyReliabilityModel': PyReliabilityModel
        },
        'renderer': {
            'PyVulkanRenderer': PyVulkanRenderer
        },
        'multi_die': {
            'PyMultiDieModel': PyMultiDieModel,
            'PyDie': PyDie,
            'PyInterconnect': PyInterconnect
        },
        'drc': {
            'PyDRCModel': PyDRCModel,
            'PyDRCRule': PyDRCRule
        },
        'advanced_visualization': {
            'PyAdvancedVisualizationModel': PyAdvancedVisualizationModel
        },
        'simulation_orchestrator': {
            'PySimulationOrchestrator': PySimulationOrchestrator,
            'PyProcessStepDefinition': PyProcessStepDefinition,
            'PySimulationProgress': PySimulationProgress,
            'PyExecutionStatistics': PyExecutionStatistics,
            'create_process_step': create_process_step,
            'get_orchestrator': get_orchestrator,
            'STEP_TYPE_OXIDATION': STEP_TYPE_OXIDATION,
            'STEP_TYPE_DOPING': STEP_TYPE_DOPING,
            'STEP_TYPE_LITHOGRAPHY': STEP_TYPE_LITHOGRAPHY,
            'STEP_TYPE_DEPOSITION': STEP_TYPE_DEPOSITION,
            'STEP_TYPE_ETCHING': STEP_TYPE_ETCHING,
            'STEP_TYPE_METALLIZATION': STEP_TYPE_METALLIZATION,
            'STEP_TYPE_ANNEALING': STEP_TYPE_ANNEALING,
            'STEP_TYPE_CMP': STEP_TYPE_CMP,
            'STEP_TYPE_INSPECTION': STEP_TYPE_INSPECTION,
            'STEP_TYPE_CUSTOM': STEP_TYPE_CUSTOM,
            'EXECUTION_MODE_SEQUENTIAL': EXECUTION_MODE_SEQUENTIAL,
            'EXECUTION_MODE_PARALLEL': EXECUTION_MODE_PARALLEL,
            'EXECUTION_MODE_PIPELINE': EXECUTION_MODE_PIPELINE,
            'EXECUTION_MODE_BATCH': EXECUTION_MODE_BATCH
        }
    }
    
    # Create mock modules
    for module_name, classes in mock_modules.items():
        mock_module = MagicMock()
        
        # Add classes to mock module
        for class_name, class_obj in classes.items():
            setattr(mock_module, class_name, class_obj)
        
        # Install in sys.modules
        sys.modules[f'src.python.{module_name}'] = mock_module
        sys.modules[f'semipro.{module_name}'] = mock_module

# Auto-install when imported during documentation building
if 'sphinx' in sys.modules or 'READTHEDOCS' in os.environ:
    install_mock_modules()

# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = src/cpp/core/simulation_orchestrator.cpp

from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map
from libcpp.memory cimport shared_ptr, make_shared
from libcpp cimport bool
from libcpp.future cimport future
import asyncio
from typing import Dict, List, Optional, Callable, Any

# C++ declarations
cdef extern from "src/cpp/core/simulation_orchestrator.hpp" namespace "SemiPRO":
    cdef enum class StepType:
        OXIDATION
        DOPING
        LITHOGRAPHY
        DEPOSITION
        ETCHING
        METALLIZATION
        ANNEALING
        CMP
        INSPECTION
        CUSTOM

    cdef enum class ExecutionMode:
        SEQUENTIAL
        PARALLEL
        PIPELINE
        BATCH

    cdef enum class SimulationState:
        IDLE
        INITIALIZING
        RUNNING
        PAUSED
        COMPLETED
        ERROR
        CANCELLED

    cdef struct ProcessStepDefinition:
        StepType type
        string name
        unordered_map[string, double] parameters
        vector[string] input_files
        vector[string] output_files
        vector[string] dependencies
        double estimated_duration
        int priority
        bool parallel_compatible

    cdef struct SimulationProgress:
        SimulationState state
        size_t current_step
        size_t total_steps
        double progress_percentage
        string current_operation
        vector[string] completed_steps
        vector[string] errors

    cdef struct ExecutionStatistics:
        double total_execution_time
        double average_step_time
        size_t total_steps_executed
        size_t successful_steps
        size_t failed_steps
        double cpu_utilization
        double memory_usage

    cdef cppclass SimulationOrchestrator:
        @staticmethod
        SimulationOrchestrator& getInstance()
        
        # Configuration management
        void loadConfiguration(const string& config_file)
        void saveConfiguration(const string& config_file)
        void setParameter(const string& key, const string& value)
        string getParameter(const string& key)
        
        # Flow management
        void loadSimulationFlow(const string& flow_file)
        void saveSimulationFlow(const string& flow_file)
        void addProcessStep(const ProcessStepDefinition& step)
        void removeProcessStep(const string& step_name)
        void reorderSteps(const vector[string]& step_order)
        
        # Execution control
        future[bool] executeSimulation(const string& wafer_name)
        future[bool] executeSimulationFlow(const string& flow_name, const string& wafer_name)
        void pauseSimulation()
        void resumeSimulation()
        void cancelSimulation()
        void resetSimulation()
        
        # Batch processing
        void addWaferToBatch(const string& wafer_name, const string& flow_name)
        future[vector[bool]] executeBatch()
        void clearBatch()
        
        # Monitoring and status
        SimulationProgress getProgress()
        vector[string] getAvailableFlows()
        vector[ProcessStepDefinition] getCurrentFlow()
        bool isRunning()
        bool isPaused()
        
        # Input/Output management
        void setInputDirectory(const string& directory)
        void setOutputDirectory(const string& directory)
        string getInputDirectory()
        string getOutputDirectory()
        
        # Performance optimization
        void setExecutionMode(ExecutionMode mode)
        ExecutionMode getExecutionMode()
        void setMaxParallelSteps(int max_steps)
        int getMaxParallelSteps()
        
        # Checkpointing
        void enableCheckpointing(bool enable, int interval_minutes)
        void saveCheckpoint(const string& filename)
        void loadCheckpoint(const string& filename)
        
        # Statistics and reporting
        ExecutionStatistics getExecutionStatistics()
        void generateExecutionReport(const string& filename)

# Python wrapper classes
cdef class PyProcessStepDefinition:
    """Python wrapper for ProcessStepDefinition"""
    cdef ProcessStepDefinition* _step
    cdef bool _owner
    
    def __cinit__(self, step_type: str, name: str):
        self._step = new ProcessStepDefinition(self._string_to_step_type(step_type), name.encode('utf-8'))
        self._owner = True
    
    def __dealloc__(self):
        if self._owner and self._step:
            del self._step
    
    @staticmethod
    cdef StepType _string_to_step_type(str step_type):
        type_map = {
            "oxidation": StepType.OXIDATION,
            "doping": StepType.DOPING,
            "lithography": StepType.LITHOGRAPHY,
            "deposition": StepType.DEPOSITION,
            "etching": StepType.ETCHING,
            "metallization": StepType.METALLIZATION,
            "annealing": StepType.ANNEALING,
            "cmp": StepType.CMP,
            "inspection": StepType.INSPECTION,
            "custom": StepType.CUSTOM
        }
        return type_map.get(step_type.lower(), StepType.CUSTOM)
    
    @property
    def name(self) -> str:
        return self._step.name.decode('utf-8')
    
    @name.setter
    def name(self, value: str):
        self._step.name = value.encode('utf-8')
    
    @property
    def parameters(self) -> Dict[str, float]:
        result = {}
        cdef unordered_map[string, double].iterator it = self._step.parameters.begin()
        while it != self._step.parameters.end():
            result[it.first.decode('utf-8')] = it.second
            it += 1
        return result
    
    @parameters.setter
    def parameters(self, value: Dict[str, float]):
        self._step.parameters.clear()
        for key, val in value.items():
            self._step.parameters[key.encode('utf-8')] = val
    
    @property
    def input_files(self) -> List[str]:
        return [f.decode('utf-8') for f in self._step.input_files]
    
    @input_files.setter
    def input_files(self, value: List[str]):
        self._step.input_files.clear()
        for f in value:
            self._step.input_files.push_back(f.encode('utf-8'))
    
    @property
    def output_files(self) -> List[str]:
        return [f.decode('utf-8') for f in self._step.output_files]
    
    @output_files.setter
    def output_files(self, value: List[str]):
        self._step.output_files.clear()
        for f in value:
            self._step.output_files.push_back(f.encode('utf-8'))
    
    @property
    def dependencies(self) -> List[str]:
        return [d.decode('utf-8') for d in self._step.dependencies]
    
    @dependencies.setter
    def dependencies(self, value: List[str]):
        self._step.dependencies.clear()
        for d in value:
            self._step.dependencies.push_back(d.encode('utf-8'))
    
    @property
    def estimated_duration(self) -> float:
        return self._step.estimated_duration
    
    @estimated_duration.setter
    def estimated_duration(self, value: float):
        self._step.estimated_duration = value
    
    @property
    def priority(self) -> int:
        return self._step.priority
    
    @priority.setter
    def priority(self, value: int):
        self._step.priority = value
    
    @property
    def parallel_compatible(self) -> bool:
        return self._step.parallel_compatible
    
    @parallel_compatible.setter
    def parallel_compatible(self, value: bool):
        self._step.parallel_compatible = value

cdef class PySimulationProgress:
    """Python wrapper for SimulationProgress"""
    cdef SimulationProgress _progress
    
    def __cinit__(self, SimulationProgress progress):
        self._progress = progress
    
    @property
    def state(self) -> str:
        state_map = {
            SimulationState.IDLE: "idle",
            SimulationState.INITIALIZING: "initializing",
            SimulationState.RUNNING: "running",
            SimulationState.PAUSED: "paused",
            SimulationState.COMPLETED: "completed",
            SimulationState.ERROR: "error",
            SimulationState.CANCELLED: "cancelled"
        }
        return state_map.get(self._progress.state, "unknown")
    
    @property
    def current_step(self) -> int:
        return self._progress.current_step
    
    @property
    def total_steps(self) -> int:
        return self._progress.total_steps
    
    @property
    def progress_percentage(self) -> float:
        return self._progress.progress_percentage
    
    @property
    def current_operation(self) -> str:
        return self._progress.current_operation.decode('utf-8')
    
    @property
    def completed_steps(self) -> List[str]:
        return [step.decode('utf-8') for step in self._progress.completed_steps]
    
    @property
    def errors(self) -> List[str]:
        return [error.decode('utf-8') for error in self._progress.errors]

cdef class PyExecutionStatistics:
    """Python wrapper for ExecutionStatistics"""
    cdef ExecutionStatistics _stats
    
    def __cinit__(self, ExecutionStatistics stats):
        self._stats = stats
    
    @property
    def total_execution_time(self) -> float:
        return self._stats.total_execution_time
    
    @property
    def average_step_time(self) -> float:
        return self._stats.average_step_time
    
    @property
    def total_steps_executed(self) -> int:
        return self._stats.total_steps_executed
    
    @property
    def successful_steps(self) -> int:
        return self._stats.successful_steps
    
    @property
    def failed_steps(self) -> int:
        return self._stats.failed_steps
    
    @property
    def cpu_utilization(self) -> float:
        return self._stats.cpu_utilization
    
    @property
    def memory_usage(self) -> float:
        return self._stats.memory_usage

cdef class PySimulationOrchestrator:
    """Python wrapper for SimulationOrchestrator"""
    cdef SimulationOrchestrator* _orchestrator
    
    def __cinit__(self):
        self._orchestrator = &SimulationOrchestrator.getInstance()
    
    # Configuration management
    def load_configuration(self, config_file: str):
        """Load simulation configuration from file"""
        self._orchestrator.loadConfiguration(config_file.encode('utf-8'))
    
    def save_configuration(self, config_file: str):
        """Save simulation configuration to file"""
        self._orchestrator.saveConfiguration(config_file.encode('utf-8'))
    
    def set_parameter(self, key: str, value: str):
        """Set a configuration parameter"""
        self._orchestrator.setParameter(key.encode('utf-8'), value.encode('utf-8'))
    
    def get_parameter(self, key: str) -> str:
        """Get a configuration parameter"""
        return self._orchestrator.getParameter(key.encode('utf-8')).decode('utf-8')
    
    # Flow management
    def load_simulation_flow(self, flow_file: str):
        """Load simulation flow from file"""
        self._orchestrator.loadSimulationFlow(flow_file.encode('utf-8'))
    
    def save_simulation_flow(self, flow_file: str):
        """Save simulation flow to file"""
        self._orchestrator.saveSimulationFlow(flow_file.encode('utf-8'))
    
    def add_process_step(self, step: PyProcessStepDefinition):
        """Add a process step to the current flow"""
        self._orchestrator.addProcessStep(step._step[0])
    
    def remove_process_step(self, step_name: str):
        """Remove a process step from the current flow"""
        self._orchestrator.removeProcessStep(step_name.encode('utf-8'))
    
    def reorder_steps(self, step_order: List[str]):
        """Reorder process steps"""
        cdef vector[string] cpp_order
        for step in step_order:
            cpp_order.push_back(step.encode('utf-8'))
        self._orchestrator.reorderSteps(cpp_order)

    # Execution control
    async def execute_simulation(self, wafer_name: str) -> bool:
        """Execute simulation asynchronously"""
        # Note: This is a simplified async wrapper
        # In a real implementation, you'd need proper async/await integration
        return await asyncio.get_event_loop().run_in_executor(
            None, self._execute_simulation_sync, wafer_name)

    def _execute_simulation_sync(self, wafer_name: str) -> bool:
        """Synchronous execution wrapper"""
        cdef future[bool] result = self._orchestrator.executeSimulation(wafer_name.encode('utf-8'))
        return result.get()  # This blocks until completion

    async def execute_simulation_flow(self, flow_name: str, wafer_name: str) -> bool:
        """Execute specific simulation flow asynchronously"""
        return await asyncio.get_event_loop().run_in_executor(
            None, self._execute_simulation_flow_sync, flow_name, wafer_name)

    def _execute_simulation_flow_sync(self, flow_name: str, wafer_name: str) -> bool:
        """Synchronous flow execution wrapper"""
        cdef future[bool] result = self._orchestrator.executeSimulationFlow(
            flow_name.encode('utf-8'), wafer_name.encode('utf-8'))
        return result.get()

    def pause_simulation(self):
        """Pause the current simulation"""
        self._orchestrator.pauseSimulation()

    def resume_simulation(self):
        """Resume the paused simulation"""
        self._orchestrator.resumeSimulation()

    def cancel_simulation(self):
        """Cancel the current simulation"""
        self._orchestrator.cancelSimulation()

    def reset_simulation(self):
        """Reset simulation state"""
        self._orchestrator.resetSimulation()

    # Batch processing
    def add_wafer_to_batch(self, wafer_name: str, flow_name: str):
        """Add wafer to batch processing queue"""
        self._orchestrator.addWaferToBatch(wafer_name.encode('utf-8'), flow_name.encode('utf-8'))

    async def execute_batch(self) -> List[bool]:
        """Execute batch processing asynchronously"""
        return await asyncio.get_event_loop().run_in_executor(
            None, self._execute_batch_sync)

    def _execute_batch_sync(self) -> List[bool]:
        """Synchronous batch execution wrapper"""
        cdef future[vector[bool]] result = self._orchestrator.executeBatch()
        cdef vector[bool] cpp_results = result.get()
        return [r for r in cpp_results]

    def clear_batch(self):
        """Clear batch processing queue"""
        self._orchestrator.clearBatch()

    # Monitoring and status
    def get_progress(self) -> PySimulationProgress:
        """Get current simulation progress"""
        cdef SimulationProgress progress = self._orchestrator.getProgress()
        return PySimulationProgress(progress)

    def get_available_flows(self) -> List[str]:
        """Get list of available simulation flows"""
        cdef vector[string] flows = self._orchestrator.getAvailableFlows()
        return [flow.decode('utf-8') for flow in flows]

    def get_current_flow(self) -> List[PyProcessStepDefinition]:
        """Get current simulation flow steps"""
        cdef vector[ProcessStepDefinition] steps = self._orchestrator.getCurrentFlow()
        result = []
        for step in steps:
            py_step = PyProcessStepDefinition.__new__(PyProcessStepDefinition)
            py_step._step = new ProcessStepDefinition(step)
            py_step._owner = True
            result.append(py_step)
        return result

    def is_running(self) -> bool:
        """Check if simulation is currently running"""
        return self._orchestrator.isRunning()

    def is_paused(self) -> bool:
        """Check if simulation is currently paused"""
        return self._orchestrator.isPaused()

    # Input/Output management
    def set_input_directory(self, directory: str):
        """Set input directory for simulation files"""
        self._orchestrator.setInputDirectory(directory.encode('utf-8'))

    def set_output_directory(self, directory: str):
        """Set output directory for simulation results"""
        self._orchestrator.setOutputDirectory(directory.encode('utf-8'))

    def get_input_directory(self) -> str:
        """Get current input directory"""
        return self._orchestrator.getInputDirectory().decode('utf-8')

    def get_output_directory(self) -> str:
        """Get current output directory"""
        return self._orchestrator.getOutputDirectory().decode('utf-8')

    # Performance optimization
    def set_execution_mode(self, mode: str):
        """Set execution mode (sequential, parallel, pipeline, batch)"""
        mode_map = {
            "sequential": ExecutionMode.SEQUENTIAL,
            "parallel": ExecutionMode.PARALLEL,
            "pipeline": ExecutionMode.PIPELINE,
            "batch": ExecutionMode.BATCH
        }
        if mode.lower() in mode_map:
            self._orchestrator.setExecutionMode(mode_map[mode.lower()])

    def get_execution_mode(self) -> str:
        """Get current execution mode"""
        cdef ExecutionMode mode = self._orchestrator.getExecutionMode()
        mode_map = {
            ExecutionMode.SEQUENTIAL: "sequential",
            ExecutionMode.PARALLEL: "parallel",
            ExecutionMode.PIPELINE: "pipeline",
            ExecutionMode.BATCH: "batch"
        }
        return mode_map.get(mode, "unknown")

    def set_max_parallel_steps(self, max_steps: int):
        """Set maximum number of parallel steps"""
        self._orchestrator.setMaxParallelSteps(max_steps)

    def get_max_parallel_steps(self) -> int:
        """Get maximum number of parallel steps"""
        return self._orchestrator.getMaxParallelSteps()

    # Checkpointing
    def enable_checkpointing(self, enable: bool, interval_minutes: int = 10):
        """Enable/disable automatic checkpointing"""
        self._orchestrator.enableCheckpointing(enable, interval_minutes)

    def save_checkpoint(self, filename: str):
        """Save simulation checkpoint"""
        self._orchestrator.saveCheckpoint(filename.encode('utf-8'))

    def load_checkpoint(self, filename: str):
        """Load simulation checkpoint"""
        self._orchestrator.loadCheckpoint(filename.encode('utf-8'))

    # Statistics and reporting
    def get_execution_statistics(self) -> PyExecutionStatistics:
        """Get execution statistics"""
        cdef ExecutionStatistics stats = self._orchestrator.getExecutionStatistics()
        return PyExecutionStatistics(stats)

    def generate_execution_report(self, filename: str):
        """Generate execution report"""
        self._orchestrator.generateExecutionReport(filename.encode('utf-8'))

# Convenience functions for Python users
def create_process_step(step_type: str, name: str, **kwargs) -> PyProcessStepDefinition:
    """Create a process step with parameters"""
    step = PyProcessStepDefinition(step_type, name)

    if 'parameters' in kwargs:
        step.parameters = kwargs['parameters']
    if 'input_files' in kwargs:
        step.input_files = kwargs['input_files']
    if 'output_files' in kwargs:
        step.output_files = kwargs['output_files']
    if 'dependencies' in kwargs:
        step.dependencies = kwargs['dependencies']
    if 'estimated_duration' in kwargs:
        step.estimated_duration = kwargs['estimated_duration']
    if 'priority' in kwargs:
        step.priority = kwargs['priority']
    if 'parallel_compatible' in kwargs:
        step.parallel_compatible = kwargs['parallel_compatible']

    return step

def get_orchestrator() -> PySimulationOrchestrator:
    """Get the global simulation orchestrator instance"""
    return PySimulationOrchestrator()

# Constants for Python users
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

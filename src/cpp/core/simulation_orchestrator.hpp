// Author: Dr. Mazharuddin Mohammed
#ifndef SIMULATION_ORCHESTRATOR_HPP
#define SIMULATION_ORCHESTRATOR_HPP

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <yaml-cpp/yaml.h>
#include "wafer.hpp"
#include "simulation_engine.hpp"

namespace SemiPRO {

// Forward declarations
class ProcessStep;
class SimulationConfig;
class InputParser;
class OutputGenerator;

/**
 * @brief Main simulation orchestrator that coordinates all process steps
 */
class SimulationOrchestrator {
public:
    // Process step types
    enum class StepType {
        OXIDATION,
        DOPING,
        LITHOGRAPHY,
        DEPOSITION,
        ETCHING,
        METALLIZATION,
        ANNEALING,
        CMP,
        INSPECTION,
        CUSTOM
    };

    // Execution modes
    enum class ExecutionMode {
        SEQUENTIAL,     // Execute steps one by one
        PARALLEL,       // Execute compatible steps in parallel
        PIPELINE,       // Pipeline execution with overlapping
        BATCH          // Batch processing multiple wafers
    };

    // Simulation state
    enum class SimulationState {
        IDLE,
        INITIALIZING,
        RUNNING,
        PAUSED,
        COMPLETED,
        ERROR,
        CANCELLED
    };

    struct ProcessStepDefinition {
        StepType type;
        std::string name;
        std::unordered_map<std::string, double> parameters;
        std::vector<std::string> input_files;
        std::vector<std::string> output_files;
        std::vector<std::string> dependencies;
        double estimated_duration;
        int priority;
        bool parallel_compatible;
        
        ProcessStepDefinition(StepType t, const std::string& n) 
            : type(t), name(n), estimated_duration(0.0), priority(0), parallel_compatible(false) {}
    };

    struct SimulationFlow {
        std::string name;
        std::string description;
        std::vector<ProcessStepDefinition> steps;
        std::unordered_map<std::string, std::string> global_parameters;
        ExecutionMode execution_mode;
        
        SimulationFlow(const std::string& n) : name(n), execution_mode(ExecutionMode::SEQUENTIAL) {}
    };

    struct SimulationProgress {
        SimulationState state;
        size_t current_step;
        size_t total_steps;
        double progress_percentage;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point estimated_completion;
        std::string current_operation;
        std::vector<std::string> completed_steps;
        std::vector<std::string> errors;
        
        SimulationProgress() : state(SimulationState::IDLE), current_step(0), 
                             total_steps(0), progress_percentage(0.0) {}
    };

public:
    static SimulationOrchestrator& getInstance();
    
    // Configuration management
    void loadConfiguration(const std::string& config_file);
    void saveConfiguration(const std::string& config_file) const;
    void setParameter(const std::string& key, const std::string& value);
    std::string getParameter(const std::string& key) const;
    
    // Flow management
    void loadSimulationFlow(const std::string& flow_file);
    void saveSimulationFlow(const std::string& flow_file) const;
    void addProcessStep(const ProcessStepDefinition& step);
    void removeProcessStep(const std::string& step_name);
    void reorderSteps(const std::vector<std::string>& step_order);
    
    // Execution control
    std::future<bool> executeSimulation(const std::string& wafer_name);
    std::future<bool> executeSimulationFlow(const std::string& flow_name, 
                                           const std::string& wafer_name);
    void pauseSimulation();
    void resumeSimulation();
    void cancelSimulation();
    void resetSimulation();
    
    // Batch processing
    void addWaferToBatch(const std::string& wafer_name, const std::string& flow_name);
    std::future<std::vector<bool>> executeBatch();
    void clearBatch();
    
    // Monitoring and status
    SimulationProgress getProgress() const;
    std::vector<std::string> getAvailableFlows() const;
    std::vector<ProcessStepDefinition> getCurrentFlow() const;
    bool isRunning() const;
    bool isPaused() const;
    
    // Event handling
    using ProgressCallback = std::function<void(const SimulationProgress&)>;
    using StepCompletedCallback = std::function<void(const std::string&, bool)>;
    using ErrorCallback = std::function<void(const std::string&, const std::string&)>;
    
    void setProgressCallback(ProgressCallback callback);
    void setStepCompletedCallback(StepCompletedCallback callback);
    void setErrorCallback(ErrorCallback callback);
    
    // Input/Output management
    void setInputDirectory(const std::string& directory);
    void setOutputDirectory(const std::string& directory);
    std::string getInputDirectory() const;
    std::string getOutputDirectory() const;
    
    // Validation
    bool validateFlow(const SimulationFlow& flow) const;
    std::vector<std::string> checkDependencies(const SimulationFlow& flow) const;
    
    // Performance optimization
    void setExecutionMode(ExecutionMode mode);
    ExecutionMode getExecutionMode() const;
    void setMaxParallelSteps(int max_steps);
    int getMaxParallelSteps() const;
    
    // Checkpointing
    void enableCheckpointing(bool enable, int interval_minutes = 10);
    void saveCheckpoint(const std::string& filename);
    void loadCheckpoint(const std::string& filename);
    
    // Statistics and reporting
    struct ExecutionStatistics {
        std::chrono::duration<double> total_execution_time;
        std::chrono::duration<double> average_step_time;
        size_t total_steps_executed;
        size_t successful_steps;
        size_t failed_steps;
        double cpu_utilization;
        double memory_usage;
        
        ExecutionStatistics() : total_steps_executed(0), successful_steps(0), 
                              failed_steps(0), cpu_utilization(0.0), memory_usage(0.0) {}
    };
    
    ExecutionStatistics getExecutionStatistics() const;
    void generateExecutionReport(const std::string& filename) const;

private:
    SimulationOrchestrator() = default;
    ~SimulationOrchestrator() = default;
    
    // Internal state
    std::unique_ptr<SimulationConfig> config_;
    std::unique_ptr<InputParser> input_parser_;
    std::unique_ptr<OutputGenerator> output_generator_;
    
    std::unordered_map<std::string, SimulationFlow> flows_;
    SimulationFlow current_flow_;
    std::vector<std::pair<std::string, std::string>> batch_queue_;
    
    mutable std::mutex state_mutex_;
    std::condition_variable state_cv_;
    std::atomic<SimulationState> current_state_{SimulationState::IDLE};
    SimulationProgress progress_;
    
    // Execution control
    std::atomic<bool> should_pause_{false};
    std::atomic<bool> should_cancel_{false};
    ExecutionMode execution_mode_{ExecutionMode::SEQUENTIAL};
    int max_parallel_steps_{4};
    
    // Callbacks
    std::vector<ProgressCallback> progress_callbacks_;
    std::vector<StepCompletedCallback> step_completion_callbacks_;
    std::vector<ErrorCallback> error_callbacks_;
    
    // Directories
    std::string input_directory_;
    std::string output_directory_;
    
    // Checkpointing
    bool checkpointing_enabled_{false};
    int checkpoint_interval_{10};
    std::thread checkpoint_thread_;
    
    // Statistics
    mutable ExecutionStatistics stats_;
    std::chrono::system_clock::time_point execution_start_time_;
    
    // Internal methods
    bool executeStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeStepSequential(const ProcessStepDefinition& step, const std::string& wafer_name);
    std::vector<std::future<bool>> executeStepsParallel(
        const std::vector<ProcessStepDefinition>& steps, const std::string& wafer_name);
    
    void updateProgress();
    void notifyProgress();
    void notifyStepCompleted(const std::string& step_name, bool success);
    void notifyError(const std::string& step_name, const std::string& error_message);
    
    bool resolveDependencies(const std::vector<ProcessStepDefinition>& steps) const;
    std::vector<std::vector<ProcessStepDefinition>> groupParallelSteps(
        const std::vector<ProcessStepDefinition>& steps) const;
    
    void checkpointWorker();
    void updateStatistics();

    // Execution flow methods
    bool executeSequentialFlow(const std::string& wafer_name);
    bool executeParallelFlow(const std::string& wafer_name);
    bool executePipelineFlow(const std::string& wafer_name);
    bool executeBatchFlow(const std::string& wafer_name);
    
    // Process step execution
    bool executeOxidationStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeDopingStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeLithographyStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeDepositionStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeEtchingStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeMetallizationStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeAnnealingStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeCMPStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeInspectionStep(const ProcessStepDefinition& step, const std::string& wafer_name);
    bool executeCustomStep(const ProcessStepDefinition& step, const std::string& wafer_name);
};

/**
 * @brief Configuration management for simulation parameters
 */
class SimulationConfig {
public:
    SimulationConfig() = default;
    explicit SimulationConfig(const std::string& config_file);

    void loadFromFile(const std::string& config_file);
    void saveToFile(const std::string& config_file) const;

    void setParameter(const std::string& section, const std::string& key, const YAML::Node& value);
    YAML::Node getParameter(const std::string& section, const std::string& key) const;

    bool hasSection(const std::string& section) const;
    bool hasParameter(const std::string& section, const std::string& key) const;

    std::vector<std::string> getSections() const;
    std::vector<std::string> getKeys(const std::string& section) const;

    void merge(const SimulationConfig& other);
    void clear();

private:
    YAML::Node config_;
    mutable std::mutex config_mutex_;
};

} // namespace SemiPRO

#endif // SIMULATION_ORCHESTRATOR_HPP

#pragma once

#include "../core/advanced_logger.hpp"
#include "../core/enhanced_error_handling.hpp"
#include "../core/config_manager.hpp"
#include "../core/wafer_enhanced.hpp"
#include "multi_layer_engine.hpp"
#include "temperature_controller.hpp"
#include "process_optimizer.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>
#include <chrono>

namespace SemiPRO {

/**
 * Advanced Process Integration Engine
 * Implements sophisticated process flow management, recipe sequencing,
 * cross-process dependencies, and automated process integration
 */

// Process flow control types
enum class ProcessFlowType {
    SEQUENTIAL,           // Sequential process execution
    PARALLEL,             // Parallel process execution
    CONDITIONAL,          // Conditional process execution
    ITERATIVE,            // Iterative process execution
    FEEDBACK_CONTROLLED,  // Feedback-controlled execution
    ADAPTIVE,             // Adaptive process flow
    HIERARCHICAL,         // Hierarchical process structure
    PIPELINE              // Pipeline process execution
};

// Process dependency types
enum class DependencyType {
    PREREQUISITE,         // Must complete before this process
    CONCURRENT,           // Can run concurrently
    EXCLUSIVE,            // Cannot run simultaneously
    CONDITIONAL,          // Depends on condition/result
    THERMAL_BUDGET,       // Thermal budget dependency
    CONTAMINATION,        // Contamination dependency
    EQUIPMENT,            // Equipment dependency
    TIMING                // Timing dependency
};

// Process execution status
enum class ProcessStatus {
    PENDING,              // Waiting to start
    READY,                // Ready to execute
    RUNNING,              // Currently executing
    COMPLETED,            // Successfully completed
    FAILED,               // Failed execution
    SKIPPED,              // Skipped due to conditions
    PAUSED,               // Paused execution
    CANCELLED             // Cancelled execution
};

// Process step definition
struct ProcessStep {
    std::string step_id;                    // Unique step identifier
    std::string process_type;               // Process type (oxidation, deposition, etc.)
    std::unordered_map<std::string, double> parameters; // Process parameters
    std::vector<std::string> prerequisites; // Required prerequisite steps
    std::vector<std::string> dependencies;  // Dependency step IDs
    DependencyType dependency_type;         // Type of dependency
    ProcessStatus status;                   // Current execution status
    double estimated_time;                  // Estimated execution time (minutes)
    double actual_time;                     // Actual execution time (minutes)
    int priority;                           // Execution priority (higher = more important)
    std::unordered_map<std::string, double> conditions; // Execution conditions
    std::unordered_map<std::string, double> results;    // Process results
    
    ProcessStep() : dependency_type(DependencyType::PREREQUISITE), 
                   status(ProcessStatus::PENDING), estimated_time(1.0), 
                   actual_time(0.0), priority(0) {}
};

// Process recipe definition
struct ProcessRecipe {
    std::string recipe_id;                  // Recipe identifier
    std::string recipe_name;                // Human-readable name
    std::string description;                // Recipe description
    ProcessFlowType flow_type;              // Flow control type
    std::vector<ProcessStep> steps;         // Process steps
    std::unordered_map<std::string, double> global_parameters; // Global parameters
    std::unordered_map<std::string, std::string> equipment_requirements; // Equipment needs
    double total_estimated_time;            // Total estimated time
    double thermal_budget_limit;            // Thermal budget limit
    std::vector<std::string> critical_dimensions; // Critical dimensions to monitor
    
    ProcessRecipe() : flow_type(ProcessFlowType::SEQUENTIAL), 
                     total_estimated_time(0.0), thermal_budget_limit(10000.0) {}
};

// Process execution context
struct ProcessExecutionContext {
    std::string execution_id;               // Unique execution identifier
    std::shared_ptr<WaferEnhanced> wafer;   // Target wafer
    ProcessRecipe recipe;                   // Recipe being executed
    std::unordered_map<std::string, double> runtime_parameters; // Runtime parameters
    std::chrono::steady_clock::time_point start_time; // Execution start time
    double elapsed_time;                    // Total elapsed time
    std::vector<std::string> completed_steps; // Completed step IDs
    std::vector<std::string> failed_steps;  // Failed step IDs
    std::unordered_map<std::string, double> measured_results; // Measured results
    bool is_paused;                         // Whether execution is paused
    
    ProcessExecutionContext() : elapsed_time(0.0), is_paused(false) {}
};

// Cross-process dependency definition
struct CrossProcessDependency {
    std::string source_process;             // Source process ID
    std::string target_process;             // Target process ID
    std::string parameter_name;             // Parameter being transferred
    std::function<double(double)> transfer_function; // Transfer function
    DependencyType dependency_type;         // Type of dependency
    bool is_critical;                       // Whether dependency is critical
    
    CrossProcessDependency() : dependency_type(DependencyType::PREREQUISITE), 
                              is_critical(true) {}
};

// Process integration results
struct ProcessIntegrationResults {
    std::string execution_id;               // Execution identifier
    ProcessStatus overall_status;           // Overall execution status
    std::vector<ProcessStep> executed_steps; // All executed steps
    double total_execution_time;            // Total execution time
    double thermal_budget_used;             // Thermal budget consumed
    std::unordered_map<std::string, double> final_results; // Final process results
    std::unordered_map<std::string, double> quality_metrics; // Quality metrics
    std::vector<std::string> warnings;      // Process warnings
    std::vector<std::string> recommendations; // Process recommendations
    
    ProcessIntegrationResults() : overall_status(ProcessStatus::PENDING), 
                                 total_execution_time(0.0), thermal_budget_used(0.0) {}
};

// Advanced process integrator
class ProcessIntegrator {
private:
    std::unordered_map<std::string, ProcessRecipe> recipes_;
    std::unordered_map<std::string, ProcessExecutionContext> active_executions_;
    std::unordered_map<std::string, CrossProcessDependency> cross_dependencies_;
    std::queue<std::string> execution_queue_;
    
    // Integration engines
    std::unique_ptr<MultiLayerEngine> multi_layer_engine_;
    std::unique_ptr<AdvancedTemperatureController> temperature_controller_;
    std::unique_ptr<ProcessOptimizer> process_optimizer_;
    
    // Configuration
    bool enable_parallel_execution_ = true;
    bool enable_dependency_checking_ = true;
    bool enable_thermal_budget_tracking_ = true;
    bool enable_adaptive_flow_ = true;
    int max_concurrent_processes_ = 4;
    double default_timeout_ = 60.0; // minutes
    
public:
    ProcessIntegrator();
    ~ProcessIntegrator() = default;
    
    // Recipe management
    void createRecipe(const std::string& recipe_id, const ProcessRecipe& recipe);
    void addProcessStep(const std::string& recipe_id, const ProcessStep& step);
    void addCrossProcessDependency(const std::string& dependency_id, 
                                  const CrossProcessDependency& dependency);
    
    ProcessRecipe& getRecipe(const std::string& recipe_id);
    const ProcessRecipe& getRecipe(const std::string& recipe_id) const;
    
    // Process execution
    ProcessIntegrationResults executeRecipe(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& recipe_id,
        const std::unordered_map<std::string, double>& runtime_parameters = {}
    );
    
    ProcessIntegrationResults executeSequentialFlow(
        ProcessExecutionContext& context
    );
    
    ProcessIntegrationResults executeParallelFlow(
        ProcessExecutionContext& context
    );
    
    ProcessIntegrationResults executeAdaptiveFlow(
        ProcessExecutionContext& context
    );
    
    // Process step execution
    bool executeProcessStep(
        ProcessExecutionContext& context,
        ProcessStep& step
    );
    
    bool validateStepPrerequisites(
        const ProcessExecutionContext& context,
        const ProcessStep& step
    ) const;
    
    // Dependency management
    std::vector<std::string> getReadySteps(
        const ProcessExecutionContext& context
    ) const;
    
    bool checkDependencies(
        const ProcessExecutionContext& context,
        const ProcessStep& step
    ) const;
    
    void propagateCrossProcessDependencies(
        ProcessExecutionContext& context,
        const ProcessStep& completed_step
    );
    
    // Flow control
    void pauseExecution(const std::string& execution_id);
    void resumeExecution(const std::string& execution_id);
    void cancelExecution(const std::string& execution_id);
    
    ProcessStatus getExecutionStatus(const std::string& execution_id) const;
    
    // Thermal budget management
    double calculateThermalBudget(
        const ProcessExecutionContext& context
    ) const;
    
    bool checkThermalBudgetLimit(
        const ProcessExecutionContext& context,
        const ProcessStep& step
    ) const;
    
    // Process optimization integration
    ProcessRecipe optimizeRecipe(
        const std::string& recipe_id,
        const std::vector<OptimizationObjective>& objectives
    );
    
    std::unordered_map<std::string, double> optimizeProcessParameters(
        const ProcessStep& step,
        const std::unordered_map<std::string, double>& constraints
    );
    
    // Quality analysis
    std::unordered_map<std::string, double> analyzeProcessQuality(
        const ProcessIntegrationResults& results
    ) const;
    
    std::vector<std::string> generateProcessRecommendations(
        const ProcessIntegrationResults& results
    ) const;
    
    // Advanced features
    ProcessRecipe generateAdaptiveRecipe(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::vector<std::string>& target_specifications
    );
    
    void implementFeedbackControl(
        ProcessExecutionContext& context,
        const std::unordered_map<std::string, double>& measurements
    );
    
    // Monitoring and diagnostics
    std::vector<ProcessExecutionContext> getActiveExecutions() const;
    std::unordered_map<std::string, double> getExecutionStatistics() const;
    std::vector<std::string> getDiagnostics() const;
    
    // Configuration
    void setMaxConcurrentProcesses(int max_processes);
    void setDefaultTimeout(double timeout_minutes);
    void enableFeatures(bool parallel = true, bool dependencies = true,
                       bool thermal_budget = true, bool adaptive = true);
    
    // Validation
    bool validateRecipe(const std::string& recipe_id, std::string& error_message) const;
    std::vector<std::string> validateCrossProcessDependencies() const;
    
    // Utility methods
    std::string flowTypeToString(ProcessFlowType type) const;
    std::string dependencyTypeToString(DependencyType type) const;
    std::string statusToString(ProcessStatus status) const;
    
private:
    void initializeIntegrator();
    
    // Process execution helpers
    bool executePhysicsProcess(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        const std::unordered_map<std::string, double>& parameters
    );
    
    // Dependency resolution
    std::vector<std::string> resolveDependencyOrder(
        const ProcessRecipe& recipe
    ) const;
    
    bool hasCyclicDependencies(
        const ProcessRecipe& recipe
    ) const;
    
    // Timing and scheduling
    double estimateExecutionTime(
        const ProcessRecipe& recipe
    ) const;
    
    std::vector<ProcessStep> scheduleParallelExecution(
        const std::vector<ProcessStep>& steps
    ) const;
    
    // Error handling and recovery
    void handleProcessFailure(
        ProcessExecutionContext& context,
        ProcessStep& failed_step
    );
    
    bool attemptProcessRecovery(
        ProcessExecutionContext& context,
        ProcessStep& failed_step
    );
    
    // Adaptive algorithms
    void adaptProcessParameters(
        ProcessExecutionContext& context,
        const std::unordered_map<std::string, double>& feedback
    );
    
    ProcessStep generateAdaptiveStep(
        const ProcessExecutionContext& context,
        const std::string& target_specification
    );
};

// Utility functions for process integration
namespace ProcessIntegrationUtils {
    // Recipe generation utilities
    ProcessRecipe generateCMOSRecipe();
    ProcessRecipe generateMemoryDeviceRecipe();
    ProcessRecipe generateMEMSRecipe();
    ProcessRecipe generatePowerDeviceRecipe();
    
    // Dependency analysis
    std::vector<std::string> analyzeCriticalPath(const ProcessRecipe& recipe);
    double calculateRecipeComplexity(const ProcessRecipe& recipe);
    std::unordered_map<std::string, double> analyzeResourceUtilization(const ProcessRecipe& recipe);
    
    // Process flow optimization
    ProcessRecipe optimizeProcessFlow(const ProcessRecipe& recipe);
    std::vector<ProcessStep> parallelizeCompatibleSteps(const std::vector<ProcessStep>& steps);
    ProcessRecipe minimizeProcessTime(const ProcessRecipe& recipe);
    
    // Quality metrics
    double calculateProcessYield(const ProcessIntegrationResults& results);
    double calculateProcessEfficiency(const ProcessIntegrationResults& results);
    std::unordered_map<std::string, double> calculateProcessMetrics(const ProcessIntegrationResults& results);
}

} // namespace SemiPRO

// Author: Dr. Mazharuddin Mohammed
#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

namespace SemiPRO {

/**
 * @brief Workflow step status
 */
enum class StepStatus {
    PENDING,
    RUNNING,
    COMPLETED,
    FAILED,
    SKIPPED,
    CANCELLED
};

/**
 * @brief Workflow execution mode
 */
enum class ExecutionMode {
    SEQUENTIAL,
    PARALLEL,
    CONDITIONAL,
    PIPELINE
};

/**
 * @brief Workflow step configuration
 */
struct WorkflowStep {
    std::string id;
    std::string name;
    std::string description;
    std::string module_name;
    std::unordered_map<std::string, std::string> parameters;
    std::vector<std::string> dependencies;
    std::vector<std::string> outputs;
    ExecutionMode execution_mode = ExecutionMode::SEQUENTIAL;
    int max_retries = 0;
    int timeout_seconds = 0;
    bool optional = false;
    std::function<bool()> condition = nullptr;
    
    // Runtime information
    StepStatus status = StepStatus::PENDING;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    std::string error_message;
    int retry_count = 0;
    std::unordered_map<std::string, std::string> results;
};

/**
 * @brief Workflow definition
 */
struct Workflow {
    std::string id;
    std::string name;
    std::string description;
    std::string version;
    std::vector<WorkflowStep> steps;
    std::unordered_map<std::string, std::string> global_parameters;
    ExecutionMode default_execution_mode = ExecutionMode::SEQUENTIAL;
    int max_parallel_steps = 4;
    bool allow_partial_failure = false;
    
    // Runtime information
    StepStatus status = StepStatus::PENDING;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    std::string current_step_id;
    double progress = 0.0;
    std::unordered_map<std::string, std::string> execution_context;
};

/**
 * @brief Workflow execution result
 */
struct WorkflowResult {
    std::string workflow_id;
    StepStatus final_status;
    std::chrono::duration<double> execution_time;
    std::vector<WorkflowStep> executed_steps;
    std::unordered_map<std::string, std::string> final_outputs;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};

/**
 * @brief Progress callback function type
 */
using ProgressCallback = std::function<void(const std::string& workflow_id, double progress, const std::string& current_step)>;

/**
 * @brief Step execution function type
 */
using StepExecutor = std::function<std::unordered_map<std::string, std::string>(const WorkflowStep&, const std::unordered_map<std::string, std::string>&)>;

/**
 * @brief Workflow manager for orchestrating complex simulation workflows
 */
class WorkflowManager {
private:
    std::unordered_map<std::string, Workflow> workflows_;
    std::unordered_map<std::string, StepExecutor> step_executors_;
    std::unordered_map<std::string, std::future<WorkflowResult>> running_workflows_;
    
    // Threading
    std::vector<std::thread> worker_threads_;
    std::queue<std::string> workflow_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> shutdown_requested_{false};
    
    // Progress tracking
    std::vector<ProgressCallback> progress_callbacks_;
    std::mutex callback_mutex_;
    
    // Configuration
    int max_concurrent_workflows_;
    int worker_thread_count_;
    
public:
    WorkflowManager(int max_concurrent = 4, int worker_threads = 2);
    ~WorkflowManager();
    
    // Workflow management
    void registerWorkflow(const Workflow& workflow);
    void unregisterWorkflow(const std::string& workflow_id);
    bool hasWorkflow(const std::string& workflow_id) const;
    Workflow getWorkflow(const std::string& workflow_id) const;
    std::vector<std::string> getWorkflowIds() const;
    
    // Step executor registration
    void registerStepExecutor(const std::string& module_name, StepExecutor executor);
    void unregisterStepExecutor(const std::string& module_name);
    
    // Workflow execution
    std::string executeWorkflow(const std::string& workflow_id, 
                               const std::unordered_map<std::string, std::string>& parameters = {});
    std::string executeWorkflowAsync(const std::string& workflow_id,
                                    const std::unordered_map<std::string, std::string>& parameters = {});
    
    // Workflow control
    bool pauseWorkflow(const std::string& execution_id);
    bool resumeWorkflow(const std::string& execution_id);
    bool cancelWorkflow(const std::string& execution_id);
    bool retryWorkflow(const std::string& execution_id, const std::string& from_step = "");
    
    // Status and monitoring
    StepStatus getWorkflowStatus(const std::string& execution_id);
    double getWorkflowProgress(const std::string& execution_id);
    std::string getCurrentStep(const std::string& execution_id);
    WorkflowResult getWorkflowResult(const std::string& execution_id);
    std::vector<std::string> getRunningWorkflows() const;
    
    // Progress callbacks
    void addProgressCallback(ProgressCallback callback);
    void removeProgressCallback(ProgressCallback callback);
    
    // Workflow templates
    void saveWorkflowTemplate(const std::string& template_name, const Workflow& workflow);
    Workflow loadWorkflowTemplate(const std::string& template_name);
    std::vector<std::string> getWorkflowTemplates() const;
    
    // Batch processing
    std::vector<std::string> executeBatch(const std::vector<std::string>& workflow_ids,
                                         const std::vector<std::unordered_map<std::string, std::string>>& parameters_list);
    
    // Workflow validation
    bool validateWorkflow(const Workflow& workflow, std::vector<std::string>& errors);
    bool validateStepDependencies(const Workflow& workflow);
    
    // Serialization
    std::string serializeWorkflow(const Workflow& workflow);
    Workflow deserializeWorkflow(const std::string& serialized);
    void saveWorkflowToFile(const Workflow& workflow, const std::string& filename);
    Workflow loadWorkflowFromFile(const std::string& filename);
    
private:
    void workerLoop();
    WorkflowResult executeWorkflowInternal(const Workflow& workflow,
                                          const std::unordered_map<std::string, std::string>& parameters);
    
    bool canExecuteStep(const WorkflowStep& step, const std::vector<WorkflowStep>& all_steps);
    std::unordered_map<std::string, std::string> executeStep(const WorkflowStep& step,
                                                             const std::unordered_map<std::string, std::string>& context);
    
    void updateProgress(const std::string& workflow_id, double progress, const std::string& current_step);
    std::string generateExecutionId();
    
    // Built-in step executors
    void registerBuiltinExecutors();
    std::unordered_map<std::string, std::string> executeProcessStep(const WorkflowStep& step,
                                                                    const std::unordered_map<std::string, std::string>& context);
    std::unordered_map<std::string, std::string> executeAnalysisStep(const WorkflowStep& step,
                                                                     const std::unordered_map<std::string, std::string>& context);
    std::unordered_map<std::string, std::string> executeExportStep(const WorkflowStep& step,
                                                                   const std::unordered_map<std::string, std::string>& context);
};

/**
 * @brief Workflow builder for easy workflow construction
 */
class WorkflowBuilder {
private:
    Workflow workflow_;
    
public:
    WorkflowBuilder(const std::string& id, const std::string& name);
    
    WorkflowBuilder& setDescription(const std::string& description);
    WorkflowBuilder& setVersion(const std::string& version);
    WorkflowBuilder& setExecutionMode(ExecutionMode mode);
    WorkflowBuilder& setMaxParallelSteps(int max_parallel);
    WorkflowBuilder& allowPartialFailure(bool allow = true);
    
    WorkflowBuilder& addGlobalParameter(const std::string& key, const std::string& value);
    WorkflowBuilder& addStep(const WorkflowStep& step);
    WorkflowBuilder& addProcessStep(const std::string& id, const std::string& module_name,
                                   const std::unordered_map<std::string, std::string>& parameters = {});
    WorkflowBuilder& addAnalysisStep(const std::string& id, const std::string& analysis_type,
                                    const std::unordered_map<std::string, std::string>& parameters = {});
    WorkflowBuilder& addConditionalStep(const std::string& id, const std::string& module_name,
                                       std::function<bool()> condition,
                                       const std::unordered_map<std::string, std::string>& parameters = {});
    
    WorkflowBuilder& addDependency(const std::string& step_id, const std::string& dependency_id);
    WorkflowBuilder& setStepTimeout(const std::string& step_id, int timeout_seconds);
    WorkflowBuilder& setStepRetries(const std::string& step_id, int max_retries);
    WorkflowBuilder& makeStepOptional(const std::string& step_id, bool optional = true);
    
    Workflow build();
};

/**
 * @brief Predefined workflow templates
 */
class WorkflowTemplates {
public:
    static Workflow createMOSFETFabricationWorkflow();
    static Workflow createProcessCharacterizationWorkflow();
    static Workflow createReliabilityTestWorkflow();
    static Workflow createParameterSweepWorkflow();
    static Workflow createBatchProcessingWorkflow();
    static Workflow createDesignRuleCheckWorkflow();
    static Workflow createMultiDieAnalysisWorkflow();
};

} // namespace SemiPRO

#include "process_integrator.hpp"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <numeric>

namespace SemiPRO {

ProcessIntegrator::ProcessIntegrator() {
    initializeIntegrator();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Process Integration Engine initialized", "ProcessIntegrator");
}

void ProcessIntegrator::createRecipe(const std::string& recipe_id, const ProcessRecipe& recipe) {
    recipes_[recipe_id] = recipe;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Created process recipe: " + recipe_id + 
                      " (" + recipe.recipe_name + ") with " + 
                      std::to_string(recipe.steps.size()) + " steps",
                      "ProcessIntegrator");
}

void ProcessIntegrator::addProcessStep(const std::string& recipe_id, const ProcessStep& step) {
    auto recipe_it = recipes_.find(recipe_id);
    if (recipe_it == recipes_.end()) {
        throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, 
                                              "Recipe not found: " + recipe_id, "RECIPE_NOT_FOUND"));
    }
    
    recipe_it->second.steps.push_back(step);
    recipe_it->second.total_estimated_time += step.estimated_time;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Added process step: " + step.step_id + 
                      " (" + step.process_type + ") to recipe " + recipe_id,
                      "ProcessIntegrator");
}

ProcessIntegrationResults ProcessIntegrator::executeRecipe(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& recipe_id,
    const std::unordered_map<std::string, double>& runtime_parameters) {
    
    SEMIPRO_PERF_TIMER("recipe_execution", "ProcessIntegrator");
    
    ProcessIntegrationResults results;
    
    try {
        auto recipe_it = recipes_.find(recipe_id);
        if (recipe_it == recipes_.end()) {
            throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, 
                                                  "Recipe not found: " + recipe_id, "RECIPE_NOT_FOUND"));
        }
        
        const auto& recipe = recipe_it->second;
        
        // Create execution context
        ProcessExecutionContext context;
        context.execution_id = recipe_id + "_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        context.wafer = wafer;
        context.recipe = recipe;
        context.runtime_parameters = runtime_parameters;
        context.start_time = std::chrono::steady_clock::now();
        
        active_executions_[context.execution_id] = context;
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Starting recipe execution: " + recipe_id + 
                          " (execution ID: " + context.execution_id + ")",
                          "ProcessIntegrator");
        
        // Route to appropriate flow execution
        switch (recipe.flow_type) {
            case ProcessFlowType::SEQUENTIAL:
                results = executeSequentialFlow(context);
                break;
                
            case ProcessFlowType::PARALLEL:
                results = executeParallelFlow(context);
                break;
                
            case ProcessFlowType::ADAPTIVE:
                results = executeAdaptiveFlow(context);
                break;
                
            default:
                // Default to sequential
                results = executeSequentialFlow(context);
                break;
        }
        
        // Calculate final results
        auto end_time = std::chrono::steady_clock::now();
        results.total_execution_time = std::chrono::duration<double>(end_time - context.start_time).count() / 60.0; // Convert to minutes
        results.execution_id = context.execution_id;
        
        // Calculate thermal budget
        results.thermal_budget_used = calculateThermalBudget(context);
        
        // Analyze quality
        results.quality_metrics = analyzeProcessQuality(results);
        
        // Generate recommendations
        results.recommendations = generateProcessRecommendations(results);
        
        // Clean up execution context
        active_executions_.erase(context.execution_id);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Recipe execution completed: " + recipe_id + 
                          " in " + std::to_string(results.total_execution_time) + " minutes",
                          "ProcessIntegrator");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Recipe execution failed: " + std::string(e.what()),
                          "ProcessIntegrator");
        results.overall_status = ProcessStatus::FAILED;
    }
    
    return results;
}

ProcessIntegrationResults ProcessIntegrator::executeSequentialFlow(
    ProcessExecutionContext& context) {
    
    ProcessIntegrationResults results;
    results.overall_status = ProcessStatus::RUNNING;
    
    try {
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Executing sequential process flow with " + 
                          std::to_string(context.recipe.steps.size()) + " steps",
                          "ProcessIntegrator");
        
        // Execute steps in order
        for (auto& step : context.recipe.steps) {
            // Check prerequisites
            if (!validateStepPrerequisites(context, step)) {
                SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::ADVANCED,
                                  "Step prerequisites not met: " + step.step_id,
                                  "ProcessIntegrator");
                step.status = ProcessStatus::SKIPPED;
                continue;
            }
            
            // Check thermal budget
            if (enable_thermal_budget_tracking_ && !checkThermalBudgetLimit(context, step)) {
                SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::ADVANCED,
                                  "Thermal budget limit exceeded for step: " + step.step_id,
                                  "ProcessIntegrator");
                results.warnings.push_back("Thermal budget limit exceeded for step: " + step.step_id);
            }
            
            // Execute step
            step.status = ProcessStatus::RUNNING;
            auto step_start = std::chrono::steady_clock::now();
            
            bool step_success = executeProcessStep(context, step);
            
            auto step_end = std::chrono::steady_clock::now();
            step.actual_time = std::chrono::duration<double>(step_end - step_start).count() / 60.0; // Convert to minutes
            
            if (step_success) {
                step.status = ProcessStatus::COMPLETED;
                context.completed_steps.push_back(step.step_id);
                results.executed_steps.push_back(step);
                
                // Propagate cross-process dependencies
                if (enable_dependency_checking_) {
                    propagateCrossProcessDependencies(context, step);
                }
                
                SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::ADVANCED,
                                  "Step completed: " + step.step_id + 
                                  " in " + std::to_string(step.actual_time) + " minutes",
                                  "ProcessIntegrator");
            } else {
                step.status = ProcessStatus::FAILED;
                context.failed_steps.push_back(step.step_id);
                
                SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                                  "Step failed: " + step.step_id,
                                  "ProcessIntegrator");
                
                // Attempt recovery
                if (!attemptProcessRecovery(context, step)) {
                    results.overall_status = ProcessStatus::FAILED;
                    break;
                }
            }
        }
        
        // Determine overall status
        if (results.overall_status != ProcessStatus::FAILED) {
            bool all_completed = std::all_of(context.recipe.steps.begin(), context.recipe.steps.end(),
                [](const ProcessStep& step) { 
                    return step.status == ProcessStatus::COMPLETED || step.status == ProcessStatus::SKIPPED; 
                });
            
            results.overall_status = all_completed ? ProcessStatus::COMPLETED : ProcessStatus::FAILED;
        }
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Sequential flow execution completed with status: " + 
                          statusToString(results.overall_status),
                          "ProcessIntegrator");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Sequential flow execution failed: " + std::string(e.what()),
                          "ProcessIntegrator");
        results.overall_status = ProcessStatus::FAILED;
    }
    
    return results;
}

ProcessIntegrationResults ProcessIntegrator::executeParallelFlow(
    ProcessExecutionContext& context) {
    
    ProcessIntegrationResults results;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Parallel flow execution not fully implemented - using sequential flow",
                      "ProcessIntegrator");
    
    // For now, fall back to sequential execution
    return executeSequentialFlow(context);
}

ProcessIntegrationResults ProcessIntegrator::executeAdaptiveFlow(
    ProcessExecutionContext& context) {
    
    ProcessIntegrationResults results;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Adaptive flow execution not fully implemented - using sequential flow",
                      "ProcessIntegrator");
    
    // For now, fall back to sequential execution
    return executeSequentialFlow(context);
}

bool ProcessIntegrator::executeProcessStep(
    ProcessExecutionContext& context,
    ProcessStep& step) {
    
    try {
        SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::ADVANCED,
                          "Executing process step: " + step.step_id + 
                          " (type: " + step.process_type + ")",
                          "ProcessIntegrator");
        
        // Merge step parameters with runtime parameters
        auto merged_parameters = step.parameters;
        for (const auto& runtime_param : context.runtime_parameters) {
            merged_parameters[runtime_param.first] = runtime_param.second;
        }
        
        // Execute the physics process
        bool success = executePhysicsProcess(context.wafer, step.process_type, merged_parameters);
        
        if (success) {
            // Store results (simplified)
            step.results["execution_time"] = step.actual_time;
            step.results["success"] = 1.0;
            
            // Update context measurements
            context.measured_results[step.step_id + "_time"] = step.actual_time;
            context.measured_results[step.step_id + "_success"] = 1.0;
        }
        
        return success;
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Process step execution failed: " + step.step_id + 
                          " - " + std::string(e.what()),
                          "ProcessIntegrator");
        return false;
    }
}

bool ProcessIntegrator::executePhysicsProcess(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type,
    const std::unordered_map<std::string, double>& parameters) {
    
    try {
        // Route to appropriate physics engine based on process type
        if (process_type == "oxidation") {
            // Create oxidation conditions from parameters
            OxidationConditions conditions;
            
            auto temp_it = parameters.find("temperature");
            if (temp_it != parameters.end()) {
                conditions.temperature = temp_it->second;
            }
            
            auto time_it = parameters.find("time");
            if (time_it != parameters.end()) {
                conditions.time = time_it->second;
            }
            
            auto ambient_it = parameters.find("ambient");
            if (ambient_it != parameters.end()) {
                conditions.atmosphere = (ambient_it->second > 0.5) ? OxidationAtmosphere::DRY_O2 : OxidationAtmosphere::WET_H2O;
            }
            
            // Execute oxidation (simplified - would use actual physics engine)
            return true;
            
        } else if (process_type == "deposition") {
            // Create deposition conditions from parameters
            DepositionConditions conditions;
            
            auto thickness_it = parameters.find("thickness");
            if (thickness_it != parameters.end()) {
                conditions.target_thickness = thickness_it->second;
            }
            
            auto temp_it = parameters.find("temperature");
            if (temp_it != parameters.end()) {
                conditions.temperature = temp_it->second;
            }
            
            // Set default material and technique
            conditions.material = MaterialType::ALUMINUM;
            conditions.technique = DepositionTechnique::CVD;
            
            // Execute deposition (simplified)
            return true;
            
        } else if (process_type == "etching") {
            // Create etching conditions from parameters
            EtchingConditions conditions;
            
            auto depth_it = parameters.find("depth");
            if (depth_it != parameters.end()) {
                conditions.target_depth = depth_it->second;
            }
            
            auto pressure_it = parameters.find("pressure");
            if (pressure_it != parameters.end()) {
                conditions.pressure = pressure_it->second;
            }
            
            // Set default material and technique
            conditions.target_material = EtchMaterial::SILICON;
            conditions.technique = EtchingTechnique::RIE;
            
            // Execute etching (simplified)
            return true;
            
        } else if (process_type == "doping") {
            // Create doping conditions from parameters
            ImplantationConditions conditions;
            
            auto energy_it = parameters.find("energy");
            if (energy_it != parameters.end()) {
                conditions.energy = energy_it->second;
            }
            
            auto dose_it = parameters.find("dose");
            if (dose_it != parameters.end()) {
                conditions.dose = dose_it->second;
            }
            
            // Set default species
            conditions.species = IonSpecies::BORON_11;
            
            // Execute doping (simplified)
            return true;
            
        } else {
            SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::ADVANCED,
                              "Unknown process type: " + process_type,
                              "ProcessIntegrator");
            return false;
        }
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Physics process execution failed: " + process_type + 
                          " - " + std::string(e.what()),
                          "ProcessIntegrator");
        return false;
    }
}

bool ProcessIntegrator::validateStepPrerequisites(
    const ProcessExecutionContext& context,
    const ProcessStep& step) const {
    
    // Check if all prerequisite steps are completed
    for (const std::string& prereq : step.prerequisites) {
        auto it = std::find(context.completed_steps.begin(), context.completed_steps.end(), prereq);
        if (it == context.completed_steps.end()) {
            return false;
        }
    }
    
    return true;
}

double ProcessIntegrator::calculateThermalBudget(
    const ProcessExecutionContext& context) const {
    
    double thermal_budget = 0.0;
    
    // Calculate thermal budget from completed steps
    for (const auto& step : context.recipe.steps) {
        if (step.status == ProcessStatus::COMPLETED) {
            // Simplified thermal budget calculation
            auto temp_it = step.parameters.find("temperature");
            if (temp_it != step.parameters.end()) {
                double temperature = temp_it->second;
                double time = step.actual_time;
                
                // Simple thermal budget: temperature * time
                thermal_budget += temperature * time;
            }
        }
    }
    
    return thermal_budget;
}

bool ProcessIntegrator::checkThermalBudgetLimit(
    const ProcessExecutionContext& context,
    const ProcessStep& step) const {
    
    double current_budget = calculateThermalBudget(context);
    
    // Estimate additional thermal budget for this step
    auto temp_it = step.parameters.find("temperature");
    if (temp_it != step.parameters.end()) {
        double additional_budget = temp_it->second * step.estimated_time;
        
        return (current_budget + additional_budget) <= context.recipe.thermal_budget_limit;
    }
    
    return true; // No temperature parameter, assume OK
}

void ProcessIntegrator::propagateCrossProcessDependencies(
    ProcessExecutionContext& context,
    const ProcessStep& completed_step) {
    
    // Propagate dependencies (simplified implementation)
    for (const auto& dependency : cross_dependencies_) {
        if (dependency.second.source_process == completed_step.step_id) {
            // Find target step and update its parameters
            for (auto& step : context.recipe.steps) {
                if (step.step_id == dependency.second.target_process) {
                    // Apply transfer function (simplified)
                    auto source_it = completed_step.results.find(dependency.second.parameter_name);
                    if (source_it != completed_step.results.end()) {
                        double transferred_value = source_it->second;
                        if (dependency.second.transfer_function) {
                            transferred_value = dependency.second.transfer_function(transferred_value);
                        }
                        step.parameters[dependency.second.parameter_name] = transferred_value;
                    }
                    break;
                }
            }
        }
    }
}

ProcessRecipe& ProcessIntegrator::getRecipe(const std::string& recipe_id) {
    auto it = recipes_.find(recipe_id);
    if (it == recipes_.end()) {
        throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, 
                                              "Recipe not found: " + recipe_id, "RECIPE_NOT_FOUND"));
    }
    return it->second;
}

const ProcessRecipe& ProcessIntegrator::getRecipe(const std::string& recipe_id) const {
    auto it = recipes_.find(recipe_id);
    if (it == recipes_.end()) {
        throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, 
                                              "Recipe not found: " + recipe_id, "RECIPE_NOT_FOUND"));
    }
    return it->second;
}

void ProcessIntegrator::initializeIntegrator() {
    // Initialize integration engines
    multi_layer_engine_ = std::make_unique<MultiLayerEngine>();
    temperature_controller_ = std::make_unique<AdvancedTemperatureController>();
    process_optimizer_ = std::make_unique<ProcessOptimizer>();
}

std::string ProcessIntegrator::flowTypeToString(ProcessFlowType type) const {
    switch (type) {
        case ProcessFlowType::SEQUENTIAL: return "Sequential";
        case ProcessFlowType::PARALLEL: return "Parallel";
        case ProcessFlowType::CONDITIONAL: return "Conditional";
        case ProcessFlowType::ITERATIVE: return "Iterative";
        case ProcessFlowType::FEEDBACK_CONTROLLED: return "Feedback Controlled";
        case ProcessFlowType::ADAPTIVE: return "Adaptive";
        case ProcessFlowType::HIERARCHICAL: return "Hierarchical";
        case ProcessFlowType::PIPELINE: return "Pipeline";
        default: return "Unknown";
    }
}

std::string ProcessIntegrator::statusToString(ProcessStatus status) const {
    switch (status) {
        case ProcessStatus::PENDING: return "Pending";
        case ProcessStatus::READY: return "Ready";
        case ProcessStatus::RUNNING: return "Running";
        case ProcessStatus::COMPLETED: return "Completed";
        case ProcessStatus::FAILED: return "Failed";
        case ProcessStatus::SKIPPED: return "Skipped";
        case ProcessStatus::PAUSED: return "Paused";
        case ProcessStatus::CANCELLED: return "Cancelled";
        default: return "Unknown";
    }
}

std::unordered_map<std::string, double> ProcessIntegrator::analyzeProcessQuality(
    const ProcessIntegrationResults& results) const {

    std::unordered_map<std::string, double> quality;

    if (results.executed_steps.empty()) {
        return quality;
    }

    // Calculate completion rate
    int completed_steps = 0;
    int total_steps = static_cast<int>(results.executed_steps.size());

    for (const auto& step : results.executed_steps) {
        if (step.status == ProcessStatus::COMPLETED) {
            completed_steps++;
        }
    }

    quality["completion_rate"] = static_cast<double>(completed_steps) / total_steps * 100.0;

    // Calculate timing efficiency
    double total_estimated = 0.0;
    double total_actual = 0.0;

    for (const auto& step : results.executed_steps) {
        total_estimated += step.estimated_time;
        total_actual += step.actual_time;
    }

    quality["timing_efficiency"] = (total_actual > 0) ? (total_estimated / total_actual) * 100.0 : 100.0;

    // Calculate thermal budget efficiency
    double budget_efficiency = 100.0;
    if (results.thermal_budget_used > 0) {
        // Assume a reasonable thermal budget limit for calculation
        double budget_limit = 10000.0; // Default limit
        budget_efficiency = std::max(0.0, (1.0 - results.thermal_budget_used / budget_limit) * 100.0);
    }
    quality["thermal_efficiency"] = budget_efficiency;

    // Overall quality score
    quality["overall_quality"] = (quality["completion_rate"] + quality["timing_efficiency"] + quality["thermal_efficiency"]) / 3.0;

    return quality;
}

std::vector<std::string> ProcessIntegrator::generateProcessRecommendations(
    const ProcessIntegrationResults& results) const {

    std::vector<std::string> recommendations;

    if (results.overall_status == ProcessStatus::COMPLETED) {
        recommendations.push_back("Process recipe executed successfully");
    } else if (results.overall_status == ProcessStatus::FAILED) {
        recommendations.push_back("Process recipe failed - review failed steps");
    }

    // Analyze timing
    if (results.total_execution_time > 0) {
        recommendations.push_back("Total execution time: " + std::to_string(results.total_execution_time) + " minutes");

        if (results.total_execution_time > 60.0) {
            recommendations.push_back("Consider optimizing process sequence to reduce total time");
        }
    }

    // Analyze thermal budget
    if (results.thermal_budget_used > 5000.0) {
        recommendations.push_back("High thermal budget usage - consider lower temperature processes");
    }

    // Quality-based recommendations
    auto quality = analyzeProcessQuality(results);
    auto overall_it = quality.find("overall_quality");
    if (overall_it != quality.end()) {
        if (overall_it->second > 90.0) {
            recommendations.push_back("Excellent process quality achieved");
        } else if (overall_it->second < 70.0) {
            recommendations.push_back("Process quality below target - review parameters");
        }
    }

    return recommendations;
}

bool ProcessIntegrator::attemptProcessRecovery(
    ProcessExecutionContext& context,
    ProcessStep& failed_step) {

    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Attempting recovery for failed step: " + failed_step.step_id,
                      "ProcessIntegrator");

    // Simple recovery strategy: retry once with modified parameters
    if (failed_step.parameters.find("retry_count") == failed_step.parameters.end()) {
        failed_step.parameters["retry_count"] = 1.0;

        // Modify parameters slightly for retry
        for (auto& param : failed_step.parameters) {
            if (param.first != "retry_count") {
                param.second *= 0.95; // Reduce by 5%
            }
        }

        // Retry execution
        failed_step.status = ProcessStatus::READY;
        bool retry_success = executeProcessStep(context, failed_step);

        if (retry_success) {
            SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                              "Recovery successful for step: " + failed_step.step_id,
                              "ProcessIntegrator");
            return true;
        }
    }

    SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::ADVANCED,
                      "Recovery failed for step: " + failed_step.step_id,
                      "ProcessIntegrator");
    return false;
}

bool ProcessIntegrator::validateRecipe(const std::string& recipe_id, std::string& error_message) const {
    auto recipe_it = recipes_.find(recipe_id);
    if (recipe_it == recipes_.end()) {
        error_message = "Recipe not found: " + recipe_id;
        return false;
    }

    const auto& recipe = recipe_it->second;

    // Check if recipe has steps
    if (recipe.steps.empty()) {
        error_message = "Recipe has no process steps";
        return false;
    }

    // Check for valid step IDs
    for (const auto& step : recipe.steps) {
        if (step.step_id.empty()) {
            error_message = "Step has empty ID";
            return false;
        }

        if (step.process_type.empty()) {
            error_message = "Step has empty process type: " + step.step_id;
            return false;
        }

        if (step.estimated_time <= 0.0) {
            error_message = "Step has invalid estimated time: " + step.step_id;
            return false;
        }
    }

    // Check for cyclic dependencies (simplified)
    if (hasCyclicDependencies(recipe)) {
        error_message = "Recipe contains cyclic dependencies";
        return false;
    }

    return true;
}

bool ProcessIntegrator::hasCyclicDependencies(const ProcessRecipe& recipe) const {
    // Simplified cycle detection
    std::unordered_map<std::string, std::vector<std::string>> dependency_graph;

    // Build dependency graph
    for (const auto& step : recipe.steps) {
        dependency_graph[step.step_id] = step.prerequisites;
    }

    // Simple cycle detection using DFS (simplified implementation)
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> recursion_stack;

    std::function<bool(const std::string&)> has_cycle = [&](const std::string& step_id) -> bool {
        if (recursion_stack.find(step_id) != recursion_stack.end()) {
            return true; // Cycle detected
        }

        if (visited.find(step_id) != visited.end()) {
            return false; // Already processed
        }

        visited.insert(step_id);
        recursion_stack.insert(step_id);

        auto it = dependency_graph.find(step_id);
        if (it != dependency_graph.end()) {
            for (const std::string& prereq : it->second) {
                if (has_cycle(prereq)) {
                    return true;
                }
            }
        }

        recursion_stack.erase(step_id);
        return false;
    };

    // Check each step for cycles
    for (const auto& step : recipe.steps) {
        if (visited.find(step.step_id) == visited.end()) {
            if (has_cycle(step.step_id)) {
                return true;
            }
        }
    }

    return false;
}

std::vector<std::string> ProcessIntegrator::getDiagnostics() const {
    std::vector<std::string> diagnostics;

    diagnostics.push_back("Process Integrator Status:");
    diagnostics.push_back("  Recipes: " + std::to_string(recipes_.size()));
    diagnostics.push_back("  Active Executions: " + std::to_string(active_executions_.size()));
    diagnostics.push_back("  Cross Dependencies: " + std::to_string(cross_dependencies_.size()));

    diagnostics.push_back("Configuration:");
    diagnostics.push_back("  Parallel Execution: " + std::string(enable_parallel_execution_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Dependency Checking: " + std::string(enable_dependency_checking_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Thermal Budget Tracking: " + std::string(enable_thermal_budget_tracking_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Adaptive Flow: " + std::string(enable_adaptive_flow_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Max Concurrent Processes: " + std::to_string(max_concurrent_processes_));

    // Recipe details
    for (const auto& recipe : recipes_) {
        diagnostics.push_back("Recipe: " + recipe.first);
        diagnostics.push_back("  Steps: " + std::to_string(recipe.second.steps.size()));
        diagnostics.push_back("  Flow Type: " + flowTypeToString(recipe.second.flow_type));
        diagnostics.push_back("  Estimated Time: " + std::to_string(recipe.second.total_estimated_time) + " minutes");
    }

    return diagnostics;
}

// Utility functions implementation
namespace ProcessIntegrationUtils {

    ProcessRecipe generateCMOSRecipe() {
        ProcessRecipe recipe;
        recipe.recipe_id = "cmos_basic";
        recipe.recipe_name = "Basic CMOS Process";
        recipe.description = "Standard CMOS fabrication sequence";
        recipe.flow_type = ProcessFlowType::SEQUENTIAL;

        // Gate oxidation
        ProcessStep gate_oxide;
        gate_oxide.step_id = "gate_oxidation";
        gate_oxide.process_type = "oxidation";
        gate_oxide.parameters["temperature"] = 1000.0;
        gate_oxide.parameters["time"] = 0.5;
        gate_oxide.parameters["ambient"] = 1.0;
        gate_oxide.estimated_time = 30.0;
        recipe.steps.push_back(gate_oxide);

        // Polysilicon deposition
        ProcessStep poly_dep;
        poly_dep.step_id = "poly_deposition";
        poly_dep.process_type = "deposition";
        poly_dep.parameters["material"] = 1.0; // Polysilicon
        poly_dep.parameters["thickness"] = 0.3;
        poly_dep.parameters["temperature"] = 620.0;
        poly_dep.estimated_time = 45.0;
        poly_dep.prerequisites.push_back("gate_oxidation");
        recipe.steps.push_back(poly_dep);

        // Source/Drain implantation
        ProcessStep sd_implant;
        sd_implant.step_id = "source_drain_implant";
        sd_implant.process_type = "doping";
        sd_implant.parameters["species"] = 0.0; // Phosphorus
        sd_implant.parameters["energy"] = 80.0;
        sd_implant.parameters["dose"] = 1e15;
        sd_implant.estimated_time = 15.0;
        sd_implant.prerequisites.push_back("poly_deposition");
        recipe.steps.push_back(sd_implant);

        // Metal contact
        ProcessStep metal_contact;
        metal_contact.step_id = "metal_contact";
        metal_contact.process_type = "deposition";
        metal_contact.parameters["material"] = 2.0; // Aluminum
        metal_contact.parameters["thickness"] = 0.8;
        metal_contact.parameters["temperature"] = 300.0;
        metal_contact.estimated_time = 30.0;
        metal_contact.prerequisites.push_back("source_drain_implant");
        recipe.steps.push_back(metal_contact);

        recipe.total_estimated_time = 120.0; // 2 hours

        return recipe;
    }

    double calculateRecipeComplexity(const ProcessRecipe& recipe) {
        double complexity = 0.0;

        // Base complexity from number of steps
        complexity += recipe.steps.size() * 10.0;

        // Add complexity for dependencies
        for (const auto& step : recipe.steps) {
            complexity += step.prerequisites.size() * 5.0;
        }

        // Add complexity for different process types
        std::unordered_set<std::string> unique_processes;
        for (const auto& step : recipe.steps) {
            unique_processes.insert(step.process_type);
        }
        complexity += unique_processes.size() * 15.0;

        return complexity;
    }

    double calculateProcessYield(const ProcessIntegrationResults& results) {
        if (results.executed_steps.empty()) {
            return 0.0;
        }

        int successful_steps = 0;
        for (const auto& step : results.executed_steps) {
            if (step.status == ProcessStatus::COMPLETED) {
                successful_steps++;
            }
        }

        return static_cast<double>(successful_steps) / results.executed_steps.size() * 100.0;
    }

} // namespace ProcessIntegrationUtils

} // namespace SemiPRO

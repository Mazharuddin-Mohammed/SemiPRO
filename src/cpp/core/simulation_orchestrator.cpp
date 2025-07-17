// Author: Dr. Mazharuddin Mohammed
#include "simulation_orchestrator.hpp"
#include "simulation_engine.hpp"
#include "input_parser.hpp"
#include "output_generator.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include <chrono>

namespace SemiPRO {

SimulationOrchestrator& SimulationOrchestrator::getInstance() {
    static SimulationOrchestrator instance;
    return instance;
}

void SimulationOrchestrator::loadConfiguration(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    try {
        config_ = std::make_unique<SimulationConfig>(config_file);
        
        // Load default directories
        if (config_->hasParameter("paths", "input_directory")) {
            input_directory_ = config_->getParameter("paths", "input_directory").as<std::string>();
        }
        if (config_->hasParameter("paths", "output_directory")) {
            output_directory_ = config_->getParameter("paths", "output_directory").as<std::string>();
        }
        
        // Load execution settings
        if (config_->hasParameter("execution", "mode")) {
            std::string mode = config_->getParameter("execution", "mode").as<std::string>();
            if (mode == "sequential") execution_mode_ = ExecutionMode::SEQUENTIAL;
            else if (mode == "parallel") execution_mode_ = ExecutionMode::PARALLEL;
            else if (mode == "pipeline") execution_mode_ = ExecutionMode::PIPELINE;
            else if (mode == "batch") execution_mode_ = ExecutionMode::BATCH;
        }
        
        if (config_->hasParameter("execution", "max_parallel_steps")) {
            max_parallel_steps_ = config_->getParameter("execution", "max_parallel_steps").as<int>();
        }
        
        // Initialize parsers
        input_parser_ = std::make_unique<InputParser>(input_directory_);
        output_generator_ = std::make_unique<OutputGenerator>(output_directory_);
        
    } catch (const std::exception& e) {
        notifyError("Configuration", "Failed to load configuration: " + std::string(e.what()));
        throw;
    }
}

void SimulationOrchestrator::saveConfiguration(const std::string& config_file) const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (config_) {
        config_->saveToFile(config_file);
    }
}

void SimulationOrchestrator::setParameter(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!config_) {
        config_ = std::make_unique<SimulationConfig>();
    }
    
    // Parse key as section.parameter
    size_t dot_pos = key.find('.');
    if (dot_pos != std::string::npos) {
        std::string section = key.substr(0, dot_pos);
        std::string param = key.substr(dot_pos + 1);
        config_->setParameter(section, param, YAML::Node(value));
    } else {
        config_->setParameter("general", key, YAML::Node(value));
    }
}

std::string SimulationOrchestrator::getParameter(const std::string& key) const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!config_) return "";
    
    size_t dot_pos = key.find('.');
    if (dot_pos != std::string::npos) {
        std::string section = key.substr(0, dot_pos);
        std::string param = key.substr(dot_pos + 1);
        if (config_->hasParameter(section, param)) {
            return config_->getParameter(section, param).as<std::string>();
        }
    } else {
        if (config_->hasParameter("general", key)) {
            return config_->getParameter("general", key).as<std::string>();
        }
    }
    
    return "";
}

void SimulationOrchestrator::loadSimulationFlow(const std::string& flow_file) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    try {
        YAML::Node flow_config = YAML::LoadFile(flow_file);
        
        SimulationFlow flow(flow_config["name"].as<std::string>());
        flow.description = flow_config["description"].as<std::string>("");
        
        // Load execution mode
        if (flow_config["execution_mode"]) {
            std::string mode = flow_config["execution_mode"].as<std::string>();
            if (mode == "sequential") flow.execution_mode = ExecutionMode::SEQUENTIAL;
            else if (mode == "parallel") flow.execution_mode = ExecutionMode::PARALLEL;
            else if (mode == "pipeline") flow.execution_mode = ExecutionMode::PIPELINE;
            else if (mode == "batch") flow.execution_mode = ExecutionMode::BATCH;
        }
        
        // Load global parameters
        if (flow_config["global_parameters"]) {
            for (const auto& param : flow_config["global_parameters"]) {
                flow.global_parameters[param.first.as<std::string>()] = param.second.as<std::string>();
            }
        }
        
        // Load process steps
        if (flow_config["steps"]) {
            for (const auto& step_node : flow_config["steps"]) {
                ProcessStepDefinition step(StepType::CUSTOM, step_node["name"].as<std::string>());
                
                // Parse step type
                std::string type_str = step_node["type"].as<std::string>();
                if (type_str == "oxidation") step.type = StepType::OXIDATION;
                else if (type_str == "doping") step.type = StepType::DOPING;
                else if (type_str == "lithography") step.type = StepType::LITHOGRAPHY;
                else if (type_str == "deposition") step.type = StepType::DEPOSITION;
                else if (type_str == "etching") step.type = StepType::ETCHING;
                else if (type_str == "metallization") step.type = StepType::METALLIZATION;
                else if (type_str == "annealing") step.type = StepType::ANNEALING;
                else if (type_str == "cmp") step.type = StepType::CMP;
                else if (type_str == "inspection") step.type = StepType::INSPECTION;
                
                // Load parameters
                if (step_node["parameters"]) {
                    for (const auto& param : step_node["parameters"]) {
                        step.parameters[param.first.as<std::string>()] = param.second.as<double>();
                    }
                }
                
                // Load other properties
                if (step_node["input_files"]) {
                    for (const auto& file : step_node["input_files"]) {
                        step.input_files.push_back(file.as<std::string>());
                    }
                }
                
                if (step_node["output_files"]) {
                    for (const auto& file : step_node["output_files"]) {
                        step.output_files.push_back(file.as<std::string>());
                    }
                }
                
                if (step_node["dependencies"]) {
                    for (const auto& dep : step_node["dependencies"]) {
                        step.dependencies.push_back(dep.as<std::string>());
                    }
                }
                
                step.estimated_duration = step_node["estimated_duration"].as<double>(0.0);
                step.priority = step_node["priority"].as<int>(0);
                step.parallel_compatible = step_node["parallel_compatible"].as<bool>(false);
                
                flow.steps.push_back(step);
            }
        }
        
        flows_[flow.name] = flow;
        current_flow_ = flow;
        
    } catch (const std::exception& e) {
        notifyError("Flow Loading", "Failed to load simulation flow: " + std::string(e.what()));
        throw;
    }
}

void SimulationOrchestrator::saveSimulationFlow(const std::string& flow_file) const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    try {
        YAML::Node flow_config;
        
        flow_config["name"] = current_flow_.name;
        flow_config["description"] = current_flow_.description;
        
        // Save execution mode
        switch (current_flow_.execution_mode) {
            case ExecutionMode::SEQUENTIAL: flow_config["execution_mode"] = "sequential"; break;
            case ExecutionMode::PARALLEL: flow_config["execution_mode"] = "parallel"; break;
            case ExecutionMode::PIPELINE: flow_config["execution_mode"] = "pipeline"; break;
            case ExecutionMode::BATCH: flow_config["execution_mode"] = "batch"; break;
        }
        
        // Save global parameters
        for (const auto& param : current_flow_.global_parameters) {
            flow_config["global_parameters"][param.first] = param.second;
        }
        
        // Save steps
        for (const auto& step : current_flow_.steps) {
            YAML::Node step_node;
            step_node["name"] = step.name;
            
            // Convert step type to string
            switch (step.type) {
                case StepType::OXIDATION: step_node["type"] = "oxidation"; break;
                case StepType::DOPING: step_node["type"] = "doping"; break;
                case StepType::LITHOGRAPHY: step_node["type"] = "lithography"; break;
                case StepType::DEPOSITION: step_node["type"] = "deposition"; break;
                case StepType::ETCHING: step_node["type"] = "etching"; break;
                case StepType::METALLIZATION: step_node["type"] = "metallization"; break;
                case StepType::ANNEALING: step_node["type"] = "annealing"; break;
                case StepType::CMP: step_node["type"] = "cmp"; break;
                case StepType::INSPECTION: step_node["type"] = "inspection"; break;
                case StepType::CUSTOM: step_node["type"] = "custom"; break;
            }
            
            // Save parameters
            for (const auto& param : step.parameters) {
                step_node["parameters"][param.first] = param.second;
            }
            
            // Save file lists
            for (const auto& file : step.input_files) {
                step_node["input_files"].push_back(file);
            }
            for (const auto& file : step.output_files) {
                step_node["output_files"].push_back(file);
            }
            for (const auto& dep : step.dependencies) {
                step_node["dependencies"].push_back(dep);
            }
            
            step_node["estimated_duration"] = step.estimated_duration;
            step_node["priority"] = step.priority;
            step_node["parallel_compatible"] = step.parallel_compatible;
            
            flow_config["steps"].push_back(step_node);
        }
        
        std::ofstream file(flow_file);
        file << flow_config;
        
    } catch (const std::exception& e) {
        notifyError("Flow Saving", "Failed to save simulation flow: " + std::string(e.what()));
        throw;
    }
}

void SimulationOrchestrator::addProcessStep(const ProcessStepDefinition& step) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    current_flow_.steps.push_back(step);
}

void SimulationOrchestrator::removeProcessStep(const std::string& step_name) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    auto it = std::remove_if(current_flow_.steps.begin(), current_flow_.steps.end(),
        [&step_name](const ProcessStepDefinition& step) {
            return step.name == step_name;
        });
    
    current_flow_.steps.erase(it, current_flow_.steps.end());
}

void SimulationOrchestrator::reorderSteps(const std::vector<std::string>& step_order) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    std::vector<ProcessStepDefinition> reordered_steps;
    
    for (const auto& step_name : step_order) {
        auto it = std::find_if(current_flow_.steps.begin(), current_flow_.steps.end(),
            [&step_name](const ProcessStepDefinition& step) {
                return step.name == step_name;
            });
        
        if (it != current_flow_.steps.end()) {
            reordered_steps.push_back(*it);
        }
    }
    
    current_flow_.steps = reordered_steps;
}

std::future<bool> SimulationOrchestrator::executeSimulation(const std::string& wafer_name) {
    return std::async(std::launch::async, [this, wafer_name]() {
        std::lock_guard<std::mutex> lock(state_mutex_);

        try {
            current_state_ = SimulationState::INITIALIZING;
            progress_.state = SimulationState::INITIALIZING;
            progress_.current_step = 0;
            progress_.total_steps = current_flow_.steps.size();
            progress_.progress_percentage = 0.0;
            progress_.start_time = std::chrono::system_clock::now();
            progress_.completed_steps.clear();
            progress_.errors.clear();

            execution_start_time_ = std::chrono::system_clock::now();

            notifyProgress();

            current_state_ = SimulationState::RUNNING;
            progress_.state = SimulationState::RUNNING;

            bool success = true;

            switch (execution_mode_) {
                case ExecutionMode::SEQUENTIAL:
                    success = executeSequentialFlow(wafer_name);
                    break;
                case ExecutionMode::PARALLEL:
                    success = executeParallelFlow(wafer_name);
                    break;
                case ExecutionMode::PIPELINE:
                    success = executePipelineFlow(wafer_name);
                    break;
                case ExecutionMode::BATCH:
                    success = executeBatchFlow(wafer_name);
                    break;
            }

            if (success && !should_cancel_) {
                current_state_ = SimulationState::COMPLETED;
                progress_.state = SimulationState::COMPLETED;
                progress_.progress_percentage = 100.0;
            } else if (should_cancel_) {
                current_state_ = SimulationState::CANCELLED;
                progress_.state = SimulationState::CANCELLED;
            } else {
                current_state_ = SimulationState::ERROR;
                progress_.state = SimulationState::ERROR;
            }

            updateStatistics();
            notifyProgress();

            return success;

        } catch (const std::exception& e) {
            current_state_ = SimulationState::ERROR;
            progress_.state = SimulationState::ERROR;
            notifyError("Simulation", "Execution failed: " + std::string(e.what()));
            return false;
        }
    });
}

std::future<bool> SimulationOrchestrator::executeSimulationFlow(const std::string& flow_name,
                                                               const std::string& wafer_name) {
    return std::async(std::launch::async, [this, flow_name, wafer_name]() {
        std::lock_guard<std::mutex> lock(state_mutex_);

        auto it = flows_.find(flow_name);
        if (it == flows_.end()) {
            notifyError("Flow Execution", "Flow not found: " + flow_name);
            return false;
        }

        current_flow_ = it->second;
        return executeSimulation(wafer_name).get();
    });
}

void SimulationOrchestrator::pauseSimulation() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (current_state_ == SimulationState::RUNNING) {
        should_pause_ = true;
        current_state_ = SimulationState::PAUSED;
        progress_.state = SimulationState::PAUSED;
        notifyProgress();
    }
}

void SimulationOrchestrator::resumeSimulation() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    if (current_state_ == SimulationState::PAUSED) {
        should_pause_ = false;
        current_state_ = SimulationState::RUNNING;
        progress_.state = SimulationState::RUNNING;
        state_cv_.notify_all();
        notifyProgress();
    }
}

void SimulationOrchestrator::cancelSimulation() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    should_cancel_ = true;
    should_pause_ = false;
    state_cv_.notify_all();
}

void SimulationOrchestrator::resetSimulation() {
    std::lock_guard<std::mutex> lock(state_mutex_);

    current_state_ = SimulationState::IDLE;
    should_pause_ = false;
    should_cancel_ = false;

    progress_ = SimulationProgress();
    stats_ = ExecutionStatistics();
}

void SimulationOrchestrator::addWaferToBatch(const std::string& wafer_name, const std::string& flow_name) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    batch_queue_.emplace_back(wafer_name, flow_name);
}

std::future<std::vector<bool>> SimulationOrchestrator::executeBatch() {
    return std::async(std::launch::async, [this]() {
        std::vector<bool> results;

        for (const auto& batch_item : batch_queue_) {
            const std::string& wafer_name = batch_item.first;
            const std::string& flow_name = batch_item.second;

            auto future = executeSimulationFlow(flow_name, wafer_name);
            results.push_back(future.get());

            if (should_cancel_) break;
        }

        return results;
    });
}

void SimulationOrchestrator::clearBatch() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    batch_queue_.clear();
}

SimulationOrchestrator::SimulationProgress SimulationOrchestrator::getProgress() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return progress_;
}

std::vector<std::string> SimulationOrchestrator::getAvailableFlows() const {
    std::lock_guard<std::mutex> lock(state_mutex_);

    std::vector<std::string> flow_names;
    for (const auto& flow : flows_) {
        flow_names.push_back(flow.first);
    }
    return flow_names;
}

std::vector<SimulationOrchestrator::ProcessStepDefinition> SimulationOrchestrator::getCurrentFlow() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return current_flow_.steps;
}

bool SimulationOrchestrator::isRunning() const {
    return current_state_ == SimulationState::RUNNING;
}

bool SimulationOrchestrator::isPaused() const {
    return current_state_ == SimulationState::PAUSED;
}

// Missing execution flow methods implementation
bool SimulationOrchestrator::executeSequentialFlow(const std::string& wafer_name) {
    for (size_t i = 0; i < current_flow_.steps.size(); ++i) {
        if (should_cancel_) return false;

        // Handle pause
        if (should_pause_) {
            std::unique_lock<std::mutex> lock(state_mutex_);
            state_cv_.wait(lock, [this] { return !should_pause_ || should_cancel_; });
            if (should_cancel_) return false;
        }

        const auto& step = current_flow_.steps[i];
        bool success = executeStep(step, wafer_name);

        if (!success) {
            notifyError(step.name, "Step execution failed");
            return false;
        }

        progress_.current_step = i + 1;
        progress_.progress_percentage = (double)(i + 1) / current_flow_.steps.size() * 100.0;
        progress_.completed_steps.push_back(step.name);
        notifyStepCompleted(step.name, true);
        updateProgress();
        notifyProgress();
    }
    return true;
}

bool SimulationOrchestrator::executeParallelFlow(const std::string& wafer_name) {
    // Group steps that can be executed in parallel
    auto parallel_groups = groupParallelSteps(current_flow_.steps);

    size_t completed_steps = 0;
    for (const auto& group : parallel_groups) {
        if (should_cancel_) return false;

        // Handle pause
        if (should_pause_) {
            std::unique_lock<std::mutex> lock(state_mutex_);
            state_cv_.wait(lock, [this] { return !should_pause_ || should_cancel_; });
            if (should_cancel_) return false;
        }

        // Execute steps in parallel
        auto futures = executeStepsParallel(group, wafer_name);

        // Wait for all steps in group to complete
        bool group_success = true;
        for (size_t i = 0; i < futures.size(); ++i) {
            bool step_success = futures[i].get();
            if (!step_success) {
                notifyError(group[i].name, "Parallel step execution failed");
                group_success = false;
            } else {
                progress_.completed_steps.push_back(group[i].name);
                notifyStepCompleted(group[i].name, true);
            }
        }

        if (!group_success) return false;

        completed_steps += group.size();
        progress_.current_step = completed_steps;
        progress_.progress_percentage = (double)completed_steps / current_flow_.steps.size() * 100.0;
        updateProgress();
        notifyProgress();
    }
    return true;
}

bool SimulationOrchestrator::executePipelineFlow(const std::string& wafer_name) {
    // Simplified pipeline implementation - execute with overlap
    std::vector<std::future<bool>> active_futures;
    std::vector<ProcessStepDefinition> active_steps;

    size_t step_index = 0;
    size_t completed_steps = 0;

    while (step_index < current_flow_.steps.size() || !active_futures.empty()) {
        if (should_cancel_) return false;

        // Handle pause
        if (should_pause_) {
            std::unique_lock<std::mutex> lock(state_mutex_);
            state_cv_.wait(lock, [this] { return !should_pause_ || should_cancel_; });
            if (should_cancel_) return false;
        }

        // Start new steps if we have capacity and steps remaining
        while (active_futures.size() < max_parallel_steps_ && step_index < current_flow_.steps.size()) {
            const auto& step = current_flow_.steps[step_index];

            // Check dependencies
            bool dependencies_met = true;
            for (const auto& dep : step.dependencies) {
                if (std::find(progress_.completed_steps.begin(), progress_.completed_steps.end(), dep)
                    == progress_.completed_steps.end()) {
                    dependencies_met = false;
                    break;
                }
            }

            if (dependencies_met) {
                auto future = std::async(std::launch::async, [this, step, wafer_name]() {
                    return executeStep(step, wafer_name);
                });
                active_futures.push_back(std::move(future));
                active_steps.push_back(step);
                step_index++;
            } else {
                break; // Wait for dependencies
            }
        }

        // Check for completed steps
        for (size_t i = 0; i < active_futures.size(); ) {
            if (active_futures[i].wait_for(std::chrono::milliseconds(10)) == std::future_status::ready) {
                bool success = active_futures[i].get();
                const auto& step = active_steps[i];

                if (!success) {
                    notifyError(step.name, "Pipeline step execution failed");
                    return false;
                }

                progress_.completed_steps.push_back(step.name);
                notifyStepCompleted(step.name, true);
                completed_steps++;

                // Remove completed future and step
                active_futures.erase(active_futures.begin() + i);
                active_steps.erase(active_steps.begin() + i);

                progress_.current_step = completed_steps;
                progress_.progress_percentage = (double)completed_steps / current_flow_.steps.size() * 100.0;
                updateProgress();
                notifyProgress();
            } else {
                i++;
            }
        }

        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return true;
}

bool SimulationOrchestrator::executeBatchFlow(const std::string& wafer_name) {
    // For single wafer, batch flow is same as sequential
    return executeSequentialFlow(wafer_name);
}

bool SimulationOrchestrator::executeStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    try {
        switch (step.type) {
            case StepType::OXIDATION:
                return executeOxidationStep(step, wafer_name);
            case StepType::DOPING:
                return executeDopingStep(step, wafer_name);
            case StepType::LITHOGRAPHY:
                return executeLithographyStep(step, wafer_name);
            case StepType::DEPOSITION:
                return executeDepositionStep(step, wafer_name);
            case StepType::ETCHING:
                return executeEtchingStep(step, wafer_name);
            case StepType::METALLIZATION:
                return executeMetallizationStep(step, wafer_name);
            case StepType::ANNEALING:
                return executeAnnealingStep(step, wafer_name);
            case StepType::CMP:
                return executeCMPStep(step, wafer_name);
            case StepType::INSPECTION:
                return executeInspectionStep(step, wafer_name);
            case StepType::CUSTOM:
                return executeCustomStep(step, wafer_name);
            default:
                notifyError(step.name, "Unknown step type");
                return false;
        }
    } catch (const std::exception& e) {
        notifyError(step.name, "Step execution exception: " + std::string(e.what()));
        return false;
    }
}

// Individual step execution methods
bool SimulationOrchestrator::executeOxidationStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    auto& engine = SimulationEngine::getInstance();

    ProcessParameters params;
    params.operation = "oxidation";
    params.duration = step.estimated_duration;
    params.priority = step.priority;

    // Extract parameters
    auto temp_it = step.parameters.find("temperature");
    auto time_it = step.parameters.find("time");
    auto atmosphere_it = step.parameters.find("atmosphere");

    if (temp_it != step.parameters.end()) {
        params.parameters["temperature"] = temp_it->second;
    }
    if (time_it != step.parameters.end()) {
        params.parameters["time"] = time_it->second;
    }
    if (atmosphere_it != step.parameters.end()) {
        params.string_parameters["atmosphere"] = std::to_string(atmosphere_it->second);
    }

    auto future = engine.simulateProcessAsync(wafer_name, params);
    return future.get();
}

bool SimulationOrchestrator::executeDopingStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    auto& engine = SimulationEngine::getInstance();

    ProcessParameters params;
    params.operation = "doping";
    params.duration = step.estimated_duration;
    params.priority = step.priority;

    // Extract parameters
    auto energy_it = step.parameters.find("energy");
    auto dose_it = step.parameters.find("dose");
    auto species_it = step.parameters.find("species");

    if (energy_it != step.parameters.end()) {
        params.parameters["energy"] = energy_it->second;
    }
    if (dose_it != step.parameters.end()) {
        params.parameters["dose"] = dose_it->second;
    }
    if (species_it != step.parameters.end()) {
        params.string_parameters["species"] = std::to_string(species_it->second);
    }

    auto future = engine.simulateProcessAsync(wafer_name, params);
    return future.get();
}

bool SimulationOrchestrator::executeLithographyStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    // Lithography is not directly supported by simulation engine, implement basic version
    auto wavelength_it = step.parameters.find("wavelength");
    auto na_it = step.parameters.find("numerical_aperture");

    if (wavelength_it == step.parameters.end() || na_it == step.parameters.end()) {
        notifyError(step.name, "Missing required lithography parameters");
        return false;
    }

    // For now, just log the operation
    std::string message = "Lithography step executed: wavelength=" +
                         std::to_string(wavelength_it->second) + "nm, NA=" +
                         std::to_string(na_it->second);

    // In a real implementation, this would call the lithography module
    return true;
}

bool SimulationOrchestrator::executeDepositionStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    auto& engine = SimulationEngine::getInstance();

    ProcessParameters params;
    params.operation = "deposition";
    params.duration = step.estimated_duration;
    params.priority = step.priority;

    // Extract parameters
    auto thickness_it = step.parameters.find("thickness");
    auto temperature_it = step.parameters.find("temperature");
    auto material_it = step.parameters.find("material");

    if (thickness_it != step.parameters.end()) {
        params.parameters["thickness"] = thickness_it->second;
    }
    if (temperature_it != step.parameters.end()) {
        params.parameters["temperature"] = temperature_it->second;
    }
    if (material_it != step.parameters.end()) {
        params.string_parameters["material"] = std::to_string(material_it->second);
    }

    auto future = engine.simulateProcessAsync(wafer_name, params);
    return future.get();
}

bool SimulationOrchestrator::executeEtchingStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    auto& engine = SimulationEngine::getInstance();

    ProcessParameters params;
    params.operation = "etching";
    params.duration = step.estimated_duration;
    params.priority = step.priority;

    // Extract parameters
    auto depth_it = step.parameters.find("depth");
    auto rate_it = step.parameters.find("rate");
    auto selectivity_it = step.parameters.find("selectivity");

    if (depth_it != step.parameters.end()) {
        params.parameters["depth"] = depth_it->second;
    }
    if (rate_it != step.parameters.end()) {
        params.parameters["rate"] = rate_it->second;
    }
    if (selectivity_it != step.parameters.end()) {
        params.parameters["selectivity"] = selectivity_it->second;
    }

    auto future = engine.simulateProcessAsync(wafer_name, params);
    return future.get();
}

bool SimulationOrchestrator::executeMetallizationStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    // Metallization is not directly supported by simulation engine, implement basic version
    auto thickness_it = step.parameters.find("thickness");
    auto metal_it = step.parameters.find("metal");

    if (thickness_it == step.parameters.end() || metal_it == step.parameters.end()) {
        notifyError(step.name, "Missing required metallization parameters");
        return false;
    }

    // For now, just log the operation
    std::string message = "Metallization step executed: thickness=" +
                         std::to_string(thickness_it->second) + "μm, metal=" +
                         std::to_string(metal_it->second);

    // In a real implementation, this would call the metallization module
    return true;
}

bool SimulationOrchestrator::executeAnnealingStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    // Annealing is not directly supported by simulation engine, implement basic version
    auto temperature_it = step.parameters.find("temperature");
    auto time_it = step.parameters.find("time");

    if (temperature_it == step.parameters.end() || time_it == step.parameters.end()) {
        notifyError(step.name, "Missing required annealing parameters");
        return false;
    }

    // For now, just log the operation
    std::string message = "Annealing step executed: temperature=" +
                         std::to_string(temperature_it->second) + "°C, time=" +
                         std::to_string(time_it->second) + "min";

    // In a real implementation, this would call the thermal module
    return true;
}

bool SimulationOrchestrator::executeCMPStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    // CMP is not directly supported by simulation engine, implement basic version
    auto pressure_it = step.parameters.find("pressure");
    auto time_it = step.parameters.find("time");

    if (pressure_it == step.parameters.end() || time_it == step.parameters.end()) {
        notifyError(step.name, "Missing required CMP parameters");
        return false;
    }

    // For now, just log the operation
    std::string message = "CMP step executed: pressure=" +
                         std::to_string(pressure_it->second) + "psi, time=" +
                         std::to_string(time_it->second) + "min";

    // In a real implementation, this would call the CMP module
    return true;
}

bool SimulationOrchestrator::executeInspectionStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    // Inspection step - basic implementation
    std::string message = "Inspection step executed: " + step.name;

    // In a real implementation, this would call inspection/metrology modules
    return true;
}

bool SimulationOrchestrator::executeCustomStep(const ProcessStepDefinition& step, const std::string& wafer_name) {
    // Custom step - basic implementation
    std::string message = "Custom step executed: " + step.name;

    // In a real implementation, this would call custom user-defined modules
    return true;
}

std::vector<std::future<bool>> SimulationOrchestrator::executeStepsParallel(
    const std::vector<ProcessStepDefinition>& steps, const std::string& wafer_name) {

    std::vector<std::future<bool>> futures;

    for (const auto& step : steps) {
        auto future = std::async(std::launch::async, [this, step, wafer_name]() {
            return executeStep(step, wafer_name);
        });
        futures.push_back(std::move(future));
    }

    return futures;
}

bool SimulationOrchestrator::resolveDependencies(const std::vector<ProcessStepDefinition>& steps) const {
    // Check if all dependencies can be resolved
    std::set<std::string> available_steps;

    for (const auto& step : steps) {
        available_steps.insert(step.name);
    }

    for (const auto& step : steps) {
        for (const auto& dep : step.dependencies) {
            if (available_steps.find(dep) == available_steps.end()) {
                return false; // Dependency not found
            }
        }
    }

    return true;
}

std::vector<std::vector<SimulationOrchestrator::ProcessStepDefinition>>
SimulationOrchestrator::groupParallelSteps(const std::vector<ProcessStepDefinition>& steps) const {

    std::vector<std::vector<ProcessStepDefinition>> groups;
    std::set<std::string> completed_steps;
    std::vector<ProcessStepDefinition> remaining_steps = steps;

    while (!remaining_steps.empty()) {
        std::vector<ProcessStepDefinition> current_group;

        // Find steps that can be executed in parallel (no dependencies or dependencies met)
        auto it = remaining_steps.begin();
        while (it != remaining_steps.end()) {
            bool dependencies_met = true;

            for (const auto& dep : it->dependencies) {
                if (completed_steps.find(dep) == completed_steps.end()) {
                    dependencies_met = false;
                    break;
                }
            }

            if (dependencies_met && it->parallel_compatible &&
                current_group.size() < max_parallel_steps_) {
                current_group.push_back(*it);
                completed_steps.insert(it->name);
                it = remaining_steps.erase(it);
            } else if (dependencies_met && current_group.empty()) {
                // If no parallel group started, add this step anyway
                current_group.push_back(*it);
                completed_steps.insert(it->name);
                it = remaining_steps.erase(it);
                break; // Only one non-parallel step per group
            } else {
                ++it;
            }
        }

        if (!current_group.empty()) {
            groups.push_back(current_group);
        } else {
            // Deadlock - circular dependencies or other issue
            break;
        }
    }

    return groups;
}

void SimulationOrchestrator::updateProgress() {
    auto current_time = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        current_time - progress_.start_time).count();

    progress_.elapsed_time = elapsed;

    if (progress_.current_step > 0 && progress_.total_steps > 0) {
        double steps_per_second = (double)progress_.current_step / elapsed;
        double remaining_steps = progress_.total_steps - progress_.current_step;
        progress_.estimated_remaining_time = remaining_steps / steps_per_second;
    }
}

void SimulationOrchestrator::notifyProgress() {
    // Notify progress callbacks if any are registered
    for (auto& callback : progress_callbacks_) {
        if (callback) {
            callback(progress_);
        }
    }
}

void SimulationOrchestrator::notifyStepCompleted(const std::string& step_name, bool success) {
    // Notify step completion callbacks if any are registered
    for (auto& callback : step_completion_callbacks_) {
        if (callback) {
            callback(step_name, success);
        }
    }
}

void SimulationOrchestrator::notifyError(const std::string& step_name, const std::string& error_message) {
    progress_.errors.push_back(step_name + ": " + error_message);

    // Notify error callbacks if any are registered
    for (auto& callback : error_callbacks_) {
        if (callback) {
            callback(step_name, error_message);
        }
    }
}

void SimulationOrchestrator::updateStatistics() {
    auto current_time = std::chrono::system_clock::now();
    auto total_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        current_time - execution_start_time_).count();

    stats_.total_execution_time = total_elapsed;
    stats_.total_steps_executed = progress_.current_step;
    stats_.successful_steps = progress_.completed_steps.size();
    stats_.failed_steps = progress_.errors.size();

    if (stats_.total_steps_executed > 0) {
        stats_.average_step_time = (double)total_elapsed / stats_.total_steps_executed;
    }
}

} // namespace SemiPRO

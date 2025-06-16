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

} // namespace SemiPRO

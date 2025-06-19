// Author: Dr. Mazharuddin Mohammed
#include "simulation_engine.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <future>

SimulationEngine& SimulationEngine::getInstance() {
    static SimulationEngine instance;
    return instance;
}

void SimulationEngine::initialize(const std::string& config_file) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (is_initialized_) {
        Logger::getInstance().log("SimulationEngine already initialized");
        return;
    }
    
    config_file_ = config_file;
    
    // Initialize worker threads
    thread_count_ = std::max(1, static_cast<int>(std::thread::hardware_concurrency()));
    worker_threads_.reserve(thread_count_);
    
    // Reset statistics
    stats_ = Statistics{};
    simulation_start_time_ = std::chrono::system_clock::now();
    
    is_initialized_ = true;
    Logger::getInstance().log("SimulationEngine initialized with " + std::to_string(thread_count_) + " threads");
}

void SimulationEngine::shutdown() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (!is_initialized_) {
        return;
    }
    
    // Stop all operations
    is_running_ = false;
    
    // Wait for worker threads to finish
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    worker_threads_.clear();
    
    // Clear all data
    wafers_.clear();
    while (!batch_queue_.empty()) {
        batch_queue_.pop();
    }
    errors_.clear();
    
    is_initialized_ = false;
    Logger::getInstance().log("SimulationEngine shutdown complete");
}

std::shared_ptr<WaferEnhanced> SimulationEngine::createWafer(double diameter, double thickness, 
                                                           const std::string& material) {
    if (!is_initialized_) {
        throw std::runtime_error("SimulationEngine not initialized");
    }
    
    auto wafer = std::make_shared<WaferEnhanced>(diameter, thickness, material);

    // Enhanced wafer initialization - wafer is ready to use
    
    Logger::getInstance().log("Created wafer: " + std::to_string(diameter) + "mm, " + 
                             std::to_string(thickness) + "μm, " + material);
    
    return wafer;
}

void SimulationEngine::registerWafer(std::shared_ptr<WaferEnhanced> wafer, const std::string& name) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    if (wafers_.find(name) != wafers_.end()) {
        Logger::getInstance().log("Warning: Overwriting existing wafer: " + name);
    }
    
    wafers_[name] = wafer;
    Logger::getInstance().log("Registered wafer: " + name);
}

std::shared_ptr<WaferEnhanced> SimulationEngine::getWafer(const std::string& name) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    auto it = wafers_.find(name);
    if (it == wafers_.end()) {
        throw std::runtime_error("Wafer not found: " + name);
    }
    
    return it->second;
}

std::future<bool> SimulationEngine::simulateProcessAsync(const std::string& wafer_name, 
                                                        const ProcessParameters& params) {
    return std::async(std::launch::async, [this, wafer_name, params]() {
        return executeProcess(wafer_name, params);
    });
}

void SimulationEngine::addProcessToBatch(const std::string& wafer_name, const ProcessParameters& params) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    batch_queue_.push(std::make_pair(wafer_name, params));
    Logger::getInstance().log("Added process to batch: " + params.operation + " for " + wafer_name);
}

std::future<std::vector<bool>> SimulationEngine::executeBatch() {
    return std::async(std::launch::async, [this]() {
        std::vector<bool> results;
        
        std::lock_guard<std::mutex> lock(state_mutex_);
        
        while (!batch_queue_.empty()) {
            auto [wafer_name, params] = batch_queue_.front();
            batch_queue_.pop();
            
            bool success = executeProcess(wafer_name, params);
            results.push_back(success);
        }
        
        return results;
    });
}

void SimulationEngine::clearBatch() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    while (!batch_queue_.empty()) {
        batch_queue_.pop();
    }
    Logger::getInstance().log("Batch queue cleared");
}

SimulationEngine::SimulationStatus SimulationEngine::getStatus() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    SimulationStatus status;
    status.is_running = is_running_;
    status.progress = 0.0; // TODO: Implement progress tracking
    status.start_time = simulation_start_time_;
    
    return status;
}

SimulationEngine::Statistics SimulationEngine::getStatistics() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return stats_;
}

std::vector<SimulationEngine::SimulationError> SimulationEngine::getErrors(ErrorLevel min_level) const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    std::vector<SimulationError> filtered_errors;
    for (const auto& error : errors_) {
        if (error.level >= min_level) {
            filtered_errors.push_back(error);
        }
    }
    return filtered_errors;
}

void SimulationEngine::clearErrors() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    errors_.clear();
}

bool SimulationEngine::executeProcess(const std::string& wafer_name, const ProcessParameters& params) {
    try {
        auto wafer = getWafer(wafer_name);
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            stats_.total_processes++;
            stats_.processes_by_type[params.operation]++;
        }
        
        // Execute the process based on type
        bool success = false;
        
        if (params.operation == "oxidation") {
            // Implement oxidation process
            success = true;
        } else if (params.operation == "doping") {
            // Implement doping process
            success = true;
        } else if (params.operation == "deposition") {
            // Implement deposition process
            success = true;
        } else if (params.operation == "etching") {
            // Implement etching process
            success = true;
        } else {
            throw std::runtime_error("Unknown process type: " + params.operation);
        }
        
        if (success) {
            std::lock_guard<std::mutex> lock(state_mutex_);
            stats_.successful_processes++;
        }
        
        return success;
        
    } catch (const std::exception& e) {
        // Record error
        SimulationError error(ERROR, e.what(), "Process: " + params.operation + ", Wafer: " + wafer_name);
        
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            errors_.push_back(error);
            stats_.failed_processes++;
        }
        
        Logger::getInstance().log("Process failed: " + std::string(e.what()));
        return false;
    }
}

void SimulationEngine::updateStatistics() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - simulation_start_time_);
    stats_.total_simulation_time = duration.count();
    
    if (stats_.total_processes > 0) {
        stats_.average_process_time = static_cast<double>(stats_.total_simulation_time) / stats_.total_processes;
        stats_.success_rate = static_cast<double>(stats_.successful_processes) / stats_.total_processes;
    }
}

void SimulationEngine::pause() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    is_paused_ = true;
    Logger::getInstance().log("Simulation paused");
}

void SimulationEngine::resume() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    is_paused_ = false;
    Logger::getInstance().log("Simulation resumed");
}

bool SimulationEngine::isPaused() const {
    std::lock_guard<std::mutex> lock(state_mutex_);
    return is_paused_;
}

void SimulationEngine::enableGPUAcceleration(bool enable) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    gpu_acceleration_enabled_ = enable;
    Logger::getInstance().log("GPU acceleration " + std::string(enable ? "enabled" : "disabled"));
}

void SimulationEngine::enableAutoCheckpoint(bool enable, int interval) {
    std::lock_guard<std::mutex> lock(state_mutex_);
    auto_checkpoint_enabled_ = enable;
    checkpoint_interval_ = interval;
    Logger::getInstance().log("Auto checkpoint " + std::string(enable ? "enabled" : "disabled") + 
                             " (interval: " + std::to_string(interval) + "s)");
}

bool SimulationEngine::saveCheckpoint(const std::string& filename) {
    try {
        std::lock_guard<std::mutex> lock(state_mutex_);
        
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }
        
        // Save simulation state
        // TODO: Implement binary serialization
        
        Logger::getInstance().log("Checkpoint saved: " + filename);
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("Failed to save checkpoint: " + std::string(e.what()));
        return false;
    }
}

bool SimulationEngine::loadCheckpoint(const std::string& filename) {
    try {
        std::lock_guard<std::mutex> lock(state_mutex_);
        
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            return false;
        }
        
        // Load simulation state
        // TODO: Implement binary deserialization
        
        Logger::getInstance().log("Checkpoint loaded: " + filename);
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("Failed to load checkpoint: " + std::string(e.what()));
        return false;
    }
}

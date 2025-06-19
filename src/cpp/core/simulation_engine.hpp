// Author: Dr. Mazharuddin Mohammed
#ifndef SIMULATION_ENGINE_HPP
#define SIMULATION_ENGINE_HPP

#include "wafer_enhanced.hpp"
#include "profiler.hpp"
#include "simulation_orchestrator.hpp"
#include "input_parser.hpp"
#include "output_generator.hpp"
#include <future>
#include <queue>
#include <functional>
#include <memory>
#include <yaml-cpp/yaml.h>

class SimulationEngine {
public:
    static SimulationEngine& getInstance();
    
    // Simulation control
    void initialize(const std::string& config_file);
    void shutdown();
    
    // Wafer management
    std::shared_ptr<WaferEnhanced> createWafer(double diameter, double thickness, 
                                              const std::string& material);
    void registerWafer(std::shared_ptr<WaferEnhanced> wafer, const std::string& name);
    std::shared_ptr<WaferEnhanced> getWafer(const std::string& name);
    
    // Process simulation
    struct ProcessParameters {
        std::string operation;
        std::unordered_map<std::string, double> parameters;
        double duration;
        int priority = 0;
        
        ProcessParameters(const std::string& op, double dur) 
            : operation(op), duration(dur) {}
    };
    
    // Asynchronous simulation
    std::future<bool> simulateProcessAsync(const std::string& wafer_name, 
                                          const ProcessParameters& params);
    
    // Batch processing
    void addProcessToBatch(const std::string& wafer_name, const ProcessParameters& params);
    std::future<std::vector<bool>> executeBatch();
    void clearBatch();
    
    // Real-time monitoring
    struct SimulationStatus {
        std::string current_operation;
        double progress;
        std::chrono::system_clock::time_point start_time;
        std::chrono::system_clock::time_point estimated_completion;
        bool is_running;
        
        SimulationStatus() : progress(0.0), is_running(false) {}
    };
    
    SimulationStatus getStatus() const;
    void pauseSimulation();
    void resumeSimulation();
    void stopSimulation();
    
    // Error handling
    enum ErrorLevel { INFO, WARNING, ERROR, CRITICAL };
    struct SimulationError {
        ErrorLevel level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::string context;
        
        SimulationError(ErrorLevel lvl, const std::string& msg, const std::string& ctx = "")
            : level(lvl), message(msg), timestamp(std::chrono::system_clock::now()), context(ctx) {}
    };
    
    void reportError(const SimulationError& error);
    std::vector<SimulationError> getErrors(ErrorLevel min_level = INFO) const;
    void clearErrors();
    
    // Performance optimization
    void setThreadCount(int count);
    int getThreadCount() const;
    void enableGPUAcceleration(bool enable);
    bool isGPUAccelerationEnabled() const;

    // Simulation control
    void pause();
    void resume();
    bool isPaused() const;
    
    // Checkpointing and recovery
    bool saveCheckpoint(const std::string& filename);
    bool loadCheckpoint(const std::string& filename);
    void enableAutoCheckpoint(bool enable, int interval_minutes = 10);
    
    // Statistics and reporting
    struct Statistics {
        size_t total_operations;
        size_t total_processes;
        size_t successful_processes;
        size_t failed_processes;
        double total_simulation_time;
        double average_operation_time;
        double average_process_time;
        double success_rate;
        size_t memory_usage;
        size_t peak_memory_usage;
        std::unordered_map<std::string, size_t> processes_by_type;

        Statistics() : total_operations(0), total_processes(0), successful_processes(0),
                      failed_processes(0), total_simulation_time(0.0),
                      average_operation_time(0.0), average_process_time(0.0),
                      success_rate(0.0), memory_usage(0), peak_memory_usage(0) {}
    };
    
    Statistics getStatistics() const;
    void resetStatistics();
    void exportReport(const std::string& filename) const;
    
private:
    SimulationEngine() = default;
    ~SimulationEngine() = default;
    
    // Internal state
    std::unordered_map<std::string, std::shared_ptr<WaferEnhanced>> wafers_;
    std::queue<std::pair<std::string, ProcessParameters>> batch_queue_;
    std::vector<SimulationError> errors_;
    
    mutable std::mutex state_mutex_;
    std::atomic<bool> is_initialized_{false};
    std::atomic<bool> is_running_{false};
    std::atomic<bool> is_paused_{false};
    
    // Threading
    std::vector<std::thread> worker_threads_;
    int thread_count_ = std::thread::hardware_concurrency();
    
    // Configuration
    std::string config_file_;
    bool gpu_acceleration_enabled_ = false;
    bool auto_checkpoint_enabled_ = false;
    int checkpoint_interval_ = 10;
    
    // Statistics
    mutable Statistics stats_;
    std::chrono::system_clock::time_point simulation_start_time_;
    
    // Internal methods
    bool executeProcess(const std::string& wafer_name, const ProcessParameters& params);
    void workerThread();
    void checkpointThread();
    void updateStatistics();

    // Physics simulation methods
    bool simulateOxidation(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params);
    bool simulateIonImplantation(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params);
    bool simulateDeposition(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params);
    bool simulateEtching(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params);
};

// Convenience macros for simulation
#define SIM_ENGINE SimulationEngine::getInstance()
#define CREATE_WAFER(name, d, t, m) SIM_ENGINE.registerWafer(SIM_ENGINE.createWafer(d, t, m), name)
#define GET_WAFER(name) SIM_ENGINE.getWafer(name)

#endif // SIMULATION_ENGINE_HPP

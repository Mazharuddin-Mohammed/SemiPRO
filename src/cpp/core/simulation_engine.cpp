// Author: Dr. Mazharuddin Mohammed
#include "simulation_engine.hpp"
#include "utils.hpp"
#include "enhanced_error_handling.hpp"
#include "advanced_logger.hpp"
#include "memory_manager.hpp"
#include "config_manager.hpp"
#include "../physics/enhanced_oxidation.hpp"
#include "../physics/enhanced_doping.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <future>

SimulationEngine& SimulationEngine::getInstance() {
    static SimulationEngine instance;

    // Initialize advanced systems on first access
    static bool systems_initialized = false;
    if (!systems_initialized) {
        // Initialize advanced logging
        auto& logger = SemiPRO::AdvancedLogger::getInstance();
        logger.addOutput(std::make_unique<SemiPRO::ConsoleOutput>(true, SemiPRO::LogLevel::INFO));
        logger.addOutput(std::make_unique<SemiPRO::FileOutput>("logs/semipro.log"));
        logger.addOutput(std::make_unique<SemiPRO::JSONOutput>("logs/semipro.json"));

        // Initialize configuration manager
        auto& config = SemiPRO::ConfigManager::getInstance();

        // Set memory limits from config
        auto& memory_mgr = SemiPRO::MemoryManager::getInstance();
        auto memory_limit = CONFIG_GET("performance.memory_limit", int, 0);
        if (memory_limit > 0) {
            memory_mgr.setMemoryLimit(static_cast<size_t>(memory_limit));
        }

        SEMIPRO_INFO("Advanced systems initialized (logging, config, memory)");
        systems_initialized = true;
    }

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
    std::cout << "DEBUG: simulateProcessAsync called for wafer: " << wafer_name << ", operation: " << params.operation << std::endl;

    return std::async(std::launch::async, [this, wafer_name, params]() {
        std::cout << "DEBUG: Inside async lambda, about to call executeProcess" << std::endl;
        try {
            bool result = executeProcess(wafer_name, params);
            std::cout << "DEBUG: executeProcess returned: " << (result ? "true" : "false") << std::endl;
            return result;
        } catch (const std::exception& e) {
            std::cout << "DEBUG: Exception in executeProcess: " << e.what() << std::endl;
            return false;
        }
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
    using namespace SemiPRO;

    // Create memory scope for tracking
    MEMORY_SCOPE("process_" + params.operation);

    // Create performance timer for the entire process
    auto process_timer = AdvancedLogger::getInstance().createTimer(
        "process_" + params.operation, "SimulationEngine"
    );

    try {
        // Log process start with memory info
        auto& memory_mgr = MemoryManager::getInstance();
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::SIMULATION,
                          "Starting process: " + params.operation + " on wafer: " + wafer_name +
                          " (Memory usage: " + std::to_string(memory_mgr.getCurrentUsage()) + " bytes)",
                          "SimulationEngine");

        auto wafer = getWafer(wafer_name);
        if (!wafer) {
            throw SystemException("Failed to retrieve wafer: " + wafer_name, SEMIPRO_ERROR_CONTEXT());
        }

        // Update statistics
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            stats_.total_processes++;
            stats_.processes_by_type[params.operation]++;
        }
        
        // Execute the process based on type
        bool success = false;

        SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::SIMULATION,
                          "Dispatching process type: " + params.operation,
                          "SimulationEngine");

        if (params.operation == "oxidation") {
            success = simulateOxidation(wafer, params);
        } else if (params.operation == "doping") {
            success = simulateIonImplantation(wafer, params);
        } else if (params.operation == "deposition") {
            success = simulateDeposition(wafer, params);
        } else if (params.operation == "etching") {
            success = simulateEtching(wafer, params);
        } else {
            throw ConfigurationException(
                "Unknown process type: " + params.operation,
                SEMIPRO_ERROR_CONTEXT()
            );
        }
        
        if (success) {
            std::lock_guard<std::mutex> lock(state_mutex_);
            stats_.successful_processes++;
            ErrorManager::getInstance().reportError(
                ErrorSeverity::INFO, ErrorCategory::SIMULATION,
                "Process completed successfully: " + params.operation,
                SEMIPRO_ERROR_CONTEXT()
            );
        } else {
            std::lock_guard<std::mutex> lock(state_mutex_);
            stats_.failed_processes++;
            ErrorManager::getInstance().reportError(
                ErrorSeverity::ERROR, ErrorCategory::SIMULATION,
                "Process failed: " + params.operation,
                SEMIPRO_ERROR_CONTEXT()
            );
        }

        return success;

    } catch (const SemiPROException& e) {
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            stats_.failed_processes++;
        }
        ErrorManager::getInstance().reportError(e.getError());
        return false;
    } catch (const std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            stats_.failed_processes++;
        }
        ErrorManager::getInstance().reportError(
            ErrorSeverity::ERROR, ErrorCategory::SYSTEM,
            "Unexpected exception in process execution: " + std::string(e.what()),
            SEMIPRO_ERROR_CONTEXT()
        );
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

// Physics simulation implementations
bool SimulationEngine::simulateOxidation(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params) {
    using namespace SemiPRO;

    // Create memory scope for tracking
    MEMORY_SCOPE("oxidation_simulation");

    try {
        // Initialize enhanced oxidation physics
        static EnhancedOxidationPhysics oxidation_engine;

        // Extract and validate parameters
        OxidationConditions conditions;

        // Temperature
        auto temp_it = params.parameters.find("temperature");
        if (temp_it != params.parameters.end()) {
            conditions.temperature = temp_it->second;
        } else {
            conditions.temperature = CONFIG_GET("physics.temperature.default", double, 1000.0);
        }

        // Time
        auto time_it = params.parameters.find("time");
        if (time_it != params.parameters.end()) {
            conditions.time = time_it->second;
        } else {
            conditions.time = 1.0; // Default 1 hour
        }

        // Atmosphere
        auto ambient_it = params.parameters.find("ambient");
        if (ambient_it != params.parameters.end()) {
            conditions.atmosphere = (ambient_it->second > 0.5) ?
                OxidationAtmosphere::WET_H2O : OxidationAtmosphere::DRY_O2;
        } else {
            conditions.atmosphere = OxidationAtmosphere::DRY_O2;
        }

        // Pressure
        auto pressure_it = params.parameters.find("pressure");
        if (pressure_it != params.parameters.end()) {
            conditions.pressure = pressure_it->second;
        } else {
            conditions.pressure = CONFIG_GET("physics.pressure.default", double, 1.0);
        }

        // Crystal orientation (default to <100>)
        conditions.orientation = CrystalOrientation::SILICON_100;

        // Run enhanced oxidation simulation
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Starting enhanced oxidation: " +
                          std::to_string(conditions.temperature) + "°C, " +
                          std::to_string(conditions.time) + "h, " +
                          oxidation_engine.atmosphereToString(conditions.atmosphere),
                          "EnhancedOxidation");

        auto results = oxidation_engine.simulateOxidation(wafer, conditions);

        // Log detailed results
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Enhanced oxidation completed: " +
                          std::to_string(results.final_thickness) + " μm oxide grown, " +
                          "Growth rate: " + std::to_string(results.growth_rate) + " μm/h, " +
                          "Regime: " + oxidation_engine.regimeToString(results.regime) + ", " +
                          "Stress: " + std::to_string(results.stress_level) + " MPa, " +
                          "Quality score: " + std::to_string(results.quality_metrics.at("overall_score")),
                          "EnhancedOxidation");

        // Validate results
        if (results.final_thickness < 0 || results.final_thickness > 10.0) {
            SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                              "Oxidation result out of physical range: " +
                              std::to_string(results.final_thickness) + " μm",
                              "EnhancedOxidation");
            return false;
        }

        return true;

    } catch (const SemiPROException& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced oxidation failed: " + std::string(e.what()),
                          "EnhancedOxidation");
        return false;
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Oxidation simulation failed: " + std::string(e.what()),
                          "EnhancedOxidation");
        return false;
    }
}

bool SimulationEngine::simulateIonImplantation(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params) {
    using namespace SemiPRO;

    // Create memory scope for tracking
    MEMORY_SCOPE("ion_implantation_simulation");

    try {
        // Initialize enhanced doping physics (singleton pattern)
        static std::unique_ptr<EnhancedDopingPhysics> doping_engine_ptr;
        if (!doping_engine_ptr) {
            doping_engine_ptr = std::make_unique<EnhancedDopingPhysics>();
        }
        auto& doping_engine = *doping_engine_ptr;

        // Extract and validate parameters
        ImplantationConditions conditions;

        // Energy
        auto energy_it = params.parameters.find("energy");
        if (energy_it != params.parameters.end()) {
            conditions.energy = energy_it->second;
        } else {
            conditions.energy = 50.0; // Default 50 keV
        }

        // Dose
        auto dose_it = params.parameters.find("dose");
        if (dose_it != params.parameters.end()) {
            conditions.dose = dose_it->second;
        } else {
            conditions.dose = 1e15; // Default 1e15 cm⁻²
        }

        // Ion species (default to boron)
        auto species_it = params.parameters.find("species");
        if (species_it != params.parameters.end()) {
            int species_id = static_cast<int>(species_it->second);
            switch (species_id) {
                case 5: conditions.species = IonSpecies::BORON_11; break;
                case 15: conditions.species = IonSpecies::PHOSPHORUS_31; break;
                case 33: conditions.species = IonSpecies::ARSENIC_75; break;
                default: conditions.species = IonSpecies::BORON_11; break;
            }
        } else {
            conditions.species = IonSpecies::BORON_11;
        }

        // Tilt angle
        auto tilt_it = params.parameters.find("tilt");
        if (tilt_it != params.parameters.end()) {
            conditions.tilt_angle = tilt_it->second;
        } else {
            conditions.tilt_angle = 7.0; // Default 7° tilt
        }

        // Temperature
        auto temp_it = params.parameters.find("temperature");
        if (temp_it != params.parameters.end()) {
            conditions.temperature = temp_it->second;
        } else {
            conditions.temperature = 25.0; // Room temperature
        }

        // Run enhanced ion implantation simulation
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Starting enhanced ion implantation: " +
                          doping_engine.ionSpeciesToString(conditions.species) +
                          ", Energy: " + std::to_string(conditions.energy) + " keV" +
                          ", Dose: " + std::to_string(conditions.dose) + " cm⁻²" +
                          ", Tilt: " + std::to_string(conditions.tilt_angle) + "°",
                          "EnhancedDoping");

        auto results = doping_engine.simulateIonImplantation(wafer, conditions);

        // Log detailed results
        std::string result_message = "Enhanced ion implantation completed: " +
                          std::string("Range: ") + std::to_string(results.projected_range) + " μm, " +
                          "Straggling: " + std::to_string(results.range_straggling) + " μm, " +
                          "Peak conc: " + std::to_string(results.peak_concentration) + " cm⁻³, " +
                          "Sheet R: " + std::to_string(results.sheet_resistance) + " Ω/sq, " +
                          "Junction depth: " + std::to_string(results.junction_depth) + " μm, " +
                          "Quality score: " + std::to_string(results.quality_metrics.at("overall_score"));

        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          result_message, "EnhancedDoping");

        // Validate results
        if (results.projected_range < 0 || results.projected_range > 100.0) {
            SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                              "Ion implantation result out of physical range: " +
                              std::to_string(results.projected_range) + " μm",
                              "EnhancedDoping");
            return false;
        }

        return true;

    } catch (const SemiPROException& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced ion implantation failed: " + std::string(e.what()),
                          "EnhancedDoping");
        return false;
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Ion implantation simulation failed: " + std::string(e.what()),
                          "EnhancedDoping");
        return false;
    }
}

bool SimulationEngine::simulateDeposition(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params) {
    using namespace SemiPRO;

    try {
        ErrorManager::getInstance().reportError(
            ErrorSeverity::DEBUG, ErrorCategory::PHYSICS,
            "Starting deposition simulation",
            SEMIPRO_MODULE_ERROR_CONTEXT("Deposition")
        );

        // Extract parameters with validation
        double thickness = 0.5; // μm
        double temperature = 400.0; // °C
        std::string material = "aluminum"; // default material

        // Parse parameters with validation
        auto thick_it = params.parameters.find("thickness");
        if (thick_it != params.parameters.end()) {
            thickness = thick_it->second;
            if (thickness <= 0.0 || thickness > 100.0) {
                throw ValidationException(
                    "Thickness out of range: " + std::to_string(thickness) + " μm (valid: 0-100 μm)",
                    SEMIPRO_MODULE_ERROR_CONTEXT("Deposition")
                );
            }
        }

        auto temp_it = params.parameters.find("temperature");
        if (temp_it != params.parameters.end()) {
            temperature = temp_it->second;
            if (temperature < 0.0 || temperature > 2000.0) {
                throw ValidationException(
                    "Temperature out of range: " + std::to_string(temperature) + "°C (valid: 0-2000°C)",
                    SEMIPRO_MODULE_ERROR_CONTEXT("Deposition")
                );
            }
        }

        // Parse material parameter
        auto mat_it = params.string_parameters.find("material");
        if (mat_it != params.string_parameters.end()) {
            material = mat_it->second;
            if (material.empty()) {
                throw ValidationException(
                    "Material name cannot be empty",
                    SEMIPRO_MODULE_ERROR_CONTEXT("Deposition")
                );
            }
        }

        // Material properties database
        std::unordered_map<std::string, double> step_coverage = {
            {"aluminum", 0.3},      // Poor step coverage
            {"copper", 0.8},        // Good step coverage (electroplating)
            {"tungsten", 0.95},     // Excellent step coverage (CVD)
            {"polysilicon", 0.7},   // Good step coverage
            {"silicon_nitride", 0.9}, // Excellent conformality
            {"silicon_dioxide", 0.85}  // Good conformality
        };

        std::unordered_map<std::string, double> deposition_rate = {
            {"aluminum", 0.1},      // μm/min (sputtering)
            {"copper", 0.5},        // μm/min (electroplating)
            {"tungsten", 0.05},     // μm/min (CVD)
            {"polysilicon", 0.02},  // μm/min (LPCVD)
            {"silicon_nitride", 0.01}, // μm/min (PECVD)
            {"silicon_dioxide", 0.015}  // μm/min (PECVD)
        };

        // Validate parameters
        if (thickness < 0.001 || thickness > 50.0) { // 1 nm to 50 μm
            Logger::getInstance().log("Deposition thickness out of range: " + std::to_string(thickness) + " μm");
            return false;
        }

        if (temperature < 25.0 || temperature > 1200.0) {
            Logger::getInstance().log("Deposition temperature out of range: " + std::to_string(temperature) + "°C");
            return false;
        }

        // Get material properties
        double coverage = step_coverage.count(material) ? step_coverage[material] : 0.5;
        double rate = deposition_rate.count(material) ? deposition_rate[material] : 0.1;

        // Temperature dependence (Arrhenius) - FIXED: Higher temperature = faster rate
        double activation_energy = 0.5; // eV (typical)
        double reference_temp = 400.0; // Reference temperature in °C
        double temp_factor = std::exp(-activation_energy * 11600.0 * (1.0/(temperature + 273.15) - 1.0/(reference_temp + 273.15)));
        double effective_rate = rate * temp_factor;

        // Calculate deposition time
        double deposition_time = thickness / effective_rate; // minutes

        // Debug logging
        std::cout << "DEBUG: Deposition calculations:" << std::endl;
        std::cout << "  material=" << material << std::endl;
        std::cout << "  thickness=" << thickness << " μm" << std::endl;
        std::cout << "  temperature=" << temperature << "°C" << std::endl;
        std::cout << "  rate=" << rate << " μm/min" << std::endl;
        std::cout << "  temp_factor=" << temp_factor << std::endl;
        std::cout << "  effective_rate=" << effective_rate << " μm/min" << std::endl;
        std::cout << "  deposition_time=" << deposition_time << " minutes" << std::endl;

        if (deposition_time > 1000.0) { // More than 16 hours is unrealistic
            std::cout << "DEBUG: Deposition time too long: " << deposition_time << " minutes" << std::endl;
            return false;
        }

        // Apply deposition to wafer
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();

        // Simple conformal deposition model
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Calculate local step coverage based on topology
                double local_coverage = coverage;

                // Check for steps/trenches (simplified)
                if (i > 0 && i < rows - 1) {
                    double height_diff = std::abs(grid(i+1, j) - grid(i-1, j));
                    if (height_diff > 0.1) { // Significant topology
                        local_coverage *= 0.7; // Reduced coverage at steps
                    }
                }

                grid(i, j) += thickness * local_coverage;
            }
        }

        wafer->setGrid(grid);

        Logger::getInstance().log("Deposition completed");
        Logger::getInstance().log("Material: " + material);
        Logger::getInstance().log("Thickness: " + std::to_string(thickness) + " μm");
        Logger::getInstance().log("Temperature: " + std::to_string(temperature) + "°C");
        Logger::getInstance().log("Deposition time: " + std::to_string(deposition_time) + " minutes");
        Logger::getInstance().log("Step coverage: " + std::to_string(coverage * 100) + "%");

        return true;

    } catch (const std::exception& e) {
        Logger::getInstance().log("Deposition simulation failed: " + std::string(e.what()));
        return false;
    }
}

bool SimulationEngine::simulateEtching(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params) {
    try {
        // Extract parameters
        double etch_depth = 0.5; // μm
        std::string etch_type = "anisotropic"; // anisotropic or isotropic
        double selectivity = 10.0; // etch selectivity

        // Parse parameters
        auto depth_it = params.parameters.find("depth");
        if (depth_it != params.parameters.end()) {
            etch_depth = depth_it->second;
        }

        auto type_it = params.parameters.find("type");
        if (type_it != params.parameters.end()) {
            etch_type = (type_it->second > 0.5) ? "anisotropic" : "isotropic";
        }

        auto sel_it = params.parameters.find("selectivity");
        if (sel_it != params.parameters.end()) {
            selectivity = sel_it->second;
        }

        // Validate parameters
        if (etch_depth < 0.001 || etch_depth > 100.0) { // 1 nm to 100 μm
            Logger::getInstance().log("Etch depth out of range: " + std::to_string(etch_depth) + " μm");
            return false;
        }

        if (selectivity < 1.0 || selectivity > 1000.0) {
            Logger::getInstance().log("Etch selectivity out of range: " + std::to_string(selectivity));
            return false;
        }

        // Apply etching to wafer
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();

        if (etch_type == "anisotropic") {
            // Anisotropic etching (RIE) - vertical etching with minimal lateral
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    // Check if there's material to etch
                    if (grid(i, j) > etch_depth) {
                        grid(i, j) -= etch_depth;
                    } else {
                        grid(i, j) = 0.0; // Etched completely
                    }
                }
            }
        } else {
            // Isotropic etching - uniform etching in all directions
            auto original_grid = grid;

            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    if (original_grid(i, j) > 0) {
                        // Calculate isotropic etch with neighbor averaging
                        double etch_amount = etch_depth;

                        // Consider neighboring cells for isotropic behavior
                        int neighbor_count = 0;
                        double neighbor_sum = 0.0;

                        for (int di = -1; di <= 1; ++di) {
                            for (int dj = -1; dj <= 1; ++dj) {
                                int ni = i + di;
                                int nj = j + dj;

                                if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                                    neighbor_sum += original_grid(ni, nj);
                                    neighbor_count++;
                                }
                            }
                        }

                        double avg_neighbor = neighbor_sum / neighbor_count;

                        // Modify etch rate based on local environment
                        if (avg_neighbor < original_grid(i, j) * 0.5) {
                            etch_amount *= 1.2; // Faster etching at edges
                        }

                        if (grid(i, j) > etch_amount) {
                            grid(i, j) -= etch_amount;
                        } else {
                            grid(i, j) = 0.0;
                        }
                    }
                }
            }
        }

        wafer->setGrid(grid);

        Logger::getInstance().log("Etching completed");
        Logger::getInstance().log("Etch depth: " + std::to_string(etch_depth) + " μm");
        Logger::getInstance().log("Etch type: " + etch_type);
        Logger::getInstance().log("Selectivity: " + std::to_string(selectivity));

        return true;

    } catch (const std::exception& e) {
        Logger::getInstance().log("Etching simulation failed: " + std::string(e.what()));
        return false;
    }
}

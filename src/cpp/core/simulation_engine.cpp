// Author: Dr. Mazharuddin Mohammed
#include "simulation_engine.hpp"
#include "utils.hpp"
#include "enhanced_error_handling.hpp"
#include "advanced_logger.hpp"
#include "memory_manager.hpp"
#include "config_manager.hpp"
#include "../physics/enhanced_oxidation.hpp"
#include "../physics/enhanced_doping.hpp"
#include "../physics/enhanced_deposition.hpp"
#include "../physics/enhanced_etching.hpp"
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

    // Create memory scope for tracking
    MEMORY_SCOPE("deposition_simulation");

    try {
        // Initialize enhanced deposition physics (singleton pattern)
        static std::unique_ptr<EnhancedDepositionPhysics> deposition_engine_ptr;
        if (!deposition_engine_ptr) {
            deposition_engine_ptr = std::make_unique<EnhancedDepositionPhysics>();
        }
        auto& deposition_engine = *deposition_engine_ptr;

        // Extract and validate parameters
        DepositionConditions conditions;

        // Thickness
        auto thick_it = params.parameters.find("thickness");
        if (thick_it != params.parameters.end()) {
            conditions.target_thickness = thick_it->second;
        } else {
            conditions.target_thickness = 0.1; // Default 0.1 μm
        }

        // Temperature
        auto temp_it = params.parameters.find("temperature");
        if (temp_it != params.parameters.end()) {
            conditions.temperature = temp_it->second;
        } else {
            conditions.temperature = 400.0; // Default 400°C
        }

        // Material (default to aluminum)
        auto mat_it = params.string_parameters.find("material");
        if (mat_it != params.string_parameters.end()) {
            std::string material_name = mat_it->second;
            if (material_name == "aluminum") {
                conditions.material = MaterialType::ALUMINUM;
            } else if (material_name == "copper") {
                conditions.material = MaterialType::COPPER;
            } else if (material_name == "tungsten") {
                conditions.material = MaterialType::TUNGSTEN;
            } else if (material_name == "silicon_dioxide") {
                conditions.material = MaterialType::SILICON_DIOXIDE;
            } else if (material_name == "silicon_nitride") {
                conditions.material = MaterialType::SILICON_NITRIDE;
            } else {
                conditions.material = MaterialType::ALUMINUM; // Default
            }
        } else {
            conditions.material = MaterialType::ALUMINUM;
        }

        // Technique (default to CVD)
        conditions.technique = DepositionTechnique::CVD;

        // Pressure
        auto pressure_it = params.parameters.find("pressure");
        if (pressure_it != params.parameters.end()) {
            conditions.pressure = pressure_it->second;
        } else {
            conditions.pressure = 1.0; // Default 1 Torr
        }

        // Run enhanced deposition simulation
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Starting enhanced deposition: " +
                          deposition_engine.materialToString(conditions.material) +
                          ", Technique: " + deposition_engine.techniqueToString(conditions.technique) +
                          ", Thickness: " + std::to_string(conditions.target_thickness) + " μm" +
                          ", Temperature: " + std::to_string(conditions.temperature) + "°C",
                          "EnhancedDeposition");

        auto results = deposition_engine.simulateDeposition(wafer, conditions);

        // Log detailed results
        std::string result_message = "Enhanced deposition completed: " +
                          std::string("Thickness: ") + std::to_string(results.final_thickness) + " μm, " +
                          "Rate: " + std::to_string(results.deposition_rate) + " μm/min, " +
                          "Step coverage: " + std::to_string(results.step_coverage * 100.0) + "%, " +
                          "Conformality: " + std::to_string(results.conformality * 100.0) + "%, " +
                          "Uniformity: " + std::to_string(results.uniformity) + "%, " +
                          "Quality score: " + std::to_string(results.quality_metrics.at("overall_score"));

        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          result_message, "EnhancedDeposition");

        // Validate results
        if (results.final_thickness < 0 || results.final_thickness > 100.0) {
            SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                              "Deposition result out of physical range: " +
                              std::to_string(results.final_thickness) + " μm",
                              "EnhancedDeposition");
            return false;
        }

        return true;

    } catch (const SemiPROException& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced deposition failed: " + std::string(e.what()),
                          "EnhancedDeposition");
        return false;
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Deposition simulation failed: " + std::string(e.what()),
                          "EnhancedDeposition");
        return false;
    }
}

bool SimulationEngine::simulateEtching(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params) {
    using namespace SemiPRO;

    // Create memory scope for tracking
    MEMORY_SCOPE("etching_simulation");

    try {
        // Initialize enhanced etching physics (singleton pattern)
        static std::unique_ptr<EnhancedEtchingPhysics> etching_engine_ptr;
        if (!etching_engine_ptr) {
            etching_engine_ptr = std::make_unique<EnhancedEtchingPhysics>();
        }
        auto& etching_engine = *etching_engine_ptr;

        // Extract and validate parameters
        EtchingConditions conditions;

        // Target depth
        auto depth_it = params.parameters.find("depth");
        if (depth_it != params.parameters.end()) {
            conditions.target_depth = depth_it->second;
        } else {
            conditions.target_depth = 0.5; // Default 0.5 μm
        }

        // Etch type (anisotropic vs isotropic)
        auto type_it = params.parameters.find("type");
        if (type_it != params.parameters.end()) {
            bool is_anisotropic = (type_it->second > 0.5);
            conditions.technique = is_anisotropic ? EtchingTechnique::RIE : EtchingTechnique::WET_CHEMICAL;
        } else {
            conditions.technique = EtchingTechnique::RIE; // Default to RIE
        }

        // Selectivity target
        auto sel_it = params.parameters.find("selectivity");
        if (sel_it != params.parameters.end()) {
            conditions.selectivity_target = sel_it->second;
        } else {
            conditions.selectivity_target = 10.0; // Default selectivity
        }

        // Material types (default to silicon)
        conditions.target_material = EtchMaterial::SILICON;
        conditions.mask_material = EtchMaterial::PHOTORESIST;

        // Chemistry (default to fluorine-based)
        conditions.chemistry = EtchChemistry::FLUORINE_BASED;

        // Process parameters
        auto pressure_it = params.parameters.find("pressure");
        if (pressure_it != params.parameters.end()) {
            conditions.pressure = pressure_it->second;
        } else {
            conditions.pressure = 10.0; // Default 10 mTorr
        }

        auto power_it = params.parameters.find("power");
        if (power_it != params.parameters.end()) {
            conditions.power = power_it->second;
        } else {
            conditions.power = 100.0; // Default 100 W
        }

        auto bias_it = params.parameters.find("bias");
        if (bias_it != params.parameters.end()) {
            conditions.bias_voltage = bias_it->second;
        } else {
            conditions.bias_voltage = 100.0; // Default 100 V
        }

        // Run enhanced etching simulation
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Starting enhanced etching: " +
                          etching_engine.materialToString(conditions.target_material) +
                          ", Technique: " + etching_engine.techniqueToString(conditions.technique) +
                          ", Depth: " + std::to_string(conditions.target_depth) + " μm" +
                          ", Pressure: " + std::to_string(conditions.pressure) + " mTorr",
                          "EnhancedEtching");

        auto results = etching_engine.simulateEtching(wafer, conditions);

        // Log detailed results
        std::string result_message = "Enhanced etching completed: " +
                          std::string("Depth: ") + std::to_string(results.final_depth) + " μm, " +
                          "Rate: " + std::to_string(results.etch_rate) + " μm/min, " +
                          "Selectivity: " + std::to_string(results.selectivity) + ", " +
                          "Anisotropy: " + std::to_string(results.anisotropy * 100.0) + "%, " +
                          "Uniformity: " + std::to_string(results.uniformity) + "%, " +
                          "Quality score: " + std::to_string(results.quality_metrics.at("overall_score"));

        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          result_message, "EnhancedEtching");

        // Validate results
        if (results.final_depth < 0 || results.final_depth > 1000.0) {
            SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                              "Etching result out of physical range: " +
                              std::to_string(results.final_depth) + " μm",
                              "EnhancedEtching");
            return false;
        }

        return true;

    } catch (const SemiPROException& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced etching failed: " + std::string(e.what()),
                          "EnhancedEtching");
        return false;
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Etching simulation failed: " + std::string(e.what()),
                          "EnhancedEtching");
        return false;
    }
}

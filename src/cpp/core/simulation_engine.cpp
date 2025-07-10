// Author: Dr. Mazharuddin Mohammed
#include "simulation_engine.hpp"
#include "utils.hpp"
#include "enhanced_error_handling.hpp"
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

    try {
        // Report process start
        ErrorManager::getInstance().reportError(
            ErrorSeverity::INFO, ErrorCategory::SIMULATION,
            "Starting process: " + params.operation + " on wafer: " + wafer_name,
            SEMIPRO_ERROR_CONTEXT()
        );

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

        ErrorManager::getInstance().reportError(
            ErrorSeverity::DEBUG, ErrorCategory::SIMULATION,
            "Dispatching process type: " + params.operation,
            SEMIPRO_ERROR_CONTEXT()
        );

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
    try {
        // Extract parameters
        double temperature = 1000.0; // Default temperature in Celsius
        double time = 1.0; // Default time in hours
        std::string ambient = "dry"; // Default ambient

        // Parse parameters from params.parameters map
        auto temp_it = params.parameters.find("temperature");
        if (temp_it != params.parameters.end()) {
            temperature = temp_it->second;
        }

        auto time_it = params.parameters.find("time");
        if (time_it != params.parameters.end()) {
            time = time_it->second;
        }

        auto ambient_it = params.parameters.find("ambient");
        if (ambient_it != params.parameters.end()) {
            // ambient is stored as double, convert to string
            ambient = (ambient_it->second > 0.5) ? "wet" : "dry";
        }

        // Deal-Grove oxidation kinetics
        // x² + Ax = B(t + τ)
        // where x is oxide thickness, t is time

        double A, B; // Deal-Grove constants

        if (ambient == "dry") {
            // Dry oxidation constants for <100> silicon
            if (temperature >= 1000) {
                A = 0.165; // μm
                B = 0.0117 * std::exp(-2.05 * 11600.0 / (8.314 * (temperature + 273.15))); // μm²/h
            } else {
                A = 0.165;
                B = 0.0117 * std::exp(-2.05 * 11600.0 / (8.314 * (temperature + 273.15)));
            }
        } else {
            // Wet oxidation constants
            A = 0.226; // μm
            B = 0.51 * std::exp(-0.78 * 11600.0 / (8.314 * (temperature + 273.15))); // μm²/h
        }

        // Solve quadratic equation: x² + Ax - Bt = 0
        double discriminant = A * A + 4 * B * time;
        if (discriminant < 0) {
            Logger::getInstance().log("Invalid oxidation parameters - negative discriminant");
            return false;
        }

        double oxide_thickness = (-A + std::sqrt(discriminant)) / 2.0; // μm

        // Validate result
        if (oxide_thickness < 0 || oxide_thickness > 10.0) { // Reasonable limits
            Logger::getInstance().log("Oxidation result out of physical range: " + std::to_string(oxide_thickness) + " μm");
            return false;
        }

        // Apply oxidation to wafer grid
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();

        // Add oxide layer uniformly
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                grid(i, j) += oxide_thickness;
            }
        }

        wafer->setGrid(grid);

        Logger::getInstance().log("Oxidation completed: " + std::to_string(oxide_thickness) + " μm oxide grown");
        Logger::getInstance().log("Conditions: " + std::to_string(temperature) + "°C, " +
                                 std::to_string(time) + "h, " + ambient);

        return true;

    } catch (const std::exception& e) {
        Logger::getInstance().log("Oxidation simulation failed: " + std::string(e.what()));
        return false;
    }
}

bool SimulationEngine::simulateIonImplantation(std::shared_ptr<WaferEnhanced> wafer, const ProcessParameters& params) {
    std::cout << "DEBUG: simulateIonImplantation function entered" << std::endl;

    try {
        std::cout << "DEBUG: Inside try block, extracting parameters" << std::endl;
        // Extract parameters
        double energy = 50.0; // keV
        double dose = 1e15; // ions/cm²
        double mass = 11.0; // amu (default: boron)
        int atomic_number = 5; // default: boron

        // Parse parameters
        auto energy_it = params.parameters.find("energy");
        if (energy_it != params.parameters.end()) {
            energy = energy_it->second;
        }

        auto dose_it = params.parameters.find("dose");
        if (dose_it != params.parameters.end()) {
            dose = dose_it->second;
        }

        auto mass_it = params.parameters.find("mass");
        if (mass_it != params.parameters.end()) {
            mass = mass_it->second;
        }

        auto z_it = params.parameters.find("atomic_number");
        if (z_it != params.parameters.end()) {
            atomic_number = static_cast<int>(z_it->second);
        }

        // LSS theory calculations
        // Target: Silicon (Z=14, M=28.09 amu, density=2.33 g/cm³)
        double target_z = 14.0;
        double target_mass = 28.09;
        double target_density = 2.33; // g/cm³

        // Reduced energy calculation
        double epsilon = 32.5 * target_mass * energy /
                        (atomic_number * target_z * (mass + target_mass) *
                         std::pow(atomic_number + target_z, 2.0/3.0));

        // LSS range calculation (Biersack-Haggmark formula)
        double reduced_range;
        if (epsilon < 10.0) {
            reduced_range = epsilon / (1.0 + 6.35 * std::sqrt(epsilon) + epsilon * (6.882 * std::sqrt(epsilon) - 1.708));
        } else {
            reduced_range = std::log(epsilon) / (2.0 * std::log(epsilon / 10.0));
        }

        // Convert to physical range in μm
        double range = 8.74e-3 * reduced_range * (mass + target_mass) /
                      (target_density * target_z * mass *
                       std::pow(atomic_number + target_z, 1.0/3.0)); // μm

        // Range straggling (Bohr straggling)
        double straggling = 0.42 * range; // Simplified model

        // Debug logging
        std::cout << "DEBUG: Ion implantation calculations:" << std::endl;
        std::cout << "  energy=" << energy << " keV" << std::endl;
        std::cout << "  dose=" << dose << " cm^-2" << std::endl;
        std::cout << "  mass=" << mass << " amu" << std::endl;
        std::cout << "  atomic_number=" << atomic_number << std::endl;
        std::cout << "  epsilon=" << epsilon << std::endl;
        std::cout << "  reduced_range=" << reduced_range << std::endl;
        std::cout << "  range=" << range << " μm" << std::endl;
        std::cout << "  straggling=" << straggling << " μm" << std::endl;

        // Validate physical limits - FIXED: More realistic range limits
        if (range < 0.00001 || range > 100.0) { // 0.01 nm to 100 μm (more realistic for low energy)
            std::cout << "DEBUG: Ion range out of physical limits: " << range << " μm" << std::endl;
            return false;
        }

        if (dose < 1e10 || dose > 1e18) { // Reasonable dose limits
            Logger::getInstance().log("Ion dose out of reasonable range: " + std::to_string(dose) + " cm⁻²");
            return false;
        }

        // Apply Gaussian distribution to wafer
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();

        // Assume wafer thickness is represented by row index
        double wafer_thickness = 10.0; // μm (assumed)

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                double depth = (static_cast<double>(i) / rows) * wafer_thickness;

                // Gaussian distribution: N(x) = (dose/√(2πσ²)) * exp(-(x-Rp)²/(2σ²))
                double gaussian = (dose / (straggling * std::sqrt(2.0 * M_PI))) *
                                 std::exp(-std::pow(depth - range, 2) / (2.0 * straggling * straggling));

                // Convert from cm⁻² to concentration (assuming 1 μm depth per grid point)
                double concentration = gaussian * 1e-4; // Convert cm⁻² to μm⁻²

                grid(i, j) += concentration;
            }
        }

        wafer->setGrid(grid);

        Logger::getInstance().log("Ion implantation completed");
        Logger::getInstance().log("Energy: " + std::to_string(energy) + " keV");
        Logger::getInstance().log("Dose: " + std::to_string(dose) + " cm⁻²");
        Logger::getInstance().log("Projected range: " + std::to_string(range) + " μm");
        Logger::getInstance().log("Straggling: " + std::to_string(straggling) + " μm");

        return true;

    } catch (const std::exception& e) {
        Logger::getInstance().log("Ion implantation simulation failed: " + std::string(e.what()));
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

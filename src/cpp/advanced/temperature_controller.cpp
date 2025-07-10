#include "temperature_controller.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace SemiPRO {

AdvancedTemperatureController::AdvancedTemperatureController() {
    initializeController();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Advanced Temperature Controller initialized", "TemperatureController");
}

void AdvancedTemperatureController::createRampProfile(const std::string& profile_id, const TemperatureRamp& ramp) {
    ramp_profiles_[profile_id] = ramp;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Created temperature ramp profile: " + profile_id + 
                      " (" + std::to_string(ramp.start_temperature) + "°C → " + 
                      std::to_string(ramp.end_temperature) + "°C at " + 
                      std::to_string(ramp.ramp_rate) + "°C/min)",
                      "TemperatureController");
}

void AdvancedTemperatureController::createCyclingProfile(const std::string& profile_id, const ThermalCycle& cycle) {
    cycling_profiles_[profile_id] = cycle;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Created thermal cycling profile: " + profile_id + 
                      " (" + std::to_string(cycle.min_temperature) + "°C ↔ " + 
                      std::to_string(cycle.max_temperature) + "°C, " + 
                      std::to_string(cycle.num_cycles) + " cycles)",
                      "TemperatureController");
}

bool AdvancedTemperatureController::startTemperatureControl(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& profile_id,
    TemperatureControlMode mode) {
    
    SEMIPRO_PERF_TIMER("temperature_control", "TemperatureController");
    
    try {
        auto ramp_it = ramp_profiles_.find(profile_id);
        if (ramp_it == ramp_profiles_.end()) {
            throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, 
                                                  "Temperature profile not found: " + profile_id, "PROFILE_NOT_FOUND"));
        }
        
        const auto& ramp = ramp_it->second;
        current_temperature_ = ramp.start_temperature;
        target_temperature_ = ramp.end_temperature;
        start_time_ = std::chrono::steady_clock::now();
        is_active_ = true;
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Started temperature control with profile: " + profile_id + 
                          " (mode: " + controlModeToString(mode) + ")",
                          "TemperatureController");
        
        return true;
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Failed to start temperature control: " + std::string(e.what()),
                          "TemperatureController");
        return false;
    }
}

bool AdvancedTemperatureController::executeTemperatureProfile(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& profile_id,
    double duration_minutes) {
    
    SEMIPRO_PERF_TIMER("temperature_profile_execution", "TemperatureController");
    
    try {
        auto ramp_it = ramp_profiles_.find(profile_id);
        if (ramp_it == ramp_profiles_.end()) {
            throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, 
                                                  "Temperature profile not found: " + profile_id, "PROFILE_NOT_FOUND"));
        }
        
        const auto& ramp = ramp_it->second;
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Executing temperature profile: " + profile_id + 
                          " for " + std::to_string(duration_minutes) + " minutes",
                          "TemperatureController");
        
        // Simulate temperature profile execution
        double time_step = 0.1; // 0.1 minute steps
        int num_steps = static_cast<int>(duration_minutes / time_step);
        
        for (int step = 0; step < num_steps; ++step) {
            double time_elapsed = step * time_step;
            
            // Calculate current temperature based on ramp profile
            current_temperature_ = calculateRampTemperature(ramp, time_elapsed);
            
            // Apply spatial temperature if enabled
            if (enable_spatial_gradients_) {
                applySpatialTemperatureUniform(wafer, current_temperature_);
            }
            
            // Model thermal effects if enabled
            if (enable_thermal_coupling_) {
                modelThermalEffectsSimplified(wafer, current_temperature_);
            }
        }
        
        // Final temperature
        current_temperature_ = ramp.end_temperature;
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Temperature profile execution completed. Final temperature: " + 
                          std::to_string(current_temperature_) + "°C",
                          "TemperatureController");
        
        return true;
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Temperature profile execution failed: " + std::string(e.what()),
                          "TemperatureController");
        return false;
    }
}

bool AdvancedTemperatureController::executeThermalCycling(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& cycle_id) {
    
    SEMIPRO_PERF_TIMER("thermal_cycling", "TemperatureController");
    
    try {
        auto cycle_it = cycling_profiles_.find(cycle_id);
        if (cycle_it == cycling_profiles_.end()) {
            throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, 
                                                  "Thermal cycle not found: " + cycle_id, "CYCLE_NOT_FOUND"));
        }
        
        const auto& cycle = cycle_it->second;
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Executing thermal cycling: " + cycle_id + 
                          " (" + std::to_string(cycle.num_cycles) + " cycles, " +
                          std::to_string(cycle.min_temperature) + "°C ↔ " + 
                          std::to_string(cycle.max_temperature) + "°C)",
                          "TemperatureController");
        
        // Execute thermal cycles
        for (int cycle_num = 0; cycle_num < cycle.num_cycles; ++cycle_num) {
            // Heat up phase
            executeRampPhase(wafer, current_temperature_, cycle.max_temperature, 10.0); // 10°C/min ramp
            
            // Hold at high temperature
            holdTemperature(wafer, cycle.max_temperature, cycle.dwell_time_hot);
            
            // Cool down phase
            executeRampPhase(wafer, cycle.max_temperature, cycle.min_temperature, -10.0); // -10°C/min ramp
            
            // Hold at low temperature
            holdTemperature(wafer, cycle.min_temperature, cycle.dwell_time_cold);
            
            SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::ADVANCED,
                              "Completed thermal cycle " + std::to_string(cycle_num + 1) + "/" + 
                              std::to_string(cycle.num_cycles),
                              "TemperatureController");
        }
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Thermal cycling completed successfully",
                          "TemperatureController");
        
        return true;
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Thermal cycling failed: " + std::string(e.what()),
                          "TemperatureController");
        return false;
    }
}

TemperatureResults AdvancedTemperatureController::analyzeTemperatureUniformity(
    std::shared_ptr<WaferEnhanced> wafer) const {
    
    TemperatureResults results;
    results.current_temperature = current_temperature_;
    results.target_temperature = target_temperature_;
    results.temperature_error = std::abs(current_temperature_ - target_temperature_);
    
    // Calculate spatial temperature profile (simplified)
    auto grid = wafer->getGrid();
    int rows = grid.rows();
    int cols = grid.cols();
    
    std::vector<double> spatial_temps;
    double temp_sum = 0.0;
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Simplified spatial temperature model
            double distance_from_center = std::sqrt(
                std::pow(i - rows/2.0, 2) + std::pow(j - cols/2.0, 2)
            );
            
            // Add small spatial variation (±2°C)
            double spatial_temp = current_temperature_ + 
                                 2.0 * std::sin(distance_from_center * 0.1) * 
                                 std::cos(distance_from_center * 0.1);
            
            spatial_temps.push_back(spatial_temp);
            temp_sum += spatial_temp;
        }
    }
    
    // Calculate uniformity
    double mean_temp = temp_sum / spatial_temps.size();
    double variance = 0.0;
    
    for (double temp : spatial_temps) {
        variance += (temp - mean_temp) * (temp - mean_temp);
    }
    variance /= spatial_temps.size();
    
    double std_dev = std::sqrt(variance);
    results.uniformity = (1.0 - std_dev / mean_temp) * 100.0; // Uniformity percentage
    results.stability = std_dev; // Temperature stability in °C RMS
    
    results.spatial_profile = spatial_temps;
    
    // Calculate thermal metrics
    results.thermal_metrics["mean_temperature"] = mean_temp;
    results.thermal_metrics["temperature_std_dev"] = std_dev;
    results.thermal_metrics["max_temperature"] = *std::max_element(spatial_temps.begin(), spatial_temps.end());
    results.thermal_metrics["min_temperature"] = *std::min_element(spatial_temps.begin(), spatial_temps.end());
    results.thermal_metrics["temperature_range"] = results.thermal_metrics["max_temperature"] - 
                                                   results.thermal_metrics["min_temperature"];
    
    return results;
}

double AdvancedTemperatureController::calculateRampTemperature(
    const TemperatureRamp& ramp,
    double time_elapsed_minutes) const {
    
    double total_ramp_time = std::abs(ramp.end_temperature - ramp.start_temperature) / ramp.ramp_rate;
    
    if (time_elapsed_minutes <= total_ramp_time) {
        // During ramp
        switch (ramp.mode) {
            case TemperatureControlMode::LINEAR_RAMP:
                return ramp.start_temperature + ramp.ramp_rate * time_elapsed_minutes;
                
            case TemperatureControlMode::EXPONENTIAL_RAMP: {
                double time_constant = total_ramp_time / 3.0; // 3 time constants for ~95% completion
                double progress = 1.0 - std::exp(-time_elapsed_minutes / time_constant);
                return ramp.start_temperature + (ramp.end_temperature - ramp.start_temperature) * progress;
            }
            
            default:
                return ramp.start_temperature + ramp.ramp_rate * time_elapsed_minutes;
        }
    } else {
        // Hold at end temperature
        return ramp.end_temperature;
    }
}

void AdvancedTemperatureController::applySpatialTemperatureUniform(
    std::shared_ptr<WaferEnhanced> wafer,
    double temperature) {
    
    // Apply uniform temperature across the wafer (simplified)
    // In a real implementation, this would apply spatial gradients
    current_temperature_ = temperature;
}

void AdvancedTemperatureController::modelThermalEffectsSimplified(
    std::shared_ptr<WaferEnhanced> wafer,
    double temperature) {
    
    // Simplified thermal effects modeling
    // In a real implementation, this would model thermal expansion, stress, etc.
    
    // Calculate thermal expansion (simplified)
    double temp_change = temperature - 25.0; // Reference temperature
    double expansion_coefficient = 2.6e-6; // Silicon thermal expansion coefficient (1/°C)
    double expansion_factor = 1.0 + expansion_coefficient * temp_change;
    
    // Apply thermal effects to wafer (simplified)
    // This would modify the wafer grid to account for thermal expansion
}

void AdvancedTemperatureController::executeRampPhase(
    std::shared_ptr<WaferEnhanced> wafer,
    double start_temp,
    double end_temp,
    double ramp_rate) {
    
    double ramp_time = std::abs(end_temp - start_temp) / std::abs(ramp_rate);
    double time_step = 0.1; // 0.1 minute steps
    int num_steps = static_cast<int>(ramp_time / time_step);
    
    for (int step = 0; step <= num_steps; ++step) {
        double progress = static_cast<double>(step) / num_steps;
        current_temperature_ = start_temp + (end_temp - start_temp) * progress;
        
        if (enable_thermal_coupling_) {
            modelThermalEffectsSimplified(wafer, current_temperature_);
        }
    }
}

void AdvancedTemperatureController::holdTemperature(
    std::shared_ptr<WaferEnhanced> wafer,
    double temperature,
    double hold_time_minutes) {
    
    current_temperature_ = temperature;
    
    if (enable_thermal_coupling_) {
        modelThermalEffectsSimplified(wafer, current_temperature_);
    }
}

void AdvancedTemperatureController::setTemperature(double temperature) {
    current_temperature_ = temperature;
    target_temperature_ = temperature;
}

void AdvancedTemperatureController::enableFeatures(bool spatial, bool cycling, bool pid, bool coupling) {
    enable_spatial_gradients_ = spatial;
    enable_thermal_cycling_ = cycling;
    enable_pid_control_ = pid;
    enable_thermal_coupling_ = coupling;
}

std::string AdvancedTemperatureController::controlModeToString(TemperatureControlMode mode) const {
    switch (mode) {
        case TemperatureControlMode::ISOTHERMAL: return "Isothermal";
        case TemperatureControlMode::LINEAR_RAMP: return "Linear Ramp";
        case TemperatureControlMode::EXPONENTIAL_RAMP: return "Exponential Ramp";
        case TemperatureControlMode::STEP_PROFILE: return "Step Profile";
        case TemperatureControlMode::SINUSOIDAL_CYCLE: return "Sinusoidal Cycle";
        case TemperatureControlMode::CUSTOM_PROFILE: return "Custom Profile";
        case TemperatureControlMode::PID_CONTROLLED: return "PID Controlled";
        case TemperatureControlMode::ZONE_CONTROLLED: return "Zone Controlled";
        default: return "Unknown";
    }
}

std::string AdvancedTemperatureController::cyclingTypeToString(ThermalCyclingType type) const {
    switch (type) {
        case ThermalCyclingType::SIMPLE_CYCLE: return "Simple Cycle";
        case ThermalCyclingType::STRESS_RELIEF: return "Stress Relief";
        case ThermalCyclingType::ANNEALING_CYCLE: return "Annealing Cycle";
        case ThermalCyclingType::RAPID_THERMAL: return "Rapid Thermal";
        case ThermalCyclingType::FURNACE_CYCLE: return "Furnace Cycle";
        case ThermalCyclingType::LASER_ANNEALING: return "Laser Annealing";
        case ThermalCyclingType::FLASH_ANNEALING: return "Flash Annealing";
        default: return "Unknown";
    }
}

void AdvancedTemperatureController::initializeController() {
    current_temperature_ = 25.0;
    target_temperature_ = 25.0;
    is_active_ = false;
    pid_integral_ = 0.0;
    pid_previous_error_ = 0.0;
    start_time_ = std::chrono::steady_clock::now();
    pid_last_time_ = start_time_;
}

TemperatureRamp AdvancedTemperatureController::optimizeRampProfile(
    double target_temperature,
    double max_ramp_rate,
    double uniformity_requirement) {

    TemperatureRamp optimized_ramp;
    optimized_ramp.start_temperature = current_temperature_;
    optimized_ramp.end_temperature = target_temperature;

    // Optimize ramp rate for uniformity
    double temp_difference = std::abs(target_temperature - current_temperature_);

    // Slower ramp rates generally provide better uniformity
    if (uniformity_requirement > 98.0) {
        optimized_ramp.ramp_rate = std::min(max_ramp_rate * 0.3, 5.0); // Very slow for high uniformity
    } else if (uniformity_requirement > 95.0) {
        optimized_ramp.ramp_rate = std::min(max_ramp_rate * 0.6, 15.0); // Moderate for good uniformity
    } else {
        optimized_ramp.ramp_rate = max_ramp_rate; // Fast ramp for lower uniformity requirements
    }

    // Calculate hold time for thermal equilibration
    optimized_ramp.hold_time = temp_difference / 100.0; // 1 minute per 100°C difference
    optimized_ramp.mode = TemperatureControlMode::LINEAR_RAMP;

    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Optimized ramp profile: " + std::to_string(optimized_ramp.ramp_rate) +
                      "°C/min for " + std::to_string(uniformity_requirement) + "% uniformity",
                      "TemperatureController");

    return optimized_ramp;
}

ThermalCycle AdvancedTemperatureController::optimizeThermalCycle(
    double stress_relief_target,
    double max_temperature,
    double cycle_time_limit) {

    ThermalCycle optimized_cycle;
    optimized_cycle.type = ThermalCyclingType::STRESS_RELIEF;
    optimized_cycle.min_temperature = current_temperature_;
    optimized_cycle.max_temperature = max_temperature;

    // Optimize cycle parameters for stress relief
    optimized_cycle.cycle_time = std::min(cycle_time_limit, 20.0); // Max 20 minutes per cycle
    optimized_cycle.num_cycles = static_cast<int>(stress_relief_target / 10.0); // More cycles for higher stress relief
    optimized_cycle.dwell_time_hot = optimized_cycle.cycle_time * 0.3; // 30% at high temp
    optimized_cycle.dwell_time_cold = optimized_cycle.cycle_time * 0.2; // 20% at low temp

    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Optimized thermal cycle: " + std::to_string(optimized_cycle.num_cycles) +
                      " cycles for " + std::to_string(stress_relief_target) + "% stress relief",
                      "TemperatureController");

    return optimized_cycle;
}

bool AdvancedTemperatureController::validateTemperatureProfile(
    const std::string& profile_id,
    std::string& error_message) const {

    auto ramp_it = ramp_profiles_.find(profile_id);
    if (ramp_it == ramp_profiles_.end()) {
        error_message = "Temperature profile not found: " + profile_id;
        return false;
    }

    const auto& ramp = ramp_it->second;

    // Validate temperature range
    if (ramp.start_temperature < -273.15 || ramp.start_temperature > 1500.0) {
        error_message = "Start temperature out of range: " + std::to_string(ramp.start_temperature) + "°C";
        return false;
    }

    if (ramp.end_temperature < -273.15 || ramp.end_temperature > 1500.0) {
        error_message = "End temperature out of range: " + std::to_string(ramp.end_temperature) + "°C";
        return false;
    }

    // Validate ramp rate
    if (ramp.ramp_rate <= 0.0 || ramp.ramp_rate > 1000.0) {
        error_message = "Ramp rate out of range: " + std::to_string(ramp.ramp_rate) + "°C/min";
        return false;
    }

    // Validate hold time
    if (ramp.hold_time < 0.0 || ramp.hold_time > 1440.0) { // Max 24 hours
        error_message = "Hold time out of range: " + std::to_string(ramp.hold_time) + " minutes";
        return false;
    }

    return true;
}

std::vector<std::string> AdvancedTemperatureController::getDiagnostics() const {
    std::vector<std::string> diagnostics;

    diagnostics.push_back("Temperature Controller Status:");
    diagnostics.push_back("  Current Temperature: " + std::to_string(current_temperature_) + "°C");
    diagnostics.push_back("  Target Temperature: " + std::to_string(target_temperature_) + "°C");
    diagnostics.push_back("  Controller Active: " + std::string(is_active_ ? "Yes" : "No"));
    diagnostics.push_back("  Temperature Error: " + std::to_string(std::abs(current_temperature_ - target_temperature_)) + "°C");

    diagnostics.push_back("Feature Status:");
    diagnostics.push_back("  Spatial Gradients: " + std::string(enable_spatial_gradients_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Thermal Cycling: " + std::string(enable_thermal_cycling_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  PID Control: " + std::string(enable_pid_control_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Thermal Coupling: " + std::string(enable_thermal_coupling_ ? "Enabled" : "Disabled"));

    diagnostics.push_back("Profile Counts:");
    diagnostics.push_back("  Ramp Profiles: " + std::to_string(ramp_profiles_.size()));
    diagnostics.push_back("  Cycling Profiles: " + std::to_string(cycling_profiles_.size()));
    diagnostics.push_back("  Spatial Configs: " + std::to_string(spatial_configs_.size()));
    diagnostics.push_back("  PID Controllers: " + std::to_string(pid_controllers_.size()));

    return diagnostics;
}

// Utility functions implementation
namespace TemperatureUtils {

    double celsiusToKelvin(double celsius) {
        return celsius + 273.15;
    }

    double kelvinToCelsius(double kelvin) {
        return kelvin - 273.15;
    }

    double celsiusToFahrenheit(double celsius) {
        return celsius * 9.0/5.0 + 32.0;
    }

    double fahrenheitToCelsius(double fahrenheit) {
        return (fahrenheit - 32.0) * 5.0/9.0;
    }

    double calculateActivationEnergy(double rate1, double temp1, double rate2, double temp2) {
        // Calculate activation energy using Arrhenius equation
        // Ea = R * ln(rate2/rate1) / (1/T1 - 1/T2)
        const double R = 8.314; // Gas constant (J/mol·K)

        double T1 = celsiusToKelvin(temp1);
        double T2 = celsiusToKelvin(temp2);

        double ln_ratio = std::log(rate2 / rate1);
        double temp_diff = (1.0/T1) - (1.0/T2);

        return R * ln_ratio / temp_diff; // J/mol
    }

    double calculateArrheniusRate(double pre_factor, double activation_energy, double temperature) {
        // Rate = A * exp(-Ea / (R*T))
        const double R = 8.314; // Gas constant (J/mol·K)
        double T = celsiusToKelvin(temperature);

        return pre_factor * std::exp(-activation_energy / (R * T));
    }

    double calculateThermalBudget(const std::vector<std::pair<double, double>>& temp_time_pairs) {
        // Calculate thermal budget as integral of temperature over time
        double thermal_budget = 0.0;

        for (size_t i = 1; i < temp_time_pairs.size(); ++i) {
            double temp_avg = (temp_time_pairs[i-1].first + temp_time_pairs[i].first) / 2.0;
            double time_diff = temp_time_pairs[i].second - temp_time_pairs[i-1].second;
            thermal_budget += temp_avg * time_diff;
        }

        return thermal_budget; // °C·min
    }

    std::vector<double> generateLinearRamp(double start_temp, double end_temp, int num_points) {
        std::vector<double> ramp(num_points);

        for (int i = 0; i < num_points; ++i) {
            double progress = static_cast<double>(i) / (num_points - 1);
            ramp[i] = start_temp + (end_temp - start_temp) * progress;
        }

        return ramp;
    }

    std::vector<double> generateExponentialRamp(double start_temp, double end_temp, double time_constant, int num_points) {
        std::vector<double> ramp(num_points);

        for (int i = 0; i < num_points; ++i) {
            double time = static_cast<double>(i) / (num_points - 1);
            double progress = 1.0 - std::exp(-time / time_constant);
            ramp[i] = start_temp + (end_temp - start_temp) * progress;
        }

        return ramp;
    }

    std::vector<double> generateSinusoidalCycle(double mean_temp, double amplitude, double period, int num_points) {
        std::vector<double> cycle(num_points);

        for (int i = 0; i < num_points; ++i) {
            double time = static_cast<double>(i) / (num_points - 1) * period;
            cycle[i] = mean_temp + amplitude * std::sin(2.0 * M_PI * time / period);
        }

        return cycle;
    }

} // namespace TemperatureUtils

} // namespace SemiPRO

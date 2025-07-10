#pragma once

#include "../core/advanced_logger.hpp"
#include "../core/enhanced_error_handling.hpp"
#include "../core/config_manager.hpp"
#include "../core/wafer_enhanced.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <chrono>

namespace SemiPRO {

/**
 * Advanced Temperature Controller
 * Implements sophisticated temperature control with dynamic ramp rates,
 * thermal cycling, spatial gradients, and advanced thermal management
 */

// Temperature control modes
enum class TemperatureControlMode {
    ISOTHERMAL,           // Constant temperature
    LINEAR_RAMP,          // Linear temperature ramp
    EXPONENTIAL_RAMP,     // Exponential temperature ramp
    STEP_PROFILE,         // Step temperature profile
    SINUSOIDAL_CYCLE,     // Sinusoidal thermal cycling
    CUSTOM_PROFILE,       // Custom temperature profile
    PID_CONTROLLED,       // PID feedback control
    ZONE_CONTROLLED       // Multi-zone temperature control
};

// Thermal cycling types
enum class ThermalCyclingType {
    SIMPLE_CYCLE,         // Simple up-down cycling
    STRESS_RELIEF,        // Stress relief cycling
    ANNEALING_CYCLE,      // Annealing thermal cycle
    RAPID_THERMAL,        // Rapid thermal processing
    FURNACE_CYCLE,        // Furnace thermal cycle
    LASER_ANNEALING,      // Laser annealing profile
    FLASH_ANNEALING       // Flash lamp annealing
};

// Spatial temperature distribution types
enum class SpatialDistributionType {
    UNIFORM,              // Uniform temperature
    RADIAL_GRADIENT,      // Radial temperature gradient
    LINEAR_GRADIENT,      // Linear temperature gradient
    ZONE_BASED,           // Zone-based temperature control
    HOTSPOT_MODEL,        // Hotspot temperature model
    EDGE_COOLING,         // Edge cooling effects
    CENTER_HEATING        // Center heating effects
};

// Temperature ramp profile
struct TemperatureRamp {
    double start_temperature;     // Starting temperature (°C)
    double end_temperature;       // Ending temperature (°C)
    double ramp_rate;            // Ramp rate (°C/min)
    double hold_time;            // Hold time at end temperature (min)
    TemperatureControlMode mode; // Ramp mode
    
    TemperatureRamp() : start_temperature(25.0), end_temperature(25.0),
                       ramp_rate(10.0), hold_time(0.0), mode(TemperatureControlMode::LINEAR_RAMP) {}
};

// Thermal cycling parameters
struct ThermalCycle {
    ThermalCyclingType type;      // Cycling type
    double min_temperature;       // Minimum temperature (°C)
    double max_temperature;       // Maximum temperature (°C)
    double cycle_time;           // Total cycle time (min)
    int num_cycles;              // Number of cycles
    double dwell_time_hot;       // Dwell time at max temp (min)
    double dwell_time_cold;      // Dwell time at min temp (min)
    std::vector<double> custom_profile; // Custom temperature profile
    
    ThermalCycle() : type(ThermalCyclingType::SIMPLE_CYCLE),
                    min_temperature(25.0), max_temperature(100.0),
                    cycle_time(10.0), num_cycles(1),
                    dwell_time_hot(2.0), dwell_time_cold(2.0) {}
};

// Spatial temperature configuration
struct SpatialTemperatureConfig {
    SpatialDistributionType distribution_type; // Distribution type
    double center_temperature;    // Center temperature (°C)
    double edge_temperature;      // Edge temperature (°C)
    double gradient_strength;     // Gradient strength (°C/mm)
    std::vector<std::pair<double, double>> zone_temperatures; // Zone temps (position, temp)
    std::unordered_map<std::string, double> hotspot_locations; // Hotspot locations
    
    SpatialTemperatureConfig() : distribution_type(SpatialDistributionType::UNIFORM),
                                center_temperature(25.0), edge_temperature(25.0),
                                gradient_strength(0.0) {}
};

// PID controller parameters
struct PIDParameters {
    double kp;                   // Proportional gain
    double ki;                   // Integral gain
    double kd;                   // Derivative gain
    double setpoint;             // Target temperature (°C)
    double integral_limit;       // Integral windup limit
    double output_limit_min;     // Minimum output limit
    double output_limit_max;     // Maximum output limit
    
    PIDParameters() : kp(1.0), ki(0.1), kd(0.01), setpoint(25.0),
                     integral_limit(100.0), output_limit_min(0.0), output_limit_max(1200.0) {}
};

// Temperature measurement and control results
struct TemperatureResults {
    double current_temperature;   // Current temperature (°C)
    double target_temperature;    // Target temperature (°C)
    double temperature_error;     // Temperature error (°C)
    double ramp_rate_actual;     // Actual ramp rate (°C/min)
    double uniformity;           // Temperature uniformity (%)
    double stability;            // Temperature stability (°C RMS)
    std::vector<double> spatial_profile; // Spatial temperature profile
    std::vector<double> time_profile;    // Time temperature profile
    std::unordered_map<std::string, double> thermal_metrics; // Thermal metrics
    
    TemperatureResults() : current_temperature(25.0), target_temperature(25.0),
                          temperature_error(0.0), ramp_rate_actual(0.0),
                          uniformity(100.0), stability(0.0) {}
};

// Advanced temperature controller
class AdvancedTemperatureController {
private:
    std::unordered_map<std::string, TemperatureRamp> ramp_profiles_;
    std::unordered_map<std::string, ThermalCycle> cycling_profiles_;
    std::unordered_map<std::string, SpatialTemperatureConfig> spatial_configs_;
    std::unordered_map<std::string, PIDParameters> pid_controllers_;
    
    // Current state
    double current_temperature_ = 25.0;
    double target_temperature_ = 25.0;
    std::chrono::steady_clock::time_point start_time_;
    bool is_active_ = false;
    
    // PID state
    double pid_integral_ = 0.0;
    double pid_previous_error_ = 0.0;
    std::chrono::steady_clock::time_point pid_last_time_;
    
    // Configuration
    bool enable_spatial_gradients_ = true;
    bool enable_thermal_cycling_ = true;
    bool enable_pid_control_ = true;
    bool enable_thermal_coupling_ = true;
    double time_step_ = 0.1; // seconds
    
public:
    AdvancedTemperatureController();
    ~AdvancedTemperatureController() = default;
    
    // Profile management
    void createRampProfile(const std::string& profile_id, const TemperatureRamp& ramp);
    void createCyclingProfile(const std::string& profile_id, const ThermalCycle& cycle);
    void createSpatialConfig(const std::string& config_id, const SpatialTemperatureConfig& config);
    void createPIDController(const std::string& controller_id, const PIDParameters& params);
    
    // Temperature control
    bool startTemperatureControl(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& profile_id,
        TemperatureControlMode mode = TemperatureControlMode::LINEAR_RAMP
    );
    
    bool executeTemperatureProfile(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& profile_id,
        double duration_minutes
    );
    
    bool executeThermalCycling(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& cycle_id
    );
    
    // Advanced control methods
    TemperatureResults updateTemperature(
        std::shared_ptr<WaferEnhanced> wafer,
        double time_elapsed_minutes
    );
    
    TemperatureResults pidControl(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& controller_id,
        double current_temp,
        double time_step_seconds
    );
    
    // Spatial temperature management
    void applySpatialTemperature(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& spatial_config_id
    );
    
    std::vector<double> calculateSpatialProfile(
        const SpatialTemperatureConfig& config,
        int grid_rows,
        int grid_cols
    );
    
    // Thermal effects modeling
    void modelThermalExpansion(
        std::shared_ptr<WaferEnhanced> wafer,
        double temperature_change
    );
    
    void modelThermalStress(
        std::shared_ptr<WaferEnhanced> wafer,
        double temperature,
        double temperature_gradient
    );
    
    void modelThermalDiffusion(
        std::shared_ptr<WaferEnhanced> wafer,
        double time_step_seconds
    );
    
    // Analysis and monitoring
    TemperatureResults analyzeTemperatureUniformity(
        std::shared_ptr<WaferEnhanced> wafer
    ) const;
    
    std::vector<double> calculateTemperatureGradients(
        std::shared_ptr<WaferEnhanced> wafer
    ) const;
    
    double calculateThermalBudget(
        const std::vector<double>& temperature_profile,
        const std::vector<double>& time_profile
    ) const;
    
    // Optimization and calibration
    TemperatureRamp optimizeRampProfile(
        double target_temperature,
        double max_ramp_rate,
        double uniformity_requirement
    );
    
    ThermalCycle optimizeThermalCycle(
        double stress_relief_target,
        double max_temperature,
        double cycle_time_limit
    );
    
    // Configuration and control
    void setTemperature(double temperature);
    double getTemperature() const { return current_temperature_; }
    double getTargetTemperature() const { return target_temperature_; }
    bool isActive() const { return is_active_; }
    
    void enableFeatures(bool spatial = true, bool cycling = true,
                       bool pid = true, bool coupling = true);
    
    void setTimeStep(double time_step_seconds);
    
    // Validation and diagnostics
    bool validateTemperatureProfile(const std::string& profile_id, std::string& error_message) const;
    std::vector<std::string> getDiagnostics() const;
    
    // Utility methods
    std::string controlModeToString(TemperatureControlMode mode) const;
    std::string cyclingTypeToString(ThermalCyclingType type) const;
    std::string distributionTypeToString(SpatialDistributionType type) const;
    
private:
    void initializeController();

    // Helper methods for temperature control
    void applySpatialTemperatureUniform(
        std::shared_ptr<WaferEnhanced> wafer,
        double temperature
    );

    void modelThermalEffectsSimplified(
        std::shared_ptr<WaferEnhanced> wafer,
        double temperature
    );

    void executeRampPhase(
        std::shared_ptr<WaferEnhanced> wafer,
        double start_temp,
        double end_temp,
        double ramp_rate
    );

    void holdTemperature(
        std::shared_ptr<WaferEnhanced> wafer,
        double temperature,
        double hold_time_minutes
    );
    
    // Temperature calculation methods
    double calculateRampTemperature(
        const TemperatureRamp& ramp,
        double time_elapsed_minutes
    ) const;
    
    double calculateCycleTemperature(
        const ThermalCycle& cycle,
        double time_elapsed_minutes
    ) const;
    
    // Thermal physics methods
    double calculateThermalDiffusivity(
        const std::string& material,
        double temperature
    ) const;
    
    double calculateThermalConductivity(
        const std::string& material,
        double temperature
    ) const;
    
    double calculateThermalExpansionCoefficient(
        const std::string& material,
        double temperature
    ) const;
    
    // Numerical methods
    std::vector<double> solveHeatEquation(
        const std::vector<double>& initial_temperature,
        const std::vector<double>& thermal_properties,
        double time_step,
        int grid_size
    ) const;
    
    // PID implementation
    double calculatePIDOutput(
        double error,
        double time_step_seconds,
        const PIDParameters& params
    );
};

// Utility functions for temperature analysis
namespace TemperatureUtils {
    // Temperature conversion utilities
    double celsiusToKelvin(double celsius);
    double kelvinToCelsius(double kelvin);
    double celsiusToFahrenheit(double celsius);
    double fahrenheitToCelsius(double fahrenheit);
    
    // Thermal analysis utilities
    double calculateActivationEnergy(double rate1, double temp1, double rate2, double temp2);
    double calculateArrheniusRate(double pre_factor, double activation_energy, double temperature);
    double calculateThermalBudget(const std::vector<std::pair<double, double>>& temp_time_pairs);
    
    // Temperature profile utilities
    std::vector<double> generateLinearRamp(double start_temp, double end_temp, int num_points);
    std::vector<double> generateExponentialRamp(double start_temp, double end_temp, double time_constant, int num_points);
    std::vector<double> generateSinusoidalCycle(double mean_temp, double amplitude, double period, int num_points);
    
    // Optimization utilities
    TemperatureRamp optimizeForUniformity(double target_temp, double max_rate, double uniformity_target);
    ThermalCycle optimizeForStressRelief(double max_temp, double stress_target, double time_limit);
}

} // namespace SemiPRO

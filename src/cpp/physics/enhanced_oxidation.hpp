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

namespace SemiPRO {

/**
 * Enhanced Oxidation Physics Module
 * Implements advanced Deal-Grove kinetics with temperature dependence, 
 * pressure effects, multi-layer oxidation, and stress modeling
 */

// Oxidation atmosphere types
enum class OxidationAtmosphere {
    DRY_O2,           // Pure dry oxygen
    WET_H2O,          // Water vapor (steam)
    PYROGENIC,        // H2 + O2 combustion
    HCL_ADDED,        // HCl-enhanced oxidation
    HIGH_PRESSURE,    // High-pressure oxidation
    PLASMA_ENHANCED   // Plasma-enhanced oxidation
};

// Crystal orientation effects
enum class CrystalOrientation {
    SILICON_100,      // <100> orientation
    SILICON_110,      // <110> orientation  
    SILICON_111       // <111> orientation
};

// Oxidation regime classification
enum class OxidationRegime {
    THIN_OXIDE,       // < 30 nm, linear regime
    INTERMEDIATE,     // 30-500 nm, transition
    THICK_OXIDE,      // > 500 nm, parabolic regime
    ULTRA_THIN        // < 5 nm, quantum effects
};

// Enhanced Deal-Grove parameters
struct DealGroveParameters {
    double A;                    // Linear rate constant (μm)
    double B;                    // Parabolic rate constant (μm²/h)
    double tau;                  // Time offset (h)
    double activation_energy_A;  // Activation energy for A (eV)
    double activation_energy_B;  // Activation energy for B (eV)
    double pressure_exponent;    // Pressure dependence exponent
    double stress_coefficient;   // Stress effect coefficient
    
    DealGroveParameters() : A(0.0), B(0.0), tau(0.0), 
                           activation_energy_A(2.05), activation_energy_B(1.23),
                           pressure_exponent(1.0), stress_coefficient(0.0) {}
};

// Oxidation conditions
struct OxidationConditions {
    double temperature;          // Temperature (°C)
    double time;                // Oxidation time (hours)
    double pressure;            // Pressure (atm)
    OxidationAtmosphere atmosphere;
    CrystalOrientation orientation;
    double initial_oxide;       // Initial oxide thickness (μm)
    double dopant_concentration; // Dopant concentration (cm⁻³)
    std::string dopant_type;    // Dopant type (B, P, As, etc.)
    
    OxidationConditions() : temperature(1000.0), time(1.0), pressure(1.0),
                           atmosphere(OxidationAtmosphere::DRY_O2),
                           orientation(CrystalOrientation::SILICON_100),
                           initial_oxide(0.0), dopant_concentration(0.0),
                           dopant_type("none") {}
};

// Oxidation results with detailed analysis
struct OxidationResults {
    double final_thickness;     // Final oxide thickness (μm)
    double growth_rate;         // Average growth rate (μm/h)
    double interface_roughness; // Si/SiO2 interface roughness (nm)
    double stress_level;        // Stress in oxide (MPa)
    double consumed_silicon;    // Silicon consumed (μm)
    OxidationRegime regime;     // Oxidation regime
    std::vector<double> thickness_profile; // Spatial thickness variation
    std::unordered_map<std::string, double> quality_metrics;
    
    OxidationResults() : final_thickness(0.0), growth_rate(0.0),
                        interface_roughness(0.0), stress_level(0.0),
                        consumed_silicon(0.0), regime(OxidationRegime::THIN_OXIDE) {}
};

// Enhanced oxidation physics engine
class EnhancedOxidationPhysics {
private:
    std::unordered_map<OxidationAtmosphere, DealGroveParameters> atmosphere_params_;
    std::unordered_map<CrystalOrientation, double> orientation_factors_;
    std::unordered_map<std::string, double> dopant_effects_;
    
    // Configuration
    bool enable_stress_effects_ = true;
    bool enable_dopant_effects_ = true;
    bool enable_orientation_effects_ = true;
    bool enable_pressure_effects_ = true;
    double numerical_precision_ = 1e-8;
    
public:
    EnhancedOxidationPhysics();
    
    // Main simulation interface
    OxidationResults simulateOxidation(
        std::shared_ptr<WaferEnhanced> wafer,
        const OxidationConditions& conditions
    );
    
    // Advanced physics calculations
    DealGroveParameters calculateEnhancedParameters(
        const OxidationConditions& conditions
    ) const;
    
    double calculateThickness(
        const DealGroveParameters& params,
        const OxidationConditions& conditions
    ) const;
    
    double calculateGrowthRate(
        const DealGroveParameters& params,
        const OxidationConditions& conditions,
        double current_thickness
    ) const;
    
    // Environmental effects
    double calculateTemperatureDependence(
        double base_value,
        double activation_energy,
        double temperature,
        double reference_temp = 1000.0
    ) const;
    
    double calculatePressureEffect(
        double base_value,
        double pressure,
        double exponent
    ) const;
    
    double calculateOrientationEffect(
        CrystalOrientation orientation
    ) const;
    
    double calculateDopantEffect(
        const std::string& dopant_type,
        double concentration
    ) const;
    
    // Stress modeling
    double calculateOxidationStress(
        double oxide_thickness,
        double temperature,
        CrystalOrientation orientation
    ) const;
    
    double calculateStressEffect(
        double stress_level,
        double base_rate
    ) const;
    
    // Multi-layer oxidation
    std::vector<double> simulateMultiLayerOxidation(
        const std::vector<double>& layer_thicknesses,
        const std::vector<std::string>& layer_materials,
        const OxidationConditions& conditions
    );
    
    // Interface modeling
    double calculateInterfaceRoughness(
        double oxide_thickness,
        double temperature,
        OxidationAtmosphere atmosphere
    ) const;
    
    double calculateSiliconConsumption(
        double oxide_thickness
    ) const;
    
    // Regime classification
    OxidationRegime classifyRegime(
        double oxide_thickness,
        const OxidationConditions& conditions
    ) const;
    
    // Quality analysis
    std::unordered_map<std::string, double> analyzeOxideQuality(
        const OxidationResults& results,
        const OxidationConditions& conditions
    ) const;
    
    // Spatial variation modeling
    std::vector<double> calculateSpatialVariation(
        std::shared_ptr<WaferEnhanced> wafer,
        const OxidationConditions& conditions,
        double base_thickness
    );
    
    // Configuration and calibration
    void setAtmosphereParameters(
        OxidationAtmosphere atmosphere,
        const DealGroveParameters& params
    );
    
    void setOrientationFactor(
        CrystalOrientation orientation,
        double factor
    );
    
    void setDopantEffect(
        const std::string& dopant_type,
        double effect_coefficient
    );
    
    void enablePhysicsEffects(
        bool stress = true,
        bool dopant = true,
        bool orientation = true,
        bool pressure = true
    );
    
    // Validation and diagnostics
    bool validateConditions(
        const OxidationConditions& conditions,
        std::string& error_message
    ) const;
    
    std::vector<std::string> getDiagnostics(
        const OxidationResults& results,
        const OxidationConditions& conditions
    ) const;
    
    // Utility methods
    std::string atmosphereToString(OxidationAtmosphere atmosphere) const;
    std::string orientationToString(CrystalOrientation orientation) const;
    std::string regimeToString(OxidationRegime regime) const;
    
private:
    void initializeDefaultParameters();
    void initializeOrientationFactors();
    void initializeDopantEffects();
    
    // Numerical methods
    double solveQuadraticEquation(
        double a, double b, double c,
        bool positive_root = true
    ) const;
    
    double calculateNumericalDerivative(
        std::function<double(double)> func,
        double x,
        double h = 1e-6
    ) const;
    
    // Advanced models
    double calculateQuantumEffects(
        double oxide_thickness,
        double temperature
    ) const;
    
    double calculatePlasmaEnhancement(
        double base_rate,
        double plasma_power,
        double pressure
    ) const;
    
    double calculateHClEnhancement(
        double base_rate,
        double hcl_concentration,
        double temperature
    ) const;
};

// Utility functions for oxidation analysis
namespace OxidationUtils {
    // Convert between different thickness units
    double convertThickness(double value, const std::string& from_unit, const std::string& to_unit);
    
    // Calculate oxidation time for target thickness
    double calculateRequiredTime(
        double target_thickness,
        const DealGroveParameters& params,
        const OxidationConditions& conditions
    );
    
    // Optimize oxidation conditions for target specifications
    OxidationConditions optimizeConditions(
        double target_thickness,
        double max_temperature,
        double max_time,
        OxidationAtmosphere preferred_atmosphere
    );
    
    // Generate oxidation recipes
    std::vector<OxidationConditions> generateRecipe(
        const std::vector<double>& target_thicknesses,
        const std::unordered_map<std::string, double>& constraints
    );
}

} // namespace SemiPRO

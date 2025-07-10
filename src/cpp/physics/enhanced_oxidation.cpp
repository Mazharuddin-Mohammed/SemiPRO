#include "enhanced_oxidation.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace SemiPRO {

EnhancedOxidationPhysics::EnhancedOxidationPhysics() {
    initializeDefaultParameters();
    initializeOrientationFactors();
    initializeDopantEffects();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                      "Enhanced Oxidation Physics initialized", "EnhancedOxidation");
}

OxidationResults EnhancedOxidationPhysics::simulateOxidation(
    std::shared_ptr<WaferEnhanced> wafer,
    const OxidationConditions& conditions) {
    
    SEMIPRO_PERF_TIMER("oxidation_simulation", "EnhancedOxidation");
    
    OxidationResults results;
    
    try {
        // Validate input conditions
        std::string error_msg;
        if (!validateConditions(conditions, error_msg)) {
            throw PhysicsException("Invalid oxidation conditions: " + error_msg);
        }
        
        // Calculate enhanced Deal-Grove parameters
        auto params = calculateEnhancedParameters(conditions);
        
        // Calculate oxide thickness
        results.final_thickness = calculateThickness(params, conditions);
        
        // Calculate growth rate
        results.growth_rate = calculateGrowthRate(params, conditions, results.final_thickness);
        
        // Calculate stress effects
        results.stress_level = calculateOxidationStress(
            results.final_thickness, conditions.temperature, conditions.orientation
        );
        
        // Calculate interface properties
        results.interface_roughness = calculateInterfaceRoughness(
            results.final_thickness, conditions.temperature, conditions.atmosphere
        );
        
        // Calculate silicon consumption
        results.consumed_silicon = calculateSiliconConsumption(results.final_thickness);
        
        // Classify oxidation regime
        results.regime = classifyRegime(results.final_thickness, conditions);
        
        // Calculate spatial variation
        results.thickness_profile = calculateSpatialVariation(
            wafer, conditions, results.final_thickness
        );
        
        // Analyze oxide quality
        results.quality_metrics = analyzeOxideQuality(results, conditions);
        
        // Apply results to wafer
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Use spatial variation if available
                double local_thickness = results.final_thickness;
                if (!results.thickness_profile.empty()) {
                    int index = i * cols + j;
                    if (index < results.thickness_profile.size()) {
                        local_thickness = results.thickness_profile[index];
                    }
                }
                grid(i, j) += local_thickness;
            }
        }
        
        wafer->setGrid(grid);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Enhanced oxidation completed: " + 
                          std::to_string(results.final_thickness) + " μm oxide grown",
                          "EnhancedOxidation");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced oxidation failed: " + std::string(e.what()),
                          "EnhancedOxidation");
        throw;
    }
    
    return results;
}

DealGroveParameters EnhancedOxidationPhysics::calculateEnhancedParameters(
    const OxidationConditions& conditions) const {
    
    // Get base parameters for atmosphere
    auto it = atmosphere_params_.find(conditions.atmosphere);
    if (it == atmosphere_params_.end()) {
        throw PhysicsException("Unknown oxidation atmosphere");
    }
    
    DealGroveParameters params = it->second;
    
    // Apply temperature dependence
    double temp_kelvin = conditions.temperature + 273.15;
    double ref_temp_kelvin = 1000.0 + 273.15;
    
    params.A *= calculateTemperatureDependence(
        1.0, params.activation_energy_A, conditions.temperature
    );
    
    params.B *= calculateTemperatureDependence(
        1.0, params.activation_energy_B, conditions.temperature
    );
    
    // Apply pressure effects
    if (enable_pressure_effects_) {
        params.B *= calculatePressureEffect(
            1.0, conditions.pressure, params.pressure_exponent
        );
    }
    
    // Apply crystal orientation effects
    if (enable_orientation_effects_) {
        double orientation_factor = calculateOrientationEffect(conditions.orientation);
        params.A *= orientation_factor;
        params.B *= orientation_factor;
    }
    
    // Apply dopant effects
    if (enable_dopant_effects_ && conditions.dopant_concentration > 0) {
        double dopant_factor = calculateDopantEffect(
            conditions.dopant_type, conditions.dopant_concentration
        );
        params.A *= dopant_factor;
        params.B *= dopant_factor;
    }
    
    return params;
}

double EnhancedOxidationPhysics::calculateThickness(
    const DealGroveParameters& params,
    const OxidationConditions& conditions) const {
    
    double effective_time = conditions.time + params.tau;
    
    // Solve quadratic equation: x² + Ax - B*t = 0
    double a = 1.0;
    double b = params.A;
    double c = -params.B * effective_time;
    
    double thickness = solveQuadraticEquation(a, b, c, true);
    
    // Add initial oxide thickness
    thickness += conditions.initial_oxide;
    
    // Apply stress effects if enabled
    if (enable_stress_effects_) {
        double stress = calculateOxidationStress(
            thickness, conditions.temperature, conditions.orientation
        );
        double stress_factor = calculateStressEffect(stress, 1.0);
        thickness *= stress_factor;
    }
    
    // Apply quantum effects for ultra-thin oxides
    if (thickness < 0.005) { // < 5 nm
        double quantum_factor = calculateQuantumEffects(thickness, conditions.temperature);
        thickness *= quantum_factor;
    }
    
    return std::max(0.0, thickness);
}

double EnhancedOxidationPhysics::calculateGrowthRate(
    const DealGroveParameters& params,
    const OxidationConditions& conditions,
    double current_thickness) const {
    
    // dx/dt = B / (2x + A)
    double denominator = 2.0 * current_thickness + params.A;
    if (denominator < numerical_precision_) {
        denominator = numerical_precision_;
    }
    
    return params.B / denominator;
}

double EnhancedOxidationPhysics::calculateTemperatureDependence(
    double base_value,
    double activation_energy,
    double temperature,
    double reference_temp) const {
    
    const double k_boltzmann = 8.617e-5; // eV/K
    double temp_kelvin = temperature + 273.15;
    double ref_temp_kelvin = reference_temp + 273.15;
    
    double exponent = -activation_energy * (1.0/temp_kelvin - 1.0/ref_temp_kelvin) / k_boltzmann;
    
    return base_value * std::exp(exponent);
}

double EnhancedOxidationPhysics::calculatePressureEffect(
    double base_value,
    double pressure,
    double exponent) const {
    
    return base_value * std::pow(pressure, exponent);
}

double EnhancedOxidationPhysics::calculateOrientationEffect(
    CrystalOrientation orientation) const {
    
    auto it = orientation_factors_.find(orientation);
    return (it != orientation_factors_.end()) ? it->second : 1.0;
}

double EnhancedOxidationPhysics::calculateDopantEffect(
    const std::string& dopant_type,
    double concentration) const {
    
    auto it = dopant_effects_.find(dopant_type);
    if (it == dopant_effects_.end()) {
        return 1.0;
    }
    
    // Logarithmic dependence on concentration
    double log_conc = std::log10(std::max(1e10, concentration));
    return 1.0 + it->second * (log_conc - 16.0); // Reference: 1e16 cm⁻³
}

double EnhancedOxidationPhysics::calculateOxidationStress(
    double oxide_thickness,
    double temperature,
    CrystalOrientation orientation) const {
    
    // Thermal expansion mismatch stress
    double alpha_si = 2.6e-6;   // Silicon thermal expansion (/K)
    double alpha_sio2 = 0.5e-6; // SiO2 thermal expansion (/K)
    double delta_alpha = alpha_si - alpha_sio2;
    
    double delta_temp = temperature - 25.0; // Stress-free temperature
    
    // Young's modulus and Poisson's ratio for SiO2
    double E_sio2 = 70e9;  // Pa
    double nu_sio2 = 0.17;
    
    double stress = E_sio2 * delta_alpha * delta_temp / (1.0 - nu_sio2);
    
    // Thickness dependence
    stress *= (1.0 + oxide_thickness * 1000.0); // Convert μm to nm
    
    // Orientation factor
    double orientation_factor = calculateOrientationEffect(orientation);
    stress *= orientation_factor;
    
    return stress / 1e6; // Convert to MPa
}

double EnhancedOxidationPhysics::calculateStressEffect(
    double stress_level,
    double base_rate) const {
    
    // Stress enhances oxidation rate (compressive stress)
    double stress_coefficient = 1e-3; // MPa⁻¹
    return base_rate * (1.0 + stress_coefficient * std::abs(stress_level));
}

OxidationRegime EnhancedOxidationPhysics::classifyRegime(
    double oxide_thickness,
    const OxidationConditions& conditions) const {
    
    double thickness_nm = oxide_thickness * 1000.0; // Convert to nm
    
    if (thickness_nm < 5.0) {
        return OxidationRegime::ULTRA_THIN;
    } else if (thickness_nm < 30.0) {
        return OxidationRegime::THIN_OXIDE;
    } else if (thickness_nm < 500.0) {
        return OxidationRegime::INTERMEDIATE;
    } else {
        return OxidationRegime::THICK_OXIDE;
    }
}

double EnhancedOxidationPhysics::calculateInterfaceRoughness(
    double oxide_thickness,
    double temperature,
    OxidationAtmosphere atmosphere) const {
    
    // Base roughness depends on atmosphere
    double base_roughness = 0.1; // nm
    
    switch (atmosphere) {
        case OxidationAtmosphere::DRY_O2:
            base_roughness = 0.1;
            break;
        case OxidationAtmosphere::WET_H2O:
            base_roughness = 0.3;
            break;
        case OxidationAtmosphere::PYROGENIC:
            base_roughness = 0.2;
            break;
        default:
            base_roughness = 0.15;
    }
    
    // Temperature dependence
    double temp_factor = 1.0 + (temperature - 1000.0) / 1000.0 * 0.1;
    
    // Thickness dependence
    double thickness_factor = 1.0 + oxide_thickness * 1000.0 * 0.001; // Convert μm to nm
    
    return base_roughness * temp_factor * thickness_factor;
}

double EnhancedOxidationPhysics::calculateSiliconConsumption(
    double oxide_thickness) const {
    
    // Stoichiometric ratio: 1 μm SiO2 consumes ~0.44 μm Si
    return oxide_thickness * 0.44;
}

void EnhancedOxidationPhysics::initializeDefaultParameters() {
    // Dry oxidation parameters
    DealGroveParameters dry_params;
    dry_params.A = 0.165;                    // μm
    dry_params.B = 0.0117;                   // μm²/h at 1000°C
    dry_params.tau = 0.0;                    // h
    dry_params.activation_energy_A = 2.05;   // eV
    dry_params.activation_energy_B = 1.23;   // eV
    dry_params.pressure_exponent = 1.0;
    atmosphere_params_[OxidationAtmosphere::DRY_O2] = dry_params;
    
    // Wet oxidation parameters
    DealGroveParameters wet_params;
    wet_params.A = 0.226;                    // μm
    wet_params.B = 0.51;                     // μm²/h at 1000°C
    wet_params.tau = 0.0;                    // h
    wet_params.activation_energy_A = 2.05;   // eV
    wet_params.activation_energy_B = 0.78;   // eV
    wet_params.pressure_exponent = 1.0;
    atmosphere_params_[OxidationAtmosphere::WET_H2O] = wet_params;
    
    // Add other atmospheres...
    atmosphere_params_[OxidationAtmosphere::PYROGENIC] = wet_params; // Similar to wet
}

void EnhancedOxidationPhysics::initializeOrientationFactors() {
    orientation_factors_[CrystalOrientation::SILICON_100] = 1.0;   // Reference
    orientation_factors_[CrystalOrientation::SILICON_110] = 1.4;   // Faster oxidation
    orientation_factors_[CrystalOrientation::SILICON_111] = 0.7;   // Slower oxidation
}

void EnhancedOxidationPhysics::initializeDopantEffects() {
    dopant_effects_["B"] = 0.1;    // Boron enhances oxidation
    dopant_effects_["P"] = -0.05;  // Phosphorus retards oxidation
    dopant_effects_["As"] = -0.03; // Arsenic slightly retards
    dopant_effects_["Sb"] = -0.02; // Antimony slightly retards
}

double EnhancedOxidationPhysics::solveQuadraticEquation(
    double a, double b, double c, bool positive_root) const {
    
    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0) {
        throw PhysicsException("Negative discriminant in quadratic equation");
    }
    
    double sqrt_discriminant = std::sqrt(discriminant);
    
    if (positive_root) {
        return (-b + sqrt_discriminant) / (2.0 * a);
    } else {
        return (-b - sqrt_discriminant) / (2.0 * a);
    }
}

bool EnhancedOxidationPhysics::validateConditions(
    const OxidationConditions& conditions,
    std::string& error_message) const {
    
    if (conditions.temperature < 600.0 || conditions.temperature > 1200.0) {
        error_message = "Temperature out of range (600-1200°C)";
        return false;
    }
    
    if (conditions.time <= 0.0 || conditions.time > 100.0) {
        error_message = "Time out of range (0-100 hours)";
        return false;
    }
    
    if (conditions.pressure <= 0.0 || conditions.pressure > 10.0) {
        error_message = "Pressure out of range (0-10 atm)";
        return false;
    }
    
    return true;
}

double EnhancedOxidationPhysics::calculateQuantumEffects(
    double oxide_thickness,
    double temperature) const {
    
    // Simplified quantum tunneling effect for ultra-thin oxides
    double thickness_nm = oxide_thickness * 1000.0;
    
    if (thickness_nm > 5.0) {
        return 1.0; // No quantum effects
    }
    
    // Tunneling probability affects growth rate
    double barrier_height = 3.2; // eV (SiO2 barrier)
    double effective_mass = 0.5;  // Relative to electron mass
    
    double tunneling_factor = std::exp(-2.0 * thickness_nm / 0.1); // Simplified
    
    return 1.0 + 0.1 * tunneling_factor; // Small enhancement
}

std::vector<double> EnhancedOxidationPhysics::calculateSpatialVariation(
    std::shared_ptr<WaferEnhanced> wafer,
    const OxidationConditions& conditions,
    double base_thickness) {

    auto grid = wafer->getGrid();
    int rows = grid.rows();
    int cols = grid.cols();

    std::vector<double> variation(rows * cols, base_thickness);

    // Add small random variations (±2%)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dis(1.0, 0.02);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            variation[index] = base_thickness * dis(gen);
        }
    }

    return variation;
}

std::unordered_map<std::string, double> EnhancedOxidationPhysics::analyzeOxideQuality(
    const OxidationResults& results,
    const OxidationConditions& conditions) const {

    std::unordered_map<std::string, double> quality;

    // Uniformity metric (lower is better)
    double uniformity = results.interface_roughness / (results.final_thickness * 1000.0);
    quality["uniformity"] = uniformity;

    // Stress metric (lower is better)
    quality["stress_level"] = std::abs(results.stress_level);

    // Growth rate stability
    quality["growth_rate"] = results.growth_rate;

    // Overall quality score (0-100)
    double quality_score = 100.0 - (uniformity * 50.0 + std::abs(results.stress_level) * 0.01);
    quality["overall_score"] = std::max(0.0, std::min(100.0, quality_score));

    return quality;
}

std::string EnhancedOxidationPhysics::atmosphereToString(OxidationAtmosphere atmosphere) const {
    switch (atmosphere) {
        case OxidationAtmosphere::DRY_O2: return "Dry O2";
        case OxidationAtmosphere::WET_H2O: return "Wet H2O";
        case OxidationAtmosphere::PYROGENIC: return "Pyrogenic";
        case OxidationAtmosphere::HCL_ADDED: return "HCl Added";
        case OxidationAtmosphere::HIGH_PRESSURE: return "High Pressure";
        case OxidationAtmosphere::PLASMA_ENHANCED: return "Plasma Enhanced";
        default: return "Unknown";
    }
}

std::string EnhancedOxidationPhysics::orientationToString(CrystalOrientation orientation) const {
    switch (orientation) {
        case CrystalOrientation::SILICON_100: return "Si(100)";
        case CrystalOrientation::SILICON_110: return "Si(110)";
        case CrystalOrientation::SILICON_111: return "Si(111)";
        default: return "Unknown";
    }
}

std::string EnhancedOxidationPhysics::regimeToString(OxidationRegime regime) const {
    switch (regime) {
        case OxidationRegime::ULTRA_THIN: return "Ultra-thin";
        case OxidationRegime::THIN_OXIDE: return "Thin oxide";
        case OxidationRegime::INTERMEDIATE: return "Intermediate";
        case OxidationRegime::THICK_OXIDE: return "Thick oxide";
        default: return "Unknown";
    }
}

// OxidationUtils implementation
namespace OxidationUtils {
    double convertThickness(double value, const std::string& from_unit, const std::string& to_unit) {
        // Conversion factors to micrometers
        std::unordered_map<std::string, double> to_um = {
            {"nm", 0.001}, {"um", 1.0}, {"mm", 1000.0}, {"A", 0.0001}
        };

        auto from_it = to_um.find(from_unit);
        auto to_it = to_um.find(to_unit);

        if (from_it == to_um.end() || to_it == to_um.end()) {
            throw std::invalid_argument("Unknown thickness unit");
        }

        // Convert to micrometers, then to target unit
        double value_um = value * from_it->second;
        return value_um / to_it->second;
    }

    double calculateRequiredTime(
        double target_thickness,
        const DealGroveParameters& params,
        const OxidationConditions& conditions) {

        // Solve for time: x² + Ax = B(t + τ)
        // t = (x² + Ax)/B - τ

        double x = target_thickness - conditions.initial_oxide;
        double required_time = (x * x + params.A * x) / params.B - params.tau;

        return std::max(0.0, required_time);
    }
}

} // namespace SemiPRO

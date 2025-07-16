#include "enhanced_deposition.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace SemiPRO {

EnhancedDepositionPhysics::EnhancedDepositionPhysics() {
    initializeMaterialDatabase();
    initializeTechniqueNames();
    
    // Only log once per process (static flag)
    static bool logged = false;
    if (!logged) {
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Enhanced Deposition Physics initialized", "EnhancedDeposition");
        logged = true;
    }
}

DepositionResults EnhancedDepositionPhysics::simulateDeposition(
    std::shared_ptr<WaferEnhanced> wafer,
    const DepositionConditions& conditions) {
    
    SEMIPRO_PERF_TIMER("deposition_simulation", "EnhancedDeposition");
    
    DepositionResults results;
    
    try {
        // Validate input conditions
        std::string error_msg;
        if (!validateConditions(conditions, error_msg)) {
            throw PhysicsException("Invalid deposition conditions: " + error_msg);
        }
        
        // Route to technique-specific simulation
        switch (conditions.technique) {
            case DepositionTechnique::CVD:
            case DepositionTechnique::LPCVD:
            case DepositionTechnique::PECVD:
            case DepositionTechnique::MOCVD:
                results = simulateCVD(wafer, conditions);
                break;
                
            case DepositionTechnique::PVD_SPUTTERING:
            case DepositionTechnique::PVD_EVAPORATION:
                results = simulatePVD(wafer, conditions);
                break;
                
            case DepositionTechnique::ALD:
                results = simulateALD(wafer, conditions);
                break;
                
            default:
                // Generic deposition model
                results.deposition_rate = calculateDepositionRate(conditions);
                results.final_thickness = conditions.target_thickness;
                break;
        }
        
        // Calculate common properties
        if (enable_conformality_analysis_) {
            results.step_coverage = calculateStepCoverage(conditions, 2.0); // 2:1 aspect ratio
            results.conformality = calculateConformality(conditions, 1.0, 0.5); // 1μm deep, 0.5μm wide
        }
        
        if (enable_stress_modeling_) {
            results.stress_level = calculateIntrinsicStress(conditions.material, conditions);
        }
        
        if (enable_grain_growth_) {
            results.grain_size = calculateGrainSize(conditions.material, conditions.temperature, results.final_thickness);
        }
        
        // Calculate surface roughness
        results.surface_roughness = calculateSurfaceRoughness(conditions.material, conditions, results.final_thickness);
        
        // Calculate spatial variation
        results.thickness_profile = calculateThicknessProfile(wafer, conditions, results.final_thickness);
        
        // Calculate uniformity
        results.uniformity = calculateUniformity(results.thickness_profile);
        
        // Analyze quality
        results.quality_metrics = analyzeDepositionQuality(results, conditions);
        
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
                          "Enhanced deposition completed: " + 
                          std::to_string(results.final_thickness) + " μm " +
                          materialToString(conditions.material) + " deposited",
                          "EnhancedDeposition");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced deposition failed: " + std::string(e.what()),
                          "EnhancedDeposition");
        throw;
    }
    
    return results;
}

DepositionResults EnhancedDepositionPhysics::simulateCVD(
    std::shared_ptr<WaferEnhanced> wafer,
    const DepositionConditions& conditions) {
    
    DepositionResults results;
    
    // CVD rate calculation with surface kinetics
    double base_rate = calculateDepositionRate(conditions, SurfaceReaction::REACTION_LIMITED);
    
    // Temperature dependence (Arrhenius)
    auto material_props = getMaterialProperties(conditions.material);
    double activation_energy = 1.5; // eV (typical for CVD)
    double temp_factor = calculateTemperatureDependence(1.0, activation_energy, conditions.temperature);
    
    // Pressure dependence
    double pressure_factor = calculatePressureDependence(1.0, conditions.pressure, 0.5);
    
    // Plasma enhancement for PECVD
    double plasma_factor = 1.0;
    if (conditions.technique == DepositionTechnique::PECVD && conditions.power > 0) {
        plasma_factor = calculatePlasmaEnhancement(1.0, conditions.power, conditions.pressure);
    }
    
    results.deposition_rate = base_rate * temp_factor * pressure_factor * plasma_factor;
    results.final_thickness = conditions.target_thickness;
    
    // CVD typically has good conformality
    results.step_coverage = 0.8 + 0.15 * (conditions.pressure / 10.0); // Better at higher pressure
    results.conformality = 0.9; // Excellent conformality
    
    return results;
}

DepositionResults EnhancedDepositionPhysics::simulatePVD(
    std::shared_ptr<WaferEnhanced> wafer,
    const DepositionConditions& conditions) {
    
    DepositionResults results;
    
    if (conditions.technique == DepositionTechnique::PVD_SPUTTERING) {
        // Sputtering rate calculation
        double sputtering_yield = calculateSputteringYield(conditions.material, 500.0); // 500 eV typical
        results.deposition_rate = sputtering_yield * 0.1; // Convert to μm/min
        
        // Sputtering has directional deposition
        results.step_coverage = 0.3; // Poor step coverage
        results.conformality = 0.4; // Poor conformality
        
    } else {
        // Evaporation rate calculation
        double evap_rate = calculateEvaporationRate(conditions.material, conditions.temperature, conditions.pressure);
        results.deposition_rate = evap_rate;
        
        // Evaporation is highly directional
        results.step_coverage = 0.2; // Very poor step coverage
        results.conformality = 0.3; // Very poor conformality
    }
    
    results.final_thickness = conditions.target_thickness;
    
    return results;
}

DepositionResults EnhancedDepositionPhysics::simulateALD(
    std::shared_ptr<WaferEnhanced> wafer,
    const DepositionConditions& conditions) {
    
    DepositionResults results;
    
    // ALD growth per cycle
    double growth_per_cycle = calculateALDGrowthPerCycle(conditions.material, conditions.temperature);
    int cycles = calculateALDCycles(conditions.target_thickness, growth_per_cycle);
    
    results.deposition_rate = growth_per_cycle * 60.0; // Assume 1 cycle per minute
    results.final_thickness = cycles * growth_per_cycle;
    
    // ALD has perfect conformality
    results.step_coverage = 1.0; // Perfect step coverage
    results.conformality = 1.0; // Perfect conformality
    
    return results;
}

double EnhancedDepositionPhysics::calculateDepositionRate(
    const DepositionConditions& conditions,
    SurfaceReaction mechanism) const {
    
    auto material_props = getMaterialProperties(conditions.material);
    
    // Base rate depends on material and technique
    double base_rate = 0.1; // μm/min default
    
    switch (conditions.material) {
        case MaterialType::ALUMINUM:
            base_rate = 0.1; // Sputtering rate
            break;
        case MaterialType::COPPER:
            base_rate = 0.5; // Electroplating rate
            break;
        case MaterialType::TUNGSTEN:
            base_rate = 0.05; // CVD rate
            break;
        case MaterialType::SILICON_DIOXIDE:
            base_rate = 0.015; // PECVD rate
            break;
        case MaterialType::SILICON_NITRIDE:
            base_rate = 0.01; // PECVD rate
            break;
        case MaterialType::POLYSILICON:
            base_rate = 0.02; // LPCVD rate
            break;
        default:
            base_rate = 0.05;
    }
    
    // Apply reaction mechanism effects
    switch (mechanism) {
        case SurfaceReaction::ADSORPTION_LIMITED:
            base_rate *= 0.8; // Slower due to adsorption limit
            break;
        case SurfaceReaction::DIFFUSION_LIMITED:
            base_rate *= 0.6; // Slower due to mass transport
            break;
        case SurfaceReaction::REACTION_LIMITED:
            // No modification - base rate
            break;
        default:
            break;
    }
    
    return base_rate;
}

double EnhancedDepositionPhysics::calculateStepCoverage(
    const DepositionConditions& conditions,
    double aspect_ratio) const {
    
    double base_coverage = 0.5;
    
    switch (conditions.technique) {
        case DepositionTechnique::CVD:
        case DepositionTechnique::LPCVD:
            base_coverage = 0.8 - 0.1 * aspect_ratio; // Decreases with aspect ratio
            break;
        case DepositionTechnique::PECVD:
            base_coverage = 0.7 - 0.15 * aspect_ratio;
            break;
        case DepositionTechnique::ALD:
            base_coverage = 1.0; // Perfect coverage
            break;
        case DepositionTechnique::PVD_SPUTTERING:
            base_coverage = 0.3 - 0.2 * aspect_ratio;
            break;
        case DepositionTechnique::PVD_EVAPORATION:
            base_coverage = 0.2 - 0.15 * aspect_ratio;
            break;
        default:
            base_coverage = 0.5;
    }
    
    return std::max(0.1, std::min(1.0, base_coverage));
}

double EnhancedDepositionPhysics::calculateConformality(
    const DepositionConditions& conditions,
    double trench_depth,
    double trench_width) const {
    
    double aspect_ratio = trench_depth / trench_width;
    return calculateStepCoverage(conditions, aspect_ratio);
}

void EnhancedDepositionPhysics::initializeMaterialDatabase() {
    // Aluminum
    MaterialProperties aluminum;
    aluminum.name = "Aluminum";
    aluminum.chemical_formula = "Al";
    aluminum.density = 2.70;
    aluminum.melting_point = 660.0;
    aluminum.thermal_conductivity = 237.0;
    aluminum.electrical_resistivity = 2.65e-6;
    aluminum.stress_intrinsic = -200.0; // Compressive
    aluminum.etch_selectivity = 0.1;
    aluminum.refractive_index = 1.44;
    aluminum.is_conductor = true;
    aluminum.is_barrier_metal = false;
    material_database_[MaterialType::ALUMINUM] = aluminum;
    
    // Silicon Dioxide
    MaterialProperties sio2;
    sio2.name = "Silicon Dioxide";
    sio2.chemical_formula = "SiO2";
    sio2.density = 2.20;
    sio2.melting_point = 1713.0;
    sio2.thermal_conductivity = 1.4;
    sio2.electrical_resistivity = 1e16;
    sio2.stress_intrinsic = 300.0; // Tensile
    sio2.etch_selectivity = 1.0; // Reference
    sio2.refractive_index = 1.46;
    sio2.is_conductor = false;
    sio2.is_barrier_metal = false;
    material_database_[MaterialType::SILICON_DIOXIDE] = sio2;

    // Polysilicon
    MaterialProperties polysilicon;
    polysilicon.name = "Polysilicon";
    polysilicon.chemical_formula = "poly-Si";
    polysilicon.density = 2.33;
    polysilicon.melting_point = 1414.0;
    polysilicon.thermal_conductivity = 150.0;
    polysilicon.electrical_resistivity = 1e-3; // Doped polysilicon
    polysilicon.stress_intrinsic = -100.0; // Compressive
    polysilicon.etch_selectivity = 0.5;
    polysilicon.refractive_index = 3.88;
    polysilicon.is_conductor = true;
    polysilicon.is_barrier_metal = false;
    material_database_[MaterialType::POLYSILICON] = polysilicon;

    // Silicon Nitride
    MaterialProperties si3n4;
    si3n4.name = "Silicon Nitride";
    si3n4.chemical_formula = "Si3N4";
    si3n4.density = 3.17;
    si3n4.melting_point = 1900.0;
    si3n4.thermal_conductivity = 30.0;
    si3n4.electrical_resistivity = 1e14;
    si3n4.stress_intrinsic = 1000.0; // Tensile
    si3n4.etch_selectivity = 10.0;
    si3n4.refractive_index = 2.01;
    si3n4.is_conductor = false;
    si3n4.is_barrier_metal = false;
    material_database_[MaterialType::SILICON_NITRIDE] = si3n4;

    // Copper
    MaterialProperties copper;
    copper.name = "Copper";
    copper.chemical_formula = "Cu";
    copper.density = 8.96;
    copper.melting_point = 1085.0;
    copper.thermal_conductivity = 401.0;
    copper.electrical_resistivity = 1.68e-6;
    copper.stress_intrinsic = -50.0; // Compressive
    copper.etch_selectivity = 0.01;
    copper.refractive_index = 0.94;
    copper.is_conductor = true;
    copper.is_barrier_metal = false;
    material_database_[MaterialType::COPPER] = copper;

    // Tungsten
    MaterialProperties tungsten;
    tungsten.name = "Tungsten";
    tungsten.chemical_formula = "W";
    tungsten.density = 19.25;
    tungsten.melting_point = 3422.0;
    tungsten.thermal_conductivity = 173.0;
    tungsten.electrical_resistivity = 5.6e-6;
    tungsten.stress_intrinsic = -500.0; // Compressive
    tungsten.etch_selectivity = 0.1;
    tungsten.refractive_index = 3.5;
    tungsten.is_conductor = true;
    tungsten.is_barrier_metal = true;
    material_database_[MaterialType::TUNGSTEN] = tungsten;

    // Add more materials as needed...
}

void EnhancedDepositionPhysics::initializeTechniqueNames() {
    technique_names_[DepositionTechnique::CVD] = "CVD";
    technique_names_[DepositionTechnique::LPCVD] = "LPCVD";
    technique_names_[DepositionTechnique::PECVD] = "PECVD";
    technique_names_[DepositionTechnique::MOCVD] = "MOCVD";
    technique_names_[DepositionTechnique::PVD_SPUTTERING] = "PVD Sputtering";
    technique_names_[DepositionTechnique::PVD_EVAPORATION] = "PVD Evaporation";
    technique_names_[DepositionTechnique::ALD] = "ALD";
    technique_names_[DepositionTechnique::ELECTROPLATING] = "Electroplating";
    technique_names_[DepositionTechnique::SPIN_COATING] = "Spin Coating";
    technique_names_[DepositionTechnique::EPITAXY] = "Epitaxy";
}

double EnhancedDepositionPhysics::calculateTemperatureDependence(
    double base_rate,
    double activation_energy,
    double temperature,
    double reference_temp) const {

    const double k_boltzmann = 8.617e-5; // eV/K
    double temp_kelvin = temperature + 273.15;
    double ref_temp_kelvin = reference_temp + 273.15;

    double exponent = -activation_energy * (1.0/temp_kelvin - 1.0/ref_temp_kelvin) / k_boltzmann;

    return base_rate * std::exp(exponent);
}

double EnhancedDepositionPhysics::calculatePressureDependence(
    double base_rate,
    double pressure,
    double pressure_exponent) const {

    return base_rate * std::pow(pressure, pressure_exponent);
}

double EnhancedDepositionPhysics::calculatePlasmaEnhancement(
    double base_rate,
    double power,
    double pressure) const {

    // Plasma enhancement factor based on power density
    double power_density = power / 100.0; // Normalize to 100W
    double enhancement = 1.0 + 2.0 * std::sqrt(power_density);

    // Pressure dependence for plasma
    double pressure_factor = std::sqrt(pressure / 1.0); // Normalize to 1 Torr

    return base_rate * enhancement * pressure_factor;
}

double EnhancedDepositionPhysics::calculateALDGrowthPerCycle(
    MaterialType material,
    double temperature) const {

    // Typical ALD growth rates (Angstroms per cycle)
    double growth_per_cycle = 1.0; // Angstroms

    switch (material) {
        case MaterialType::ALUMINUM_OXIDE:
            growth_per_cycle = 1.1; // Al2O3
            break;
        case MaterialType::HAFNIUM_OXIDE:
            growth_per_cycle = 1.0; // HfO2
            break;
        case MaterialType::TITANIUM_NITRIDE:
            growth_per_cycle = 0.4; // TiN
            break;
        default:
            growth_per_cycle = 1.0;
    }

    // Temperature dependence (mild for ALD)
    double temp_factor = 1.0 + 0.001 * (temperature - 200.0);

    return growth_per_cycle * temp_factor * 1e-4; // Convert to μm
}

int EnhancedDepositionPhysics::calculateALDCycles(
    double target_thickness,
    double growth_per_cycle) const {

    return static_cast<int>(std::ceil(target_thickness / growth_per_cycle));
}

double EnhancedDepositionPhysics::calculateSputteringYield(
    MaterialType target_material,
    double ion_energy,
    double angle) const {

    // Simplified sputtering yield model
    double threshold_energy = 25.0; // eV

    if (ion_energy < threshold_energy) {
        return 0.0;
    }

    // Linear dependence above threshold
    double yield = 0.1 * (ion_energy - threshold_energy) / 100.0;

    // Angular dependence
    double angle_factor = 1.0 / std::cos(angle * M_PI / 180.0);

    return yield * angle_factor;
}

double EnhancedDepositionPhysics::calculateEvaporationRate(
    MaterialType material,
    double temperature,
    double pressure) const {

    auto material_props = getMaterialProperties(material);

    // Simplified evaporation rate (Hertz-Knudsen equation)
    double vapor_pressure = 1e-6 * std::exp(-material_props.melting_point / (temperature + 273.15));
    double rate = vapor_pressure / std::sqrt(temperature + 273.15);

    return rate * 0.001; // Convert to μm/min
}

std::vector<double> EnhancedDepositionPhysics::calculateThicknessProfile(
    std::shared_ptr<WaferEnhanced> wafer,
    const DepositionConditions& conditions,
    double base_thickness) {

    auto grid = wafer->getGrid();
    int rows = grid.rows();
    int cols = grid.cols();

    std::vector<double> profile(rows * cols, base_thickness);

    // Add small random variations (±5%)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dis(1.0, 0.05);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            profile[index] = base_thickness * dis(gen);
        }
    }

    return profile;
}

double EnhancedDepositionPhysics::calculateUniformity(
    const std::vector<double>& thickness_profile) const {

    if (thickness_profile.empty()) {
        return 0.0;
    }

    double mean = 0.0;
    for (double thickness : thickness_profile) {
        mean += thickness;
    }
    mean /= thickness_profile.size();

    double variance = 0.0;
    for (double thickness : thickness_profile) {
        variance += (thickness - mean) * (thickness - mean);
    }
    variance /= thickness_profile.size();

    double sigma = std::sqrt(variance);

    return (1.0 - sigma / mean) * 100.0; // Uniformity percentage
}

MaterialProperties EnhancedDepositionPhysics::getMaterialProperties(MaterialType material) const {
    auto it = material_database_.find(material);
    if (it == material_database_.end()) {
        throw PhysicsException("Unknown material type");
    }
    return it->second;
}

std::string EnhancedDepositionPhysics::techniqueToString(DepositionTechnique technique) const {
    auto it = technique_names_.find(technique);
    return (it != technique_names_.end()) ? it->second : "Unknown";
}

std::string EnhancedDepositionPhysics::materialToString(MaterialType material) const {
    auto props = getMaterialProperties(material);
    return props.name;
}

bool EnhancedDepositionPhysics::validateConditions(
    const DepositionConditions& conditions,
    std::string& error_message) const {

    if (conditions.temperature < 0.0 || conditions.temperature > 2000.0) {
        error_message = "Temperature out of range (0-2000°C)";
        return false;
    }

    if (conditions.target_thickness <= 0.0 || conditions.target_thickness > 100.0) {
        error_message = "Thickness out of range (0-100 μm)";
        return false;
    }

    if (conditions.pressure <= 0.0 || conditions.pressure > 1000.0) {
        error_message = "Pressure out of range (0-1000 Torr)";
        return false;
    }

    return true;
}

std::unordered_map<std::string, double> EnhancedDepositionPhysics::analyzeDepositionQuality(
    const DepositionResults& results,
    const DepositionConditions& conditions) const {

    std::unordered_map<std::string, double> quality;

    // Uniformity score
    quality["uniformity"] = results.uniformity;

    // Step coverage score
    quality["step_coverage"] = results.step_coverage * 100.0;

    // Conformality score
    quality["conformality"] = results.conformality * 100.0;

    // Overall quality score
    double overall = (results.uniformity + results.step_coverage * 100.0 + results.conformality * 100.0) / 3.0;
    quality["overall_score"] = overall;

    return quality;
}

double EnhancedDepositionPhysics::calculateGrainSize(
    MaterialType material,
    double temperature,
    double thickness) const {

    // Simplified grain size model
    double base_grain_size = 10.0; // nm

    // Temperature dependence
    double temp_factor = 1.0 + (temperature - 400.0) / 1000.0;

    // Thickness dependence
    double thickness_factor = std::sqrt(thickness * 1000.0); // Convert μm to nm

    return base_grain_size * temp_factor * thickness_factor;
}

double EnhancedDepositionPhysics::calculateIntrinsicStress(
    MaterialType material,
    const DepositionConditions& conditions) const {

    auto material_props = getMaterialProperties(material);
    double base_stress = material_props.stress_intrinsic;

    // Temperature dependence
    double temp_factor = 1.0 - (conditions.temperature - 400.0) / 1000.0;

    return base_stress * temp_factor;
}

double EnhancedDepositionPhysics::calculateSurfaceRoughness(
    MaterialType material,
    const DepositionConditions& conditions,
    double thickness) const {

    // Base roughness depends on technique
    double base_roughness = 1.0; // nm

    switch (conditions.technique) {
        case DepositionTechnique::ALD:
            base_roughness = 0.1; // Very smooth
            break;
        case DepositionTechnique::CVD:
            base_roughness = 0.5;
            break;
        case DepositionTechnique::PVD_SPUTTERING:
            base_roughness = 2.0;
            break;
        default:
            base_roughness = 1.0;
    }

    // Thickness dependence
    double thickness_factor = std::sqrt(thickness * 1000.0); // Convert μm to nm

    return base_roughness * thickness_factor * 0.1;
}

} // namespace SemiPRO

#include "enhanced_etching.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace SemiPRO {

EnhancedEtchingPhysics::EnhancedEtchingPhysics() {
    initializeEtchRateDatabase();
    initializeTechniqueNames();
    
    // Only log once per process (static flag)
    static bool logged = false;
    if (!logged) {
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Enhanced Etching Physics initialized", "EnhancedEtching");
        logged = true;
    }
}

EtchingResults EnhancedEtchingPhysics::simulateEtching(
    std::shared_ptr<WaferEnhanced> wafer,
    const EtchingConditions& conditions) {
    
    SEMIPRO_PERF_TIMER("etching_simulation", "EnhancedEtching");
    
    EtchingResults results;
    
    try {
        // Validate input conditions
        std::string error_msg;
        if (!validateConditions(conditions, error_msg)) {
            throw PhysicsException("Invalid etching conditions: " + error_msg);
        }
        
        // Route to technique-specific simulation
        switch (conditions.technique) {
            case EtchingTechnique::RIE:
            case EtchingTechnique::DRIE:
            case EtchingTechnique::ICP:
            case EtchingTechnique::CCP:
                results = simulatePlasmaEtching(wafer, conditions);
                break;
                
            case EtchingTechnique::WET_CHEMICAL:
            case EtchingTechnique::TMAH:
            case EtchingTechnique::KOH:
                results = simulateWetEtching(wafer, conditions);
                break;
                
            case EtchingTechnique::IBE:
            case EtchingTechnique::CAIBE:
                results = simulateIonBeamEtching(wafer, conditions);
                break;
                
            default:
                // Generic etching model
                results.etch_rate = calculateEtchRate(conditions);
                results.final_depth = conditions.target_depth;
                break;
        }
        
        // Calculate common properties
        results.selectivity = calculateSelectivity(
            conditions.target_material, conditions.mask_material, conditions
        );
        
        results.anisotropy = calculateAnisotropy(conditions);
        results.sidewall_angle = calculateSidewallAngle(conditions);
        
        if (enable_surface_chemistry_) {
            results.surface_roughness = calculateSurfaceRoughness(
                conditions.target_material, conditions
            );
        }
        
        // Calculate profiles
        results.etch_profile = calculateEtchProfile(wafer, conditions, results.final_depth);
        results.sidewall_profile = calculateSidewallProfile(conditions, results.final_depth);
        
        // Calculate uniformity
        results.uniformity = calculateUniformity(results.etch_profile);
        
        // Analyze quality
        results.quality_metrics = analyzeEtchingQuality(results, conditions);
        
        // Apply results to wafer (etching removes material)
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Use spatial variation if available
                double local_depth = results.final_depth;
                if (!results.etch_profile.empty()) {
                    int index = i * cols + j;
                    if (index < results.etch_profile.size()) {
                        local_depth = results.etch_profile[index];
                    }
                }
                
                // Etch removes material
                if (grid(i, j) > local_depth) {
                    grid(i, j) -= local_depth;
                } else {
                    grid(i, j) = 0.0; // Completely etched
                }
            }
        }
        
        wafer->setGrid(grid);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Enhanced etching completed: " + 
                          std::to_string(results.final_depth) + " μm " +
                          materialToString(conditions.target_material) + " etched",
                          "EnhancedEtching");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced etching failed: " + std::string(e.what()),
                          "EnhancedEtching");
        throw;
    }
    
    return results;
}

EtchingResults EnhancedEtchingPhysics::simulatePlasmaEtching(
    std::shared_ptr<WaferEnhanced> wafer,
    const EtchingConditions& conditions) {
    
    EtchingResults results;
    
    // Calculate plasma parameters
    double ion_energy = calculateIonEnergy(conditions.bias_voltage, conditions.pressure);
    double ion_flux = calculateIonFlux(conditions.power, conditions.pressure);
    double radical_density = calculateRadicalDensity(conditions.chemistry, conditions.power, conditions.pressure);
    
    // Base etch rate calculation
    double base_rate = calculateEtchRate(conditions);
    
    // Ion bombardment enhancement
    double ion_enhancement = 1.0;
    if (enable_ion_bombardment_) {
        ion_enhancement = 1.0 + 0.1 * std::sqrt(ion_energy / 100.0); // Normalize to 100 eV
    }
    
    // Radical density effect
    double radical_factor = std::sqrt(radical_density / 1e15); // Normalize to 1e15 cm⁻³
    
    results.etch_rate = base_rate * ion_enhancement * radical_factor;
    results.final_depth = conditions.target_depth;
    
    // Plasma etching typically has good anisotropy
    results.anisotropy = 0.8 + 0.15 * (conditions.bias_voltage / 200.0); // Better at higher bias
    results.anisotropy = std::min(1.0, results.anisotropy);
    
    // Calculate mask erosion
    double mask_etch_rate = results.etch_rate / results.selectivity;
    results.mask_erosion = mask_etch_rate * (results.final_depth / results.etch_rate);
    
    return results;
}

EtchingResults EnhancedEtchingPhysics::simulateWetEtching(
    std::shared_ptr<WaferEnhanced> wafer,
    const EtchingConditions& conditions) {
    
    EtchingResults results;
    
    // Wet etching is typically isotropic
    results.etch_rate = calculateEtchRate(conditions);
    results.final_depth = conditions.target_depth;
    results.anisotropy = 0.1; // Very isotropic
    results.sidewall_angle = 54.7; // Typical for <111> planes in silicon
    
    // Wet etching can have excellent selectivity
    results.selectivity = calculateSelectivity(
        conditions.target_material, conditions.mask_material, conditions
    );
    
    // Temperature dependence for wet etching
    double temp_factor = std::exp(-0.5 * (1.0/(conditions.temperature + 273.15) - 1.0/298.15));
    results.etch_rate *= temp_factor;
    
    return results;
}

EtchingResults EnhancedEtchingPhysics::simulateIonBeamEtching(
    std::shared_ptr<WaferEnhanced> wafer,
    const EtchingConditions& conditions) {
    
    EtchingResults results;
    
    // Ion beam etching is purely physical
    double sputtering_yield = calculateSputteringYield(conditions.target_material, 500.0); // 500 eV typical
    results.etch_rate = sputtering_yield * 0.01; // Convert to μm/min
    results.final_depth = conditions.target_depth;
    
    // IBE has excellent anisotropy but poor selectivity
    results.anisotropy = 0.95; // Excellent anisotropy
    results.selectivity = 2.0; // Poor selectivity (physical process)
    
    return results;
}

double EnhancedEtchingPhysics::calculateEtchRate(
    const EtchingConditions& conditions) const {
    
    // Base etch rates (μm/min) for different material/chemistry combinations
    double base_rate = 0.1; // Default
    
    auto material_it = etch_rates_.find(conditions.target_material);
    if (material_it != etch_rates_.end()) {
        auto chemistry_it = material_it->second.find(conditions.chemistry);
        if (chemistry_it != material_it->second.end()) {
            base_rate = chemistry_it->second;
        }
    }
    
    // Power dependence
    double power_factor = std::sqrt(conditions.power / 100.0); // Normalize to 100W
    
    // Pressure dependence
    double pressure_factor = std::sqrt(10.0 / conditions.pressure); // Inverse relationship
    
    return base_rate * power_factor * pressure_factor;
}

double EnhancedEtchingPhysics::calculateSelectivity(
    EtchMaterial target,
    EtchMaterial mask,
    const EtchingConditions& conditions) const {
    
    // Base selectivity from database
    double base_selectivity = 10.0; // Default
    
    // Simple selectivity lookup (would be more complex in real implementation)
    if (target == EtchMaterial::SILICON && mask == EtchMaterial::SILICON_DIOXIDE) {
        if (conditions.chemistry == EtchChemistry::FLUORINE_BASED) {
            base_selectivity = 50.0; // Excellent selectivity
        }
    } else if (target == EtchMaterial::SILICON_DIOXIDE && mask == EtchMaterial::PHOTORESIST) {
        if (conditions.chemistry == EtchChemistry::FLUORINE_BASED) {
            base_selectivity = 5.0; // Moderate selectivity
        }
    }
    
    // Temperature dependence
    double temp_factor = 1.0 + 0.01 * (conditions.temperature - 25.0);
    
    return base_selectivity * temp_factor;
}

double EnhancedEtchingPhysics::calculateAnisotropy(
    const EtchingConditions& conditions) const {
    
    double base_anisotropy = 0.5;
    
    switch (conditions.technique) {
        case EtchingTechnique::RIE:
        case EtchingTechnique::DRIE:
            base_anisotropy = 0.8; // Good anisotropy
            break;
        case EtchingTechnique::ICP:
            base_anisotropy = 0.9; // Excellent anisotropy
            break;
        case EtchingTechnique::WET_CHEMICAL:
            base_anisotropy = 0.1; // Poor anisotropy (isotropic)
            break;
        case EtchingTechnique::IBE:
            base_anisotropy = 0.95; // Excellent anisotropy
            break;
        default:
            base_anisotropy = 0.5;
    }
    
    // Bias voltage enhances anisotropy
    double bias_factor = 1.0 + 0.002 * conditions.bias_voltage;
    
    return std::min(1.0, base_anisotropy * bias_factor);
}

void EnhancedEtchingPhysics::initializeEtchRateDatabase() {
    // Silicon etch rates (μm/min)
    etch_rates_[EtchMaterial::SILICON][EtchChemistry::FLUORINE_BASED] = 0.5;
    etch_rates_[EtchMaterial::SILICON][EtchChemistry::CHLORINE_BASED] = 0.3;
    etch_rates_[EtchMaterial::SILICON][EtchChemistry::BROMINE_BASED] = 0.2;
    
    // Silicon dioxide etch rates
    etch_rates_[EtchMaterial::SILICON_DIOXIDE][EtchChemistry::FLUORINE_BASED] = 0.1;
    etch_rates_[EtchMaterial::SILICON_DIOXIDE][EtchChemistry::WET_ACID] = 0.05;
    
    // Silicon nitride etch rates
    etch_rates_[EtchMaterial::SILICON_NITRIDE][EtchChemistry::FLUORINE_BASED] = 0.02;
    etch_rates_[EtchMaterial::SILICON_NITRIDE][EtchChemistry::WET_ACID] = 0.001;
    
    // Aluminum etch rates
    etch_rates_[EtchMaterial::ALUMINUM][EtchChemistry::CHLORINE_BASED] = 0.2;
    etch_rates_[EtchMaterial::ALUMINUM][EtchChemistry::WET_ACID] = 0.1;
    
    // Photoresist etch rates
    etch_rates_[EtchMaterial::PHOTORESIST][EtchChemistry::OXYGEN_BASED] = 0.3;
    etch_rates_[EtchMaterial::PHOTORESIST][EtchChemistry::FLUORINE_BASED] = 0.01;
}

void EnhancedEtchingPhysics::initializeTechniqueNames() {
    technique_names_[EtchingTechnique::WET_CHEMICAL] = "Wet Chemical";
    technique_names_[EtchingTechnique::RIE] = "RIE";
    technique_names_[EtchingTechnique::DRIE] = "DRIE";
    technique_names_[EtchingTechnique::ICP] = "ICP";
    technique_names_[EtchingTechnique::CCP] = "CCP";
    technique_names_[EtchingTechnique::IBE] = "IBE";
    technique_names_[EtchingTechnique::CAIBE] = "CAIBE";
    technique_names_[EtchingTechnique::XeF2] = "XeF2";
    technique_names_[EtchingTechnique::TMAH] = "TMAH";
    technique_names_[EtchingTechnique::KOH] = "KOH";
}

double EnhancedEtchingPhysics::calculateIonEnergy(
    double bias_voltage,
    double pressure) const {

    // Ion energy in plasma (simplified model)
    double sheath_voltage = bias_voltage * 0.8; // Voltage drop across sheath
    double collision_factor = 1.0 / (1.0 + pressure / 10.0); // Collisional losses

    return sheath_voltage * collision_factor;
}

double EnhancedEtchingPhysics::calculateIonFlux(
    double power,
    double pressure,
    double area) const {

    // Ion flux calculation (simplified)
    double plasma_density = power * 1e10 / (pressure + 1.0); // cm⁻³
    double bohm_velocity = 1e5; // cm/s (typical)

    return plasma_density * bohm_velocity / 4.0; // cm⁻²s⁻¹
}

double EnhancedEtchingPhysics::calculateRadicalDensity(
    EtchChemistry chemistry,
    double power,
    double pressure) const {

    // Radical density calculation
    double base_density = 1e15; // cm⁻³
    double power_factor = std::sqrt(power / 100.0);
    double pressure_factor = std::sqrt(pressure / 10.0);

    return base_density * power_factor * pressure_factor;
}

std::vector<double> EnhancedEtchingPhysics::calculateEtchProfile(
    std::shared_ptr<WaferEnhanced> wafer,
    const EtchingConditions& conditions,
    double base_depth) {

    auto grid = wafer->getGrid();
    int rows = grid.rows();
    int cols = grid.cols();

    std::vector<double> profile(rows * cols, base_depth);

    // Add small random variations (±3%)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dis(1.0, 0.03);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int index = i * cols + j;
            profile[index] = base_depth * dis(gen);
        }
    }

    return profile;
}

std::vector<double> EnhancedEtchingPhysics::calculateSidewallProfile(
    const EtchingConditions& conditions,
    double depth) {

    std::vector<double> profile;
    int num_points = 100;

    for (int i = 0; i < num_points; ++i) {
        double z = (static_cast<double>(i) / (num_points - 1)) * depth;

        // Sidewall angle calculation
        double angle_rad = calculateSidewallAngle(conditions) * M_PI / 180.0;
        double lateral_position = z * std::tan(M_PI/2.0 - angle_rad);

        profile.push_back(lateral_position);
    }

    return profile;
}

double EnhancedEtchingPhysics::calculateSidewallAngle(
    const EtchingConditions& conditions) const {

    double base_angle = 90.0; // Vertical

    // Anisotropy affects sidewall angle
    double anisotropy = calculateAnisotropy(conditions);
    base_angle = 90.0 * anisotropy + 45.0 * (1.0 - anisotropy);

    return base_angle;
}

double EnhancedEtchingPhysics::calculateSputteringYield(
    EtchMaterial material,
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

double EnhancedEtchingPhysics::calculateUniformity(
    const std::vector<double>& etch_profile) const {

    if (etch_profile.empty()) {
        return 0.0;
    }

    double mean = 0.0;
    for (double depth : etch_profile) {
        mean += depth;
    }
    mean /= etch_profile.size();

    double variance = 0.0;
    for (double depth : etch_profile) {
        variance += (depth - mean) * (depth - mean);
    }
    variance /= etch_profile.size();

    double sigma = std::sqrt(variance);

    return (1.0 - sigma / mean) * 100.0; // Uniformity percentage
}

double EnhancedEtchingPhysics::calculateSurfaceRoughness(
    EtchMaterial material,
    const EtchingConditions& conditions) const {

    // Base roughness depends on technique
    double base_roughness = 1.0; // nm

    switch (conditions.technique) {
        case EtchingTechnique::WET_CHEMICAL:
            base_roughness = 0.1; // Very smooth
            break;
        case EtchingTechnique::RIE:
            base_roughness = 2.0;
            break;
        case EtchingTechnique::IBE:
            base_roughness = 5.0; // Rough due to sputtering
            break;
        default:
            base_roughness = 1.0;
    }

    // Ion energy affects roughness
    double ion_energy = calculateIonEnergy(conditions.bias_voltage, conditions.pressure);
    double energy_factor = 1.0 + ion_energy / 1000.0;

    return base_roughness * energy_factor;
}

std::string EnhancedEtchingPhysics::techniqueToString(EtchingTechnique technique) const {
    auto it = technique_names_.find(technique);
    return (it != technique_names_.end()) ? it->second : "Unknown";
}

std::string EnhancedEtchingPhysics::materialToString(EtchMaterial material) const {
    switch (material) {
        case EtchMaterial::SILICON: return "Silicon";
        case EtchMaterial::SILICON_DIOXIDE: return "Silicon Dioxide";
        case EtchMaterial::SILICON_NITRIDE: return "Silicon Nitride";
        case EtchMaterial::POLYSILICON: return "Polysilicon";
        case EtchMaterial::ALUMINUM: return "Aluminum";
        case EtchMaterial::COPPER: return "Copper";
        case EtchMaterial::TUNGSTEN: return "Tungsten";
        case EtchMaterial::PHOTORESIST: return "Photoresist";
        default: return "Unknown";
    }
}

bool EnhancedEtchingPhysics::validateConditions(
    const EtchingConditions& conditions,
    std::string& error_message) const {

    if (conditions.target_depth <= 0.0 || conditions.target_depth > 1000.0) {
        error_message = "Target depth out of range (0-1000 μm)";
        return false;
    }

    if (conditions.pressure <= 0.0 || conditions.pressure > 1000.0) {
        error_message = "Pressure out of range (0-1000 mTorr)";
        return false;
    }

    if (conditions.power < 0.0 || conditions.power > 10000.0) {
        error_message = "Power out of range (0-10000 W)";
        return false;
    }

    return true;
}

std::unordered_map<std::string, double> EnhancedEtchingPhysics::analyzeEtchingQuality(
    const EtchingResults& results,
    const EtchingConditions& conditions) const {

    std::unordered_map<std::string, double> quality;

    // Uniformity score
    quality["uniformity"] = results.uniformity;

    // Anisotropy score
    quality["anisotropy"] = results.anisotropy * 100.0;

    // Selectivity score (normalized)
    double selectivity_score = std::min(100.0, results.selectivity / conditions.selectivity_target * 100.0);
    quality["selectivity"] = selectivity_score;

    // Overall quality score
    double overall = (results.uniformity + results.anisotropy * 100.0 + selectivity_score) / 3.0;
    quality["overall_score"] = overall;

    return quality;
}



} // namespace SemiPRO

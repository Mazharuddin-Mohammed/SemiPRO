#include "enhanced_doping.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace SemiPRO {

EnhancedDopingPhysics::EnhancedDopingPhysics() {
    initializeIonDatabase();
    initializeChannelingFactors();

    // Only log once per process (static flag)
    static bool logged = false;
    if (!logged) {
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Enhanced Doping Physics initialized", "EnhancedDoping");
        logged = true;
    }
}

ImplantationResults EnhancedDopingPhysics::simulateIonImplantation(
    std::shared_ptr<WaferEnhanced> wafer,
    const ImplantationConditions& conditions) {
    
    SEMIPRO_PERF_TIMER("ion_implantation_simulation", "EnhancedDoping");
    
    ImplantationResults results;
    
    try {
        // Validate input conditions
        std::string error_msg;
        if (!validateImplantationConditions(conditions, error_msg)) {
            throw PhysicsException("Invalid implantation conditions: " + error_msg);
        }
        
        // Calculate basic LSS parameters
        results.projected_range = calculateProjectedRange(conditions);
        results.range_straggling = calculateRangeStraggling(conditions, results.projected_range);
        results.lateral_straggling = calculateLateralStraggling(conditions, results.projected_range);
        
        // Calculate channeling effects
        if (enable_channeling_effects_) {
            results.channeling_fraction = calculateChannelingFraction(conditions);
        }
        
        // Calculate sputtering effects
        if (enable_sputtering_) {
            results.sputtering_yield = calculateSputteringYield(conditions);
        }
        
        // Generate depth grid for calculations
        int num_points = 1000;
        double max_depth = results.projected_range + 5.0 * results.range_straggling;
        std::vector<double> depths(num_points);
        for (int i = 0; i < num_points; ++i) {
            depths[i] = (static_cast<double>(i) / (num_points - 1)) * max_depth;
        }
        
        // Calculate concentration profile
        results.concentration_profile = calculateConcentrationProfile(
            conditions, depths, enable_channeling_effects_
        );
        
        // Calculate damage profile
        if (enable_damage_modeling_) {
            results.damage_profile = calculateDamageProfile(conditions, depths);
            results.amorphous_regions = calculateAmorphousRegions(
                results.damage_profile, depths
            );
        }
        
        // Calculate electrical properties
        results.peak_concentration = calculatePeakConcentration(conditions);
        results.sheet_resistance = calculateSheetResistance(
            results.concentration_profile, depths, conditions.species
        );
        results.junction_depth = calculateJunctionDepth(
            results.concentration_profile, depths
        );
        
        // Analyze quality
        results.quality_metrics = analyzeImplantationQuality(results, conditions);
        
        // Apply results to wafer
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        // Map concentration profile to wafer grid
        double wafer_thickness = 10.0; // μm (assumed)
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                double depth = (static_cast<double>(i) / rows) * wafer_thickness;
                
                // Interpolate concentration at this depth
                double concentration = 0.0;
                if (depth <= max_depth) {
                    int index = static_cast<int>((depth / max_depth) * (num_points - 1));
                    if (index < results.concentration_profile.size()) {
                        concentration = results.concentration_profile[index];
                    }
                }
                
                grid(i, j) += concentration * 1e-15; // Scale for grid representation
            }
        }
        
        wafer->setGrid(grid);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::PHYSICS,
                          "Enhanced ion implantation completed: " + 
                          ionSpeciesToString(conditions.species) + 
                          ", Range: " + std::to_string(results.projected_range) + " μm" +
                          ", Peak: " + std::to_string(results.peak_concentration) + " cm⁻³",
                          "EnhancedDoping");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::PHYSICS,
                          "Enhanced ion implantation failed: " + std::string(e.what()),
                          "EnhancedDoping");
        throw;
    }
    
    return results;
}

double EnhancedDopingPhysics::calculateProjectedRange(
    const ImplantationConditions& conditions) const {
    
    auto ion_props = getIonProperties(conditions.species);
    
    // Target: Silicon properties
    double target_z = 14.0;
    double target_mass = 28.09;
    double target_density = 2.33; // g/cm³
    
    // Calculate reduced energy
    double epsilon = calculateReducedEnergy(conditions);
    
    // LSS range calculation (Biersack-Haggmark formula)
    double reduced_range;
    if (epsilon < 10.0) {
        reduced_range = epsilon / (1.0 + 6.35 * std::sqrt(epsilon) + 
                                  epsilon * (6.882 * std::sqrt(epsilon) - 1.708));
    } else {
        reduced_range = std::log(epsilon) / (2.0 * std::log(epsilon / 10.0));
    }
    
    // Convert to physical range in μm
    double range = 8.74e-3 * reduced_range * (ion_props.atomic_mass + target_mass) /
                  (target_density * target_z * ion_props.atomic_mass *
                   std::pow(ion_props.atomic_number + target_z, 1.0/3.0)); // μm
    
    return range;
}

double EnhancedDopingPhysics::calculateRangeStraggling(
    const ImplantationConditions& conditions,
    double projected_range) const {
    
    // Bohr straggling model with improvements
    double base_straggling = 0.42 * projected_range;
    
    // Energy dependence
    double energy_factor = 1.0 + 0.1 * std::log(conditions.energy / 50.0);
    
    // Mass dependence
    auto ion_props = getIonProperties(conditions.species);
    double mass_factor = std::sqrt(ion_props.atomic_mass / 11.0); // Normalized to boron
    
    return base_straggling * energy_factor * mass_factor;
}

double EnhancedDopingPhysics::calculateLateralStraggling(
    const ImplantationConditions& conditions,
    double projected_range) const {
    
    // Lateral straggling is typically 0.6-0.8 times the longitudinal straggling
    double longitudinal_straggling = calculateRangeStraggling(conditions, projected_range);
    return 0.7 * longitudinal_straggling;
}

double EnhancedDopingPhysics::calculateChannelingFraction(
    const ImplantationConditions& conditions) const {
    
    if (conditions.channeling == ChannelingDirection::RANDOM) {
        return 0.0;
    }
    
    // Get channeling factor for direction
    auto it = channeling_factors_.find(conditions.channeling);
    double base_fraction = (it != channeling_factors_.end()) ? it->second : 0.1;
    
    // Tilt angle reduces channeling
    double tilt_factor = std::exp(-std::pow(conditions.tilt_angle / 2.0, 2));
    
    // Energy dependence (higher energy = more channeling)
    double energy_factor = std::min(1.0, conditions.energy / 100.0);
    
    return base_fraction * tilt_factor * energy_factor;
}

std::vector<double> EnhancedDopingPhysics::calculateConcentrationProfile(
    const ImplantationConditions& conditions,
    const std::vector<double>& depths,
    bool include_channeling) const {
    
    std::vector<double> profile(depths.size(), 0.0);
    
    double rp = calculateProjectedRange(conditions);
    double delta_rp = calculateRangeStraggling(conditions, rp);
    
    // Gaussian distribution for random component (convert units properly)
    double delta_rp_cm = delta_rp * 1e-4; // Convert μm to cm
    double normalization = conditions.dose / (delta_rp_cm * std::sqrt(2.0 * M_PI));
    
    for (size_t i = 0; i < depths.size(); ++i) {
        double depth = depths[i];
        
        // Random component (Gaussian) - depth in μm, convert for calculation
        double depth_cm = depth * 1e-4; // Convert μm to cm
        double rp_cm = rp * 1e-4; // Convert μm to cm
        double random_component = normalization *
            std::exp(-std::pow(depth_cm - rp_cm, 2) / (2.0 * delta_rp_cm * delta_rp_cm));
        
        profile[i] = random_component;
        
        // Add channeling component if enabled
        if (include_channeling && enable_channeling_effects_) {
            double channeling_fraction = calculateChannelingFraction(conditions);
            if (channeling_fraction > 0) {
                // Channeled ions penetrate deeper with exponential tail
                double channel_range = rp * (1.0 + 2.0 * channeling_fraction);
                double channel_component = (conditions.dose * channeling_fraction) / channel_range *
                    std::exp(-depth / channel_range);
                
                profile[i] += channel_component;
            }
        }
    }
    
    return profile;
}

double EnhancedDopingPhysics::calculatePeakConcentration(
    const ImplantationConditions& conditions) const {

    double rp = calculateProjectedRange(conditions);
    double delta_rp = calculateRangeStraggling(conditions, rp);

    // Peak concentration at projected range (proper unit conversion)
    // dose is in cm⁻², delta_rp is in μm, need to convert to cm
    double delta_rp_cm = delta_rp * 1e-4; // Convert μm to cm
    double peak_conc = conditions.dose / (delta_rp_cm * std::sqrt(2.0 * M_PI));

    // Ensure reasonable limits (typical semiconductor doping: 1e15 - 1e21 cm⁻³)
    return std::min(peak_conc, 1e21);
}

double EnhancedDopingPhysics::calculateSheetResistance(
    const std::vector<double>& concentration_profile,
    const std::vector<double>& depths,
    IonSpecies species,
    double temperature) const {
    
    if (concentration_profile.empty() || depths.empty()) {
        return 1e6; // Very high resistance for empty profile
    }
    
    auto ion_props = getIonProperties(species);
    
    // Calculate sheet conductance by integrating
    double sheet_conductance = 0.0;
    double q = 1.602e-19; // Elementary charge (C)
    
    for (size_t i = 0; i < concentration_profile.size() - 1; ++i) {
        double conc = concentration_profile[i] * 1e6; // Convert cm⁻³ to m⁻³
        double mobility = DopingUtils::calculateMobility(species, conc / 1e6, temperature);
        mobility *= 1e-4; // Convert cm²/V·s to m²/V·s
        
        double conductivity = q * conc * mobility;
        double layer_thickness = (depths[i+1] - depths[i]) * 1e-6; // Convert μm to m
        
        sheet_conductance += conductivity * layer_thickness;
    }
    
    return (sheet_conductance > 0) ? 1.0 / sheet_conductance : 1e6;
}

double EnhancedDopingPhysics::calculateJunctionDepth(
    const std::vector<double>& concentration_profile,
    const std::vector<double>& depths,
    double background_doping) const {
    
    // Find depth where concentration equals background doping
    for (size_t i = 0; i < concentration_profile.size(); ++i) {
        if (concentration_profile[i] <= background_doping) {
            return depths[i];
        }
    }
    
    // If not found, return maximum depth
    return depths.empty() ? 0.0 : depths.back();
}

void EnhancedDopingPhysics::initializeIonDatabase() {
    // Boron-11
    IonProperties boron;
    boron.symbol = "B";
    boron.atomic_mass = 10.81;
    boron.atomic_number = 5;
    boron.binding_energy = 8.3;
    boron.is_p_type = true;
    boron.diffusivity_d0 = 0.76;      // cm²/s
    boron.diffusion_ea = 3.46;        // eV
    boron.solubility_limit = 5e20;    // cm⁻³
    ion_database_[IonSpecies::BORON_11] = boron;
    
    // Phosphorus-31
    IonProperties phosphorus;
    phosphorus.symbol = "P";
    phosphorus.atomic_mass = 30.97;
    phosphorus.atomic_number = 15;
    phosphorus.binding_energy = 10.5;
    phosphorus.is_p_type = false;
    phosphorus.diffusivity_d0 = 3.85;     // cm²/s
    phosphorus.diffusion_ea = 3.66;       // eV
    phosphorus.solubility_limit = 1e21;   // cm⁻³
    ion_database_[IonSpecies::PHOSPHORUS_31] = phosphorus;
    
    // Arsenic-75
    IonProperties arsenic;
    arsenic.symbol = "As";
    arsenic.atomic_mass = 74.92;
    arsenic.atomic_number = 33;
    arsenic.binding_energy = 9.8;
    arsenic.is_p_type = false;
    arsenic.diffusivity_d0 = 0.066;       // cm²/s
    arsenic.diffusion_ea = 4.05;          // eV
    arsenic.solubility_limit = 2e21;      // cm⁻³
    ion_database_[IonSpecies::ARSENIC_75] = arsenic;
}

void EnhancedDopingPhysics::initializeChannelingFactors() {
    channeling_factors_[ChannelingDirection::RANDOM] = 0.0;
    channeling_factors_[ChannelingDirection::CHANNEL_100] = 0.3;
    channeling_factors_[ChannelingDirection::CHANNEL_110] = 0.2;
    channeling_factors_[ChannelingDirection::CHANNEL_111] = 0.4;
    channeling_factors_[ChannelingDirection::PLANAR_100] = 0.15;
    channeling_factors_[ChannelingDirection::PLANAR_110] = 0.1;
    channeling_factors_[ChannelingDirection::PLANAR_111] = 0.2;
}

double EnhancedDopingPhysics::calculateReducedEnergy(
    const ImplantationConditions& conditions) const {
    
    auto ion_props = getIonProperties(conditions.species);
    double target_z = 14.0;
    double target_mass = 28.09;
    
    return 32.5 * target_mass * conditions.energy /
           (ion_props.atomic_number * target_z * (ion_props.atomic_mass + target_mass) *
            std::pow(ion_props.atomic_number + target_z, 2.0/3.0));
}

IonProperties EnhancedDopingPhysics::getIonProperties(IonSpecies species) const {
    auto it = ion_database_.find(species);
    if (it == ion_database_.end()) {
        throw PhysicsException("Unknown ion species");
    }
    return it->second;
}

bool EnhancedDopingPhysics::validateImplantationConditions(
    const ImplantationConditions& conditions,
    std::string& error_message) const {
    
    if (conditions.energy <= 0 || conditions.energy > 1000.0) {
        error_message = "Energy out of range (0-1000 keV)";
        return false;
    }
    
    if (conditions.dose <= 0 || conditions.dose > 1e18) {
        error_message = "Dose out of range (0-1e18 cm⁻²)";
        return false;
    }
    
    if (conditions.tilt_angle < 0 || conditions.tilt_angle > 45.0) {
        error_message = "Tilt angle out of range (0-45°)";
        return false;
    }
    
    return true;
}

std::string EnhancedDopingPhysics::ionSpeciesToString(IonSpecies species) const {
    auto props = getIonProperties(species);
    return props.symbol;
}

// DopingUtils implementation
namespace DopingUtils {
    double calculateMobility(IonSpecies species, double concentration, double temperature) {
        // Simplified mobility model (Caughey-Thomas)
        double mu_min, mu_max, n_ref, alpha;
        
        auto props = EnhancedDopingPhysics().getIonProperties(species);
        
        if (props.is_p_type) {
            // Hole mobility parameters
            mu_min = 44.9;
            mu_max = 470.5;
            n_ref = 2.23e17;
            alpha = 0.719;
        } else {
            // Electron mobility parameters  
            mu_min = 68.5;
            mu_max = 1414.0;
            n_ref = 9.20e16;
            alpha = 0.711;
        }
        
        // Temperature dependence
        double temp_factor = std::pow(temperature / 300.0, -2.3);
        
        return (mu_min + (mu_max - mu_min) / (1.0 + std::pow(concentration / n_ref, alpha))) * temp_factor;
    }
}

std::vector<double> EnhancedDopingPhysics::calculateDamageProfile(
    const ImplantationConditions& conditions,
    const std::vector<double>& depths) const {

    std::vector<double> damage(depths.size(), 0.0);

    auto ion_props = getIonProperties(conditions.species);
    double rp = calculateProjectedRange(conditions);

    // Simplified damage model based on nuclear stopping power
    for (size_t i = 0; i < depths.size(); ++i) {
        double depth = depths[i];

        // Damage peaks before projected range
        double damage_peak_depth = rp * 0.6;
        double damage_width = rp * 0.3;

        // Gaussian-like damage distribution
        double damage_density = conditions.dose *
            std::exp(-std::pow(depth - damage_peak_depth, 2) / (2.0 * damage_width * damage_width));

        // Scale by atomic number (heavier ions cause more damage)
        damage_density *= ion_props.atomic_number / 5.0; // Normalized to boron

        damage[i] = damage_density;
    }

    return damage;
}

std::vector<double> EnhancedDopingPhysics::calculateAmorphousRegions(
    const std::vector<double>& damage_profile,
    const std::vector<double>& depths) const {

    std::vector<double> amorphous_regions;

    // Critical damage density for amorphization (simplified)
    double critical_damage = 1e22; // cm⁻³

    bool in_amorphous = false;
    double start_depth = 0.0;

    for (size_t i = 0; i < damage_profile.size(); ++i) {
        if (damage_profile[i] > critical_damage && !in_amorphous) {
            // Start of amorphous region
            start_depth = depths[i];
            in_amorphous = true;
        } else if (damage_profile[i] <= critical_damage && in_amorphous) {
            // End of amorphous region
            amorphous_regions.push_back(start_depth);
            amorphous_regions.push_back(depths[i]);
            in_amorphous = false;
        }
    }

    // Close region if it extends to the end
    if (in_amorphous) {
        amorphous_regions.push_back(start_depth);
        amorphous_regions.push_back(depths.back());
    }

    return amorphous_regions;
}

double EnhancedDopingPhysics::calculateSputteringYield(
    const ImplantationConditions& conditions) const {

    if (!enable_sputtering_) {
        return 0.0;
    }

    auto ion_props = getIonProperties(conditions.species);

    // Simplified sputtering yield model
    double threshold_energy = 25.0; // eV (typical for Si)

    if (conditions.energy * 1000.0 < threshold_energy) {
        return 0.0;
    }

    // Linear dependence above threshold (simplified)
    double yield = 0.1 * (conditions.energy * 1000.0 - threshold_energy) / 1000.0;

    // Mass dependence
    yield *= std::sqrt(ion_props.atomic_mass / 28.09); // Normalized to Si

    return std::min(yield, 10.0); // Cap at reasonable value
}

std::unordered_map<std::string, double> EnhancedDopingPhysics::analyzeImplantationQuality(
    const ImplantationResults& results,
    const ImplantationConditions& conditions) const {

    std::unordered_map<std::string, double> quality;

    // Range accuracy (how close to expected)
    double expected_range = calculateProjectedRange(conditions);
    double range_accuracy = 1.0 - std::abs(results.projected_range - expected_range) / expected_range;
    quality["range_accuracy"] = std::max(0.0, range_accuracy);

    // Profile uniformity (based on straggling)
    double uniformity = 1.0 / (1.0 + results.range_straggling / results.projected_range);
    quality["uniformity"] = uniformity;

    // Channeling control
    double channeling_control = 1.0 - results.channeling_fraction;
    quality["channeling_control"] = channeling_control;

    // Overall quality score
    double overall = (range_accuracy + uniformity + channeling_control) / 3.0 * 100.0;
    quality["overall_score"] = overall;

    return quality;
}

std::string EnhancedDopingPhysics::channelingDirectionToString(ChannelingDirection direction) const {
    switch (direction) {
        case ChannelingDirection::RANDOM: return "Random";
        case ChannelingDirection::CHANNEL_100: return "<100> Channel";
        case ChannelingDirection::CHANNEL_110: return "<110> Channel";
        case ChannelingDirection::CHANNEL_111: return "<111> Channel";
        case ChannelingDirection::PLANAR_100: return "{100} Planar";
        case ChannelingDirection::PLANAR_110: return "{110} Planar";
        case ChannelingDirection::PLANAR_111: return "{111} Planar";
        default: return "Unknown";
    }
}

std::string EnhancedDopingPhysics::damageModelToString(DamageModel model) const {
    switch (model) {
        case DamageModel::KINCHIN_PEASE: return "Kinchin-Pease";
        case DamageModel::MARLOWE: return "MARLOWE";
        case DamageModel::TRIM_SRIM: return "TRIM/SRIM";
        case DamageModel::CRYSTAL_TRIM: return "Crystal-TRIM";
        case DamageModel::ADVANCED_BCA: return "Advanced BCA";
        default: return "Unknown";
    }
}

} // namespace SemiPRO

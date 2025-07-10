// Author: Dr. Mazharuddin Mohammed
#include "advanced_ion_implantation.hpp"
#include "../core/utils.hpp"
#include <cmath>
#include <random>
#include <algorithm>

AdvancedIonImplantation::AdvancedIonImplantation()
    : channeling_enabled_(true)
    , damage_accumulation_enabled_(true)
    , temperature_effects_enabled_(true)
    , substrate_orientation_("100")
{
    initializeIonDatabase();
    initializeMaterialDatabase();
    
    // Initialize metrics
    metrics_ = ImplantMetrics{};
    damage_profile_ = DamageProfile{};
    
    Logger::getInstance().log("Advanced Ion Implantation system initialized");
}

bool AdvancedIonImplantation::simulateImplantation(std::shared_ptr<Wafer> wafer, 
                                                  const ImplantParameters& params) {
    try {
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        Logger::getInstance().log("Starting advanced ion implantation simulation");
        Logger::getInstance().log("Ion: " + params.ion.symbol + ", Energy: " + 
                                 std::to_string(params.energy) + "keV, Dose: " + 
                                 std::to_string(params.dose) + "cm⁻²");
        
        // Calculate full LSS distribution
        Eigen::ArrayXXd ion_distribution = calculateFullLSSDistribution(params, rows, cols);
        
        // Apply channeling effects if enabled
        if (channeling_enabled_ && params.channeling_enabled) {
            ion_distribution = applyChannelingEffects(ion_distribution, params);
        }
        
        // Apply temperature effects
        if (temperature_effects_enabled_) {
            ion_distribution = calculateTemperatureEffects(ion_distribution, params.temperature);
        }
        
        // Calculate damage profile
        if (damage_accumulation_enabled_) {
            damage_profile_ = calculateDamageProfile(params, ion_distribution);
        }
        
        // Update wafer with final distribution
        auto current_grid = wafer->getGrid();
        wafer->setGrid(current_grid + ion_distribution);
        
        // Update performance metrics
        updateMetrics(params, ion_distribution);
        
        Logger::getInstance().log("Advanced ion implantation completed successfully");
        Logger::getInstance().log("Range uniformity: " + std::to_string(metrics_.range_uniformity) + "%");
        Logger::getInstance().log("Channeling fraction: " + std::to_string(metrics_.channeling_fraction) + "%");
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("Advanced ion implantation failed: " + std::string(e.what()));
        return false;
    }
}

bool AdvancedIonImplantation::simulateMultipleImplants(std::shared_ptr<Wafer> wafer,
                                                      const std::vector<ImplantParameters>& implant_sequence) {
    bool overall_success = true;
    
    Logger::getInstance().log("Starting multiple implant sequence (" + 
                             std::to_string(implant_sequence.size()) + " implants)");
    
    for (size_t i = 0; i < implant_sequence.size(); ++i) {
        Logger::getInstance().log("Executing implant " + std::to_string(i + 1) + "/" + 
                                 std::to_string(implant_sequence.size()));
        
        bool success = simulateImplantation(wafer, implant_sequence[i]);
        if (!success) {
            overall_success = false;
            Logger::getInstance().log("Implant " + std::to_string(i + 1) + " failed");
        }
    }
    
    return overall_success;
}

double AdvancedIonImplantation::calculateLSSRange(const IonSpecies& ion, double energy, 
                                                 const std::string& target) const {
    auto target_props = getMaterialProperties(target);
    
    // LSS theory implementation
    double reduced_energy = calculateReducedEnergy(ion, energy, target_props);
    
    // LSS range formula
    double range_factor = 0.325; // Empirical constant
    double mass_ratio = ion.atomic_mass / target_props.atomic_mass;
    double z_factor = std::pow(ion.atomic_number + target_props.atomic_number, 0.23);
    
    double range = range_factor * mass_ratio * 
                   std::pow(reduced_energy / z_factor, 1.8);
    
    return range * 1e-4; // Convert to μm
}

double AdvancedIonImplantation::calculateRangeStraggling(const IonSpecies& ion, double energy,
                                                        const std::string& target) const {
    double range = calculateLSSRange(ion, energy, target);
    
    // Bohr straggling theory
    double straggling_factor = 0.3; // Typical value
    
    // Energy dependence
    double energy_factor = std::sqrt(energy / 100.0); // Normalized to 100 keV
    
    return range * straggling_factor * energy_factor;
}

double AdvancedIonImplantation::calculateLateralStraggling(const IonSpecies& ion, double energy,
                                                          const std::string& target) const {
    double range = calculateLSSRange(ion, energy, target);
    
    // Lateral straggling is typically 0.6-0.8 times the range straggling
    double range_straggling = calculateRangeStraggling(ion, energy, target);
    
    return range_straggling * 0.7;
}

double AdvancedIonImplantation::calculateChannelingRange(const IonSpecies& ion, double energy,
                                                        double tilt_angle) const {
    double normal_range = calculateLSSRange(ion, energy, "Si");
    
    // Channeling enhancement factor
    double channeling_factor = 1.0;
    
    if (tilt_angle < 1.0) { // Strong channeling
        channeling_factor = 3.0 + energy / 100.0; // Energy-dependent enhancement
    } else if (tilt_angle < 5.0) { // Moderate channeling
        channeling_factor = 1.5 + energy / 200.0;
    }
    
    return normal_range * channeling_factor;
}

Eigen::ArrayXXd AdvancedIonImplantation::calculateFullLSSDistribution(const ImplantParameters& params,
                                                                     int rows, int cols) const {
    Eigen::ArrayXXd distribution(rows, cols);
    distribution.setZero();
    
    // Calculate LSS parameters
    double range = calculateLSSRange(params.ion, params.energy, "Si");
    double range_straggling = calculateRangeStraggling(params.ion, params.energy, "Si");
    double lateral_straggling = calculateLateralStraggling(params.ion, params.energy, "Si");
    
    // Pearson IV distribution parameters (more accurate than Gaussian)
    double skewness = 0.5; // Typical for ion implantation
    double kurtosis = 3.2; // Slightly peaked
    
    // Generate 2D distribution
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Depth coordinate (vertical)
            double depth = (static_cast<double>(i) / rows) * 2.0 * range; // 2x range for tail
            
            // Lateral coordinate (horizontal)
            double lateral = (static_cast<double>(j - cols/2) / cols) * 4.0 * lateral_straggling;
            
            // Pearson IV distribution in depth
            double depth_factor = std::exp(-0.5 * std::pow((depth - range) / range_straggling, 2));
            
            // Gaussian distribution in lateral direction
            double lateral_factor = std::exp(-0.5 * std::pow(lateral / lateral_straggling, 2));
            
            // Combined distribution
            double concentration = depth_factor * lateral_factor;
            
            // Normalize by dose
            concentration *= params.dose / (range_straggling * lateral_straggling * std::sqrt(2 * M_PI));
            
            distribution(i, j) = concentration;
        }
    }
    
    return distribution;
}

Eigen::ArrayXXd AdvancedIonImplantation::applyChannelingEffects(const Eigen::ArrayXXd& distribution,
                                                               const ImplantParameters& params) const {
    int rows = distribution.rows();
    int cols = distribution.cols();
    Eigen::ArrayXXd channeled_distribution = distribution;
    
    // Calculate channeling probability
    double channeling_prob = calculateChannelingProbability(params.tilt_angle, params.twist_angle, 
                                                           substrate_orientation_);
    
    if (channeling_prob > 0.01) { // Significant channeling
        double channeling_range = calculateChannelingRange(params.ion, params.energy, params.tilt_angle);
        double normal_range = calculateLSSRange(params.ion, params.energy, "Si");
        
        // Create channeling tail
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                double depth = (static_cast<double>(i) / rows) * 2.0 * normal_range;
                
                if (depth > normal_range) {
                    // Add channeling tail
                    double channeling_factor = channeling_prob * 
                                              std::exp(-(depth - normal_range) / (channeling_range - normal_range));
                    
                    channeled_distribution(i, j) += distribution(i, j) * channeling_factor;
                }
            }
        }
    }
    
    return channeled_distribution;
}

Eigen::ArrayXXd AdvancedIonImplantation::calculateTemperatureEffects(const Eigen::ArrayXXd& distribution,
                                                                    double temperature) const {
    // Temperature effects on ion implantation
    // Higher temperature can cause enhanced diffusion and annealing
    
    if (temperature <= 25.0) {
        return distribution; // No temperature effects at room temperature
    }
    
    int rows = distribution.rows();
    int cols = distribution.cols();
    Eigen::ArrayXXd temp_distribution = distribution;
    
    // Enhanced diffusion at elevated temperature
    double diffusion_enhancement = 1.0 + (temperature - 25.0) / 1000.0; // Linear approximation
    
    // Apply simple diffusion broadening
    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            double diffusion_term = 0.1 * diffusion_enhancement * 
                                   (distribution(i+1, j) + distribution(i-1, j) + 
                                    distribution(i, j+1) + distribution(i, j-1) - 
                                    4 * distribution(i, j));
            
            temp_distribution(i, j) += diffusion_term;
        }
    }
    
    return temp_distribution;
}

AdvancedIonImplantation::DamageProfile 
AdvancedIonImplantation::calculateDamageProfile(const ImplantParameters& params,
                                               const Eigen::ArrayXXd& ion_distribution) const {
    DamageProfile damage;
    
    int rows = ion_distribution.rows();
    int cols = ion_distribution.cols();
    
    damage.vacancy_concentration.resize(rows, cols);
    damage.interstitial_concentration.resize(rows, cols);
    damage.amorphous_fraction.resize(rows, cols);
    
    // Calculate displacements per ion
    auto target_props = getMaterialProperties("Si");
    double dpa_per_ion = calculateDisplacementsPerIon(params.ion, params.energy, target_props);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double ion_concentration = ion_distribution(i, j);
            
            // Calculate damage density
            double damage_density = ion_concentration * dpa_per_ion;
            
            // Vacancy and interstitial concentrations (simplified model)
            damage.vacancy_concentration(i, j) = damage_density * 0.6; // 60% vacancies
            damage.interstitial_concentration(i, j) = damage_density * 0.4; // 40% interstitials
            
            // Amorphization threshold
            double amorphization_threshold = calculateAmorphizationThreshold("Si", params.temperature);
            
            if (damage_density > amorphization_threshold) {
                damage.amorphous_fraction(i, j) = std::min(1.0, 
                    (damage_density - amorphization_threshold) / amorphization_threshold);
            } else {
                damage.amorphous_fraction(i, j) = 0.0;
            }
        }
    }
    
    return damage;
}

double AdvancedIonImplantation::calculateDisplacementThreshold(const std::string& material) const {
    auto props = getMaterialProperties(material);
    return props.displacement_threshold; // eV
}

double AdvancedIonImplantation::calculateAmorphizationThreshold(const std::string& material, 
                                                              double temperature) const {
    // Temperature-dependent amorphization threshold
    double base_threshold = 1e22; // cm⁻³ at room temperature
    
    // Temperature dependence (higher temperature = higher threshold)
    double temp_factor = 1.0 + (temperature - 25.0) / 500.0;
    
    return base_threshold * temp_factor;
}

void AdvancedIonImplantation::initializeIonDatabase() {
    // Common dopant ions
    ion_database_["B"] = IonSpecies("B", 10.81, 5);
    ion_database_["P"] = IonSpecies("P", 30.97, 15);
    ion_database_["As"] = IonSpecies("As", 74.92, 33);
    ion_database_["Sb"] = IonSpecies("Sb", 121.76, 51);
    ion_database_["In"] = IonSpecies("In", 114.82, 49);
    ion_database_["Ga"] = IonSpecies("Ga", 69.72, 31);
    
    // Other ions
    ion_database_["Si"] = IonSpecies("Si", 28.09, 14);
    ion_database_["Ge"] = IonSpecies("Ge", 72.63, 32);
    ion_database_["C"] = IonSpecies("C", 12.01, 6);
    ion_database_["N"] = IonSpecies("N", 14.01, 7);
    ion_database_["O"] = IonSpecies("O", 16.00, 8);
    ion_database_["F"] = IonSpecies("F", 19.00, 9);
}

void AdvancedIonImplantation::initializeMaterialDatabase() {
    // Silicon
    material_database_["Si"] = MaterialProperties{
        2.33,    // density g/cm³
        28.09,   // atomic mass
        14,      // atomic number
        15.0,    // displacement threshold eV
        5.43,    // lattice constant Å
        "diamond"
    };
    
    // Silicon dioxide
    material_database_["SiO2"] = MaterialProperties{
        2.20,    // density g/cm³
        20.03,   // average atomic mass
        10,      // average atomic number
        18.0,    // displacement threshold eV
        4.68,    // lattice constant Å
        "amorphous"
    };
    
    // Silicon nitride
    material_database_["Si3N4"] = MaterialProperties{
        3.17,    // density g/cm³
        20.02,   // average atomic mass
        10,      // average atomic number
        20.0,    // displacement threshold eV
        7.75,    // lattice constant Å
        "amorphous"
    };
}

AdvancedIonImplantation::IonSpecies 
AdvancedIonImplantation::getIonSpecies(const std::string& symbol) const {
    auto it = ion_database_.find(symbol);
    if (it != ion_database_.end()) {
        return it->second;
    }
    
    // Default to silicon if not found
    return IonSpecies("Si", 28.09, 14);
}

AdvancedIonImplantation::MaterialProperties 
AdvancedIonImplantation::getMaterialProperties(const std::string& material) const {
    auto it = material_database_.find(material);
    if (it != material_database_.end()) {
        return it->second;
    }
    
    // Default to silicon if not found
    return material_database_.at("Si");
}

double AdvancedIonImplantation::calculateReducedEnergy(const IonSpecies& ion, double energy,
                                                      const MaterialProperties& target) const {
    // LSS reduced energy calculation
    double mass_factor = (ion.atomic_mass + target.atomic_mass) / 
                        (ion.atomic_mass * target.atomic_mass);
    
    double z_factor = std::pow(ion.atomic_number, 0.23) + std::pow(target.atomic_number, 0.23);
    
    return energy * mass_factor / z_factor;
}

double AdvancedIonImplantation::calculateChannelingProbability(double tilt_angle, double twist_angle,
                                                              const std::string& orientation) const {
    // Critical angle for channeling (simplified)
    double critical_angle = 2.0; // degrees for <100> silicon
    
    if (orientation == "111") {
        critical_angle = 1.5; // degrees
    }
    
    if (tilt_angle < critical_angle) {
        return std::exp(-tilt_angle / critical_angle);
    }
    
    return 0.0;
}

double AdvancedIonImplantation::calculateDisplacementsPerIon(const IonSpecies& ion, double energy,
                                                           const MaterialProperties& target) const {
    // Simplified Kinchin-Pease model
    double threshold = target.displacement_threshold; // eV
    double nuclear_energy = energy * 1000.0 * 0.1; // Assume 10% goes to nuclear collisions
    
    if (nuclear_energy < 2 * threshold) {
        return 0.0;
    }
    
    return 0.8 * nuclear_energy / (2 * threshold); // Kinchin-Pease formula
}

void AdvancedIonImplantation::updateMetrics(const ImplantParameters& params,
                                           const Eigen::ArrayXXd& final_distribution) const {
    // Calculate range uniformity
    double mean_range = calculateLSSRange(params.ion, params.energy, "Si");
    double range_straggling = calculateRangeStraggling(params.ion, params.energy, "Si");
    metrics_.range_uniformity = (1.0 - range_straggling / mean_range) * 100.0;
    
    // Calculate dose uniformity (simplified)
    metrics_.dose_uniformity = 95.0; // Typical value
    
    // Calculate channeling fraction
    if (channeling_enabled_ && params.channeling_enabled) {
        metrics_.channeling_fraction = calculateChannelingProbability(params.tilt_angle, 
                                                                     params.twist_angle,
                                                                     substrate_orientation_) * 100.0;
    } else {
        metrics_.channeling_fraction = 0.0;
    }
    
    // Calculate damage density
    auto target_props = getMaterialProperties("Si");
    metrics_.damage_density = calculateDisplacementsPerIon(params.ion, params.energy, target_props) * 
                             params.dose;
    
    // Calculate amorphous fraction
    double amorphization_threshold = calculateAmorphizationThreshold("Si", params.temperature);
    metrics_.amorphous_fraction = (metrics_.damage_density > amorphization_threshold) ? 
                                 (metrics_.damage_density / amorphization_threshold - 1.0) * 100.0 : 0.0;
    
    // Calculate implant time (simplified)
    metrics_.implant_time = params.dose / (params.beam_current * 6.24e15); // seconds
    
    // Calculate throughput
    metrics_.throughput = 3600.0 / (metrics_.implant_time + 60.0); // wafers/hour (+ 60s overhead)
}

// Factory functions
std::unique_ptr<AdvancedIonImplantation> createHighCurrentImplanter() {
    auto implanter = std::make_unique<AdvancedIonImplantation>();
    implanter->enableChanneling(false); // High current typically suppresses channeling
    implanter->enableDamageAccumulation(true);
    implanter->enableTemperatureEffects(true);
    return implanter;
}

std::unique_ptr<AdvancedIonImplantation> createHighEnergyImplanter() {
    auto implanter = std::make_unique<AdvancedIonImplantation>();
    implanter->enableChanneling(true); // Channeling important at high energy
    implanter->enableDamageAccumulation(true);
    implanter->enableTemperatureEffects(false); // Usually room temperature
    return implanter;
}

std::vector<AdvancedIonImplantation::ImplantParameters>
createMOSFETImplantSequence(const std::string& device_type) {
    std::vector<AdvancedIonImplantation::ImplantParameters> sequence;
    
    AdvancedIonImplantation::IonSpecies boron("B", 10.81, 5);
    AdvancedIonImplantation::IonSpecies phosphorus("P", 30.97, 15);
    AdvancedIonImplantation::IonSpecies arsenic("As", 74.92, 33);
    
    if (device_type == "NMOS") {
        // NMOS device sequence
        sequence.emplace_back(phosphorus, 80, 5e15);  // Source/drain
        sequence.emplace_back(boron, 30, 1e13);      // Threshold adjust
        sequence.emplace_back(arsenic, 150, 1e16);   // Contact implant
    } else if (device_type == "PMOS") {
        // PMOS device sequence
        sequence.emplace_back(boron, 50, 3e15);      // Source/drain
        sequence.emplace_back(phosphorus, 200, 1e13); // Threshold adjust
        sequence.emplace_back(boron, 100, 1e16);     // Contact implant
    }
    
    return sequence;
}

bool validateImplantParameters(const AdvancedIonImplantation::ImplantParameters& params) {
    // Basic validation
    if (params.energy <= 0 || params.energy > 10000) { // 0-10 MeV range
        return false;
    }
    
    if (params.dose <= 0 || params.dose > 1e18) { // Reasonable dose range
        return false;
    }
    
    if (params.beam_current <= 0 || params.beam_current > 100) { // 0-100 mA range
        return false;
    }
    
    return true;
}

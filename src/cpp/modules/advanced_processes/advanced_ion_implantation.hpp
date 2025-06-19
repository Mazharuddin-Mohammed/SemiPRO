// Author: Dr. Mazharuddin Mohammed
#ifndef ADVANCED_ION_IMPLANTATION_HPP
#define ADVANCED_ION_IMPLANTATION_HPP

#include "../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <Eigen/Dense>

/**
 * Advanced Ion Implantation Simulation
 * Implements sophisticated ion implantation physics including:
 * - Full LSS theory implementation
 * - Channeling effects
 * - Damage accumulation
 * - Amorphization modeling
 * - Temperature-dependent effects
 */
class AdvancedIonImplantation {
public:
    AdvancedIonImplantation();
    ~AdvancedIonImplantation() = default;
    
    // Ion species definition
    struct IonSpecies {
        std::string symbol;
        double atomic_mass;     // amu
        int atomic_number;
        double charge_state;
        
        IonSpecies(const std::string& sym, double mass, int z, double charge = 1.0)
            : symbol(sym), atomic_mass(mass), atomic_number(z), charge_state(charge) {}
    };
    
    // Implantation parameters
    struct ImplantParameters {
        IonSpecies ion;
        double energy;          // keV
        double dose;           // ions/cm²
        double beam_current;   // mA
        double temperature;    // °C
        double tilt_angle;     // degrees
        double twist_angle;    // degrees
        bool channeling_enabled;
        
        ImplantParameters(const IonSpecies& i, double e, double d)
            : ion(i), energy(e), dose(d), beam_current(1.0), temperature(25.0),
              tilt_angle(7.0), twist_angle(0.0), channeling_enabled(true) {}
    };
    
    // Damage and amorphization
    struct DamageProfile {
        Eigen::ArrayXXd vacancy_concentration;
        Eigen::ArrayXXd interstitial_concentration;
        Eigen::ArrayXXd amorphous_fraction;
        double critical_damage_density;
        
        DamageProfile() : critical_damage_density(1e22) {} // cm⁻³
    };
    
    // Main simulation methods
    bool simulateImplantation(std::shared_ptr<Wafer> wafer, const ImplantParameters& params);
    bool simulateMultipleImplants(std::shared_ptr<Wafer> wafer, 
                                 const std::vector<ImplantParameters>& implant_sequence);
    
    // Advanced features
    void enableChanneling(bool enable) { channeling_enabled_ = enable; }
    void enableDamageAccumulation(bool enable) { damage_accumulation_enabled_ = enable; }
    void enableTemperatureEffects(bool enable) { temperature_effects_enabled_ = enable; }
    void setSubstrateOrientation(const std::string& orientation) { substrate_orientation_ = orientation; }
    
    // Physics calculations
    double calculateLSSRange(const IonSpecies& ion, double energy, const std::string& target) const;
    double calculateRangeStraggling(const IonSpecies& ion, double energy, const std::string& target) const;
    double calculateLateralStraggling(const IonSpecies& ion, double energy, const std::string& target) const;
    double calculateChannelingRange(const IonSpecies& ion, double energy, double tilt_angle) const;
    
    // Damage modeling
    DamageProfile calculateDamageProfile(const ImplantParameters& params, 
                                       const Eigen::ArrayXXd& ion_distribution) const;
    double calculateDisplacementThreshold(const std::string& material) const;
    double calculateAmorphizationThreshold(const std::string& material, double temperature) const;
    
    // Performance metrics
    struct ImplantMetrics {
        double range_uniformity;        // %
        double dose_uniformity;         // %
        double channeling_fraction;     // %
        double damage_density;          // cm⁻³
        double amorphous_fraction;      // %
        double implant_time;           // seconds
        double throughput;             // wafers/hour
    };
    
    ImplantMetrics getMetrics() const { return metrics_; }
    DamageProfile getDamageProfile() const { return damage_profile_; }

private:
    // Configuration flags
    bool channeling_enabled_;
    bool damage_accumulation_enabled_;
    bool temperature_effects_enabled_;
    std::string substrate_orientation_;
    
    // Performance tracking
    mutable ImplantMetrics metrics_;
    mutable DamageProfile damage_profile_;
    
    // Ion species database
    std::unordered_map<std::string, IonSpecies> ion_database_;
    
    // Material properties database
    struct MaterialProperties {
        double density;              // g/cm³
        double atomic_mass;         // amu
        int atomic_number;
        double displacement_threshold; // eV
        double lattice_constant;    // Å
        std::string crystal_structure;
    };
    std::unordered_map<std::string, MaterialProperties> material_database_;
    
    // Advanced physics methods
    Eigen::ArrayXXd calculateFullLSSDistribution(const ImplantParameters& params, 
                                                int rows, int cols) const;
    Eigen::ArrayXXd applyChannelingEffects(const Eigen::ArrayXXd& distribution,
                                         const ImplantParameters& params) const;
    Eigen::ArrayXXd calculateTemperatureEffects(const Eigen::ArrayXXd& distribution,
                                               double temperature) const;
    
    // LSS theory implementation
    double calculateReducedEnergy(const IonSpecies& ion, double energy, 
                                const MaterialProperties& target) const;
    double calculateNuclearStoppingPower(const IonSpecies& ion, double energy,
                                        const MaterialProperties& target) const;
    double calculateElectronicStoppingPower(const IonSpecies& ion, double energy,
                                           const MaterialProperties& target) const;
    
    // Channeling physics
    double calculateChannelingProbability(double tilt_angle, double twist_angle,
                                         const std::string& orientation) const;
    double calculateDechanneling(double depth, const IonSpecies& ion, double energy) const;
    
    // Damage physics
    double calculateDisplacementsPerIon(const IonSpecies& ion, double energy,
                                       const MaterialProperties& target) const;
    double calculateCascadeDamage(double nuclear_energy, const MaterialProperties& target) const;
    
    // Utility methods
    void initializeIonDatabase();
    void initializeMaterialDatabase();
    IonSpecies getIonSpecies(const std::string& symbol) const;
    MaterialProperties getMaterialProperties(const std::string& material) const;
    void updateMetrics(const ImplantParameters& params, const Eigen::ArrayXXd& final_distribution) const;
    
    // Statistical methods
    double calculatePearsonIVMoments(double range, double straggling, double skewness, double kurtosis) const;
    Eigen::ArrayXXd generatePearsonIVDistribution(double range, double straggling, 
                                                 double skewness, double kurtosis, 
                                                 int rows, int cols) const;
};

// Factory functions for common implant configurations
std::unique_ptr<AdvancedIonImplantation> createHighCurrentImplanter();
std::unique_ptr<AdvancedIonImplantation> createHighEnergyImplanter();
std::unique_ptr<AdvancedIonImplantation> createClusterImplanter();

// Utility functions
std::vector<AdvancedIonImplantation::ImplantParameters> 
createMOSFETImplantSequence(const std::string& device_type);

std::vector<AdvancedIonImplantation::ImplantParameters>
createBipolarImplantSequence();

bool validateImplantParameters(const AdvancedIonImplantation::ImplantParameters& params);

#endif // ADVANCED_ION_IMPLANTATION_HPP

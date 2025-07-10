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
 * Enhanced Doping Physics Module
 * Implements advanced ion implantation with channeling effects, damage modeling,
 * annealing simulation, and sophisticated dopant diffusion
 */

// Ion species with detailed properties
enum class IonSpecies {
    BORON_11,         // B-11 (p-type)
    PHOSPHORUS_31,    // P-31 (n-type)
    ARSENIC_75,       // As-75 (n-type)
    ANTIMONY_121,     // Sb-121 (n-type)
    INDIUM_115,       // In-115 (p-type)
    GALLIUM_69,       // Ga-69 (p-type)
    ALUMINUM_27,      // Al-27 (p-type)
    NITROGEN_14,      // N-14 (for nitridation)
    CARBON_12,        // C-12 (for SiC)
    GERMANIUM_74      // Ge-74 (for SiGe)
};

// Crystal channeling directions
enum class ChannelingDirection {
    RANDOM,           // Random orientation (no channeling)
    CHANNEL_100,      // <100> channeling
    CHANNEL_110,      // <110> channeling
    CHANNEL_111,      // <111> channeling
    PLANAR_100,       // {100} planar channeling
    PLANAR_110,       // {110} planar channeling
    PLANAR_111        // {111} planar channeling
};

// Implantation damage models
enum class DamageModel {
    KINCHIN_PEASE,    // Simple Kinchin-Pease model
    MARLOWE,          // MARLOWE Monte Carlo
    TRIM_SRIM,        // TRIM/SRIM calculations
    CRYSTAL_TRIM,     // Crystal-TRIM for channeling
    ADVANCED_BCA      // Advanced Binary Collision Approximation
};

// Annealing atmosphere
enum class AnnealingAtmosphere {
    VACUUM,           // Vacuum annealing
    NITROGEN,         // N2 atmosphere
    ARGON,            // Ar atmosphere
    FORMING_GAS,      // H2/N2 mixture
    OXYGEN,           // O2 atmosphere (oxidizing)
    HYDROGEN          // H2 atmosphere (reducing)
};

// Ion implantation conditions
struct ImplantationConditions {
    IonSpecies species;
    double energy;              // keV
    double dose;               // ions/cm²
    double beam_current;       // μA
    double tilt_angle;         // degrees from normal
    double twist_angle;        // degrees rotation
    double temperature;        // °C during implantation
    ChannelingDirection channeling;
    DamageModel damage_model;
    bool enable_sputtering;
    double beam_divergence;    // mrad
    
    ImplantationConditions() : species(IonSpecies::BORON_11), energy(50.0), dose(1e15),
                              beam_current(10.0), tilt_angle(7.0), twist_angle(0.0),
                              temperature(25.0), channeling(ChannelingDirection::RANDOM),
                              damage_model(DamageModel::KINCHIN_PEASE), enable_sputtering(false),
                              beam_divergence(1.0) {}
};

// Annealing conditions
struct AnnealingConditions {
    double temperature;        // °C
    double time;              // minutes
    double ramp_rate;         // °C/min
    AnnealingAtmosphere atmosphere;
    double pressure;          // Torr
    bool rapid_thermal;       // RTA vs furnace
    std::vector<double> temperature_profile; // For complex profiles
    
    AnnealingConditions() : temperature(900.0), time(30.0), ramp_rate(10.0),
                           atmosphere(AnnealingAtmosphere::NITROGEN), pressure(760.0),
                           rapid_thermal(false) {}
};

// Ion properties database
struct IonProperties {
    std::string symbol;
    double atomic_mass;        // amu
    int atomic_number;
    double binding_energy;     // eV
    bool is_p_type;           // true for p-type, false for n-type
    double diffusivity_d0;    // cm²/s (pre-exponential)
    double diffusion_ea;      // eV (activation energy)
    double solubility_limit;  // cm⁻³ at 1000°C
    
    IonProperties() : atomic_mass(0), atomic_number(0), binding_energy(0),
                     is_p_type(false), diffusivity_d0(0), diffusion_ea(0),
                     solubility_limit(0) {}
};

// Implantation results with detailed analysis
struct ImplantationResults {
    double projected_range;    // μm (Rp)
    double range_straggling;   // μm (ΔRp)
    double lateral_straggling; // μm
    double peak_concentration; // cm⁻³
    double sheet_resistance;   // Ω/sq
    double junction_depth;     // μm
    std::vector<double> concentration_profile; // vs depth
    std::vector<double> damage_profile;        // damage density vs depth
    std::vector<double> amorphous_regions;     // amorphous layer boundaries
    double channeling_fraction; // Fraction of channeled ions
    double sputtering_yield;    // Atoms sputtered per ion
    std::unordered_map<std::string, double> quality_metrics;
    
    ImplantationResults() : projected_range(0), range_straggling(0), lateral_straggling(0),
                           peak_concentration(0), sheet_resistance(0), junction_depth(0),
                           channeling_fraction(0), sputtering_yield(0) {}
};

// Annealing results
struct AnnealingResults {
    double final_junction_depth;     // μm
    double final_sheet_resistance;   // Ω/sq
    double dopant_activation;        // Fraction activated
    double defect_density;           // cm⁻³
    std::vector<double> final_concentration_profile;
    std::vector<double> electrical_profile; // Electrically active dopants
    double diffusion_length;         // μm
    bool recrystallization_complete; // For amorphous regions
    std::unordered_map<std::string, double> annealing_metrics;
    
    AnnealingResults() : final_junction_depth(0), final_sheet_resistance(0),
                        dopant_activation(0), defect_density(0), diffusion_length(0),
                        recrystallization_complete(false) {}
};

// Enhanced doping physics engine
class EnhancedDopingPhysics {
private:
    std::unordered_map<IonSpecies, IonProperties> ion_database_;
    std::unordered_map<ChannelingDirection, double> channeling_factors_;
    
    // Configuration
    bool enable_channeling_effects_ = true;
    bool enable_damage_modeling_ = true;
    bool enable_sputtering_ = true;
    bool enable_clustering_ = true;
    double numerical_precision_ = 1e-8;
    
public:
    EnhancedDopingPhysics();
    
    // Main simulation interfaces
    ImplantationResults simulateIonImplantation(
        std::shared_ptr<WaferEnhanced> wafer,
        const ImplantationConditions& conditions
    );
    
    AnnealingResults simulateAnnealing(
        std::shared_ptr<WaferEnhanced> wafer,
        const AnnealingConditions& conditions,
        const ImplantationResults& implant_results
    );
    
    // Advanced LSS theory calculations
    double calculateProjectedRange(
        const ImplantationConditions& conditions
    ) const;
    
    double calculateRangeStraggling(
        const ImplantationConditions& conditions,
        double projected_range
    ) const;
    
    double calculateLateralStraggling(
        const ImplantationConditions& conditions,
        double projected_range
    ) const;
    
    // Channeling effects
    double calculateChannelingFraction(
        const ImplantationConditions& conditions
    ) const;
    
    std::vector<double> calculateChannelingProfile(
        const ImplantationConditions& conditions,
        const std::vector<double>& depths
    ) const;
    
    // Damage modeling
    std::vector<double> calculateDamageProfile(
        const ImplantationConditions& conditions,
        const std::vector<double>& depths
    ) const;
    
    double calculateDisplacementThreshold(
        IonSpecies species,
        double energy
    ) const;
    
    std::vector<double> calculateAmorphousRegions(
        const std::vector<double>& damage_profile,
        const std::vector<double>& depths
    ) const;
    
    // Concentration profiles
    std::vector<double> calculateConcentrationProfile(
        const ImplantationConditions& conditions,
        const std::vector<double>& depths,
        bool include_channeling = true
    ) const;
    
    double calculatePeakConcentration(
        const ImplantationConditions& conditions
    ) const;
    
    // Electrical properties
    double calculateSheetResistance(
        const std::vector<double>& concentration_profile,
        const std::vector<double>& depths,
        IonSpecies species,
        double temperature = 300.0
    ) const;
    
    double calculateJunctionDepth(
        const std::vector<double>& concentration_profile,
        const std::vector<double>& depths,
        double background_doping = 1e15
    ) const;
    
    // Annealing and diffusion
    std::vector<double> simulateDiffusion(
        const std::vector<double>& initial_profile,
        const std::vector<double>& depths,
        const AnnealingConditions& conditions,
        IonSpecies species
    ) const;
    
    double calculateDiffusivity(
        IonSpecies species,
        double temperature,
        double concentration = 0
    ) const;
    
    double calculateDopantActivation(
        const std::vector<double>& concentration_profile,
        IonSpecies species,
        double annealing_temperature
    ) const;
    
    // Sputtering effects
    double calculateSputteringYield(
        const ImplantationConditions& conditions
    ) const;
    
    double calculateSurfaceRecession(
        const ImplantationConditions& conditions
    ) const;
    
    // Advanced effects
    std::vector<double> calculateClusteringEffects(
        const std::vector<double>& concentration_profile,
        IonSpecies species,
        double temperature
    ) const;
    
    double calculateTransientEnhancedDiffusion(
        double damage_density,
        double temperature,
        double time
    ) const;
    
    // Validation and analysis
    bool validateImplantationConditions(
        const ImplantationConditions& conditions,
        std::string& error_message
    ) const;
    
    bool validateAnnealingConditions(
        const AnnealingConditions& conditions,
        std::string& error_message
    ) const;
    
    std::unordered_map<std::string, double> analyzeImplantationQuality(
        const ImplantationResults& results,
        const ImplantationConditions& conditions
    ) const;
    
    // Configuration and calibration
    void setIonProperties(IonSpecies species, const IonProperties& properties);
    void setChannelingFactor(ChannelingDirection direction, double factor);
    void enablePhysicsEffects(bool channeling = true, bool damage = true, 
                             bool sputtering = true, bool clustering = true);
    
    // Utility methods
    std::string ionSpeciesToString(IonSpecies species) const;
    std::string channelingDirectionToString(ChannelingDirection direction) const;
    std::string damageModelToString(DamageModel model) const;
    std::string annealingAtmosphereToString(AnnealingAtmosphere atmosphere) const;
    
    IonProperties getIonProperties(IonSpecies species) const;
    
private:
    void initializeIonDatabase();
    void initializeChannelingFactors();
    
    // Numerical methods
    std::vector<double> solveDiffusionEquation(
        const std::vector<double>& initial_profile,
        const std::vector<double>& depths,
        double diffusivity,
        double time
    ) const;
    
    double calculateReducedEnergy(
        const ImplantationConditions& conditions
    ) const;
    
    double calculateNuclearStoppingPower(
        const ImplantationConditions& conditions,
        double energy
    ) const;
    
    double calculateElectronicStoppingPower(
        const ImplantationConditions& conditions,
        double energy
    ) const;
};

// Utility functions for doping analysis
namespace DopingUtils {
    // Convert between different concentration units
    double convertConcentration(double value, const std::string& from_unit, const std::string& to_unit);
    
    // Calculate mobility from concentration
    double calculateMobility(IonSpecies species, double concentration, double temperature = 300.0);
    
    // Calculate resistivity from concentration and mobility
    double calculateResistivity(double concentration, double mobility, bool is_p_type);
    
    // Optimize implantation conditions for target profile
    ImplantationConditions optimizeImplantation(
        double target_junction_depth,
        double target_sheet_resistance,
        IonSpecies species
    );
    
    // Generate implantation recipes for complex profiles
    std::vector<ImplantationConditions> generateMultiEnergyRecipe(
        const std::vector<double>& target_profile,
        const std::vector<double>& depths,
        IonSpecies species
    );
}

} // namespace SemiPRO

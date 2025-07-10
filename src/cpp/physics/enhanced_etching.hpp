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
 * Enhanced Etching Physics Module
 * Implements advanced plasma etching with selectivity, anisotropy,
 * surface chemistry modeling, and sophisticated etch profiles
 */

// Etching techniques
enum class EtchingTechnique {
    WET_CHEMICAL,     // Wet chemical etching
    RIE,              // Reactive Ion Etching
    DRIE,             // Deep Reactive Ion Etching
    ICP,              // Inductively Coupled Plasma
    CCP,              // Capacitively Coupled Plasma
    IBE,              // Ion Beam Etching
    CAIBE,            // Chemically Assisted Ion Beam Etching
    XeF2,             // Xenon difluoride etching
    TMAH,             // Tetramethylammonium hydroxide
    KOH               // Potassium hydroxide
};

// Etch chemistry types
enum class EtchChemistry {
    FLUORINE_BASED,   // CF4, SF6, NF3 based
    CHLORINE_BASED,   // Cl2, BCl3, HBr based
    BROMINE_BASED,    // HBr, Br2 based
    OXYGEN_BASED,     // O2 plasma for organics
    HYDROGEN_BASED,   // H2 plasma
    NOBLE_GAS,        // Ar, He physical sputtering
    WET_ACID,         // HF, HCl, H2SO4
    WET_BASE,         // KOH, TMAH
    MIXED_CHEMISTRY   // Multiple gas mixtures
};

// Material etch properties
enum class EtchMaterial {
    SILICON,          // Si substrate
    SILICON_DIOXIDE,  // SiO2
    SILICON_NITRIDE,  // Si3N4
    POLYSILICON,      // Poly-Si
    ALUMINUM,         // Al metal
    COPPER,           // Cu metal
    TUNGSTEN,         // W metal
    TITANIUM,         // Ti metal
    PHOTORESIST,      // Organic resist
    HARD_MASK,        // SiO2, Si3N4 hard mask
    DIELECTRIC,       // Low-k dielectrics
    BARRIER_METAL     // TiN, TaN
};

// Etching conditions
struct EtchingConditions {
    EtchingTechnique technique;
    EtchChemistry chemistry;
    EtchMaterial target_material;
    EtchMaterial mask_material;
    double target_depth;         // μm
    double pressure;            // mTorr
    double power;               // W
    double bias_voltage;        // V
    double temperature;         // °C
    std::vector<std::string> gas_species; // Etch gases
    std::unordered_map<std::string, double> gas_flows; // sccm
    double selectivity_target;  // Desired selectivity
    
    EtchingConditions() : technique(EtchingTechnique::RIE),
                         chemistry(EtchChemistry::FLUORINE_BASED),
                         target_material(EtchMaterial::SILICON),
                         mask_material(EtchMaterial::PHOTORESIST),
                         target_depth(1.0), pressure(10.0), power(100.0),
                         bias_voltage(100.0), temperature(25.0),
                         selectivity_target(10.0) {}
};

// Etching results with detailed analysis
struct EtchingResults {
    double final_depth;          // μm
    double etch_rate;            // μm/min
    double selectivity;          // Actual selectivity achieved
    double anisotropy;           // Vertical/lateral etch ratio
    double uniformity;           // % across wafer
    double sidewall_angle;       // degrees from vertical
    double surface_roughness;    // nm RMS
    double mask_erosion;         // μm mask consumed
    std::vector<double> etch_profile; // Depth vs position
    std::vector<double> sidewall_profile; // Sidewall shape
    std::unordered_map<std::string, double> quality_metrics;
    
    EtchingResults() : final_depth(0), etch_rate(0), selectivity(0),
                      anisotropy(0), uniformity(0), sidewall_angle(90.0),
                      surface_roughness(0), mask_erosion(0) {}
};

// Enhanced etching physics engine
class EnhancedEtchingPhysics {
private:
    std::unordered_map<EtchMaterial, std::unordered_map<EtchChemistry, double>> etch_rates_;
    std::unordered_map<EtchingTechnique, std::string> technique_names_;
    
    // Configuration
    bool enable_surface_chemistry_ = true;
    bool enable_ion_bombardment_ = true;
    bool enable_sidewall_passivation_ = true;
    bool enable_loading_effects_ = true;
    double numerical_precision_ = 1e-8;
    
public:
    EnhancedEtchingPhysics();
    
    // Main simulation interface
    EtchingResults simulateEtching(
        std::shared_ptr<WaferEnhanced> wafer,
        const EtchingConditions& conditions
    );
    
    // Technique-specific simulations
    EtchingResults simulatePlasmaEtching(
        std::shared_ptr<WaferEnhanced> wafer,
        const EtchingConditions& conditions
    );
    
    EtchingResults simulateWetEtching(
        std::shared_ptr<WaferEnhanced> wafer,
        const EtchingConditions& conditions
    );
    
    EtchingResults simulateIonBeamEtching(
        std::shared_ptr<WaferEnhanced> wafer,
        const EtchingConditions& conditions
    );
    
    // Etch rate calculations
    double calculateEtchRate(
        const EtchingConditions& conditions
    ) const;
    
    double calculateSelectivity(
        EtchMaterial target,
        EtchMaterial mask,
        const EtchingConditions& conditions
    ) const;
    
    double calculateAnisotropy(
        const EtchingConditions& conditions
    ) const;
    
    // Surface chemistry modeling
    double calculateSurfaceCoverage(
        EtchChemistry chemistry,
        double pressure,
        double temperature
    ) const;
    
    double calculateReactionProbability(
        EtchChemistry chemistry,
        EtchMaterial material,
        double ion_energy
    ) const;
    
    double calculateSputteringYield(
        EtchMaterial material,
        double ion_energy,
        double angle = 0.0
    ) const;
    
    // Plasma physics modeling
    double calculateIonEnergy(
        double bias_voltage,
        double pressure
    ) const;
    
    double calculateIonFlux(
        double power,
        double pressure,
        double area = 1.0
    ) const;
    
    double calculateRadicalDensity(
        EtchChemistry chemistry,
        double power,
        double pressure
    ) const;
    
    // Profile modeling
    std::vector<double> calculateEtchProfile(
        std::shared_ptr<WaferEnhanced> wafer,
        const EtchingConditions& conditions,
        double base_depth
    );
    
    std::vector<double> calculateSidewallProfile(
        const EtchingConditions& conditions,
        double depth
    );
    
    double calculateSidewallAngle(
        const EtchingConditions& conditions
    ) const;
    
    // Advanced effects
    double calculateLoadingEffect(
        double open_area_ratio,
        const EtchingConditions& conditions
    ) const;
    
    double calculateMicroloading(
        double feature_size,
        const EtchingConditions& conditions
    ) const;
    
    double calculateAspectRatioEffect(
        double aspect_ratio,
        const EtchingConditions& conditions
    ) const;
    
    // Passivation modeling
    double calculatePassivationRate(
        const EtchingConditions& conditions
    ) const;
    
    double calculatePassivationThickness(
        const EtchingConditions& conditions,
        double time
    ) const;
    
    // Quality analysis
    std::unordered_map<std::string, double> analyzeEtchingQuality(
        const EtchingResults& results,
        const EtchingConditions& conditions
    ) const;
    
    double calculateUniformity(
        const std::vector<double>& etch_profile
    ) const;
    
    double calculateSurfaceRoughness(
        EtchMaterial material,
        const EtchingConditions& conditions
    ) const;
    
    // Validation and diagnostics
    bool validateConditions(
        const EtchingConditions& conditions,
        std::string& error_message
    ) const;
    
    std::vector<std::string> getDiagnostics(
        const EtchingResults& results,
        const EtchingConditions& conditions
    ) const;
    
    // Configuration and calibration
    void setEtchRate(EtchMaterial material, EtchChemistry chemistry, double rate);
    void enablePhysicsEffects(bool surface_chemistry = true, bool ion_bombardment = true,
                             bool sidewall_passivation = true, bool loading_effects = true);
    
    // Utility methods
    std::string techniqueToString(EtchingTechnique technique) const;
    std::string chemistryToString(EtchChemistry chemistry) const;
    std::string materialToString(EtchMaterial material) const;
    
private:
    void initializeEtchRateDatabase();
    void initializeTechniqueNames();
    
    // Numerical methods
    double solveEtchKinetics(
        const EtchingConditions& conditions,
        double time
    ) const;
    
    std::vector<double> solveDiffusionReactionEquation(
        const std::vector<double>& initial_profile,
        const EtchingConditions& conditions,
        double time
    ) const;
    
    // Advanced models
    double calculateBoschProcess(
        const EtchingConditions& conditions,
        int cycle_number
    ) const;
    
    double calculateCryogenicEtching(
        const EtchingConditions& conditions
    ) const;
    
    double calculateAtomicLayerEtching(
        EtchMaterial material,
        int cycle_number
    ) const;
};

// Utility functions for etching analysis
namespace EtchingUtils {
    // Convert between different depth units
    double convertDepth(double value, const std::string& from_unit, const std::string& to_unit);
    
    // Calculate etch time for target depth
    double calculateRequiredTime(
        double target_depth,
        double etch_rate
    );
    
    // Optimize etching conditions for target specifications
    EtchingConditions optimizeConditions(
        EtchMaterial target_material,
        double target_depth,
        double target_selectivity,
        double max_power
    );
    
    // Generate etching recipes
    std::vector<EtchingConditions> generateMultiStepRecipe(
        const std::vector<EtchMaterial>& materials,
        const std::vector<double>& depths
    );
}

} // namespace SemiPRO

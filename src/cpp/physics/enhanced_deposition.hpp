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
 * Enhanced Deposition Physics Module
 * Implements advanced CVD, PVD, and ALD models with surface kinetics,
 * conformality analysis, and sophisticated material properties
 */

// Deposition techniques
enum class DepositionTechnique {
    CVD,              // Chemical Vapor Deposition
    LPCVD,            // Low Pressure CVD
    PECVD,            // Plasma Enhanced CVD
    MOCVD,            // Metal Organic CVD
    PVD_SPUTTERING,   // Physical Vapor Deposition - Sputtering
    PVD_EVAPORATION,  // Physical Vapor Deposition - Evaporation
    ALD,              // Atomic Layer Deposition
    ELECTROPLATING,   // Electrochemical deposition
    SPIN_COATING,     // Spin-on deposition
    EPITAXY           // Epitaxial growth
};

// Material types with detailed properties
enum class MaterialType {
    ALUMINUM,         // Al metal
    COPPER,           // Cu metal
    TUNGSTEN,         // W metal
    TITANIUM,         // Ti metal
    TANTALUM,         // Ta metal
    POLYSILICON,      // Poly-Si
    AMORPHOUS_SI,     // a-Si
    SILICON_DIOXIDE,  // SiO2
    SILICON_NITRIDE,  // Si3N4
    SILICON_CARBIDE,  // SiC
    TITANIUM_NITRIDE, // TiN
    TANTALUM_NITRIDE, // TaN
    HAFNIUM_OXIDE,    // HfO2
    ALUMINUM_OXIDE,   // Al2O3
    PHOTORESIST       // Organic polymer
};

// Surface reaction mechanisms
enum class SurfaceReaction {
    ADSORPTION_LIMITED,    // Surface adsorption controls rate
    REACTION_LIMITED,      // Surface reaction controls rate
    DIFFUSION_LIMITED,     // Mass transport controls rate
    DESORPTION_LIMITED,    // Product desorption controls rate
    NUCLEATION_LIMITED     // Nucleation controls rate
};

// Deposition conditions
struct DepositionConditions {
    DepositionTechnique technique;
    MaterialType material;
    double temperature;        // °C
    double pressure;          // Torr
    double target_thickness;  // μm
    double flow_rate;         // sccm (standard cubic cm/min)
    double power;            // W (for plasma processes)
    double bias_voltage;     // V (for sputtering)
    std::vector<std::string> precursors; // Chemical precursors
    std::unordered_map<std::string, double> gas_composition; // Gas ratios
    
    DepositionConditions() : technique(DepositionTechnique::CVD), 
                            material(MaterialType::SILICON_DIOXIDE),
                            temperature(400.0), pressure(1.0), target_thickness(0.1),
                            flow_rate(100.0), power(0.0), bias_voltage(0.0) {}
};

// Material properties database
struct MaterialProperties {
    std::string name;
    std::string chemical_formula;
    double density;              // g/cm³
    double melting_point;        // °C
    double thermal_conductivity; // W/m·K
    double electrical_resistivity; // Ω·cm
    double stress_intrinsic;     // MPa (tensile positive)
    double etch_selectivity;     // Relative to SiO2
    double refractive_index;     // At 633 nm
    bool is_conductor;
    bool is_barrier_metal;
    
    MaterialProperties() : density(0), melting_point(0), thermal_conductivity(0),
                          electrical_resistivity(0), stress_intrinsic(0),
                          etch_selectivity(1.0), refractive_index(1.0),
                          is_conductor(false), is_barrier_metal(false) {}
};

// Deposition results with detailed analysis
struct DepositionResults {
    double final_thickness;      // μm
    double deposition_rate;      // μm/min
    double uniformity;           // % (1-sigma/mean)
    double step_coverage;        // % in trenches/vias
    double conformality;         // Thickness ratio (sidewall/top)
    double stress_level;         // MPa
    double grain_size;           // nm
    double surface_roughness;    // nm RMS
    std::vector<double> thickness_profile; // Spatial variation
    std::vector<double> composition_profile; // Depth composition
    std::unordered_map<std::string, double> quality_metrics;
    
    DepositionResults() : final_thickness(0), deposition_rate(0), uniformity(0),
                         step_coverage(0), conformality(0), stress_level(0),
                         grain_size(0), surface_roughness(0) {}
};

// Enhanced deposition physics engine
class EnhancedDepositionPhysics {
private:
    std::unordered_map<MaterialType, MaterialProperties> material_database_;
    std::unordered_map<DepositionTechnique, std::string> technique_names_;
    
    // Configuration
    bool enable_surface_kinetics_ = true;
    bool enable_stress_modeling_ = true;
    bool enable_grain_growth_ = true;
    bool enable_conformality_analysis_ = true;
    double numerical_precision_ = 1e-8;
    
public:
    EnhancedDepositionPhysics();
    
    // Main simulation interface
    DepositionResults simulateDeposition(
        std::shared_ptr<WaferEnhanced> wafer,
        const DepositionConditions& conditions
    );
    
    // Technique-specific simulations
    DepositionResults simulateCVD(
        std::shared_ptr<WaferEnhanced> wafer,
        const DepositionConditions& conditions
    );
    
    DepositionResults simulatePVD(
        std::shared_ptr<WaferEnhanced> wafer,
        const DepositionConditions& conditions
    );
    
    DepositionResults simulateALD(
        std::shared_ptr<WaferEnhanced> wafer,
        const DepositionConditions& conditions
    );
    
    // Surface kinetics modeling
    double calculateDepositionRate(
        const DepositionConditions& conditions,
        SurfaceReaction mechanism = SurfaceReaction::REACTION_LIMITED
    ) const;
    
    double calculateSurfaceCoverage(
        const DepositionConditions& conditions,
        double time
    ) const;
    
    double calculateStickingCoefficient(
        MaterialType material,
        double temperature,
        double pressure
    ) const;
    
    // Conformality and step coverage
    double calculateStepCoverage(
        const DepositionConditions& conditions,
        double aspect_ratio
    ) const;
    
    double calculateConformality(
        const DepositionConditions& conditions,
        double trench_depth,
        double trench_width
    ) const;
    
    std::vector<double> calculateThicknessProfile(
        std::shared_ptr<WaferEnhanced> wafer,
        const DepositionConditions& conditions,
        double base_thickness
    );
    
    // Material-specific modeling
    double calculateGrainSize(
        MaterialType material,
        double temperature,
        double thickness
    ) const;
    
    double calculateIntrinsicStress(
        MaterialType material,
        const DepositionConditions& conditions
    ) const;
    
    double calculateSurfaceRoughness(
        MaterialType material,
        const DepositionConditions& conditions,
        double thickness
    ) const;
    
    // Advanced effects
    double calculateTemperatureDependence(
        double base_rate,
        double activation_energy,
        double temperature,
        double reference_temp = 400.0
    ) const;
    
    double calculatePressureDependence(
        double base_rate,
        double pressure,
        double pressure_exponent = 0.5
    ) const;
    
    double calculatePlasmaEnhancement(
        double base_rate,
        double power,
        double pressure
    ) const;
    
    // ALD-specific modeling
    double calculateALDGrowthPerCycle(
        MaterialType material,
        double temperature
    ) const;
    
    int calculateALDCycles(
        double target_thickness,
        double growth_per_cycle
    ) const;
    
    // PVD-specific modeling
    double calculateSputteringYield(
        MaterialType target_material,
        double ion_energy,
        double angle = 0.0
    ) const;
    
    double calculateEvaporationRate(
        MaterialType material,
        double temperature,
        double pressure
    ) const;
    
    // Quality analysis
    std::unordered_map<std::string, double> analyzeDepositionQuality(
        const DepositionResults& results,
        const DepositionConditions& conditions
    ) const;
    
    double calculateUniformity(
        const std::vector<double>& thickness_profile
    ) const;
    
    // Validation and diagnostics
    bool validateConditions(
        const DepositionConditions& conditions,
        std::string& error_message
    ) const;
    
    std::vector<std::string> getDiagnostics(
        const DepositionResults& results,
        const DepositionConditions& conditions
    ) const;
    
    // Configuration and calibration
    void setMaterialProperties(MaterialType material, const MaterialProperties& properties);
    void enablePhysicsEffects(bool surface_kinetics = true, bool stress = true,
                             bool grain_growth = true, bool conformality = true);
    
    // Utility methods
    std::string techniqueToString(DepositionTechnique technique) const;
    std::string materialToString(MaterialType material) const;
    std::string reactionToString(SurfaceReaction reaction) const;
    
    MaterialProperties getMaterialProperties(MaterialType material) const;
    
private:
    void initializeMaterialDatabase();
    void initializeTechniqueNames();
    
    // Numerical methods
    double solveKineticsEquation(
        const DepositionConditions& conditions,
        double time
    ) const;
    
    std::vector<double> solveDiffusionEquation(
        const std::vector<double>& initial_profile,
        double diffusivity,
        double time
    ) const;
    
    // Advanced models
    double calculateNucleationRate(
        MaterialType material,
        double temperature,
        double supersaturation
    ) const;
    
    double calculateGrainBoundaryDiffusion(
        MaterialType material,
        double temperature,
        double grain_size
    ) const;
};

// Utility functions for deposition analysis
namespace DepositionUtils {
    // Convert between different thickness units
    double convertThickness(double value, const std::string& from_unit, const std::string& to_unit);
    
    // Calculate deposition time for target thickness
    double calculateRequiredTime(
        double target_thickness,
        double deposition_rate
    );
    
    // Optimize deposition conditions for target specifications
    DepositionConditions optimizeConditions(
        MaterialType material,
        double target_thickness,
        double max_temperature,
        double max_time
    );
    
    // Generate deposition recipes
    std::vector<DepositionConditions> generateMultiLayerRecipe(
        const std::vector<MaterialType>& materials,
        const std::vector<double>& thicknesses
    );
}

} // namespace SemiPRO

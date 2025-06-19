// Author: Dr. Mazharuddin Mohammed
#ifndef ADVANCED_INTERCONNECTS_HPP
#define ADVANCED_INTERCONNECTS_HPP

#include "../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <Eigen/Dense>

/**
 * Advanced Interconnect Technology Simulation
 * Implements state-of-the-art interconnect processes including:
 * - Dual damascene copper interconnects
 * - Low-k dielectric integration
 * - Barrier/seed layer optimization
 * - CMP (Chemical Mechanical Planarization)
 * - Electromigration and reliability modeling
 * - Advanced packaging interconnects (TSV, RDL)
 */
class AdvancedInterconnects {
public:
    AdvancedInterconnects();
    ~AdvancedInterconnects() = default;
    
    // Interconnect layer definition
    struct InterconnectLayer {
        std::string name;
        int level;                    // Metal level (M1, M2, etc.)
        double line_width;           // nm
        double line_spacing;         // nm
        double thickness;            // nm
        std::string metal_material;  // Cu, Al, Co, Ru
        std::string dielectric;      // SiO2, low-k, ultra-low-k
        double aspect_ratio;
        bool is_global_layer;        // Global vs local interconnect
        
        InterconnectLayer(const std::string& n, int lvl, double width, double space, double thick)
            : name(n), level(lvl), line_width(width), line_spacing(space), thickness(thick),
              metal_material("Cu"), dielectric("low-k"), aspect_ratio(thick/width), 
              is_global_layer(false) {}
    };
    
    // Damascene process parameters
    struct DamasceneParameters {
        bool dual_damascene;         // Single vs dual damascene
        std::string barrier_material; // TaN, Ta, TiN, Co
        double barrier_thickness;    // nm
        std::string seed_material;   // Cu, Ru
        double seed_thickness;       // nm
        std::string fill_method;     // ECD, CVD, ALD
        bool cmp_enabled;           // Chemical mechanical planarization
        double overplating_factor;   // For CMP
        
        DamasceneParameters() : dual_damascene(true), barrier_material("TaN"), 
                              barrier_thickness(2.0), seed_material("Cu"), 
                              seed_thickness(5.0), fill_method("ECD"), 
                              cmp_enabled(true), overplating_factor(1.3) {}
    };
    
    // Low-k dielectric properties
    struct DielectricProperties {
        std::string material;
        double dielectric_constant;  // k-value
        double breakdown_voltage;    // V/nm
        double thermal_conductivity; // W/m·K
        double young_modulus;        // GPa
        double poisson_ratio;
        bool porous;                // Porous vs dense
        double porosity;            // % (if porous)
        
        DielectricProperties(const std::string& mat, double k) 
            : material(mat), dielectric_constant(k), breakdown_voltage(5.0),
              thermal_conductivity(0.2), young_modulus(10.0), poisson_ratio(0.25),
              porous(false), porosity(0.0) {}
    };
    
    // Main simulation methods
    bool simulateDualDamascene(std::shared_ptr<Wafer> wafer, 
                              const InterconnectLayer& layer,
                              const DamasceneParameters& params);
    
    bool simulateMultilevelInterconnect(std::shared_ptr<Wafer> wafer,
                                       const std::vector<InterconnectLayer>& layers);
    
    bool simulateTSV(std::shared_ptr<Wafer> wafer, double diameter, double depth, 
                    const std::string& fill_material = "Cu");
    
    bool simulateRDL(std::shared_ptr<Wafer> wafer, const InterconnectLayer& rdl_layer);
    
    // Process steps
    bool simulateBarrierDeposition(std::shared_ptr<Wafer> wafer, 
                                  const std::string& material, double thickness);
    bool simulateSeedDeposition(std::shared_ptr<Wafer> wafer,
                               const std::string& material, double thickness);
    bool simulateElectroplating(std::shared_ptr<Wafer> wafer, 
                               const std::string& material, double thickness);
    bool simulateCMP(std::shared_ptr<Wafer> wafer, double target_thickness);
    
    // Reliability analysis
    struct ReliabilityMetrics {
        double electromigration_lifetime; // hours
        double stress_migration_risk;     // relative
        double thermal_cycling_stress;    // MPa
        double via_resistance;            // Ω
        double line_resistance;           // Ω/μm
        double capacitance;              // fF/μm
        double crosstalk_coupling;       // %
        double yield_prediction;         // %
    };
    
    ReliabilityMetrics analyzeReliability(const InterconnectLayer& layer, 
                                         double current_density, double temperature) const;
    
    // Advanced features
    void enableAirGapIntegration(bool enable) { air_gap_enabled_ = enable; }
    void enableCobraHead(bool enable) { cobra_head_enabled_ = enable; }
    void setProcessTemperature(double temp) { process_temperature_ = temp; }
    void enableAdvancedBarriers(bool enable) { advanced_barriers_enabled_ = enable; }
    
    // Performance metrics
    struct InterconnectMetrics {
        double rc_delay;             // ps
        double power_consumption;    // mW
        double signal_integrity;     // dB
        double manufacturing_yield;  // %
        double cost_per_layer;      // $/cm²
        double thermal_resistance;   // K·cm²/W
    };
    
    InterconnectMetrics getMetrics() const { return metrics_; }

private:
    // Configuration flags
    bool air_gap_enabled_;
    bool cobra_head_enabled_;
    bool advanced_barriers_enabled_;
    double process_temperature_;
    
    // Performance tracking
    mutable InterconnectMetrics metrics_;
    mutable ReliabilityMetrics reliability_;
    
    // Material properties database
    struct MetalProperties {
        std::string symbol;
        double resistivity;          // μΩ·cm
        double thermal_conductivity; // W/m·K
        double melting_point;        // °C
        double electromigration_activation; // eV
        double young_modulus;        // GPa
        double poisson_ratio;
        
        MetalProperties(const std::string& sym, double res) 
            : symbol(sym), resistivity(res), thermal_conductivity(400.0),
              melting_point(1000.0), electromigration_activation(0.7),
              young_modulus(100.0), poisson_ratio(0.35) {}
    };
    
    std::unordered_map<std::string, MetalProperties> metal_database_;
    std::unordered_map<std::string, DielectricProperties> dielectric_database_;
    
    // Advanced simulation methods
    Eigen::ArrayXXd simulateBarrierCoverage(const InterconnectLayer& layer,
                                           const DamasceneParameters& params) const;
    Eigen::ArrayXXd simulateSeedContinuity(const InterconnectLayer& layer,
                                          const DamasceneParameters& params) const;
    Eigen::ArrayXXd simulateElectroplatingFill(const InterconnectLayer& layer,
                                              const DamasceneParameters& params) const;
    Eigen::ArrayXXd simulateCMPPlanarization(const Eigen::ArrayXXd& surface_profile) const;
    
    // Physics calculations
    double calculateLineResistance(const InterconnectLayer& layer, 
                                  const MetalProperties& metal) const;
    double calculateViaResistance(double diameter, double height,
                                 const MetalProperties& metal) const;
    double calculateCapacitance(const InterconnectLayer& layer,
                               const DielectricProperties& dielectric) const;
    double calculateElectromigrationLifetime(const InterconnectLayer& layer,
                                           double current_density, double temperature) const;
    
    // Stress analysis
    double calculateThermalStress(const InterconnectLayer& layer, 
                                 double temperature_delta) const;
    double calculateMechanicalStress(const InterconnectLayer& layer,
                                    const DielectricProperties& dielectric) const;
    
    // Yield modeling
    double calculateManufacturingYield(const InterconnectLayer& layer) const;
    double calculateDefectDensity(const InterconnectLayer& layer) const;
    
    // Advanced process modeling
    bool simulateAirGapFormation(std::shared_ptr<Wafer> wafer,
                                const InterconnectLayer& layer) const;
    bool simulateCobraHeadSuppression(std::shared_ptr<Wafer> wafer,
                                     const DamasceneParameters& params) const;
    
    // Utility methods
    void initializeMetalDatabase();
    void initializeDielectricDatabase();
    MetalProperties getMetalProperties(const std::string& material) const;
    DielectricProperties getDielectricProperties(const std::string& material) const;
    void updateMetrics(const InterconnectLayer& layer) const;
    
    // Parasitic extraction
    double extractResistance(const InterconnectLayer& layer) const;
    double extractCapacitance(const InterconnectLayer& layer) const;
    double extractInductance(const InterconnectLayer& layer) const;
    
    // Signal integrity analysis
    double calculateCrosstalk(const InterconnectLayer& layer) const;
    double calculateSignalDelay(const InterconnectLayer& layer) const;
    double calculatePowerNoise(const InterconnectLayer& layer) const;
};

// Factory functions for different interconnect technologies
std::unique_ptr<AdvancedInterconnects> createAdvancedCopperInterconnect();
std::unique_ptr<AdvancedInterconnects> createCobaltInterconnect();
std::unique_ptr<AdvancedInterconnects> createRutheniumInterconnect();

// Utility functions
std::vector<AdvancedInterconnects::InterconnectLayer> 
createStandardInterconnectStack(int num_levels, double technology_node);

std::vector<AdvancedInterconnects::InterconnectLayer>
createAdvancedPackagingStack();

bool validateInterconnectDesign(const std::vector<AdvancedInterconnects::InterconnectLayer>& layers);

// Technology scaling functions
double calculateInterconnectScaling(double technology_node, const std::string& parameter);
double predictFuturePerformance(double current_node, double target_node, const std::string& metric);

#endif // ADVANCED_INTERCONNECTS_HPP

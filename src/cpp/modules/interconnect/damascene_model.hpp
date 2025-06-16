#ifndef DAMASCENE_MODEL_HPP
#define DAMASCENE_MODEL_HPP

#include "interconnect_interface.hpp"
#include "../../core/utils.hpp"
#include <cmath>
#include <random>

class DamasceneModel : public InterconnectInterface {
public:
    DamasceneModel();
    ~DamasceneModel() override = default;
    
    // Implement interface methods
    void createInterconnectStack(
        std::shared_ptr<Wafer> wafer,
        const std::vector<MetalLevel>& metal_levels
    ) override;
    
    void simulateDamasceneProcess(
        std::shared_ptr<Wafer> wafer,
        const MetalLevel& level,
        InterconnectType type
    ) override;
    
    void createVias(
        std::shared_ptr<Wafer> wafer,
        const std::vector<std::pair<double, double>>& via_positions,
        double via_diameter,
        int from_level,
        int to_level
    ) override;
    
    void simulateCMP(
        std::shared_ptr<Wafer> wafer,
        const std::string& target_material,
        double target_thickness
    ) override;
    
    std::unordered_map<std::string, double> calculateElectricalProperties(
        std::shared_ptr<Wafer> wafer,
        const InterconnectStructure& structure
    ) override;
    
    double calculateResistance(
        const MetalLevel& level,
        double length
    ) override;
    
    double calculateCapacitance(
        const MetalLevel& level1,
        const MetalLevel& level2,
        double overlap_area
    ) override;
    
    double calculateElectromigrationMTTF(
        const MetalLevel& level,
        double current_density,
        double temperature
    ) override;
    
    std::vector<double> analyzeStressMigration(
        std::shared_ptr<Wafer> wafer,
        const InterconnectStructure& structure
    ) override;
    
    // Damascene-specific processes
    void simulateTrenchEtching(
        std::shared_ptr<Wafer> wafer,
        const MetalLevel& level,
        const std::vector<std::pair<double, double>>& trench_positions
    );
    
    void simulateViaEtching(
        std::shared_ptr<Wafer> wafer,
        const std::vector<std::pair<double, double>>& via_positions,
        double via_diameter,
        double etch_depth
    );
    
    void simulateBarrierDeposition(
        std::shared_ptr<Wafer> wafer,
        const std::string& barrier_material,
        double thickness
    );
    
    void simulateSeedDeposition(
        std::shared_ptr<Wafer> wafer,
        const std::string& seed_material,
        double thickness
    );
    
    void simulateElectroplating(
        std::shared_ptr<Wafer> wafer,
        const std::string& metal,
        double target_thickness,
        double current_density
    );
    
    // Process monitoring and control
    struct ProcessParameters {
        double etch_rate;
        double selectivity;
        double aspect_ratio_limit;
        double plating_efficiency;
        double cmp_removal_rate;
        std::unordered_map<std::string, double> material_properties;
    };
    
    void setProcessParameters(const ProcessParameters& params);
    ProcessParameters getProcessParameters() const { return process_params_; }
    
    // Defect analysis
    enum DefectType {
        VOID,
        SEAM,
        OVERHANG,
        UNDERCUT,
        DISHING,
        EROSION
    };
    
    struct Defect {
        DefectType type;
        double x, y, z;
        double severity;
        std::string description;
        
        Defect(DefectType t, double px, double py, double pz, double sev)
            : type(t), x(px), y(py), z(pz), severity(sev) {}
    };
    
    std::vector<Defect> analyzeDefects(
        std::shared_ptr<Wafer> wafer,
        const InterconnectStructure& structure
    );
    
    // Advanced modeling
    void simulateStressEvolution(
        std::shared_ptr<Wafer> wafer,
        const MetalLevel& level,
        double temperature,
        double time
    );
    
    void simulateThermomechanicalStress(
        std::shared_ptr<Wafer> wafer,
        const std::vector<MetalLevel>& levels,
        double temperature_cycle_amplitude
    );
    
    // Low-k dielectric integration
    void simulateLowKIntegration(
        std::shared_ptr<Wafer> wafer,
        const std::string& low_k_material,
        double dielectric_constant,
        const MetalLevel& level
    );
    
private:
    ProcessParameters process_params_;
    std::unordered_map<std::string, double> material_resistivities_;
    std::unordered_map<std::string, double> material_thermal_expansion_;
    mutable std::mt19937 rng_;
    
    // Helper functions
    double calculateAspectRatio(double width, double depth);
    double calculatePlatingUniformity(double current_density, double aspect_ratio);
    double calculateCMPUniformity(const std::string& material, double pattern_density);
    
    // Physical models
    double calculateVoidProbability(double aspect_ratio, double plating_rate);
    double calculateStressMagnitude(
        const std::string& material1,
        const std::string& material2,
        double temperature
    );
    
    // Pattern density effects
    double calculatePatternDensity(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        double window_size
    );
    
    void applyPatternDensityEffects(
        std::shared_ptr<Wafer> wafer,
        const MetalLevel& level
    );
};

#endif // DAMASCENE_MODEL_HPP

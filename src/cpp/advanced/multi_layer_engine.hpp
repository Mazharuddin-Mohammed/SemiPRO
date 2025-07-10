#pragma once

#include "../core/advanced_logger.hpp"
#include "../core/enhanced_error_handling.hpp"
#include "../core/config_manager.hpp"
#include "../core/wafer_enhanced.hpp"
#include "../physics/enhanced_oxidation.hpp"
#include "../physics/enhanced_doping.hpp"
#include "../physics/enhanced_deposition.hpp"
#include "../physics/enhanced_etching.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cmath>

namespace SemiPRO {

/**
 * Advanced Multi-Layer Processing Engine
 * Implements sophisticated multi-layer stack processing with layer interactions,
 * interface effects, stack optimization, and cross-layer dependencies
 */

// Layer types with detailed properties
enum class LayerType {
    SUBSTRATE,        // Base substrate (Si, GaAs, etc.)
    EPITAXIAL,        // Epitaxial layer
    OXIDE,            // Oxide layers (SiO2, HfO2, etc.)
    NITRIDE,          // Nitride layers (Si3N4, TiN, etc.)
    METAL,            // Metal layers (Al, Cu, W, etc.)
    POLYSILICON,      // Polysilicon layers
    DIELECTRIC,       // Dielectric layers (low-k, etc.)
    BARRIER,          // Barrier layers (TiN, TaN, etc.)
    PHOTORESIST,      // Photoresist layers
    HARD_MASK,        // Hard mask layers
    SACRIFICIAL,      // Sacrificial layers
    PASSIVATION       // Passivation layers
};

// Interface types and properties
enum class InterfaceType {
    ABRUPT,           // Sharp interface
    GRADED,           // Graded interface
    INTERMIXED,       // Intermixed interface
    ROUGH,            // Rough interface
    DEFECTIVE         // Interface with defects
};

// Stack optimization objectives
enum class OptimizationObjective {
    MINIMIZE_STRESS,      // Minimize overall stack stress
    MAXIMIZE_UNIFORMITY,  // Maximize thickness uniformity
    MINIMIZE_DEFECTS,     // Minimize defect density
    OPTIMIZE_ELECTRICAL,  // Optimize electrical properties
    MINIMIZE_COST,        // Minimize process cost
    MAXIMIZE_YIELD        // Maximize process yield
};

// Advanced layer definition
struct AdvancedLayer {
    std::string id;                    // Unique layer identifier
    LayerType type;                    // Layer type
    std::string material;              // Material composition
    double target_thickness;           // Target thickness (μm)
    double actual_thickness;           // Actual thickness (μm)
    double uniformity;                 // Thickness uniformity (%)
    double stress;                     // Intrinsic stress (MPa)
    double roughness;                  // Surface roughness (nm)
    std::unordered_map<std::string, double> properties; // Material properties
    std::unordered_map<std::string, double> process_params; // Process parameters
    std::vector<std::string> dependencies; // Layer dependencies
    
    AdvancedLayer() : type(LayerType::SUBSTRATE), target_thickness(0), 
                     actual_thickness(0), uniformity(100.0), stress(0), roughness(0) {}
};

// Interface definition
struct LayerInterface {
    std::string upper_layer_id;        // Upper layer ID
    std::string lower_layer_id;        // Lower layer ID
    InterfaceType type;                // Interface type
    double roughness;                  // Interface roughness (nm)
    double intermixing_depth;          // Intermixing depth (nm)
    double defect_density;             // Interface defect density (cm⁻²)
    std::unordered_map<std::string, double> properties; // Interface properties
    
    LayerInterface() : type(InterfaceType::ABRUPT), roughness(0), 
                      intermixing_depth(0), defect_density(0) {}
};

// Multi-layer stack definition
struct LayerStack {
    std::string stack_id;              // Stack identifier
    std::vector<AdvancedLayer> layers; // Ordered layers (bottom to top)
    std::vector<LayerInterface> interfaces; // Layer interfaces
    double total_thickness;            // Total stack thickness
    double total_stress;               // Total stack stress
    double overall_uniformity;         // Overall uniformity
    std::unordered_map<std::string, double> stack_properties; // Stack-level properties
    
    LayerStack() : total_thickness(0), total_stress(0), overall_uniformity(100.0) {}
};

// Process sequence definition
struct ProcessSequence {
    std::string sequence_id;           // Sequence identifier
    std::vector<std::string> process_steps; // Ordered process steps
    std::unordered_map<std::string, std::unordered_map<std::string, double>> step_parameters;
    std::vector<std::string> critical_dimensions; // Critical dimensions to monitor
    std::unordered_map<std::string, double> target_specs; // Target specifications
    
    ProcessSequence() {}
};

// Stack optimization results
struct OptimizationResults {
    LayerStack optimized_stack;        // Optimized stack configuration
    ProcessSequence optimized_sequence; // Optimized process sequence
    double objective_value;            // Optimization objective value
    std::vector<std::string> recommendations; // Process recommendations
    std::unordered_map<std::string, double> quality_metrics; // Quality metrics
    
    OptimizationResults() : objective_value(0) {}
};

// Multi-layer processing engine
class MultiLayerEngine {
private:
    std::unordered_map<std::string, LayerStack> stacks_;
    std::unordered_map<std::string, ProcessSequence> sequences_;
    std::unordered_map<LayerType, std::string> layer_type_names_;
    
    // Physics engines
    std::unique_ptr<EnhancedOxidationPhysics> oxidation_engine_;
    std::unique_ptr<EnhancedDopingPhysics> doping_engine_;
    std::unique_ptr<EnhancedDepositionPhysics> deposition_engine_;
    std::unique_ptr<EnhancedEtchingPhysics> etching_engine_;
    
    // Configuration
    bool enable_interface_effects_ = true;
    bool enable_stress_coupling_ = true;
    bool enable_thermal_coupling_ = true;
    bool enable_optimization_ = true;
    double convergence_tolerance_ = 1e-6;
    
public:
    MultiLayerEngine();
    ~MultiLayerEngine() = default;
    
    // Stack management
    void createStack(const std::string& stack_id);
    void addLayerToStack(const std::string& stack_id, const AdvancedLayer& layer);
    void removeLayerFromStack(const std::string& stack_id, const std::string& layer_id);
    LayerStack& getStack(const std::string& stack_id);
    const LayerStack& getStack(const std::string& stack_id) const;
    
    // Layer processing
    bool processLayer(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& stack_id,
        const std::string& layer_id,
        const std::string& process_type,
        const std::unordered_map<std::string, double>& parameters
    );
    
    bool processStack(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& stack_id,
        const ProcessSequence& sequence
    );
    
    // Interface modeling
    void calculateInterfaceProperties(
        const std::string& stack_id,
        const std::string& interface_id
    );
    
    void modelInterfaceEffects(
        std::shared_ptr<WaferEnhanced> wafer,
        const LayerInterface& interface
    );
    
    // Stress analysis
    void calculateStackStress(const std::string& stack_id);
    void propagateStressEffects(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& stack_id
    );
    
    // Thermal analysis
    void calculateThermalProfile(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& stack_id,
        double ambient_temperature = 300.0
    );
    
    void modelThermalCoupling(
        const std::string& stack_id,
        double temperature
    );
    
    // Cross-layer dependencies
    void analyzeDependencies(const std::string& stack_id);
    void propagateDependencyEffects(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& stack_id
    );
    
    // Stack optimization
    OptimizationResults optimizeStack(
        const std::string& stack_id,
        OptimizationObjective objective,
        const std::unordered_map<std::string, double>& constraints
    );
    
    OptimizationResults optimizeProcessSequence(
        const std::string& stack_id,
        const ProcessSequence& initial_sequence,
        OptimizationObjective objective
    );
    
    // Quality analysis
    std::unordered_map<std::string, double> analyzeStackQuality(
        const std::string& stack_id
    ) const;
    
    std::vector<std::string> validateStack(
        const std::string& stack_id
    ) const;
    
    // Advanced features
    void simulateProcessVariation(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& stack_id,
        double variation_percentage = 5.0
    );
    
    void modelDefectPropagation(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& stack_id
    );
    
    // Configuration and calibration
    void enablePhysicsEffects(bool interface = true, bool stress = true,
                             bool thermal = true, bool optimization = true);
    
    void setConvergenceTolerance(double tolerance);
    
    // Utility methods
    std::string layerTypeToString(LayerType type) const;
    std::string interfaceTypeToString(InterfaceType type) const;
    std::string objectiveToString(OptimizationObjective objective) const;
    
    // Validation and diagnostics
    bool validateStackConfiguration(const std::string& stack_id, std::string& error_message) const;
    std::vector<std::string> getDiagnostics(const std::string& stack_id) const;

private:
    // Process-specific methods
    bool processOxidation(
        std::shared_ptr<WaferEnhanced> wafer,
        AdvancedLayer& layer,
        const std::unordered_map<std::string, double>& parameters
    );

    bool processDeposition(
        std::shared_ptr<WaferEnhanced> wafer,
        AdvancedLayer& layer,
        const std::unordered_map<std::string, double>& parameters
    );

    bool processEtching(
        std::shared_ptr<WaferEnhanced> wafer,
        AdvancedLayer& layer,
        const std::unordered_map<std::string, double>& parameters
    );

    bool processDoping(
        std::shared_ptr<WaferEnhanced> wafer,
        AdvancedLayer& layer,
        const std::unordered_map<std::string, double>& parameters
    );

    // Interface analysis
    void calculateInterfaceEffects(
        const std::string& stack_id,
        const std::string& layer_id
    );
    void initializeLayerTypeNames();
    void initializePhysicsEngines();
    
    // Optimization algorithms
    OptimizationResults geneticAlgorithmOptimization(
        const std::string& stack_id,
        OptimizationObjective objective,
        const std::unordered_map<std::string, double>& constraints
    );
    
    OptimizationResults gradientDescentOptimization(
        const std::string& stack_id,
        OptimizationObjective objective,
        const std::unordered_map<std::string, double>& constraints
    );
    
    // Objective functions
    double calculateObjectiveValue(
        const LayerStack& stack,
        OptimizationObjective objective
    ) const;
    
    // Helper methods
    double calculateInterfaceRoughness(
        const AdvancedLayer& upper_layer,
        const AdvancedLayer& lower_layer
    ) const;
    
    double calculateStressMismatch(
        const AdvancedLayer& layer1,
        const AdvancedLayer& layer2
    ) const;
    
    void updateLayerFromProcess(
        AdvancedLayer& layer,
        const std::string& process_type,
        const std::unordered_map<std::string, double>& parameters
    );
};

// Utility functions for multi-layer analysis
namespace MultiLayerUtils {
    // Stack analysis utilities
    double calculateStackThickness(const LayerStack& stack);
    double calculateStackStress(const LayerStack& stack);
    double calculateStackUniformity(const LayerStack& stack);
    
    // Layer compatibility analysis
    bool areLayersCompatible(const AdvancedLayer& layer1, const AdvancedLayer& layer2);
    double calculateCompatibilityScore(const AdvancedLayer& layer1, const AdvancedLayer& layer2);
    
    // Process optimization utilities
    std::vector<std::string> generateProcessSequence(const LayerStack& stack);
    std::unordered_map<std::string, double> estimateProcessCost(const ProcessSequence& sequence);
    
    // Quality metrics
    double calculateDefectDensity(const LayerStack& stack);
    double calculateElectricalPerformance(const LayerStack& stack);
    double calculateReliabilityScore(const LayerStack& stack);
}

} // namespace SemiPRO

#include "multi_layer_engine.hpp"
#include <algorithm>
#include <cmath>
#include <random>

namespace SemiPRO {

MultiLayerEngine::MultiLayerEngine() {
    initializeLayerTypeNames();
    initializePhysicsEngines();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Multi-Layer Processing Engine initialized", "MultiLayerEngine");
}

void MultiLayerEngine::createStack(const std::string& stack_id) {
    LayerStack stack;
    stack.stack_id = stack_id;
    stacks_[stack_id] = stack;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Created layer stack: " + stack_id, "MultiLayerEngine");
}

void MultiLayerEngine::addLayerToStack(const std::string& stack_id, const AdvancedLayer& layer) {
    auto it = stacks_.find(stack_id);
    if (it == stacks_.end()) {
        throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, "Stack not found: " + stack_id, "STACK_NOT_FOUND"));
    }
    
    it->second.layers.push_back(layer);
    it->second.total_thickness += layer.target_thickness;
    
    // Create interface if not the first layer
    if (it->second.layers.size() > 1) {
        LayerInterface interface;
        interface.lower_layer_id = it->second.layers[it->second.layers.size() - 2].id;
        interface.upper_layer_id = layer.id;
        interface.type = InterfaceType::ABRUPT;
        it->second.interfaces.push_back(interface);
    }
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Added layer " + layer.id + " to stack " + stack_id, "MultiLayerEngine");
}

bool MultiLayerEngine::processLayer(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& stack_id,
    const std::string& layer_id,
    const std::string& process_type,
    const std::unordered_map<std::string, double>& parameters) {
    
    SEMIPRO_PERF_TIMER("multi_layer_process", "MultiLayerEngine");
    
    try {
        auto& stack = getStack(stack_id);
        
        // Find the layer
        auto layer_it = std::find_if(stack.layers.begin(), stack.layers.end(),
            [&layer_id](const AdvancedLayer& layer) { return layer.id == layer_id; });
        
        if (layer_it == stack.layers.end()) {
            throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, "Layer not found: " + layer_id, "LAYER_NOT_FOUND"));
        }
        
        // Process based on type
        bool success = false;
        if (process_type == "oxidation") {
            success = processOxidation(wafer, *layer_it, parameters);
        } else if (process_type == "deposition") {
            success = processDeposition(wafer, *layer_it, parameters);
        } else if (process_type == "etching") {
            success = processEtching(wafer, *layer_it, parameters);
        } else if (process_type == "doping") {
            success = processDoping(wafer, *layer_it, parameters);
        } else {
            throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, "Unknown process type: " + process_type, "UNKNOWN_PROCESS"));
        }
        
        if (success) {
            // Update layer properties
            updateLayerFromProcess(*layer_it, process_type, parameters);
            
            // Calculate interface effects if enabled
            if (enable_interface_effects_) {
                calculateInterfaceEffects(stack_id, layer_id);
            }
            
            // Propagate stress effects if enabled
            if (enable_stress_coupling_) {
                propagateStressEffects(wafer, stack_id);
            }
        }
        
        return success;
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Layer processing failed: " + std::string(e.what()),
                          "MultiLayerEngine");
        return false;
    }
}

bool MultiLayerEngine::processStack(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& stack_id,
    const ProcessSequence& sequence) {
    
    SEMIPRO_PERF_TIMER("stack_processing", "MultiLayerEngine");
    
    try {
        auto& stack = getStack(stack_id);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Processing stack " + stack_id + " with " + 
                          std::to_string(sequence.process_steps.size()) + " steps",
                          "MultiLayerEngine");
        
        // Process each step in sequence
        for (size_t i = 0; i < sequence.process_steps.size(); ++i) {
            const std::string& step = sequence.process_steps[i];
            
            // Extract process type and layer from step
            auto pos = step.find(':');
            if (pos == std::string::npos) {
                throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, "Invalid process step format: " + step, "INVALID_STEP_FORMAT"));
            }
            
            std::string process_type = step.substr(0, pos);
            std::string layer_id = step.substr(pos + 1);
            
            // Get parameters for this step
            auto param_it = sequence.step_parameters.find(step);
            std::unordered_map<std::string, double> parameters;
            if (param_it != sequence.step_parameters.end()) {
                parameters = param_it->second;
            }
            
            // Process the layer
            bool success = processLayer(wafer, stack_id, layer_id, process_type, parameters);
            if (!success) {
                SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                                  "Failed at step " + std::to_string(i) + ": " + step,
                                  "MultiLayerEngine");
                return false;
            }
        }
        
        // Final stack analysis
        calculateStackStress(stack_id);
        analyzeDependencies(stack_id);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Stack processing completed successfully", "MultiLayerEngine");
        
        return true;
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Stack processing failed: " + std::string(e.what()),
                          "MultiLayerEngine");
        return false;
    }
}

void MultiLayerEngine::calculateStackStress(const std::string& stack_id) {
    auto& stack = getStack(stack_id);
    
    double total_stress = 0.0;
    double total_thickness = 0.0;
    
    for (const auto& layer : stack.layers) {
        // Calculate layer contribution to total stress
        double layer_contribution = layer.stress * layer.actual_thickness;
        total_stress += layer_contribution;
        total_thickness += layer.actual_thickness;
    }
    
    // Calculate average stress
    if (total_thickness > 0) {
        stack.total_stress = total_stress / total_thickness;
    }
    
    // Add interface stress contributions
    for (const auto& interface : stack.interfaces) {
        // Simplified interface stress model
        double interface_stress = interface.defect_density * 1e-6; // Convert to MPa
        stack.total_stress += interface_stress;
    }
    
    SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::ADVANCED,
                      "Stack stress calculated: " + std::to_string(stack.total_stress) + " MPa",
                      "MultiLayerEngine");
}

void MultiLayerEngine::propagateStressEffects(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& stack_id) {
    
    auto& stack = getStack(stack_id);
    
    // Create stress field based on stack stress
    auto grid = wafer->getGrid();
    int rows = grid.rows();
    int cols = grid.cols();
    
    Eigen::ArrayXXd stress_field(rows, cols);
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            // Simplified stress propagation model
            double local_stress = stack.total_stress;
            
            // Add spatial variation (edge effects)
            double edge_factor = 1.0;
            if (i < rows * 0.1 || i > rows * 0.9 || j < cols * 0.1 || j > cols * 0.9) {
                edge_factor = 1.2; // Higher stress at edges
            }
            
            stress_field(i, j) = local_stress * edge_factor;
        }
    }
    
    // Apply stress effects to wafer (simplified)
    wafer->calculateStress();
}

OptimizationResults MultiLayerEngine::optimizeStack(
    const std::string& stack_id,
    OptimizationObjective objective,
    const std::unordered_map<std::string, double>& constraints) {
    
    SEMIPRO_PERF_TIMER("stack_optimization", "MultiLayerEngine");
    
    OptimizationResults results;
    
    try {
        auto& stack = getStack(stack_id);
        results.optimized_stack = stack; // Start with current stack
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Starting stack optimization for objective: " + 
                          objectiveToString(objective), "MultiLayerEngine");
        
        // Use genetic algorithm for optimization
        results = geneticAlgorithmOptimization(stack_id, objective, constraints);
        
        // Generate recommendations
        results.recommendations.push_back("Optimized stack configuration generated");
        results.recommendations.push_back("Consider process parameter fine-tuning");
        results.recommendations.push_back("Monitor critical dimensions during processing");
        
        // Calculate quality metrics
        results.quality_metrics = analyzeStackQuality(stack_id);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Stack optimization completed with objective value: " + 
                          std::to_string(results.objective_value), "MultiLayerEngine");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Stack optimization failed: " + std::string(e.what()),
                          "MultiLayerEngine");
    }
    
    return results;
}

std::unordered_map<std::string, double> MultiLayerEngine::analyzeStackQuality(
    const std::string& stack_id) const {
    
    std::unordered_map<std::string, double> quality;
    
    auto& stack = getStack(stack_id);
    
    // Calculate uniformity score
    double uniformity_sum = 0.0;
    for (const auto& layer : stack.layers) {
        uniformity_sum += layer.uniformity;
    }
    quality["uniformity"] = uniformity_sum / stack.layers.size();
    
    // Calculate stress score (lower is better)
    double stress_score = 100.0 - std::min(100.0, std::abs(stack.total_stress) / 1000.0 * 100.0);
    quality["stress_score"] = stress_score;
    
    // Calculate interface quality
    double interface_quality = 100.0;
    for (const auto& interface : stack.interfaces) {
        interface_quality -= interface.defect_density / 1e12 * 10.0; // Normalize
    }
    quality["interface_quality"] = std::max(0.0, interface_quality);
    
    // Overall quality score
    double overall = (quality["uniformity"] + quality["stress_score"] + quality["interface_quality"]) / 3.0;
    quality["overall_score"] = overall;
    
    return quality;
}

LayerStack& MultiLayerEngine::getStack(const std::string& stack_id) {
    auto it = stacks_.find(stack_id);
    if (it == stacks_.end()) {
        throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, "Stack not found: " + stack_id, "STACK_NOT_FOUND"));
    }
    return it->second;
}

const LayerStack& MultiLayerEngine::getStack(const std::string& stack_id) const {
    auto it = stacks_.find(stack_id);
    if (it == stacks_.end()) {
        throw SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::VALIDATION, "Stack not found: " + stack_id, "STACK_NOT_FOUND"));
    }
    return it->second;
}

void MultiLayerEngine::initializeLayerTypeNames() {
    layer_type_names_[LayerType::SUBSTRATE] = "Substrate";
    layer_type_names_[LayerType::EPITAXIAL] = "Epitaxial";
    layer_type_names_[LayerType::OXIDE] = "Oxide";
    layer_type_names_[LayerType::NITRIDE] = "Nitride";
    layer_type_names_[LayerType::METAL] = "Metal";
    layer_type_names_[LayerType::POLYSILICON] = "Polysilicon";
    layer_type_names_[LayerType::DIELECTRIC] = "Dielectric";
    layer_type_names_[LayerType::BARRIER] = "Barrier";
    layer_type_names_[LayerType::PHOTORESIST] = "Photoresist";
    layer_type_names_[LayerType::HARD_MASK] = "Hard Mask";
    layer_type_names_[LayerType::SACRIFICIAL] = "Sacrificial";
    layer_type_names_[LayerType::PASSIVATION] = "Passivation";
}

void MultiLayerEngine::initializePhysicsEngines() {
    oxidation_engine_ = std::make_unique<EnhancedOxidationPhysics>();
    doping_engine_ = std::make_unique<EnhancedDopingPhysics>();
    deposition_engine_ = std::make_unique<EnhancedDepositionPhysics>();
    etching_engine_ = std::make_unique<EnhancedEtchingPhysics>();
}

std::string MultiLayerEngine::layerTypeToString(LayerType type) const {
    auto it = layer_type_names_.find(type);
    return (it != layer_type_names_.end()) ? it->second : "Unknown";
}

std::string MultiLayerEngine::objectiveToString(OptimizationObjective objective) const {
    switch (objective) {
        case OptimizationObjective::MINIMIZE_STRESS: return "Minimize Stress";
        case OptimizationObjective::MAXIMIZE_UNIFORMITY: return "Maximize Uniformity";
        case OptimizationObjective::MINIMIZE_DEFECTS: return "Minimize Defects";
        case OptimizationObjective::OPTIMIZE_ELECTRICAL: return "Optimize Electrical";
        case OptimizationObjective::MINIMIZE_COST: return "Minimize Cost";
        case OptimizationObjective::MAXIMIZE_YIELD: return "Maximize Yield";
        default: return "Unknown";
    }
}

bool MultiLayerEngine::processOxidation(
    std::shared_ptr<WaferEnhanced> wafer,
    AdvancedLayer& layer,
    const std::unordered_map<std::string, double>& parameters) {

    // Create oxidation conditions
    OxidationConditions conditions;

    auto temp_it = parameters.find("temperature");
    if (temp_it != parameters.end()) {
        conditions.temperature = temp_it->second;
    }

    auto time_it = parameters.find("time");
    if (time_it != parameters.end()) {
        conditions.time = time_it->second;
    }

    auto ambient_it = parameters.find("ambient");
    if (ambient_it != parameters.end()) {
        conditions.atmosphere = (ambient_it->second > 0.5) ? OxidationAtmosphere::DRY_O2 : OxidationAtmosphere::WET_H2O;
    }

    // Run oxidation simulation
    auto results = oxidation_engine_->simulateOxidation(wafer, conditions);

    // Update layer properties
    layer.actual_thickness = results.final_thickness;
    layer.stress = results.stress_level;
    layer.roughness = results.interface_roughness;

    return true;
}

bool MultiLayerEngine::processDeposition(
    std::shared_ptr<WaferEnhanced> wafer,
    AdvancedLayer& layer,
    const std::unordered_map<std::string, double>& parameters) {

    // Create deposition conditions
    DepositionConditions conditions;
    conditions.target_thickness = layer.target_thickness;

    auto temp_it = parameters.find("temperature");
    if (temp_it != parameters.end()) {
        conditions.temperature = temp_it->second;
    }

    auto pressure_it = parameters.find("pressure");
    if (pressure_it != parameters.end()) {
        conditions.pressure = pressure_it->second;
    }

    // Set material and technique based on layer type
    if (layer.type == LayerType::METAL) {
        conditions.material = MaterialType::ALUMINUM;
        conditions.technique = DepositionTechnique::PVD_SPUTTERING;
    } else if (layer.type == LayerType::OXIDE) {
        conditions.material = MaterialType::SILICON_DIOXIDE;
        conditions.technique = DepositionTechnique::CVD;
    } else {
        conditions.material = MaterialType::POLYSILICON;
        conditions.technique = DepositionTechnique::CVD;
    }

    // Run deposition simulation
    auto results = deposition_engine_->simulateDeposition(wafer, conditions);

    // Update layer properties
    layer.actual_thickness = results.final_thickness;
    layer.uniformity = results.uniformity;
    layer.roughness = results.surface_roughness;

    return true;
}

bool MultiLayerEngine::processEtching(
    std::shared_ptr<WaferEnhanced> wafer,
    AdvancedLayer& layer,
    const std::unordered_map<std::string, double>& parameters) {

    // Create etching conditions
    EtchingConditions conditions;
    conditions.target_depth = layer.target_thickness;

    auto pressure_it = parameters.find("pressure");
    if (pressure_it != parameters.end()) {
        conditions.pressure = pressure_it->second;
    }

    auto power_it = parameters.find("power");
    if (power_it != parameters.end()) {
        conditions.power = power_it->second;
    }

    // Set material and technique based on layer type
    if (layer.type == LayerType::OXIDE) {
        conditions.target_material = EtchMaterial::SILICON_DIOXIDE;
        conditions.chemistry = EtchChemistry::FLUORINE_BASED;
    } else if (layer.type == LayerType::METAL) {
        conditions.target_material = EtchMaterial::ALUMINUM;
        conditions.chemistry = EtchChemistry::CHLORINE_BASED;
    } else {
        conditions.target_material = EtchMaterial::SILICON;
        conditions.chemistry = EtchChemistry::FLUORINE_BASED;
    }

    conditions.technique = EtchingTechnique::RIE;

    // Run etching simulation
    auto results = etching_engine_->simulateEtching(wafer, conditions);

    // Update layer properties (etching removes material)
    layer.actual_thickness -= results.final_depth;
    layer.uniformity = results.uniformity;
    layer.roughness = results.surface_roughness;

    return true;
}

bool MultiLayerEngine::processDoping(
    std::shared_ptr<WaferEnhanced> wafer,
    AdvancedLayer& layer,
    const std::unordered_map<std::string, double>& parameters) {

    // Create implantation conditions
    ImplantationConditions conditions;

    auto energy_it = parameters.find("energy");
    if (energy_it != parameters.end()) {
        conditions.energy = energy_it->second;
    }

    auto dose_it = parameters.find("dose");
    if (dose_it != parameters.end()) {
        conditions.dose = dose_it->second;
    }

    auto species_it = parameters.find("species");
    if (species_it != parameters.end()) {
        conditions.species = (species_it->second > 0.5) ? IonSpecies::BORON_11 : IonSpecies::PHOSPHORUS_31;
    }

    // Run doping simulation
    auto results = doping_engine_->simulateIonImplantation(wafer, conditions);

    // Update layer properties
    layer.properties["peak_concentration"] = results.peak_concentration;
    layer.properties["junction_depth"] = results.junction_depth;
    layer.properties["sheet_resistance"] = results.sheet_resistance;

    return true;
}

void MultiLayerEngine::calculateInterfaceEffects(
    const std::string& stack_id,
    const std::string& layer_id) {

    auto& stack = getStack(stack_id);

    // Find interfaces involving this layer
    for (auto& interface : stack.interfaces) {
        if (interface.upper_layer_id == layer_id || interface.lower_layer_id == layer_id) {
            // Calculate interface roughness
            auto upper_it = std::find_if(stack.layers.begin(), stack.layers.end(),
                [&interface](const AdvancedLayer& layer) { return layer.id == interface.upper_layer_id; });
            auto lower_it = std::find_if(stack.layers.begin(), stack.layers.end(),
                [&interface](const AdvancedLayer& layer) { return layer.id == interface.lower_layer_id; });

            if (upper_it != stack.layers.end() && lower_it != stack.layers.end()) {
                interface.roughness = calculateInterfaceRoughness(*upper_it, *lower_it);
                interface.defect_density = interface.roughness * 1e10; // Simplified correlation
            }
        }
    }
}

double MultiLayerEngine::calculateInterfaceRoughness(
    const AdvancedLayer& upper_layer,
    const AdvancedLayer& lower_layer) const {

    // Simplified interface roughness model
    double roughness_contribution = (upper_layer.roughness + lower_layer.roughness) / 2.0;

    // Add stress mismatch contribution
    double stress_mismatch = std::abs(upper_layer.stress - lower_layer.stress);
    double stress_contribution = stress_mismatch / 1000.0; // Convert MPa to nm

    return roughness_contribution + stress_contribution;
}

void MultiLayerEngine::updateLayerFromProcess(
    AdvancedLayer& layer,
    const std::string& process_type,
    const std::unordered_map<std::string, double>& parameters) {

    // Update process parameters
    for (const auto& param : parameters) {
        layer.process_params[process_type + "_" + param.first] = param.second;
    }

    // Update layer properties based on process
    if (process_type == "oxidation") {
        layer.type = LayerType::OXIDE;
        layer.material = "SiO2";
    } else if (process_type == "deposition") {
        // Material already set in process function
    } else if (process_type == "etching") {
        // Thickness already updated in process function
    }
}

OptimizationResults MultiLayerEngine::geneticAlgorithmOptimization(
    const std::string& stack_id,
    OptimizationObjective objective,
    const std::unordered_map<std::string, double>& constraints) {

    OptimizationResults results;
    auto& stack = getStack(stack_id);

    // Simplified genetic algorithm
    const int population_size = 20;
    const int generations = 50;

    std::vector<LayerStack> population(population_size, stack);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.8, 1.2);

    // Initialize population with variations
    for (auto& individual : population) {
        for (auto& layer : individual.layers) {
            layer.target_thickness *= dis(gen);
        }
    }

    // Evolution loop
    for (int gen = 0; gen < generations; ++gen) {
        // Evaluate fitness
        std::vector<double> fitness(population_size);
        for (int i = 0; i < population_size; ++i) {
            fitness[i] = calculateObjectiveValue(population[i], objective);
        }

        // Find best individual
        auto best_it = std::max_element(fitness.begin(), fitness.end());
        int best_idx = std::distance(fitness.begin(), best_it);

        if (gen == generations - 1) {
            results.optimized_stack = population[best_idx];
            results.objective_value = *best_it;
        }
    }

    return results;
}

double MultiLayerEngine::calculateObjectiveValue(
    const LayerStack& stack,
    OptimizationObjective objective) const {

    switch (objective) {
        case OptimizationObjective::MINIMIZE_STRESS:
            return 1000.0 / (1.0 + std::abs(stack.total_stress));

        case OptimizationObjective::MAXIMIZE_UNIFORMITY:
            return stack.overall_uniformity;

        case OptimizationObjective::MINIMIZE_DEFECTS: {
            double total_defects = 0.0;
            for (const auto& interface : stack.interfaces) {
                total_defects += interface.defect_density;
            }
            return 1e12 / (1.0 + total_defects);
        }

        default:
            return 50.0; // Default score
    }
}

void MultiLayerEngine::analyzeDependencies(const std::string& stack_id) {
    auto& stack = getStack(stack_id);

    // Analyze layer dependencies
    for (auto& layer : stack.layers) {
        layer.dependencies.clear();

        // Check for dependencies based on layer type
        if (layer.type == LayerType::METAL) {
            // Metal layers depend on barrier layers
            for (const auto& other_layer : stack.layers) {
                if (other_layer.type == LayerType::BARRIER) {
                    layer.dependencies.push_back(other_layer.id);
                }
            }
        }
    }
}

} // namespace SemiPRO

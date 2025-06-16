#include "damascene_model.hpp"
#include <algorithm>
#include <cmath>

DamasceneModel::DamasceneModel() : rng_(std::random_device{}()) {
    // Initialize default process parameters
    process_params_.etch_rate = 100.0;  // nm/min
    process_params_.selectivity = 50.0;
    process_params_.aspect_ratio_limit = 4.0;
    process_params_.plating_efficiency = 0.95;
    process_params_.cmp_removal_rate = 200.0;  // nm/min
    
    // Material resistivities (μΩ·cm)
    material_resistivities_["Cu"] = 1.7;
    material_resistivities_["Al"] = 2.7;
    material_resistivities_["W"] = 5.6;
    material_resistivities_["TiN"] = 150.0;  // Barrier
    material_resistivities_["Ta"] = 13.0;    // Barrier
    
    // Thermal expansion coefficients (ppm/K)
    material_thermal_expansion_["Cu"] = 16.5;
    material_thermal_expansion_["Al"] = 23.1;
    material_thermal_expansion_["SiO2"] = 0.5;
    material_thermal_expansion_["Si"] = 2.6;
    
    Logger::getInstance().log("DamasceneModel initialized with default parameters");
}

void DamasceneModel::createInterconnectStack(
    std::shared_ptr<Wafer> wafer,
    const std::vector<MetalLevel>& metal_levels) {
    
    for (const auto& level : metal_levels) {
        // Deposit dielectric
        wafer->addFilmLayer(level.thickness * 1.5, level.dielectric_material);
        
        // Simulate damascene process for this level
        simulateDamasceneProcess(wafer, level, SINGLE_DAMASCENE);
        
        Logger::getInstance().log("Created metal level " + std::to_string(level.level) + 
                                " with " + level.metal_type);
    }
}

void DamasceneModel::simulateDamasceneProcess(
    std::shared_ptr<Wafer> wafer,
    const MetalLevel& level,
    InterconnectType type) {
    
    // Step 1: Trench etching
    std::vector<std::pair<double, double>> trench_positions;
    // Generate trench pattern (simplified)
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            trench_positions.emplace_back(i * level.line_spacing, j * level.line_spacing);
        }
    }
    simulateTrenchEtching(wafer, level, trench_positions);
    
    // Step 2: Via etching (for dual damascene)
    if (type == DUAL_DAMASCENE && level.level > 1) {
        std::vector<std::pair<double, double>> via_positions;
        for (size_t i = 0; i < trench_positions.size(); i += 4) {
            via_positions.push_back(trench_positions[i]);
        }
        simulateViaEtching(wafer, via_positions, level.via_size, level.thickness);
    }
    
    // Step 3: Barrier deposition
    simulateBarrierDeposition(wafer, "TiN", 5.0);  // 5 nm barrier
    
    // Step 4: Seed deposition
    simulateSeedDeposition(wafer, level.metal_type, 20.0);  // 20 nm seed
    
    // Step 5: Electroplating
    simulateElectroplating(wafer, level.metal_type, level.thickness, 2.0);  // 2 mA/cm²
    
    // Step 6: CMP
    simulateCMP(wafer, level.metal_type, level.thickness);
    
    Logger::getInstance().log("Completed damascene process for level " + 
                            std::to_string(level.level));
}

void DamasceneModel::createVias(
    std::shared_ptr<Wafer> wafer,
    const std::vector<std::pair<double, double>>& via_positions,
    double via_diameter,
    int from_level,
    int to_level) {
    
    double via_depth = std::abs(to_level - from_level) * 200.0;  // Assume 200 nm per level
    simulateViaEtching(wafer, via_positions, via_diameter, via_depth);
    
    // Fill vias with tungsten (typical for contact vias)
    simulateBarrierDeposition(wafer, "TiN", 10.0);
    simulateElectroplating(wafer, "W", via_depth, 1.0);
    simulateCMP(wafer, "W", 0.0);  // Planarize
    
    Logger::getInstance().log("Created " + std::to_string(via_positions.size()) + 
                            " vias from level " + std::to_string(from_level) + 
                            " to " + std::to_string(to_level));
}

void DamasceneModel::simulateCMP(
    std::shared_ptr<Wafer> wafer,
    const std::string& target_material,
    double target_thickness) {
    
    // Simulate chemical mechanical polishing
    double removal_rate = process_params_.cmp_removal_rate;
    double pattern_density = 0.5;  // Simplified
    
    // Pattern density affects removal rate
    double effective_removal_rate = removal_rate * (0.8 + 0.4 * pattern_density);
    
    // Calculate dishing and erosion effects
    double dishing = calculateCMPUniformity(target_material, pattern_density);
    
    // Update wafer surface (simplified)
    if (!wafer->getFilmLayers().empty()) {
        auto layers = wafer->getFilmLayers();
        if (!layers.empty() && layers.back().second == target_material) {
            // Remove excess material
            double excess = layers.back().first - target_thickness;
            if (excess > 0) {
                layers.back().first = target_thickness;
                // Update wafer layers (this would need a proper interface)
            }
        }
    }
    
    Logger::getInstance().log("CMP process completed for " + target_material + 
                            ", dishing: " + std::to_string(dishing) + " nm");
}

std::unordered_map<std::string, double> DamasceneModel::calculateElectricalProperties(
    std::shared_ptr<Wafer> wafer,
    const InterconnectStructure& structure) {
    
    std::unordered_map<std::string, double> properties;
    
    double total_resistance = 0.0;
    double total_capacitance = 0.0;
    
    for (size_t i = 0; i < structure.metal_levels.size(); ++i) {
        const auto& level = structure.metal_levels[i];
        
        // Calculate resistance for this level
        double line_length = 1000.0;  // μm (example)
        double resistance = calculateResistance(level, line_length);
        total_resistance += resistance;
        
        // Calculate capacitance to adjacent levels
        if (i > 0) {
            double overlap_area = level.line_width * line_length;
            double capacitance = calculateCapacitance(structure.metal_levels[i-1], level, overlap_area);
            total_capacitance += capacitance;
        }
    }
    
    properties["total_resistance"] = total_resistance;
    properties["total_capacitance"] = total_capacitance;
    properties["rc_delay"] = total_resistance * total_capacitance;
    
    return properties;
}

double DamasceneModel::calculateResistance(
    const MetalLevel& level,
    double length) {
    
    double resistivity = material_resistivities_[level.metal_type];
    double cross_sectional_area = level.line_width * level.thickness;  // μm²
    
    // Convert units: resistivity in μΩ·cm, dimensions in μm
    double resistance = (resistivity * 1e-4) * (length * 1e-4) / (cross_sectional_area * 1e-8);
    
    return resistance;  // Ohms
}

double DamasceneModel::calculateCapacitance(
    const MetalLevel& level1,
    const MetalLevel& level2,
    double overlap_area) {
    
    double epsilon_0 = 8.854e-12;  // F/m
    double epsilon_r = 3.9;        // SiO2 relative permittivity
    double distance = std::abs(level2.level - level1.level) * 200e-9;  // m
    
    // Parallel plate capacitor model
    double capacitance = epsilon_0 * epsilon_r * (overlap_area * 1e-12) / distance;
    
    return capacitance * 1e12;  // pF
}

double DamasceneModel::calculateElectromigrationMTTF(
    const MetalLevel& level,
    double current_density,
    double temperature) {
    
    // Black's equation: MTTF = A * J^(-n) * exp(Ea / kT)
    double A = 1e6;  // Pre-exponential factor
    double n = 2.0;  // Current density exponent
    double Ea = 0.7; // Activation energy (eV) for Cu
    double k = 8.617e-5;  // Boltzmann constant (eV/K)
    
    if (level.metal_type == "Al") {
        Ea = 0.6;
    } else if (level.metal_type == "W") {
        Ea = 1.0;
    }
    
    double mttf = A * std::pow(current_density, -n) * std::exp(Ea / (k * temperature));
    
    return mttf;  // seconds
}

std::vector<double> DamasceneModel::analyzeStressMigration(
    std::shared_ptr<Wafer> wafer,
    const InterconnectStructure& structure) {
    
    std::vector<double> stress_values;
    
    for (const auto& level : structure.metal_levels) {
        double thermal_stress = calculateStressMagnitude(
            level.metal_type, 
            level.dielectric_material, 
            300.0  // Room temperature
        );
        stress_values.push_back(thermal_stress);
    }
    
    return stress_values;
}

void DamasceneModel::simulateTrenchEtching(
    std::shared_ptr<Wafer> wafer,
    const MetalLevel& level,
    const std::vector<std::pair<double, double>>& trench_positions) {
    
    double aspect_ratio = calculateAspectRatio(level.line_width, level.thickness);
    
    if (aspect_ratio > process_params_.aspect_ratio_limit) {
        Logger::getInstance().log("WARNING: Aspect ratio " + std::to_string(aspect_ratio) + 
                                " exceeds limit of " + std::to_string(process_params_.aspect_ratio_limit));
    }
    
    // Simulate etch profile and sidewall angle
    double etch_time = level.thickness / process_params_.etch_rate;
    
    Logger::getInstance().log("Etched " + std::to_string(trench_positions.size()) + 
                            " trenches, aspect ratio: " + std::to_string(aspect_ratio));
}

// Helper function implementations
double DamasceneModel::calculateAspectRatio(double width, double depth) {
    return depth / width;
}

double DamasceneModel::calculateCMPUniformity(const std::string& material, double pattern_density) {
    // Simplified model for dishing/erosion
    double base_dishing = 5.0;  // nm
    if (material == "Cu") {
        base_dishing *= (1.0 + pattern_density);
    }
    return base_dishing;
}

double DamasceneModel::calculateStressMagnitude(
    const std::string& material1,
    const std::string& material2,
    double temperature) {
    
    double cte1 = material_thermal_expansion_[material1];
    double cte2 = material_thermal_expansion_[material2];
    double delta_cte = std::abs(cte1 - cte2);
    
    double delta_T = temperature - 298.0;  // Reference temperature
    double stress = delta_cte * delta_T * 200e9;  // Assume 200 GPa modulus
    
    return stress / 1e6;  // Convert to MPa
}

// Author: Dr. Mazharuddin Mohammed
#include "advanced_interconnects.hpp"
#include "../core/utils.hpp"
#include <cmath>
#include <algorithm>

AdvancedInterconnects::AdvancedInterconnects()
    : air_gap_enabled_(false)
    , cobra_head_enabled_(true)
    , advanced_barriers_enabled_(true)
    , process_temperature_(400.0)
{
    initializeMetalDatabase();
    initializeDielectricDatabase();
    
    // Initialize metrics
    metrics_ = InterconnectMetrics{};
    reliability_ = ReliabilityMetrics{};
    
    Logger::getInstance().log("Advanced Interconnects system initialized");
}

bool AdvancedInterconnects::simulateDualDamascene(std::shared_ptr<Wafer> wafer,
                                                 const InterconnectLayer& layer,
                                                 const DamasceneParameters& params) {
    try {
        Logger::getInstance().log("Starting dual damascene simulation for " + layer.name);
        Logger::getInstance().log("Line width: " + std::to_string(layer.line_width) + "nm");
        
        // Simulate barrier deposition
        bool barrier_success = simulateBarrierDeposition(wafer, params.barrier_material, 
                                                        params.barrier_thickness);
        if (!barrier_success) {
            Logger::getInstance().log("Barrier deposition failed");
            return false;
        }
        
        // Simulate seed deposition
        bool seed_success = simulateSeedDeposition(wafer, params.seed_material, 
                                                  params.seed_thickness);
        if (!seed_success) {
            Logger::getInstance().log("Seed deposition failed");
            return false;
        }
        
        // Simulate electroplating
        double fill_thickness = layer.thickness * params.overplating_factor;
        bool plating_success = simulateElectroplating(wafer, layer.metal_material, fill_thickness);
        if (!plating_success) {
            Logger::getInstance().log("Electroplating failed");
            return false;
        }
        
        // Simulate CMP if enabled
        if (params.cmp_enabled) {
            bool cmp_success = simulateCMP(wafer, layer.thickness);
            if (!cmp_success) {
                Logger::getInstance().log("CMP failed");
                return false;
            }
        }
        
        // Update metrics
        updateMetrics(layer);
        
        Logger::getInstance().log("Dual damascene completed successfully");
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("Dual damascene failed: " + std::string(e.what()));
        return false;
    }
}

bool AdvancedInterconnects::simulateBarrierDeposition(std::shared_ptr<Wafer> wafer,
                                                     const std::string& material, 
                                                     double thickness) {
    try {
        Logger::getInstance().log("Depositing " + material + " barrier (" + 
                                 std::to_string(thickness) + "nm)");
        
        // Simulate barrier coverage (simplified)
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        // Add barrier layer effect
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                grid(i, j) += thickness / 1000.0; // Convert nm to μm
            }
        }
        
        wafer->setGrid(grid);
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("Barrier deposition failed: " + std::string(e.what()));
        return false;
    }
}

bool AdvancedInterconnects::simulateSeedDeposition(std::shared_ptr<Wafer> wafer,
                                                  const std::string& material,
                                                  double thickness) {
    try {
        Logger::getInstance().log("Depositing " + material + " seed (" + 
                                 std::to_string(thickness) + "nm)");
        
        // Simulate seed layer (simplified)
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        // Add seed layer effect
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                grid(i, j) += thickness / 1000.0; // Convert nm to μm
            }
        }
        
        wafer->setGrid(grid);
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("Seed deposition failed: " + std::string(e.what()));
        return false;
    }
}

bool AdvancedInterconnects::simulateElectroplating(std::shared_ptr<Wafer> wafer,
                                                   const std::string& material,
                                                   double thickness) {
    try {
        Logger::getInstance().log("Electroplating " + material + " (" + 
                                 std::to_string(thickness) + "nm)");
        
        // Simulate electroplating fill (simplified)
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        // Add plated metal
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                grid(i, j) += thickness / 1000.0; // Convert nm to μm
            }
        }
        
        wafer->setGrid(grid);
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("Electroplating failed: " + std::string(e.what()));
        return false;
    }
}

bool AdvancedInterconnects::simulateCMP(std::shared_ptr<Wafer> wafer, double target_thickness) {
    try {
        Logger::getInstance().log("CMP planarization to " + std::to_string(target_thickness) + "nm");
        
        // Simulate CMP planarization (simplified)
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        // Find maximum height
        double max_height = grid.maxCoeff();
        double removal_amount = max_height - target_thickness / 1000.0;
        
        if (removal_amount > 0) {
            // Remove excess material
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < cols; ++j) {
                    grid(i, j) = std::max(target_thickness / 1000.0, grid(i, j) - removal_amount);
                }
            }
        }
        
        wafer->setGrid(grid);
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("CMP failed: " + std::string(e.what()));
        return false;
    }
}

void AdvancedInterconnects::initializeMetalDatabase() {
    // Copper
    metal_database_["Cu"] = MetalProperties("Cu", 1.7); // μΩ·cm
    metal_database_["Cu"].thermal_conductivity = 400.0;
    metal_database_["Cu"].electromigration_activation = 0.7;
    
    // Aluminum
    metal_database_["Al"] = MetalProperties("Al", 2.8);
    metal_database_["Al"].thermal_conductivity = 237.0;
    metal_database_["Al"].electromigration_activation = 0.5;
    
    // Cobalt
    metal_database_["Co"] = MetalProperties("Co", 6.2);
    metal_database_["Co"].thermal_conductivity = 100.0;
    metal_database_["Co"].electromigration_activation = 1.2;
    
    // Ruthenium
    metal_database_["Ru"] = MetalProperties("Ru", 7.1);
    metal_database_["Ru"].thermal_conductivity = 117.0;
    metal_database_["Ru"].electromigration_activation = 1.5;
}

void AdvancedInterconnects::initializeDielectricDatabase() {
    // Silicon dioxide
    dielectric_database_["SiO2"] = DielectricProperties("SiO2", 3.9);
    
    // Low-k dielectrics
    dielectric_database_["low-k"] = DielectricProperties("low-k", 2.7);
    dielectric_database_["ultra-low-k"] = DielectricProperties("ultra-low-k", 2.2);
    dielectric_database_["ultra-low-k"].porous = true;
    dielectric_database_["ultra-low-k"].porosity = 30.0;
    
    // Air gaps
    dielectric_database_["air"] = DielectricProperties("air", 1.0);
}

double AdvancedInterconnects::calculateLineResistance(const InterconnectLayer& layer,
                                                     const MetalProperties& metal) const {
    // Line resistance calculation
    double cross_sectional_area = layer.line_width * layer.thickness * 1e-18; // m²
    double length = 1e-6; // 1 μm length
    
    return metal.resistivity * 1e-8 * length / cross_sectional_area; // Ω/μm
}

void AdvancedInterconnects::updateMetrics(const InterconnectLayer& layer) const {
    auto metal_props = getMetalProperties(layer.metal_material);
    auto dielectric_props = getDielectricProperties(layer.dielectric);
    
    // Calculate line resistance
    metrics_.rc_delay = calculateLineResistance(layer, metal_props) * 
                       calculateCapacitance(layer, dielectric_props) * 1e12; // ps
    
    // Simplified metrics
    metrics_.power_consumption = 1.0; // mW
    metrics_.signal_integrity = 40.0; // dB
    metrics_.manufacturing_yield = 95.0; // %
    metrics_.cost_per_layer = 0.1; // $/cm²
    metrics_.thermal_resistance = 1.0; // K·cm²/W
}

AdvancedInterconnects::MetalProperties 
AdvancedInterconnects::getMetalProperties(const std::string& material) const {
    auto it = metal_database_.find(material);
    if (it != metal_database_.end()) {
        return it->second;
    }
    return metal_database_.at("Cu"); // Default to copper
}

AdvancedInterconnects::DielectricProperties 
AdvancedInterconnects::getDielectricProperties(const std::string& material) const {
    auto it = dielectric_database_.find(material);
    if (it != dielectric_database_.end()) {
        return it->second;
    }
    return dielectric_database_.at("SiO2"); // Default to oxide
}

double AdvancedInterconnects::calculateCapacitance(const InterconnectLayer& layer,
                                                  const DielectricProperties& dielectric) const {
    // Simplified capacitance calculation
    double epsilon_0 = 8.854e-12; // F/m
    double area = layer.line_width * 1e-9; // m
    double spacing = layer.line_spacing * 1e-9; // m
    
    return epsilon_0 * dielectric.dielectric_constant * area / spacing; // F/μm
}

// Factory functions
std::unique_ptr<AdvancedInterconnects> createAdvancedCopperInterconnect() {
    auto interconnect = std::make_unique<AdvancedInterconnects>();
    interconnect->enableAdvancedBarriers(true);
    interconnect->enableCobraHead(true);
    interconnect->setProcessTemperature(400.0);
    return interconnect;
}

std::vector<AdvancedInterconnects::InterconnectLayer>
createStandardInterconnectStack(int num_levels, double technology_node) {
    std::vector<AdvancedInterconnects::InterconnectLayer> layers;
    
    for (int i = 1; i <= num_levels; ++i) {
        double line_width = technology_node * (i <= 3 ? 1.0 : 2.0); // Local vs global scaling
        double line_spacing = line_width;
        double thickness = line_width * 1.5; // Typical aspect ratio
        
        std::string name = "M" + std::to_string(i);
        layers.emplace_back(name, i, line_width, line_spacing, thickness);
    }
    
    return layers;
}

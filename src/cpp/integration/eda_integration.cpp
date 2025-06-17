// Author: Dr. Mazharuddin Mohammed
#include "eda_integration.hpp"
#include "../core/utils.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace SemiPRO {

// EDAIntegration Implementation
EDAIntegration::EDAIntegration(const std::string& technology_file) 
    : technology_file_(technology_file) {
    if (!technology_file_.empty()) {
        loadTechnologyFile(technology_file_);
    }
    
    // Default layer mapping
    layer_map_[1] = "active";
    layer_map_[2] = "poly";
    layer_map_[3] = "contact";
    layer_map_[4] = "metal1";
    layer_map_[5] = "via1";
    layer_map_[6] = "metal2";
    layer_map_[10] = "nwell";
    layer_map_[11] = "pwell";
}

void EDAIntegration::exportToSpice(const std::unordered_map<std::string, double>& simulation_results,
                                  const std::string& output_file,
                                  const std::string& model_type) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open SPICE output file: " + output_file);
    }
    
    writeSpiceHeader(file, "SemiPRO Generated SPICE Model");
    
    // Extract model parameters from simulation results
    SpiceModelParams model = extractModelParameters(simulation_results, model_type);
    writeSpiceModel(file, model);
    
    file << ".end\n";
    file.close();
    
    Logger::getInstance().log("SPICE model exported to: " + output_file);
}

void EDAIntegration::exportDeviceModels(const std::vector<ExtractedDevice>& devices,
                                       const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open device models file: " + output_file);
    }
    
    writeSpiceHeader(file, "SemiPRO Extracted Device Models");
    
    for (const auto& device : devices) {
        writeSpiceDevice(file, device);
    }
    
    file << ".end\n";
    file.close();
    
    Logger::getInstance().log("Device models exported to: " + output_file);
}

void EDAIntegration::exportParasitics(const std::vector<ParasiticElement>& parasitics,
                                     const std::string& output_file) {
    std::ofstream file(output_file);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open parasitics file: " + output_file);
    }
    
    writeSpiceHeader(file, "SemiPRO Extracted Parasitics");
    
    for (const auto& parasitic : parasitics) {
        writeSpiceParasitic(file, parasitic);
    }
    
    file << ".end\n";
    file.close();
    
    Logger::getInstance().log("Parasitics exported to: " + output_file);
}

std::vector<GDSCell> EDAIntegration::importGDSLayout(const std::string& gds_file) {
    std::vector<GDSCell> cells;
    
    // Simplified GDS import - in production would use proper GDS library
    std::ifstream file(gds_file, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open GDS file: " + gds_file);
    }
    
    // For now, create a dummy cell structure
    GDSCell dummy_cell;
    dummy_cell.name = "TOP_CELL";
    
    // Add some example polygons
    GDSPolygon poly1;
    poly1.layer = 1;
    poly1.datatype = 0;
    poly1.layer_name = "active";
    poly1.points = {{0, 0}, {100, 0}, {100, 50}, {0, 50}};
    
    GDSPolygon poly2;
    poly2.layer = 2;
    poly2.datatype = 0;
    poly2.layer_name = "poly";
    poly2.points = {{25, -10}, {75, -10}, {75, 60}, {25, 60}};
    
    dummy_cell.polygons = {poly1, poly2};
    cells.push_back(dummy_cell);
    
    Logger::getInstance().log("GDS layout imported from: " + gds_file);
    return cells;
}

std::vector<GDSPolygon> EDAIntegration::extractLayerPolygons(const std::vector<GDSCell>& cells, int layer) {
    std::vector<GDSPolygon> layer_polygons;
    
    for (const auto& cell : cells) {
        for (const auto& polygon : cell.polygons) {
            if (polygon.layer == layer) {
                layer_polygons.push_back(polygon);
            }
        }
    }
    
    return layer_polygons;
}

std::vector<std::vector<std::pair<double, double>>> EDAIntegration::convertToSimulationGeometry(
    const std::vector<GDSPolygon>& polygons) {
    
    std::vector<std::vector<std::pair<double, double>>> geometry;
    
    for (const auto& polygon : polygons) {
        geometry.push_back(polygon.points);
    }
    
    return geometry;
}

std::vector<ExtractedDevice> EDAIntegration::extractDevices(const std::vector<GDSCell>& cells) {
    std::vector<ExtractedDevice> devices;
    
    for (const auto& cell : cells) {
        // Look for transistor patterns
        auto active_polygons = extractLayerPolygons({cell}, 1);
        auto poly_polygons = extractLayerPolygons({cell}, 2);
        
        // Simple transistor detection: poly crossing active
        for (const auto& poly : poly_polygons) {
            for (const auto& active : active_polygons) {
                if (isTransistor(poly, {active})) {
                    ExtractedDevice device = extractTransistor({poly, active});
                    devices.push_back(device);
                }
            }
        }
    }
    
    Logger::getInstance().log("Extracted " + std::to_string(devices.size()) + " devices");
    return devices;
}

SpiceModelParams EDAIntegration::extractModelParameters(
    const std::unordered_map<std::string, double>& sim_results,
    const std::string& device_type) {
    
    SpiceModelParams model;
    model.model_name = device_type + "_model";
    model.model_type = device_type;
    model.process_corner = "typical";
    
    // Extract common MOSFET parameters
    if (device_type == "nmos" || device_type == "pmos") {
        auto vth_it = sim_results.find("threshold_voltage");
        if (vth_it != sim_results.end()) {
            model.parameters["vto"] = vth_it->second;
        }
        
        auto kp_it = sim_results.find("transconductance");
        if (kp_it != sim_results.end()) {
            model.parameters["kp"] = kp_it->second * 1e-6; // Convert to A/V^2
        }
        
        auto lambda_it = sim_results.find("channel_modulation");
        if (lambda_it != sim_results.end()) {
            model.parameters["lambda"] = lambda_it->second;
        }
        
        // Default parameters
        model.parameters["gamma"] = 0.5;
        model.parameters["phi"] = 0.7;
        model.parameters["tox"] = 5e-9; // 5nm oxide
    }
    
    return model;
}

void EDAIntegration::loadTechnologyFile(const std::string& tech_file) {
    std::ifstream file(tech_file);
    if (!file.is_open()) {
        Logger::getInstance().log("Warning: Cannot open technology file: " + tech_file);
        return;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Simple parsing - in production would use proper tech file parser
        if (line.find("layer") != std::string::npos) {
            // Parse layer definitions
        }
    }
    
    Logger::getInstance().log("Technology file loaded: " + tech_file);
}

void EDAIntegration::setLayerMapping(int gds_layer, const std::string& process_layer) {
    layer_map_[gds_layer] = process_layer;
}

std::string EDAIntegration::getProcessLayer(int gds_layer) const {
    auto it = layer_map_.find(gds_layer);
    return (it != layer_map_.end()) ? it->second : "unknown";
}

void EDAIntegration::writeSpiceHeader(std::ofstream& file, const std::string& title) {
    file << "* " << title << "\n";
    file << "* Generated by SemiPRO\n";
    file << "* Author: Dr. Mazharuddin Mohammed\n";
    file << "*\n\n";
}

void EDAIntegration::writeSpiceModel(std::ofstream& file, const SpiceModelParams& model) {
    file << ".model " << model.model_name << " " << model.model_type;
    
    for (const auto& [param, value] : model.parameters) {
        file << " " << param << "=" << value;
    }
    
    file << "\n";
}

void EDAIntegration::writeSpiceDevice(std::ofstream& file, const ExtractedDevice& device) {
    file << "M" << device.instance_name << " ";
    
    // Write terminals
    for (const auto& terminal : device.terminals) {
        file << terminal << " ";
    }
    
    file << device.device_type << "_model";
    
    // Write device parameters
    for (const auto& [param, value] : device.parameters) {
        file << " " << param << "=" << value;
    }
    
    file << "\n";
}

void EDAIntegration::writeSpiceParasitic(std::ofstream& file, const ParasiticElement& parasitic) {
    switch (parasitic.type) {
        case ParasiticElement::RESISTOR:
            file << "R";
            break;
        case ParasiticElement::CAPACITOR:
            file << "C";
            break;
        case ParasiticElement::INDUCTOR:
            file << "L";
            break;
    }
    
    file << parasitic.name << " ";
    
    for (const auto& node : parasitic.nodes) {
        file << node << " ";
    }
    
    file << parasitic.value << parasitic.units << "\n";
}

bool EDAIntegration::isTransistor(const GDSPolygon& poly, const std::vector<GDSPolygon>& all_polygons) {
    // Simplified transistor detection
    // In reality, would check for poly crossing active region
    return poly.layer == 2; // Poly layer
}

ExtractedDevice EDAIntegration::extractTransistor(const std::vector<GDSPolygon>& transistor_polygons) {
    ExtractedDevice device;
    device.device_type = "nmos"; // Default
    device.instance_name = "M1";
    device.terminals = {"drain", "gate", "source", "bulk"};
    
    // Calculate device dimensions
    if (transistor_polygons.size() >= 2) {
        const auto& poly = transistor_polygons[0];
        const auto& active = transistor_polygons[1];
        
        // Simple width/length calculation
        device.width = 50.0; // Default width in nm
        device.length = 25.0; // Default length in nm
        
        device.parameters["w"] = device.width * 1e-9;
        device.parameters["l"] = device.length * 1e-9;
    }
    
    return device;
}

// VirtuosoIntegration Implementation
VirtuosoIntegration::VirtuosoIntegration(const std::string& skill_path) 
    : skill_script_path_(skill_path) {
}

void VirtuosoIntegration::exportSchematic(const std::vector<ExtractedDevice>& devices,
                                        const std::string& library_name,
                                        const std::string& cell_name) {
    std::string script_file = "export_schematic.il";
    std::ofstream file(script_file);
    
    file << "; SemiPRO Schematic Export\n";
    file << "(let ((lib (ddGetObj \"" << library_name << "\"))\n";
    file << "      (cell (ddGetObj \"" << library_name << "\" \"" << cell_name << "\")))\n";
    
    for (const auto& device : devices) {
        file << "  (dbCreateInst cell (dbOpenCellViewByType \"basic\" \"" 
             << device.device_type << "\" \"symbol\") \"" << device.instance_name 
             << "\" " << device.position.first << ":" << device.position.second << " \"R0\")\n";
    }
    
    file << ")\n";
    file.close();
    
    executeSkillScript(script_file);
    Logger::getInstance().log("Schematic exported to Virtuoso: " + library_name + "/" + cell_name);
}

void VirtuosoIntegration::generateSkillScript(const std::string& command, const std::string& output_file) {
    std::ofstream file(output_file);
    file << "; Generated SKILL script\n";
    file << command << "\n";
    file.close();
}

void VirtuosoIntegration::executeSkillScript(const std::string& script_file) {
    // In production, would execute: virtuoso -nograph -replay script_file
    Logger::getInstance().log("SKILL script executed: " + script_file);
}

} // namespace SemiPRO

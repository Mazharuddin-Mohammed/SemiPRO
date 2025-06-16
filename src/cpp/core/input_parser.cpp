// Author: Dr. Mazharuddin Mohammed
#include "input_parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <json/json.h>

namespace SemiPRO {

InputParser::InputParser(const std::string& base_directory) 
    : base_directory_(base_directory), verbose_(false) {
    
    // Ensure base directory exists
    if (!std::filesystem::exists(base_directory_)) {
        std::filesystem::create_directories(base_directory_);
    }
}

InputParser::FileFormat InputParser::detectFileFormat(const std::string& filename) const {
    std::string extension = std::filesystem::path(filename).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    if (extension == ".yaml" || extension == ".yml") return FileFormat::YAML;
    if (extension == ".json") return FileFormat::JSON;
    if (extension == ".gds" || extension == ".gdsii") return FileFormat::GDS;
    if (extension == ".oas" || extension == ".oasis") return FileFormat::OASIS;
    if (extension == ".mask") return FileFormat::MASK;
    if (extension == ".sp" || extension == ".spice" || extension == ".cir") return FileFormat::SPICE;
    if (extension == ".tcad" || extension == ".msh") return FileFormat::TCAD;
    if (extension == ".csv") return FileFormat::CSV;
    if (extension == ".txt" || extension == ".dat") return FileFormat::TXT;
    if (extension == ".bin" || extension == ".binary") return FileFormat::BINARY;
    
    return FileFormat::UNKNOWN;
}

bool InputParser::parseFile(const std::string& filename) {
    FileFormat format = detectFileFormat(filename);
    return parseFile(filename, format);
}

bool InputParser::parseFile(const std::string& filename, FileFormat format) {
    if (!fileExists(filename)) {
        logError("File not found: " + filename);
        return false;
    }
    
    try {
        switch (format) {
            case FileFormat::YAML:
                return parseYAMLFile(filename);
            case FileFormat::JSON:
                return parseJSONFile(filename);
            case FileFormat::GDS:
                return parseGDSFile(filename);
            case FileFormat::MASK:
                return parseMaskFile(filename);
            case FileFormat::SPICE:
                return parseSpiceFile(filename);
            case FileFormat::CSV:
                return parseCSVFile(filename);
            case FileFormat::TCAD:
                return parseTCADFile(filename);
            default:
                logError("Unsupported file format for: " + filename);
                return false;
        }
    } catch (const std::exception& e) {
        logError("Error parsing file " + filename + ": " + e.what());
        return false;
    }
}

bool InputParser::parseYAMLFile(const std::string& filename) {
    try {
        std::string full_path = getFullPath(filename);
        YAML::Node data = YAML::LoadFile(full_path);
        yaml_data_[filename] = data;
        
        logInfo("Successfully parsed YAML file: " + filename);
        return true;
        
    } catch (const YAML::Exception& e) {
        logError("YAML parsing error in " + filename + ": " + e.what());
        return false;
    }
}

bool InputParser::parseJSONFile(const std::string& filename) {
    try {
        std::string full_path = getFullPath(filename);
        std::ifstream file(full_path);
        
        if (!file.is_open()) {
            logError("Cannot open JSON file: " + filename);
            return false;
        }
        
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::string errors;
        
        if (!Json::parseFromStream(builder, file, &root, &errors)) {
            logError("JSON parsing error in " + filename + ": " + errors);
            return false;
        }
        
        // Convert JSON to YAML for unified storage
        YAML::Node yaml_node;
        convertJsonToYaml(root, yaml_node);
        yaml_data_[filename] = yaml_node;
        
        logInfo("Successfully parsed JSON file: " + filename);
        return true;
        
    } catch (const std::exception& e) {
        logError("Error parsing JSON file " + filename + ": " + e.what());
        return false;
    }
}

bool InputParser::parseGDSFile(const std::string& filename) {
    try {
        std::string full_path = getFullPath(filename);
        std::ifstream file(full_path, std::ios::binary);
        
        if (!file.is_open()) {
            logError("Cannot open GDS file: " + filename);
            return false;
        }
        
        std::vector<MaskLayer> layers;
        MaskLayer current_layer;
        
        // Simple GDS parser (basic implementation)
        while (file.good()) {
            if (!parseGDSRecord(file, current_layer)) {
                break;
            }
        }
        
        if (!current_layer.name.empty()) {
            layers.push_back(current_layer);
        }
        
        mask_data_[filename] = layers;
        
        logInfo("Successfully parsed GDS file: " + filename + " (" + 
                std::to_string(layers.size()) + " layers)");
        return true;
        
    } catch (const std::exception& e) {
        logError("Error parsing GDS file " + filename + ": " + e.what());
        return false;
    }
}

bool InputParser::parseMaskFile(const std::string& filename) {
    try {
        std::string full_path = getFullPath(filename);
        std::ifstream file(full_path);
        
        if (!file.is_open()) {
            logError("Cannot open mask file: " + filename);
            return false;
        }
        
        std::vector<MaskLayer> layers;
        std::string line;
        MaskLayer current_layer;
        
        while (std::getline(file, line)) {
            // Skip comments and empty lines
            if (line.empty() || line[0] == '#') continue;
            
            std::istringstream iss(line);
            std::string command;
            iss >> command;
            
            if (command == "LAYER") {
                if (!current_layer.name.empty()) {
                    layers.push_back(current_layer);
                }
                current_layer = MaskLayer();
                iss >> current_layer.layer_number >> current_layer.name;
            }
            else if (command == "MATERIAL") {
                iss >> current_layer.material;
            }
            else if (command == "THICKNESS") {
                iss >> current_layer.thickness;
            }
            else if (command == "POLYGON") {
                std::string polygon_str;
                std::getline(iss, polygon_str);
                auto polygon = MaskParser::parsePolygon(polygon_str);
                current_layer.polygons.push_back(polygon);
            }
        }
        
        if (!current_layer.name.empty()) {
            layers.push_back(current_layer);
        }
        
        mask_data_[filename] = layers;
        
        logInfo("Successfully parsed mask file: " + filename + " (" + 
                std::to_string(layers.size()) + " layers)");
        return true;
        
    } catch (const std::exception& e) {
        logError("Error parsing mask file " + filename + ": " + e.what());
        return false;
    }
}

bool InputParser::parseSpiceFile(const std::string& filename) {
    try {
        std::string full_path = getFullPath(filename);
        std::ifstream file(full_path);
        
        if (!file.is_open()) {
            logError("Cannot open SPICE file: " + filename);
            return false;
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        std::vector<std::string> components;
        if (!parseSpiceNetlist(content, components)) {
            logError("Failed to parse SPICE netlist: " + filename);
            return false;
        }
        
        // Store as YAML for unified access
        YAML::Node spice_data;
        spice_data["components"] = components;
        yaml_data_[filename] = spice_data;
        
        logInfo("Successfully parsed SPICE file: " + filename + " (" + 
                std::to_string(components.size()) + " components)");
        return true;
        
    } catch (const std::exception& e) {
        logError("Error parsing SPICE file " + filename + ": " + e.what());
        return false;
    }
}

bool InputParser::parseCSVFile(const std::string& filename) {
    try {
        std::string full_path = getFullPath(filename);
        std::ifstream file(full_path);
        
        if (!file.is_open()) {
            logError("Cannot open CSV file: " + filename);
            return false;
        }
        
        std::vector<std::vector<std::string>> data;
        std::string line;
        
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            
            auto tokens = tokenizeLine(line, ',');
            data.push_back(tokens);
        }
        
        // Convert to YAML structure
        YAML::Node csv_data;
        if (!data.empty()) {
            // First row as headers
            const auto& headers = data[0];
            
            for (size_t i = 1; i < data.size(); ++i) {
                YAML::Node row;
                for (size_t j = 0; j < headers.size() && j < data[i].size(); ++j) {
                    row[headers[j]] = data[i][j];
                }
                csv_data["data"].push_back(row);
            }
        }
        
        yaml_data_[filename] = csv_data;
        
        logInfo("Successfully parsed CSV file: " + filename + " (" + 
                std::to_string(data.size()) + " rows)");
        return true;
        
    } catch (const std::exception& e) {
        logError("Error parsing CSV file " + filename + ": " + e.what());
        return false;
    }
}

bool InputParser::parseTCADFile(const std::string& filename) {
    try {
        std::string full_path = getFullPath(filename);
        std::ifstream file(full_path);
        
        if (!file.is_open()) {
            logError("Cannot open TCAD file: " + filename);
            return false;
        }
        
        // Simple TCAD mesh parser
        std::vector<std::vector<double>> nodes;
        std::vector<std::vector<int>> elements;
        
        std::string line;
        std::string section;
        
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            if (line.find("NODES") != std::string::npos) {
                section = "nodes";
                continue;
            }
            else if (line.find("ELEMENTS") != std::string::npos) {
                section = "elements";
                continue;
            }
            
            if (section == "nodes") {
                std::istringstream iss(line);
                std::vector<double> node;
                double value;
                while (iss >> value) {
                    node.push_back(value);
                }
                if (!node.empty()) {
                    nodes.push_back(node);
                }
            }
            else if (section == "elements") {
                std::istringstream iss(line);
                std::vector<int> element;
                int value;
                while (iss >> value) {
                    element.push_back(value);
                }
                if (!element.empty()) {
                    elements.push_back(element);
                }
            }
        }
        
        // Convert to Eigen matrix for storage
        if (!nodes.empty()) {
            Eigen::MatrixXd node_matrix(nodes.size(), nodes[0].size());
            for (size_t i = 0; i < nodes.size(); ++i) {
                for (size_t j = 0; j < nodes[i].size(); ++j) {
                    node_matrix(i, j) = nodes[i][j];
                }
            }
            matrix_data_[filename + "_nodes"] = node_matrix;
        }
        
        logInfo("Successfully parsed TCAD file: " + filename + " (" + 
                std::to_string(nodes.size()) + " nodes, " + 
                std::to_string(elements.size()) + " elements)");
        return true;
        
    } catch (const std::exception& e) {
        logError("Error parsing TCAD file " + filename + ": " + e.what());
        return false;
    }
}

} // namespace SemiPRO

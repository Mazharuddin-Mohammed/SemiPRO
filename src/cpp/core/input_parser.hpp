// Author: Dr. Mazharuddin Mohammed
#ifndef INPUT_PARSER_HPP
#define INPUT_PARSER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <regex>
#include <yaml-cpp/yaml.h>
#include <Eigen/Dense>

namespace SemiPRO {

/**
 * @brief Comprehensive input file parser for various semiconductor simulation formats
 */
class InputParser {
public:
    // Supported file formats
    enum class FileFormat {
        YAML,           // YAML configuration files
        JSON,           // JSON configuration files
        GDS,            // GDSII layout files
        OASIS,          // OASIS layout files
        MASK,           // Mask definition files
        SPICE,          // SPICE netlist files
        TCAD,           // TCAD mesh files
        CSV,            // CSV data files
        TXT,            // Plain text files
        BINARY,         // Binary data files
        UNKNOWN
    };

    // Data structures for parsed content
    struct MaskLayer {
        int layer_number;
        int datatype;
        std::string name;
        std::string material;
        double thickness;
        std::vector<std::vector<std::pair<double, double>>> polygons;
        
        MaskLayer() : layer_number(0), datatype(0), thickness(0.0) {}
    };

    struct ProcessParameters {
        std::string process_name;
        std::unordered_map<std::string, double> numeric_params;
        std::unordered_map<std::string, std::string> string_params;
        std::unordered_map<std::string, bool> boolean_params;
        std::vector<std::string> dependencies;
        
        ProcessParameters() = default;
        ProcessParameters(const std::string& name) : process_name(name) {}
    };

    struct MaterialProperties {
        std::string name;
        double density;
        double thermal_conductivity;
        double specific_heat;
        double electrical_resistivity;
        double dielectric_constant;
        double bandgap;
        double electron_mobility;
        double hole_mobility;
        std::unordered_map<std::string, double> custom_properties;
        
        MaterialProperties() : density(0.0), thermal_conductivity(0.0), specific_heat(0.0),
                             electrical_resistivity(0.0), dielectric_constant(0.0), 
                             bandgap(0.0), electron_mobility(0.0), hole_mobility(0.0) {}
    };

    struct DopingProfile {
        std::string dopant_type;
        std::vector<double> positions;
        std::vector<double> concentrations;
        std::string profile_type; // "gaussian", "uniform", "exponential", etc.
        std::unordered_map<std::string, double> parameters;
        
        DopingProfile() = default;
        DopingProfile(const std::string& type) : dopant_type(type) {}
    };

    struct TechnologyNode {
        std::string name;
        double feature_size;
        std::vector<std::string> available_layers;
        std::unordered_map<std::string, double> design_rules;
        std::unordered_map<std::string, MaterialProperties> materials;
        
        TechnologyNode() : feature_size(0.0) {}
        TechnologyNode(const std::string& n, double fs) : name(n), feature_size(fs) {}
    };

public:
    explicit InputParser(const std::string& base_directory = ".");
    ~InputParser() = default;

    // File format detection and parsing
    FileFormat detectFileFormat(const std::string& filename) const;
    bool parseFile(const std::string& filename);
    bool parseFile(const std::string& filename, FileFormat format);
    
    // Specific parsers
    bool parseYAMLFile(const std::string& filename);
    bool parseJSONFile(const std::string& filename);
    bool parseGDSFile(const std::string& filename);
    bool parseMaskFile(const std::string& filename);
    bool parseSpiceFile(const std::string& filename);
    bool parseCSVFile(const std::string& filename);
    bool parseTCADFile(const std::string& filename);
    
    // Configuration parsing
    ProcessParameters parseProcessConfig(const std::string& filename);
    std::vector<ProcessParameters> parseProcessFlow(const std::string& filename);
    TechnologyNode parseTechnologyFile(const std::string& filename);
    std::vector<MaterialProperties> parseMaterialsFile(const std::string& filename);
    
    // Mask and layout parsing
    std::vector<MaskLayer> parseMaskLayers(const std::string& filename);
    Eigen::MatrixXd parseGeometryMesh(const std::string& filename);
    
    // Doping and profile parsing
    std::vector<DopingProfile> parseDopingProfiles(const std::string& filename);
    Eigen::VectorXd parseConcentrationProfile(const std::string& filename);
    
    // Data extraction
    std::vector<std::string> getAvailableDatasets() const;
    bool hasDataset(const std::string& name) const;
    
    template<typename T>
    std::vector<T> getDataset(const std::string& name) const;
    
    // Validation
    bool validateFile(const std::string& filename) const;
    std::vector<std::string> getValidationErrors() const;
    
    // Configuration
    void setBaseDirectory(const std::string& directory);
    std::string getBaseDirectory() const;
    void setVerbose(bool verbose);
    bool isVerbose() const;
    
    // Error handling
    std::string getLastError() const;
    std::vector<std::string> getAllErrors() const;
    void clearErrors();

private:
    std::string base_directory_;
    bool verbose_;
    mutable std::string last_error_;
    mutable std::vector<std::string> errors_;
    
    // Parsed data storage
    std::unordered_map<std::string, YAML::Node> yaml_data_;
    std::unordered_map<std::string, std::vector<MaskLayer>> mask_data_;
    std::unordered_map<std::string, std::vector<DopingProfile>> doping_data_;
    std::unordered_map<std::string, TechnologyNode> technology_data_;
    std::unordered_map<std::string, std::vector<MaterialProperties>> material_data_;
    std::unordered_map<std::string, Eigen::MatrixXd> matrix_data_;
    std::unordered_map<std::string, Eigen::VectorXd> vector_data_;
    
    // Helper methods
    bool fileExists(const std::string& filename) const;
    std::string getFullPath(const std::string& filename) const;
    void logError(const std::string& error) const;
    void logInfo(const std::string& info) const;
    
    // Format-specific helpers
    bool parseGDSRecord(std::ifstream& file, MaskLayer& layer);
    bool parseSpiceNetlist(const std::string& content, std::vector<std::string>& components);
    std::vector<std::string> tokenizeLine(const std::string& line, char delimiter = ',') const;
    double parseEngineering(const std::string& value) const; // Parse values like "1.5e-6", "10u", etc.
    
    // Validation helpers
    bool validateYAMLStructure(const YAML::Node& node, const std::string& expected_schema) const;
    bool validateMaskLayer(const MaskLayer& layer) const;
    bool validateDopingProfile(const DopingProfile& profile) const;
    bool validateMaterialProperties(const MaterialProperties& material) const;
};

/**
 * @brief Specialized parser for mask definition files
 */
class MaskParser {
public:
    struct MaskDefinition {
        std::string name;
        std::vector<InputParser::MaskLayer> layers;
        double scale_factor;
        std::string units;
        std::unordered_map<std::string, std::string> metadata;
        
        MaskDefinition() : scale_factor(1.0), units("um") {}
    };

    static MaskDefinition parseMaskFile(const std::string& filename);
    static bool writeMaskFile(const std::string& filename, const MaskDefinition& mask);
    static std::vector<std::pair<double, double>> parsePolygon(const std::string& polygon_str);
    static std::string formatPolygon(const std::vector<std::pair<double, double>>& polygon);
};

/**
 * @brief Specialized parser for process configuration files
 */
class ProcessConfigParser {
public:
    struct ProcessFlow {
        std::string name;
        std::string description;
        std::vector<InputParser::ProcessParameters> steps;
        std::unordered_map<std::string, std::string> global_settings;
        
        ProcessFlow() = default;
        ProcessFlow(const std::string& n) : name(n) {}
    };

    static ProcessFlow parseProcessFlow(const std::string& filename);
    static bool writeProcessFlow(const std::string& filename, const ProcessFlow& flow);
    static InputParser::ProcessParameters parseProcessStep(const YAML::Node& step_node);
    static YAML::Node formatProcessStep(const InputParser::ProcessParameters& step);
};

/**
 * @brief Specialized parser for technology files
 */
class TechnologyParser {
public:
    static InputParser::TechnologyNode parseTechnologyFile(const std::string& filename);
    static bool writeTechnologyFile(const std::string& filename, const InputParser::TechnologyNode& tech);
    static std::unordered_map<std::string, double> parseDesignRules(const YAML::Node& rules_node);
    static std::unordered_map<std::string, InputParser::MaterialProperties> parseMaterials(const YAML::Node& materials_node);
};

} // namespace SemiPRO

#endif // INPUT_PARSER_HPP

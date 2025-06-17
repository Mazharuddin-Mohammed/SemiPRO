// Author: Dr. Mazharuddin Mohammed
#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>

namespace SemiPRO {

/**
 * @brief SPICE model parameters
 */
struct SpiceModelParams {
    std::string model_name;
    std::string model_type; // nmos, pmos, resistor, capacitor, etc.
    std::unordered_map<std::string, double> parameters;
    std::string temperature_dependence;
    std::string process_corner; // typical, fast, slow, etc.
};

/**
 * @brief GDS polygon structure
 */
struct GDSPolygon {
    int layer;
    int datatype;
    std::vector<std::pair<double, double>> points;
    std::string layer_name;
};

/**
 * @brief GDS cell structure
 */
struct GDSCell {
    std::string name;
    std::vector<GDSPolygon> polygons;
    std::vector<std::pair<std::string, std::pair<double, double>>> references; // cell_name, position
    double scale_factor = 1.0;
};

/**
 * @brief Device extraction result
 */
struct ExtractedDevice {
    std::string device_type;
    std::string instance_name;
    std::unordered_map<std::string, double> parameters;
    std::vector<std::string> terminals;
    std::pair<double, double> position;
    double width, length;
};

/**
 * @brief Parasitic element
 */
struct ParasiticElement {
    enum Type { RESISTOR, CAPACITOR, INDUCTOR };
    Type type;
    std::string name;
    std::vector<std::string> nodes;
    double value;
    std::string units;
};

/**
 * @brief EDA tool integration class
 */
class EDAIntegration {
private:
    std::string technology_file_;
    std::unordered_map<std::string, SpiceModelParams> device_models_;
    std::unordered_map<int, std::string> layer_map_;
    
public:
    EDAIntegration(const std::string& technology_file = "");
    
    // SPICE export functionality
    void exportToSpice(const std::unordered_map<std::string, double>& simulation_results,
                      const std::string& output_file,
                      const std::string& model_type = "nmos");
    
    void exportDeviceModels(const std::vector<ExtractedDevice>& devices,
                           const std::string& output_file);
    
    void exportParasitics(const std::vector<ParasiticElement>& parasitics,
                         const std::string& output_file);
    
    // GDS import functionality
    std::vector<GDSCell> importGDSLayout(const std::string& gds_file);
    std::vector<GDSPolygon> extractLayerPolygons(const std::vector<GDSCell>& cells, int layer);
    
    // Geometry conversion
    std::vector<std::vector<std::pair<double, double>>> convertToSimulationGeometry(
        const std::vector<GDSPolygon>& polygons);
    
    // Device extraction
    std::vector<ExtractedDevice> extractDevices(const std::vector<GDSCell>& cells);
    std::vector<ParasiticElement> extractParasitics(const std::vector<GDSCell>& cells);
    
    // Technology file management
    void loadTechnologyFile(const std::string& tech_file);
    void setLayerMapping(int gds_layer, const std::string& process_layer);
    std::string getProcessLayer(int gds_layer) const;
    
    // Model parameter extraction
    SpiceModelParams extractModelParameters(const std::unordered_map<std::string, double>& sim_results,
                                           const std::string& device_type);
    
    // Netlist generation
    void generateNetlist(const std::vector<ExtractedDevice>& devices,
                        const std::vector<ParasiticElement>& parasitics,
                        const std::string& output_file);
    
    // Layout vs Schematic (LVS) support
    bool compareLayoutSchematic(const std::string& layout_netlist,
                               const std::string& schematic_netlist);
    
    // Design Rule Check (DRC) integration
    std::vector<std::string> runDRCCheck(const std::vector<GDSCell>& cells,
                                        const std::string& drc_rules_file);
    
private:
    void writeSpiceHeader(std::ofstream& file, const std::string& title);
    void writeSpiceModel(std::ofstream& file, const SpiceModelParams& model);
    void writeSpiceDevice(std::ofstream& file, const ExtractedDevice& device);
    void writeSpiceParasitic(std::ofstream& file, const ParasiticElement& parasitic);
    
    GDSCell parseGDSCell(const std::string& cell_data);
    GDSPolygon parseGDSPolygon(const std::string& polygon_data);
    
    bool isTransistor(const GDSPolygon& poly, const std::vector<GDSPolygon>& all_polygons);
    ExtractedDevice extractTransistor(const std::vector<GDSPolygon>& transistor_polygons);
    
    double calculateCapacitance(const GDSPolygon& poly1, const GDSPolygon& poly2);
    double calculateResistance(const GDSPolygon& poly);
};

/**
 * @brief Cadence Virtuoso integration
 */
class VirtuosoIntegration {
private:
    std::string skill_script_path_;
    
public:
    VirtuosoIntegration(const std::string& skill_path = "");
    
    // Schematic integration
    void exportSchematic(const std::vector<ExtractedDevice>& devices,
                        const std::string& library_name,
                        const std::string& cell_name);
    
    // Layout integration
    void importLayout(const std::string& library_name,
                     const std::string& cell_name,
                     std::vector<GDSCell>& cells);
    
    // Simulation setup
    void setupSimulation(const std::string& testbench_name,
                        const std::unordered_map<std::string, double>& parameters);
    
    // Results import
    std::unordered_map<std::string, std::vector<double>> importSimulationResults(
        const std::string& results_file);
    
private:
    void generateSkillScript(const std::string& command, const std::string& output_file);
    void executeSkillScript(const std::string& script_file);
};

/**
 * @brief Synopsys integration
 */
class SynopsysIntegration {
public:
    // TCAD integration
    void exportToSentaurus(const std::unordered_map<std::string, double>& simulation_results,
                          const std::string& output_file);
    
    void importFromSentaurus(const std::string& input_file,
                            std::unordered_map<std::string, double>& results);
    
    // Design Compiler integration
    void exportToDesignCompiler(const std::vector<ExtractedDevice>& devices,
                               const std::string& liberty_file);
    
    // IC Compiler integration
    void exportToICCompiler(const std::vector<GDSCell>& cells,
                           const std::string& def_file);
    
private:
    void writeLibertyHeader(std::ofstream& file);
    void writeLibertyCell(std::ofstream& file, const ExtractedDevice& device);
    void writeDEFHeader(std::ofstream& file);
    void writeDEFComponent(std::ofstream& file, const GDSCell& cell);
};

/**
 * @brief Mentor Graphics integration
 */
class MentorIntegration {
public:
    // Calibre DRC/LVS integration
    void runCalibreDRC(const std::string& gds_file,
                      const std::string& rule_file,
                      const std::string& output_file);
    
    void runCalibreLVS(const std::string& gds_file,
                      const std::string& netlist_file,
                      const std::string& rule_file,
                      const std::string& output_file);
    
    // Eldo SPICE integration
    void exportToEldo(const std::vector<ExtractedDevice>& devices,
                     const std::string& output_file);
    
    // Questa simulation integration
    void setupQuestaSimulation(const std::string& testbench,
                              const std::unordered_map<std::string, std::string>& parameters);
    
private:
    void generateCalibreRunset(const std::string& rule_file,
                              const std::string& input_file,
                              const std::string& output_file);
};

/**
 * @brief Universal format converters
 */
class FormatConverters {
public:
    // GDSII to other formats
    static void gdsToOAS(const std::string& gds_file, const std::string& oas_file);
    static void gdsToDXF(const std::string& gds_file, const std::string& dxf_file);
    static void gdsToSVG(const std::string& gds_file, const std::string& svg_file);
    
    // SPICE to other formats
    static void spiceToVerilogA(const std::string& spice_file, const std::string& va_file);
    static void spiceToHSPICE(const std::string& spice_file, const std::string& hsp_file);
    
    // Netlist conversions
    static void spiceToSpectre(const std::string& spice_file, const std::string& scs_file);
    static void spectreToSpice(const std::string& scs_file, const std::string& spice_file);
    
    // Data format conversions
    static void csvToTechfile(const std::string& csv_file, const std::string& tech_file);
    static void techfileToCSV(const std::string& tech_file, const std::string& csv_file);
};

} // namespace SemiPRO

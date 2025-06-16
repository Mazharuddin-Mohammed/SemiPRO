// Author: Dr. Mazharuddin Mohammed
#ifndef OUTPUT_GENERATOR_HPP
#define OUTPUT_GENERATOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include <Eigen/Dense>

namespace SemiPRO {

// Forward declarations
class Wafer;

/**
 * @brief Comprehensive output file generator for simulation results
 */
class OutputGenerator {
public:
    // Supported output formats
    enum class OutputFormat {
        YAML,           // YAML data files
        JSON,           // JSON data files
        CSV,            // CSV data files
        VTK,            // VTK visualization files
        HDF5,           // HDF5 scientific data format
        MATLAB,         // MATLAB .mat files
        TECPLOT,        // Tecplot data files
        GDSII,          // GDSII layout files
        PNG,            // PNG image files
        SVG,            // SVG vector graphics
        PDF,            // PDF reports
        HTML,           // HTML reports
        SPICE,          // SPICE netlist output
        BINARY          // Binary data files
    };

    // Data export types
    enum class DataType {
        DOPING_PROFILE,
        TEMPERATURE_FIELD,
        STRESS_FIELD,
        ELECTRIC_FIELD,
        CURRENT_DENSITY,
        LAYER_STRUCTURE,
        GEOMETRY_MESH,
        PROCESS_HISTORY,
        MATERIAL_PROPERTIES,
        SIMULATION_STATISTICS,
        DRC_VIOLATIONS,
        RELIABILITY_DATA
    };

    // Report types
    enum class ReportType {
        PROCESS_SUMMARY,
        ELECTRICAL_ANALYSIS,
        THERMAL_ANALYSIS,
        MECHANICAL_ANALYSIS,
        RELIABILITY_ANALYSIS,
        DRC_REPORT,
        STATISTICAL_REPORT,
        COMPARISON_REPORT,
        FULL_REPORT
    };

    struct ExportOptions {
        OutputFormat format;
        std::string filename;
        bool include_metadata;
        bool compress_data;
        double precision;
        std::string units;
        std::unordered_map<std::string, std::string> custom_options;
        
        ExportOptions() : format(OutputFormat::YAML), include_metadata(true), 
                         compress_data(false), precision(6), units("um") {}
    };

    struct ReportOptions {
        ReportType type;
        std::string title;
        std::string author;
        bool include_plots;
        bool include_tables;
        bool include_statistics;
        std::vector<std::string> sections;
        std::string template_file;
        
        ReportOptions() : type(ReportType::PROCESS_SUMMARY), include_plots(true), 
                         include_tables(true), include_statistics(true) {}
    };

public:
    explicit OutputGenerator(const std::string& base_directory = ".");
    ~OutputGenerator() = default;

    // Configuration
    void setBaseDirectory(const std::string& directory);
    std::string getBaseDirectory() const;
    void setDefaultFormat(OutputFormat format);
    OutputFormat getDefaultFormat() const;
    
    // Data export methods
    bool exportData(const std::string& filename, DataType type, 
                   const std::shared_ptr<Wafer>& wafer, 
                   const ExportOptions& options = ExportOptions());
    
    bool exportDopingProfile(const std::string& filename, 
                           const std::shared_ptr<Wafer>& wafer,
                           const ExportOptions& options = ExportOptions());
    
    bool exportTemperatureField(const std::string& filename, 
                              const std::shared_ptr<Wafer>& wafer,
                              const ExportOptions& options = ExportOptions());
    
    bool exportStressField(const std::string& filename, 
                         const std::shared_ptr<Wafer>& wafer,
                         const ExportOptions& options = ExportOptions());
    
    bool exportElectricField(const std::string& filename, 
                           const std::shared_ptr<Wafer>& wafer,
                           const ExportOptions& options = ExportOptions());
    
    bool exportLayerStructure(const std::string& filename, 
                            const std::shared_ptr<Wafer>& wafer,
                            const ExportOptions& options = ExportOptions());
    
    bool exportGeometryMesh(const std::string& filename, 
                          const std::shared_ptr<Wafer>& wafer,
                          const ExportOptions& options = ExportOptions());
    
    // Process data export
    bool exportProcessHistory(const std::string& filename, 
                            const std::vector<std::string>& process_steps,
                            const ExportOptions& options = ExportOptions());
    
    bool exportMaterialProperties(const std::string& filename, 
                                const std::unordered_map<std::string, std::unordered_map<std::string, double>>& materials,
                                const ExportOptions& options = ExportOptions());
    
    bool exportSimulationStatistics(const std::string& filename, 
                                   const std::unordered_map<std::string, double>& stats,
                                   const ExportOptions& options = ExportOptions());
    
    // Visualization export
    bool exportVisualization(const std::string& filename, 
                           const std::shared_ptr<Wafer>& wafer,
                           const std::string& view_type = "cross_section",
                           const ExportOptions& options = ExportOptions());
    
    bool exportCrossSection(const std::string& filename, 
                          const std::shared_ptr<Wafer>& wafer,
                          const std::string& direction = "x",
                          double position = 0.0,
                          const ExportOptions& options = ExportOptions());
    
    bool exportTopView(const std::string& filename, 
                     const std::shared_ptr<Wafer>& wafer,
                     const ExportOptions& options = ExportOptions());
    
    bool export3DVisualization(const std::string& filename, 
                             const std::shared_ptr<Wafer>& wafer,
                             const ExportOptions& options = ExportOptions());
    
    // Report generation
    bool generateReport(const std::string& filename, 
                       const std::shared_ptr<Wafer>& wafer,
                       const ReportOptions& options = ReportOptions());
    
    bool generateProcessSummary(const std::string& filename, 
                              const std::shared_ptr<Wafer>& wafer,
                              const std::vector<std::string>& process_steps);
    
    bool generateElectricalAnalysis(const std::string& filename, 
                                  const std::shared_ptr<Wafer>& wafer);
    
    bool generateThermalAnalysis(const std::string& filename, 
                               const std::shared_ptr<Wafer>& wafer);
    
    bool generateReliabilityAnalysis(const std::string& filename, 
                                   const std::shared_ptr<Wafer>& wafer);
    
    bool generateDRCReport(const std::string& filename, 
                         const std::vector<std::string>& violations);
    
    bool generateComparisonReport(const std::string& filename, 
                                const std::vector<std::shared_ptr<Wafer>>& wafers,
                                const std::vector<std::string>& labels);
    
    // Batch export
    bool exportBatch(const std::string& directory, 
                    const std::shared_ptr<Wafer>& wafer,
                    const std::vector<DataType>& data_types,
                    const ExportOptions& options = ExportOptions());
    
    // Format-specific exporters
    bool exportToYAML(const std::string& filename, const YAML::Node& data);
    bool exportToJSON(const std::string& filename, const YAML::Node& data);
    bool exportToCSV(const std::string& filename, const Eigen::MatrixXd& data, 
                    const std::vector<std::string>& headers = {});
    bool exportToVTK(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToHDF5(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToMATLAB(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToTecplot(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToGDSII(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToPNG(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToSVG(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToPDF(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    bool exportToHTML(const std::string& filename, const std::shared_ptr<Wafer>& wafer);
    
    // Utility methods
    std::vector<std::string> getSupportedFormats() const;
    bool isFormatSupported(OutputFormat format) const;
    std::string getFormatExtension(OutputFormat format) const;
    OutputFormat detectFormat(const std::string& filename) const;
    
    // Validation
    bool validateExportOptions(const ExportOptions& options) const;
    bool validateReportOptions(const ReportOptions& options) const;
    
    // Error handling
    std::string getLastError() const;
    std::vector<std::string> getAllErrors() const;
    void clearErrors();
    
    // Statistics
    struct ExportStatistics {
        size_t files_exported;
        size_t total_data_size;
        std::chrono::duration<double> total_export_time;
        std::unordered_map<std::string, size_t> format_counts;
        
        ExportStatistics() : files_exported(0), total_data_size(0) {}
    };
    
    ExportStatistics getExportStatistics() const;
    void resetStatistics();

private:
    std::string base_directory_;
    OutputFormat default_format_;
    mutable std::string last_error_;
    mutable std::vector<std::string> errors_;
    mutable ExportStatistics stats_;
    
    // Helper methods
    std::string getFullPath(const std::string& filename) const;
    void logError(const std::string& error) const;
    void logInfo(const std::string& info) const;
    bool createDirectoryIfNeeded(const std::string& filepath) const;
    
    // Data conversion helpers
    YAML::Node waferToYAML(const std::shared_ptr<Wafer>& wafer) const;
    Eigen::MatrixXd extractFieldData(const std::shared_ptr<Wafer>& wafer, DataType type) const;
    std::vector<std::string> generateFieldHeaders(DataType type) const;
    
    // Format-specific helpers
    std::string formatNumber(double value, int precision) const;
    std::string formatUnits(const std::string& base_units) const;
    std::string generateMetadata(const std::shared_ptr<Wafer>& wafer) const;
    
    // Report generation helpers
    std::string generateHTMLHeader(const ReportOptions& options) const;
    std::string generateHTMLFooter() const;
    std::string generateProcessTable(const std::vector<std::string>& process_steps) const;
    std::string generateStatisticsTable(const std::unordered_map<std::string, double>& stats) const;
    
    // Visualization helpers
    bool generateCrossSectionPlot(const std::string& filename, 
                                const std::shared_ptr<Wafer>& wafer,
                                const std::string& direction, double position) const;
    bool generateTopViewPlot(const std::string& filename, 
                           const std::shared_ptr<Wafer>& wafer) const;
    bool generate3DPlot(const std::string& filename, 
                       const std::shared_ptr<Wafer>& wafer) const;
};

} // namespace SemiPRO

#endif // OUTPUT_GENERATOR_HPP

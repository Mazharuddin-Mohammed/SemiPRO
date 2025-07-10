#pragma once

#include "../core/advanced_logger.hpp"
#include "../core/enhanced_error_handling.hpp"
#include "../core/config_manager.hpp"
#include "../core/wafer_enhanced.hpp"
#include "../advanced/multi_layer_engine.hpp"
#include "../advanced/temperature_controller.hpp"
#include "../advanced/process_optimizer.hpp"
#include "../advanced/process_integrator.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <sstream>

namespace SemiPRO {

/**
 * Advanced Visualization Engine
 * Implements sophisticated real-time visualization, interactive displays,
 * enhanced output formatting, and comprehensive analysis tools
 */

// Visualization types
enum class VisualizationType {
    WAFER_2D_MAP,         // 2D wafer surface visualization
    WAFER_3D_PROFILE,     // 3D wafer profile visualization
    LAYER_STACK,          // Multi-layer stack visualization
    TEMPERATURE_MAP,      // Temperature distribution map
    PROCESS_FLOW,         // Process flow diagram
    OPTIMIZATION_PLOT,    // Optimization convergence plot
    REAL_TIME_MONITOR,    // Real-time process monitoring
    STATISTICAL_CHART,    // Statistical analysis charts
    QUALITY_DASHBOARD,    // Quality metrics dashboard
    INTERACTIVE_3D        // Interactive 3D visualization
};

// Output formats
enum class OutputFormat {
    HTML_REPORT,          // Interactive HTML report
    JSON_DATA,            // JSON data export
    CSV_TABLE,            // CSV table format
    XML_STRUCTURED,       // XML structured data
    MATPLOTLIB_PYTHON,    // Python matplotlib script
    GNUPLOT_SCRIPT,       // Gnuplot visualization script
    SVG_VECTOR,           // SVG vector graphics
    PNG_RASTER,           // PNG raster image
    PDF_DOCUMENT,         // PDF document
    LATEX_REPORT          // LaTeX report format
};

// Visualization themes
enum class VisualizationTheme {
    PROFESSIONAL,         // Professional scientific theme
    DARK_MODE,            // Dark mode theme
    HIGH_CONTRAST,        // High contrast accessibility theme
    COLORBLIND_FRIENDLY,  // Colorblind-friendly palette
    PRESENTATION,         // Presentation-ready theme
    PUBLICATION,          // Publication-quality theme
    INTERACTIVE,          // Interactive web theme
    MINIMAL               // Minimal clean theme
};

// Data visualization structure
struct VisualizationData {
    std::string title;                              // Visualization title
    std::string description;                        // Description
    VisualizationType type;                         // Visualization type
    std::vector<std::vector<double>> data_2d;       // 2D data array
    std::vector<std::vector<std::vector<double>>> data_3d; // 3D data array
    std::unordered_map<std::string, std::vector<double>> series; // Data series
    std::unordered_map<std::string, std::string> metadata; // Metadata
    std::vector<std::string> labels;                // Axis labels
    std::vector<std::string> legends;               // Legend entries
    std::pair<double, double> x_range;              // X-axis range
    std::pair<double, double> y_range;              // Y-axis range
    std::pair<double, double> z_range;              // Z-axis range
    
    VisualizationData() : type(VisualizationType::WAFER_2D_MAP),
                         x_range({0.0, 1.0}), y_range({0.0, 1.0}), z_range({0.0, 1.0}) {}
};

// Interactive visualization controls
struct InteractiveControls {
    bool enable_zoom = true;                        // Enable zoom functionality
    bool enable_pan = true;                         // Enable pan functionality
    bool enable_rotation = true;                    // Enable 3D rotation
    bool enable_animation = true;                   // Enable animations
    bool enable_tooltips = true;                    // Enable interactive tooltips
    bool enable_selection = true;                   // Enable data selection
    bool enable_export = true;                      // Enable export functionality
    double animation_speed = 1.0;                   // Animation speed multiplier
    int max_data_points = 10000;                    // Maximum data points for performance
};

// Real-time monitoring configuration
struct RealTimeConfig {
    bool enable_real_time = false;                  // Enable real-time updates
    double update_interval = 1.0;                   // Update interval (seconds)
    int max_history_points = 1000;                  // Maximum history points
    bool auto_scale = true;                         // Auto-scale axes
    bool show_statistics = true;                    // Show real-time statistics
    std::vector<std::string> monitored_parameters;  // Parameters to monitor
};

// Visualization output
struct VisualizationOutput {
    std::string output_id;                          // Unique output identifier
    OutputFormat format;                            // Output format
    std::string file_path;                          // Output file path
    std::string content;                            // Generated content
    std::unordered_map<std::string, std::string> assets; // Associated assets
    double generation_time;                         // Generation time (seconds)
    size_t file_size;                              // Output file size (bytes)
    
    VisualizationOutput() : format(OutputFormat::HTML_REPORT), 
                           generation_time(0.0), file_size(0) {}
};

// Advanced visualization engine
class VisualizationEngine {
private:
    std::unordered_map<std::string, VisualizationData> visualizations_;
    std::unordered_map<std::string, VisualizationOutput> outputs_;
    
    // Configuration
    VisualizationTheme current_theme_ = VisualizationTheme::PROFESSIONAL;
    InteractiveControls interactive_config_;
    RealTimeConfig realtime_config_;
    
    // Output settings
    std::string output_directory_ = "output/visualizations/";
    bool enable_high_quality_ = true;
    bool enable_interactive_ = true;
    bool enable_responsive_ = true;
    int default_width_ = 1200;
    int default_height_ = 800;
    int default_dpi_ = 300;
    
public:
    VisualizationEngine();
    ~VisualizationEngine() = default;
    
    // Wafer visualization
    VisualizationData createWafer2DMap(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& property = "thickness"
    );
    
    VisualizationData createWafer3DProfile(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& property = "thickness"
    );
    
    VisualizationData createLayerStackVisualization(
        std::shared_ptr<MultiLayerEngine> layer_engine
    );
    
    // Temperature visualization
    VisualizationData createTemperatureMap(
        std::shared_ptr<AdvancedTemperatureController> temp_controller
    );
    
    VisualizationData createTemperatureProfile(
        const std::vector<double>& time_points,
        const std::vector<double>& temperatures
    );
    
    // Process visualization
    VisualizationData createProcessFlowDiagram(
        const ProcessRecipe& recipe
    );
    
    VisualizationData createOptimizationPlot(
        const ProcessOptimizationResults& optimization_results
    );
    
    // Real-time monitoring
    VisualizationData createRealTimeMonitor(
        const std::vector<std::string>& parameter_names
    );
    
    void updateRealTimeData(
        const std::string& visualization_id,
        const std::unordered_map<std::string, double>& new_data
    );
    
    // Statistical visualization
    VisualizationData createStatisticalChart(
        const std::unordered_map<std::string, std::vector<double>>& data,
        const std::string& chart_type = "histogram"
    );
    
    VisualizationData createQualityDashboard(
        const std::unordered_map<std::string, double>& quality_metrics
    );
    
    // Output generation
    VisualizationOutput generateHTMLReport(
        const std::vector<std::string>& visualization_ids,
        const std::string& report_title = "SemiPRO Simulation Report"
    );
    
    VisualizationOutput generateInteractiveHTML(
        const std::string& visualization_id
    );
    
    VisualizationOutput generatePythonScript(
        const std::string& visualization_id
    );
    
    VisualizationOutput generateGnuplotScript(
        const std::string& visualization_id
    );
    
    VisualizationOutput exportToJSON(
        const std::string& visualization_id
    );
    
    VisualizationOutput exportToCSV(
        const std::string& visualization_id
    );
    
    // Advanced features
    VisualizationData createComparativeAnalysis(
        const std::vector<std::string>& visualization_ids,
        const std::string& comparison_type = "overlay"
    );
    
    VisualizationData createAnimatedSequence(
        const std::vector<std::string>& visualization_ids,
        double frame_duration = 0.5
    );
    
    VisualizationData createInteractive3D(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::vector<std::string>& properties
    );
    
    // Configuration and customization
    void setTheme(VisualizationTheme theme);
    void setInteractiveControls(const InteractiveControls& controls);
    void setRealTimeConfig(const RealTimeConfig& config);
    void setOutputDirectory(const std::string& directory);
    void setDefaultDimensions(int width, int height, int dpi = 300);
    
    // Data management
    void addVisualization(const std::string& id, const VisualizationData& data);
    VisualizationData& getVisualization(const std::string& id);
    const VisualizationData& getVisualization(const std::string& id) const;
    void removeVisualization(const std::string& id);
    
    // Utility functions
    std::vector<std::string> getAvailableVisualizations() const;
    std::vector<std::string> getSupportedFormats() const;
    std::unordered_map<std::string, double> getVisualizationStatistics() const;
    
    // Validation and diagnostics
    bool validateVisualizationData(const VisualizationData& data, std::string& error_message) const;
    std::vector<std::string> getDiagnostics() const;
    
    // Theme and styling
    std::string getThemeCSS() const;
    std::string getColorPalette() const;
    std::unordered_map<std::string, std::string> getThemeSettings() const;
    
private:
    void initializeVisualizationEngine();
    
    // HTML generation helpers
    std::string generateHTMLHeader(const std::string& title) const;
    std::string generateHTMLFooter() const;
    std::string generatePlotlyScript(const VisualizationData& data) const;
    std::string generateD3Script(const VisualizationData& data) const;
    
    // Data processing helpers
    std::vector<std::vector<double>> processWaferData(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& property
    ) const;
    
    std::vector<double> calculateStatistics(const std::vector<double>& data) const;
    std::vector<std::vector<double>> interpolateData(
        const std::vector<std::vector<double>>& data,
        int target_resolution
    ) const;
    
    // Color and styling helpers
    std::vector<std::string> generateColorPalette(int num_colors) const;
    std::string getColorForValue(double value, double min_val, double max_val) const;
    std::string formatNumber(double value, int precision = 3) const;
    
    // File I/O helpers
    bool writeToFile(const std::string& file_path, const std::string& content) const;
    std::string readTemplate(const std::string& template_name) const;
    void ensureOutputDirectory() const;
};

// Utility functions for visualization
namespace VisualizationUtils {
    // Data processing utilities
    std::vector<std::vector<double>> resampleData(
        const std::vector<std::vector<double>>& data,
        int new_width, int new_height
    );
    
    std::vector<double> smoothData(
        const std::vector<double>& data,
        int window_size = 5
    );
    
    std::pair<double, double> calculateDataRange(
        const std::vector<std::vector<double>>& data
    );
    
    // Color utilities
    std::string rgbToHex(int r, int g, int b);
    std::tuple<int, int, int> hsvToRgb(double h, double s, double v);
    std::vector<std::string> generateGradient(
        const std::string& start_color,
        const std::string& end_color,
        int steps
    );
    
    // Format conversion utilities
    std::string convertToSVG(const VisualizationData& data);
    std::string convertToGnuplot(const VisualizationData& data);
    std::string convertToMatplotlib(const VisualizationData& data);
    
    // Statistical utilities
    std::unordered_map<std::string, double> calculateDescriptiveStats(
        const std::vector<double>& data
    );
    
    std::vector<double> calculateHistogram(
        const std::vector<double>& data,
        int num_bins = 20
    );
    
    double calculateCorrelation(
        const std::vector<double>& x,
        const std::vector<double>& y
    );
}

} // namespace SemiPRO

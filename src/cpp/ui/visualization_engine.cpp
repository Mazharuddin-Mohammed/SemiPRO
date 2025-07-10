#include "visualization_engine.hpp"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <numeric>
#include <iomanip>
#include <filesystem>

namespace SemiPRO {

VisualizationEngine::VisualizationEngine() {
    initializeVisualizationEngine();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                      "Visualization Engine initialized", "VisualizationEngine");
}

VisualizationData VisualizationEngine::createWafer2DMap(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& property) {
    
    SEMIPRO_PERF_TIMER("wafer_2d_visualization", "VisualizationEngine");
    
    VisualizationData viz_data;
    viz_data.title = "Wafer 2D Map - " + property;
    viz_data.description = "2D visualization of wafer " + property + " distribution";
    viz_data.type = VisualizationType::WAFER_2D_MAP;
    
    try {
        // Process wafer data
        viz_data.data_2d = processWaferData(wafer, property);
        
        // Set up axes and ranges
        viz_data.labels = {"X Position (mm)", "Y Position (mm)", property};
        
        if (!viz_data.data_2d.empty()) {
            auto range = VisualizationUtils::calculateDataRange(viz_data.data_2d);
            viz_data.z_range = range;
            
            // Set spatial ranges based on wafer diameter
            double radius = wafer->getDiameter() / 2.0;
            viz_data.x_range = {-radius, radius};
            viz_data.y_range = {-radius, radius};
        }
        
        viz_data.metadata["wafer_diameter"] = std::to_string(wafer->getDiameter());
        viz_data.metadata["wafer_thickness"] = std::to_string(wafer->getThickness());
        viz_data.metadata["property"] = property;
        viz_data.metadata["generation_time"] = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                          "Created wafer 2D map for property: " + property,
                          "VisualizationEngine");

    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to create wafer 2D map: " + std::string(e.what()),
                          "VisualizationEngine");
    }
    
    return viz_data;
}

VisualizationData VisualizationEngine::createWafer3DProfile(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& property) {
    
    VisualizationData viz_data;
    viz_data.title = "Wafer 3D Profile - " + property;
    viz_data.description = "3D visualization of wafer " + property + " profile";
    viz_data.type = VisualizationType::WAFER_3D_PROFILE;
    
    try {
        // Create 3D data from 2D wafer data
        auto data_2d = processWaferData(wafer, property);
        
        // Convert 2D data to 3D format (add Z coordinate based on property value)
        viz_data.data_3d.resize(data_2d.size());
        for (size_t i = 0; i < data_2d.size(); ++i) {
            viz_data.data_3d[i].resize(data_2d[i].size());
            for (size_t j = 0; j < data_2d[i].size(); ++j) {
                viz_data.data_3d[i][j] = {
                    static_cast<double>(i), 
                    static_cast<double>(j), 
                    data_2d[i][j]
                };
            }
        }
        
        viz_data.labels = {"X Position (mm)", "Y Position (mm)", property + " (μm)"};
        viz_data.metadata["property"] = property;
        viz_data.metadata["visualization_type"] = "3D_surface";
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                          "Created wafer 3D profile for property: " + property,
                          "VisualizationEngine");

    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to create wafer 3D profile: " + std::string(e.what()),
                          "VisualizationEngine");
    }
    
    return viz_data;
}

VisualizationData VisualizationEngine::createTemperatureProfile(
    const std::vector<double>& time_points,
    const std::vector<double>& temperatures) {
    
    VisualizationData viz_data;
    viz_data.title = "Temperature Profile";
    viz_data.description = "Temperature vs. time profile";
    viz_data.type = VisualizationType::TEMPERATURE_MAP;
    
    try {
        if (time_points.size() != temperatures.size()) {
            throw std::invalid_argument("Time points and temperatures must have the same size");
        }
        
        viz_data.series["time"] = time_points;
        viz_data.series["temperature"] = temperatures;
        viz_data.labels = {"Time (minutes)", "Temperature (°C)"};
        
        if (!time_points.empty()) {
            viz_data.x_range = {*std::min_element(time_points.begin(), time_points.end()),
                               *std::max_element(time_points.begin(), time_points.end())};
        }
        
        if (!temperatures.empty()) {
            viz_data.y_range = {*std::min_element(temperatures.begin(), temperatures.end()),
                               *std::max_element(temperatures.begin(), temperatures.end())};
        }
        
        viz_data.metadata["data_points"] = std::to_string(time_points.size());
        viz_data.metadata["max_temperature"] = std::to_string(viz_data.y_range.second);
        viz_data.metadata["min_temperature"] = std::to_string(viz_data.y_range.first);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                          "Created temperature profile with " + std::to_string(time_points.size()) + " points",
                          "VisualizationEngine");

    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to create temperature profile: " + std::string(e.what()),
                          "VisualizationEngine");
    }
    
    return viz_data;
}

VisualizationData VisualizationEngine::createOptimizationPlot(
    const ProcessOptimizationResults& optimization_results) {
    
    VisualizationData viz_data;
    viz_data.title = "Optimization Convergence";
    viz_data.description = "Process optimization convergence plot";
    viz_data.type = VisualizationType::OPTIMIZATION_PLOT;
    
    try {
        // Extract fitness values from optimization results
        std::vector<double> generations;
        std::vector<double> fitness_values;
        
        for (size_t i = 0; i < optimization_results.evaluations.size(); ++i) {
            generations.push_back(static_cast<double>(i));
            fitness_values.push_back(optimization_results.evaluations[i].fitness_score);
        }
        
        viz_data.series["generation"] = generations;
        viz_data.series["fitness"] = fitness_values;
        viz_data.labels = {"Generation", "Fitness Score"};
        
        if (!generations.empty()) {
            viz_data.x_range = {0.0, static_cast<double>(generations.size() - 1)};
        }
        
        if (!fitness_values.empty()) {
            viz_data.y_range = {*std::min_element(fitness_values.begin(), fitness_values.end()),
                               *std::max_element(fitness_values.begin(), fitness_values.end())};
        }
        
        viz_data.metadata["total_evaluations"] = std::to_string(optimization_results.total_evaluations);
        viz_data.metadata["best_fitness"] = std::to_string(optimization_results.best_solution.fitness_score);
        viz_data.metadata["convergence"] = optimization_results.has_converged ? "true" : "false";
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                          "Created optimization plot with " + std::to_string(fitness_values.size()) + " evaluations",
                          "VisualizationEngine");

    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to create optimization plot: " + std::string(e.what()),
                          "VisualizationEngine");
    }
    
    return viz_data;
}

VisualizationOutput VisualizationEngine::generateHTMLReport(
    const std::vector<std::string>& visualization_ids,
    const std::string& report_title) {
    
    SEMIPRO_PERF_TIMER("html_report_generation", "VisualizationEngine");
    
    VisualizationOutput output;
    output.format = OutputFormat::HTML_REPORT;
    output.output_id = "html_report_" + std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    
    auto start_time = std::chrono::steady_clock::now();
    
    try {
        ensureOutputDirectory();
        
        std::stringstream html;
        
        // Generate HTML header
        html << generateHTMLHeader(report_title);
        
        // Add report content
        html << "<div class='container'>\n";
        html << "<h1>" << report_title << "</h1>\n";
        html << "<div class='report-meta'>\n";
        html << "<p>Generated: " << std::chrono::system_clock::now().time_since_epoch().count() << "</p>\n";
        html << "<p>Visualizations: " << visualization_ids.size() << "</p>\n";
        html << "</div>\n";
        
        // Add each visualization
        for (const auto& viz_id : visualization_ids) {
            auto viz_it = visualizations_.find(viz_id);
            if (viz_it != visualizations_.end()) {
                const auto& viz_data = viz_it->second;
                
                html << "<div class='visualization-section'>\n";
                html << "<h2>" << viz_data.title << "</h2>\n";
                html << "<p>" << viz_data.description << "</p>\n";
                
                // Generate visualization-specific content
                html << "<div class='visualization-container' id='" << viz_id << "'>\n";
                html << generatePlotlyScript(viz_data);
                html << "</div>\n";
                
                // Add metadata table
                if (!viz_data.metadata.empty()) {
                    html << "<div class='metadata-table'>\n";
                    html << "<h3>Metadata</h3>\n";
                    html << "<table>\n";
                    for (const auto& meta : viz_data.metadata) {
                        html << "<tr><td>" << meta.first << "</td><td>" << meta.second << "</td></tr>\n";
                    }
                    html << "</table>\n";
                    html << "</div>\n";
                }
                
                html << "</div>\n";
            }
        }
        
        html << "</div>\n";
        html << generateHTMLFooter();
        
        // Write to file
        output.file_path = output_directory_ + output.output_id + ".html";
        output.content = html.str();
        
        if (writeToFile(output.file_path, output.content)) {
            output.file_size = output.content.size();
            
            auto end_time = std::chrono::steady_clock::now();
            output.generation_time = std::chrono::duration<double>(end_time - start_time).count();
            
            SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                              "Generated HTML report: " + output.file_path +
                              " (" + std::to_string(output.file_size) + " bytes)",
                              "VisualizationEngine");
        } else {
            throw std::runtime_error("Failed to write HTML report to file");
        }

    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to generate HTML report: " + std::string(e.what()),
                          "VisualizationEngine");
    }
    
    return output;
}

VisualizationOutput VisualizationEngine::generatePythonScript(
    const std::string& visualization_id) {
    
    VisualizationOutput output;
    output.format = OutputFormat::MATPLOTLIB_PYTHON;
    output.output_id = "python_" + visualization_id;
    
    try {
        auto viz_it = visualizations_.find(visualization_id);
        if (viz_it == visualizations_.end()) {
            throw std::invalid_argument("Visualization not found: " + visualization_id);
        }
        
        const auto& viz_data = viz_it->second;
        
        std::stringstream python;
        
        // Python script header
        python << "#!/usr/bin/env python3\n";
        python << "# Generated by SemiPRO Visualization Engine\n";
        python << "# Visualization: " << viz_data.title << "\n\n";
        
        python << "import matplotlib.pyplot as plt\n";
        python << "import numpy as np\n";
        python << "from mpl_toolkits.mplot3d import Axes3D\n\n";
        
        // Generate data arrays
        if (!viz_data.data_2d.empty()) {
            python << "# 2D Data\n";
            python << "data_2d = np.array([\n";
            for (const auto& row : viz_data.data_2d) {
                python << "    [";
                for (size_t i = 0; i < row.size(); ++i) {
                    python << formatNumber(row[i]);
                    if (i < row.size() - 1) python << ", ";
                }
                python << "],\n";
            }
            python << "])\n\n";
        }
        
        // Generate series data
        for (const auto& series : viz_data.series) {
            python << "# " << series.first << " data\n";
            python << series.first << " = np.array([";
            for (size_t i = 0; i < series.second.size(); ++i) {
                python << formatNumber(series.second[i]);
                if (i < series.second.size() - 1) python << ", ";
            }
            python << "])\n\n";
        }
        
        // Generate plot based on visualization type
        switch (viz_data.type) {
            case VisualizationType::WAFER_2D_MAP:
                python << "# Create 2D wafer map\n";
                python << "fig, ax = plt.subplots(figsize=(10, 8))\n";
                python << "im = ax.imshow(data_2d, cmap='viridis', aspect='equal')\n";
                python << "plt.colorbar(im, ax=ax)\n";
                break;
                
            case VisualizationType::TEMPERATURE_MAP:
                python << "# Create temperature profile\n";
                python << "fig, ax = plt.subplots(figsize=(12, 6))\n";
                python << "ax.plot(time, temperature, 'b-', linewidth=2)\n";
                python << "ax.grid(True, alpha=0.3)\n";
                break;
                
            case VisualizationType::OPTIMIZATION_PLOT:
                python << "# Create optimization plot\n";
                python << "fig, ax = plt.subplots(figsize=(10, 6))\n";
                python << "ax.plot(generation, fitness, 'r-', linewidth=2, marker='o')\n";
                python << "ax.grid(True, alpha=0.3)\n";
                break;
                
            default:
                python << "# Generic plot\n";
                python << "fig, ax = plt.subplots(figsize=(10, 6))\n";
                break;
        }
        
        // Add labels and title
        python << "ax.set_title('" << viz_data.title << "')\n";
        if (viz_data.labels.size() >= 2) {
            python << "ax.set_xlabel('" << viz_data.labels[0] << "')\n";
            python << "ax.set_ylabel('" << viz_data.labels[1] << "')\n";
        }
        
        python << "plt.tight_layout()\n";
        python << "plt.show()\n";
        
        output.content = python.str();
        output.file_path = output_directory_ + output.output_id + ".py";
        
        if (writeToFile(output.file_path, output.content)) {
            output.file_size = output.content.size();
            
            SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                              "Generated Python script: " + output.file_path,
                              "VisualizationEngine");
        }

    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to generate Python script: " + std::string(e.what()),
                          "VisualizationEngine");
    }
    
    return output;
}

std::vector<std::vector<double>> VisualizationEngine::processWaferData(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& property) const {
    
    std::vector<std::vector<double>> data;
    
    try {
        // Get wafer dimensions
        int grid_x = 50; // Default grid size
        int grid_y = 50;
        
        data.resize(grid_y);
        for (int i = 0; i < grid_y; ++i) {
            data[i].resize(grid_x);
            for (int j = 0; j < grid_x; ++j) {
                // Generate sample data based on property
                if (property == "thickness") {
                    // Simulate thickness variation
                    double x = (j - grid_x/2.0) / (grid_x/2.0);
                    double y = (i - grid_y/2.0) / (grid_y/2.0);
                    double r = std::sqrt(x*x + y*y);
                    
                    if (r <= 1.0) { // Within wafer radius
                        data[i][j] = wafer->getThickness() * (1.0 - 0.1 * r * r); // Parabolic variation
                    } else {
                        data[i][j] = 0.0; // Outside wafer
                    }
                } else if (property == "temperature") {
                    // Simulate temperature distribution
                    double x = (j - grid_x/2.0) / (grid_x/2.0);
                    double y = (i - grid_y/2.0) / (grid_y/2.0);
                    double r = std::sqrt(x*x + y*y);
                    
                    if (r <= 1.0) {
                        data[i][j] = 300.0 + 50.0 * std::exp(-r*r); // Gaussian temperature profile
                    } else {
                        data[i][j] = 25.0; // Ambient temperature
                    }
                } else {
                    // Default: uniform distribution
                    data[i][j] = 1.0;
                }
            }
        }
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to process wafer data: " + std::string(e.what()),
                          "VisualizationEngine");
    }
    
    return data;
}

void VisualizationEngine::initializeVisualizationEngine() {
    // Set default configuration
    current_theme_ = VisualizationTheme::PROFESSIONAL;
    
    // Create output directory if it doesn't exist
    ensureOutputDirectory();
}

std::string VisualizationEngine::generateHTMLHeader(const std::string& title) const {
    std::stringstream header;
    
    header << "<!DOCTYPE html>\n";
    header << "<html lang='en'>\n";
    header << "<head>\n";
    header << "    <meta charset='UTF-8'>\n";
    header << "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
    header << "    <title>" << title << "</title>\n";
    header << "    <script src='https://cdn.plot.ly/plotly-latest.min.js'></script>\n";
    header << "    <style>\n";
    header << getThemeCSS();
    header << "    </style>\n";
    header << "</head>\n";
    header << "<body>\n";
    
    return header.str();
}

std::string VisualizationEngine::generateHTMLFooter() const {
    return "</body>\n</html>\n";
}

std::string VisualizationEngine::getThemeCSS() const {
    std::stringstream css;
    
    css << "        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }\n";
    css << "        .container { max-width: 1200px; margin: 0 auto; background-color: white; padding: 30px; border-radius: 8px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }\n";
    css << "        h1 { color: #2c3e50; border-bottom: 3px solid #3498db; padding-bottom: 10px; }\n";
    css << "        h2 { color: #34495e; margin-top: 30px; }\n";
    css << "        .report-meta { background-color: #ecf0f1; padding: 15px; border-radius: 5px; margin: 20px 0; }\n";
    css << "        .visualization-section { margin: 30px 0; border: 1px solid #bdc3c7; border-radius: 5px; padding: 20px; }\n";
    css << "        .visualization-container { margin: 20px 0; min-height: 400px; }\n";
    css << "        .metadata-table table { width: 100%; border-collapse: collapse; margin-top: 10px; }\n";
    css << "        .metadata-table td { padding: 8px; border: 1px solid #ddd; }\n";
    css << "        .metadata-table td:first-child { background-color: #f8f9fa; font-weight: bold; width: 30%; }\n";
    
    return css.str();
}

std::string VisualizationEngine::generatePlotlyScript(const VisualizationData& data) const {
    std::stringstream script;

    script << "<script>\n";
    script << "var plotData = [];\n";
    script << "var layout = {\n";
    script << "    title: '" << data.title << "',\n";
    script << "    autosize: true,\n";
    script << "    margin: { l: 60, r: 60, t: 60, b: 60 }\n";
    script << "};\n";

    // Generate data based on visualization type
    switch (data.type) {
        case VisualizationType::WAFER_2D_MAP:
            if (!data.data_2d.empty()) {
                script << "var z = [\n";
                for (const auto& row : data.data_2d) {
                    script << "    [";
                    for (size_t i = 0; i < row.size(); ++i) {
                        script << formatNumber(row[i]);
                        if (i < row.size() - 1) script << ", ";
                    }
                    script << "],\n";
                }
                script << "];\n";
                script << "plotData.push({\n";
                script << "    z: z,\n";
                script << "    type: 'heatmap',\n";
                script << "    colorscale: 'Viridis'\n";
                script << "});\n";
            }
            break;

        case VisualizationType::TEMPERATURE_MAP:
        case VisualizationType::OPTIMIZATION_PLOT:
            for (const auto& series : data.series) {
                if (series.first == "time" || series.first == "generation") {
                    continue; // Skip x-axis data
                }

                script << "var x_data = [";
                auto x_it = data.series.find(series.first == "temperature" ? "time" : "generation");
                if (x_it != data.series.end()) {
                    for (size_t i = 0; i < x_it->second.size(); ++i) {
                        script << formatNumber(x_it->second[i]);
                        if (i < x_it->second.size() - 1) script << ", ";
                    }
                }
                script << "];\n";

                script << "var y_data = [";
                for (size_t i = 0; i < series.second.size(); ++i) {
                    script << formatNumber(series.second[i]);
                    if (i < series.second.size() - 1) script << ", ";
                }
                script << "];\n";

                script << "plotData.push({\n";
                script << "    x: x_data,\n";
                script << "    y: y_data,\n";
                script << "    type: 'scatter',\n";
                script << "    mode: 'lines+markers',\n";
                script << "    name: '" << series.first << "'\n";
                script << "});\n";
            }
            break;

        default:
            script << "// Unsupported visualization type\n";
            break;
    }

    script << "Plotly.newPlot('" << data.title << "', plotData, layout, {responsive: true});\n";
    script << "</script>\n";

    return script.str();
}

void VisualizationEngine::addVisualization(const std::string& id, const VisualizationData& data) {
    visualizations_[id] = data;

    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::USER_INTERFACE,
                      "Added visualization: " + id + " (" + data.title + ")",
                      "VisualizationEngine");
}

VisualizationData& VisualizationEngine::getVisualization(const std::string& id) {
    auto it = visualizations_.find(id);
    if (it == visualizations_.end()) {
        throw std::invalid_argument("Visualization not found: " + id);
    }
    return it->second;
}

const VisualizationData& VisualizationEngine::getVisualization(const std::string& id) const {
    auto it = visualizations_.find(id);
    if (it == visualizations_.end()) {
        throw std::invalid_argument("Visualization not found: " + id);
    }
    return it->second;
}

std::vector<std::string> VisualizationEngine::getAvailableVisualizations() const {
    std::vector<std::string> viz_ids;
    for (const auto& viz : visualizations_) {
        viz_ids.push_back(viz.first);
    }
    return viz_ids;
}

std::vector<std::string> VisualizationEngine::getSupportedFormats() const {
    return {
        "HTML_REPORT", "JSON_DATA", "CSV_TABLE", "MATPLOTLIB_PYTHON",
        "GNUPLOT_SCRIPT", "SVG_VECTOR", "PNG_RASTER"
    };
}

bool VisualizationEngine::validateVisualizationData(const VisualizationData& data, std::string& error_message) const {
    if (data.title.empty()) {
        error_message = "Visualization title cannot be empty";
        return false;
    }

    if (data.type == VisualizationType::WAFER_2D_MAP && data.data_2d.empty()) {
        error_message = "2D wafer map requires 2D data";
        return false;
    }

    if ((data.type == VisualizationType::TEMPERATURE_MAP || data.type == VisualizationType::OPTIMIZATION_PLOT)
        && data.series.empty()) {
        error_message = "Line plots require series data";
        return false;
    }

    return true;
}

std::vector<std::string> VisualizationEngine::getDiagnostics() const {
    std::vector<std::string> diagnostics;

    diagnostics.push_back("Visualization Engine Status:");
    diagnostics.push_back("  Visualizations: " + std::to_string(visualizations_.size()));
    diagnostics.push_back("  Outputs: " + std::to_string(outputs_.size()));
    diagnostics.push_back("  Theme: " + std::to_string(static_cast<int>(current_theme_)));

    diagnostics.push_back("Configuration:");
    diagnostics.push_back("  Output Directory: " + output_directory_);
    diagnostics.push_back("  High Quality: " + std::string(enable_high_quality_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Interactive: " + std::string(enable_interactive_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Default Dimensions: " + std::to_string(default_width_) + "x" + std::to_string(default_height_));

    diagnostics.push_back("Interactive Controls:");
    diagnostics.push_back("  Zoom: " + std::string(interactive_config_.enable_zoom ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Pan: " + std::string(interactive_config_.enable_pan ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Animation: " + std::string(interactive_config_.enable_animation ? "Enabled" : "Disabled"));

    return diagnostics;
}

std::string VisualizationEngine::formatNumber(double value, int precision) const {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

bool VisualizationEngine::writeToFile(const std::string& file_path, const std::string& content) const {
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) {
            return false;
        }

        file << content;
        file.close();
        return true;

    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::USER_INTERFACE,
                          "Failed to write file: " + file_path + " - " + std::string(e.what()),
                          "VisualizationEngine");
        return false;
    }
}

void VisualizationEngine::ensureOutputDirectory() const {
    try {
        std::filesystem::create_directories(output_directory_);
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::USER_INTERFACE,
                          "Failed to create output directory: " + std::string(e.what()),
                          "VisualizationEngine");
    }
}

// Utility functions implementation
namespace VisualizationUtils {

    std::pair<double, double> calculateDataRange(const std::vector<std::vector<double>>& data) {
        if (data.empty() || data[0].empty()) {
            return {0.0, 1.0};
        }

        double min_val = data[0][0];
        double max_val = data[0][0];

        for (const auto& row : data) {
            for (double val : row) {
                min_val = std::min(min_val, val);
                max_val = std::max(max_val, val);
            }
        }

        return {min_val, max_val};
    }

    std::string rgbToHex(int r, int g, int b) {
        std::stringstream ss;
        ss << "#" << std::hex << std::setfill('0') << std::setw(2) << r
           << std::setw(2) << g << std::setw(2) << b;
        return ss.str();
    }

    std::vector<double> smoothData(const std::vector<double>& data, int window_size) {
        if (data.size() < window_size) {
            return data;
        }

        std::vector<double> smoothed;
        smoothed.reserve(data.size());

        int half_window = window_size / 2;

        for (size_t i = 0; i < data.size(); ++i) {
            double sum = 0.0;
            int count = 0;

            for (int j = -half_window; j <= half_window; ++j) {
                int idx = static_cast<int>(i) + j;
                if (idx >= 0 && idx < static_cast<int>(data.size())) {
                    sum += data[idx];
                    count++;
                }
            }

            smoothed.push_back(sum / count);
        }

        return smoothed;
    }

    std::unordered_map<std::string, double> calculateDescriptiveStats(const std::vector<double>& data) {
        std::unordered_map<std::string, double> stats;

        if (data.empty()) {
            return stats;
        }

        // Mean
        double sum = std::accumulate(data.begin(), data.end(), 0.0);
        double mean = sum / data.size();
        stats["mean"] = mean;

        // Min and Max
        auto minmax = std::minmax_element(data.begin(), data.end());
        stats["min"] = *minmax.first;
        stats["max"] = *minmax.second;

        // Standard deviation
        double variance = 0.0;
        for (double val : data) {
            variance += (val - mean) * (val - mean);
        }
        variance /= data.size();
        stats["std_dev"] = std::sqrt(variance);

        // Median
        std::vector<double> sorted_data = data;
        std::sort(sorted_data.begin(), sorted_data.end());
        if (sorted_data.size() % 2 == 0) {
            stats["median"] = (sorted_data[sorted_data.size()/2 - 1] + sorted_data[sorted_data.size()/2]) / 2.0;
        } else {
            stats["median"] = sorted_data[sorted_data.size()/2];
        }

        return stats;
    }

} // namespace VisualizationUtils

} // namespace SemiPRO

#include "metrology_model.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

MetrologyModel::MetrologyModel() : rng_(std::random_device{}()) {
    // Set default measurement parameters
    measurement_parameters_["noise_level"] = 0.01;  // 1% noise
    measurement_parameters_["wavelength"] = 632.8;  // nm
    measurement_parameters_["numerical_aperture"] = 0.9;
    measurement_parameters_["spot_size"] = 1.0;     // μm
    
    // Set default calibration factors
    calibration_factors_["thickness"] = 1.0;
    calibration_factors_["cd"] = 1.0;
    calibration_factors_["overlay"] = 1.0;
    
    Logger::getInstance().log("MetrologyModel initialized with default parameters");
}

std::vector<MetrologyModel::MeasurementResult> MetrologyModel::performMeasurement(
    std::shared_ptr<Wafer> wafer,
    MeasurementType type,
    const std::vector<std::pair<double, double>>& measurement_points) {
    
    std::vector<MeasurementResult> results;
    results.reserve(measurement_points.size());
    
    for (const auto& point : measurement_points) {
        switch (type) {
            case THICKNESS:
                results.push_back(measureThickness(wafer, point.first, point.second));
                break;
            case CRITICAL_DIMENSION:
                results.push_back(measureCriticalDimension(wafer, point.first, point.second, "line"));
                break;
            case OVERLAY:
                results.push_back(measureOverlay(wafer, point.first, point.second, "layer1", "layer2"));
                break;
            case ROUGHNESS:
                results.push_back(MeasurementResult(ROUGHNESS, 
                    addMeasurementNoise(0.5, 0.1), 0.05, "nm"));
                break;
            case STRESS:
                results.push_back(MeasurementResult(STRESS,
                    addMeasurementNoise(100.0, 10.0), 5.0, "MPa"));
                break;
            case RESISTIVITY:
                results.push_back(MeasurementResult(RESISTIVITY,
                    addMeasurementNoise(1e-6, 1e-7), 5e-8, "Ω·cm"));
                break;
            case REFLECTANCE:
                results.push_back(MeasurementResult(REFLECTANCE,
                    addMeasurementNoise(0.3, 0.01), 0.005, ""));
                break;
            case ELLIPSOMETRY:
                results.push_back(performEllipsometry(wafer, point.first, point.second));
                break;
        }
    }
    
    Logger::getInstance().log("Performed " + std::to_string(results.size()) + " measurements");
    return results;
}

MetrologyModel::MeasurementResult MetrologyModel::measureThickness(
    std::shared_ptr<Wafer> wafer,
    double x, double y,
    const std::string& layer_material) {
    
    double thickness = 0.0;
    double uncertainty = 0.1; // nm
    
    if (layer_material.empty()) {
        // Measure total thickness
        thickness = wafer->getThickness();
        for (const auto& layer : wafer->getFilmLayers()) {
            thickness += layer.first;
        }
    } else {
        // Measure specific layer thickness
        for (const auto& layer : wafer->getFilmLayers()) {
            if (layer.second == layer_material) {
                thickness = layer.first;
                break;
            }
        }
    }
    
    // Add measurement noise and systematic errors
    double noise_level = measurement_parameters_["noise_level"];
    double measured_thickness = addMeasurementNoise(thickness, thickness * noise_level);
    
    // Apply calibration factor
    measured_thickness *= calibration_factors_["thickness"];
    
    MeasurementResult result(THICKNESS, measured_thickness, uncertainty, "μm");
    result.metadata["x_position"] = x;
    result.metadata["y_position"] = y;
    result.string_metadata["layer_material"] = layer_material.empty() ? "total" : layer_material;
    
    return result;
}

MetrologyModel::MeasurementResult MetrologyModel::measureCriticalDimension(
    std::shared_ptr<Wafer> wafer,
    double x, double y,
    const std::string& feature_type) {
    
    // Simulate CD measurement based on photoresist pattern
    auto grid_coords = convertToGridCoordinates(wafer, x, y);
    double pattern_value = 0.0;
    
    if (wafer->getGrid().rows() > 0 && wafer->getGrid().cols() > 0) {
        pattern_value = interpolateGridValue(wafer->getPhotoresistPattern(), 
                                           grid_coords.first, grid_coords.second);
    }
    
    // Convert pattern value to CD (simplified model)
    double nominal_cd = 100.0; // nm
    double cd = nominal_cd * (0.8 + 0.4 * pattern_value);
    
    // Add measurement noise
    double noise_level = measurement_parameters_["noise_level"];
    double measured_cd = addMeasurementNoise(cd, cd * noise_level);
    measured_cd *= calibration_factors_["cd"];
    
    MeasurementResult result(CRITICAL_DIMENSION, measured_cd, 2.0, "nm");
    result.metadata["x_position"] = x;
    result.metadata["y_position"] = y;
    result.string_metadata["feature_type"] = feature_type;
    
    return result;
}

MetrologyModel::MeasurementResult MetrologyModel::measureOverlay(
    std::shared_ptr<Wafer> wafer,
    double x, double y,
    const std::string& layer1,
    const std::string& layer2) {
    
    // Simulate overlay measurement (misalignment between layers)
    double overlay_x = addMeasurementNoise(0.0, 5.0); // nm
    double overlay_y = addMeasurementNoise(0.0, 5.0); // nm
    double total_overlay = std::sqrt(overlay_x * overlay_x + overlay_y * overlay_y);
    
    total_overlay *= calibration_factors_["overlay"];
    
    MeasurementResult result(OVERLAY, total_overlay, 1.0, "nm");
    result.metadata["x_position"] = x;
    result.metadata["y_position"] = y;
    result.string_metadata["layer1"] = layer1;
    result.string_metadata["layer2"] = layer2;
    result.metadata["overlay_x"] = overlay_x;
    result.metadata["overlay_y"] = overlay_y;
    
    return result;
}

std::unordered_map<std::string, double> MetrologyModel::calculateStatistics(
    const std::vector<MeasurementResult>& results) {
    
    if (results.empty()) {
        return {};
    }
    
    std::vector<double> values;
    values.reserve(results.size());
    for (const auto& result : results) {
        values.push_back(result.value);
    }
    
    std::sort(values.begin(), values.end());
    
    double mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    
    double variance = 0.0;
    for (double value : values) {
        variance += (value - mean) * (value - mean);
    }
    variance /= values.size();
    double std_dev = std::sqrt(variance);
    
    std::unordered_map<std::string, double> stats;
    stats["mean"] = mean;
    stats["std_dev"] = std_dev;
    stats["min"] = values.front();
    stats["max"] = values.back();
    stats["median"] = values[values.size() / 2];
    stats["range"] = values.back() - values.front();
    stats["count"] = static_cast<double>(values.size());
    
    if (values.size() >= 4) {
        stats["q1"] = values[values.size() / 4];
        stats["q3"] = values[3 * values.size() / 4];
    }
    
    return stats;
}

void MetrologyModel::calibrateInstrument(const std::string& calibration_type) {
    if (calibration_type == "thickness") {
        calibration_factors_["thickness"] = 1.0 + addMeasurementNoise(0.0, 0.001);
    } else if (calibration_type == "cd") {
        calibration_factors_["cd"] = 1.0 + addMeasurementNoise(0.0, 0.002);
    } else if (calibration_type == "overlay") {
        calibration_factors_["overlay"] = 1.0 + addMeasurementNoise(0.0, 0.001);
    }
    
    Logger::getInstance().log("Calibrated " + calibration_type + " measurement");
}

void MetrologyModel::setMeasurementParameters(
    const std::unordered_map<std::string, double>& params) {
    
    for (const auto& param : params) {
        measurement_parameters_[param.first] = param.second;
    }
    
    Logger::getInstance().log("Updated measurement parameters");
}

double MetrologyModel::addMeasurementNoise(double true_value, double noise_level) {
    std::normal_distribution<double> noise_dist(0.0, noise_level);
    return true_value + noise_dist(rng_);
}

double MetrologyModel::interpolateGridValue(const Eigen::ArrayXXd& grid, double x, double y) {
    if (grid.rows() == 0 || grid.cols() == 0) return 0.0;
    
    int i = static_cast<int>(std::clamp(x, 0.0, static_cast<double>(grid.rows() - 1)));
    int j = static_cast<int>(std::clamp(y, 0.0, static_cast<double>(grid.cols() - 1)));
    
    return grid(i, j);
}

std::pair<double, double> MetrologyModel::convertToGridCoordinates(
    std::shared_ptr<Wafer> wafer, double x, double y) {
    
    // Convert physical coordinates (μm) to grid coordinates
    double diameter = wafer->getDiameter() * 1000.0; // Convert mm to μm
    double grid_x = (x + diameter / 2.0) / diameter * wafer->getGrid().rows();
    double grid_y = (y + diameter / 2.0) / diameter * wafer->getGrid().cols();
    
    return {grid_x, grid_y};
}

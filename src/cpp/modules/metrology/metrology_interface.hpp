#ifndef METROLOGY_INTERFACE_HPP
#define METROLOGY_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class MetrologyInterface {
public:
    virtual ~MetrologyInterface() = default;
    
    // Measurement types
    enum MeasurementType {
        THICKNESS,
        CRITICAL_DIMENSION,
        OVERLAY,
        ROUGHNESS,
        STRESS,
        RESISTIVITY,
        REFLECTANCE,
        ELLIPSOMETRY
    };
    
    // Measurement result structure
    struct MeasurementResult {
        MeasurementType type;
        double value;
        double uncertainty;
        std::string units;
        std::unordered_map<std::string, double> metadata;
        
        MeasurementResult(MeasurementType t, double v, double u, const std::string& unit)
            : type(t), value(v), uncertainty(u), units(unit) {}
    };
    
    // Core measurement functions
    virtual std::vector<MeasurementResult> performMeasurement(
        std::shared_ptr<Wafer> wafer,
        MeasurementType type,
        const std::vector<std::pair<double, double>>& measurement_points
    ) = 0;
    
    virtual MeasurementResult measureThickness(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& layer_material = ""
    ) = 0;
    
    virtual MeasurementResult measureCriticalDimension(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& feature_type
    ) = 0;
    
    virtual MeasurementResult measureOverlay(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& layer1,
        const std::string& layer2
    ) = 0;
    
    // Statistical analysis
    virtual std::unordered_map<std::string, double> calculateStatistics(
        const std::vector<MeasurementResult>& results
    ) = 0;
    
    // Calibration and setup
    virtual void calibrateInstrument(const std::string& calibration_type) = 0;
    virtual void setMeasurementParameters(const std::unordered_map<std::string, double>& params) = 0;
};

#endif // METROLOGY_INTERFACE_HPP

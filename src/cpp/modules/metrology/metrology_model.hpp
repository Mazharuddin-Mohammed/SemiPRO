#ifndef METROLOGY_MODEL_HPP
#define METROLOGY_MODEL_HPP

#include "metrology_interface.hpp"
#include "../../core/utils.hpp"
#include <random>
#include <cmath>

class MetrologyModel : public MetrologyInterface {
public:
    MetrologyModel();
    ~MetrologyModel() override = default;
    
    // Implement interface methods
    std::vector<MeasurementResult> performMeasurement(
        std::shared_ptr<Wafer> wafer,
        MeasurementType type,
        const std::vector<std::pair<double, double>>& measurement_points
    ) override;
    
    MeasurementResult measureThickness(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& layer_material = ""
    ) override;
    
    MeasurementResult measureCriticalDimension(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& feature_type
    ) override;
    
    MeasurementResult measureOverlay(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& layer1,
        const std::string& layer2
    ) override;
    
    std::unordered_map<std::string, double> calculateStatistics(
        const std::vector<MeasurementResult>& results
    ) override;
    
    void calibrateInstrument(const std::string& calibration_type) override;
    void setMeasurementParameters(const std::unordered_map<std::string, double>& params) override;
    
    // Advanced measurement techniques
    MeasurementResult performEllipsometry(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        double wavelength = 632.8  // nm
    );
    
    MeasurementResult performReflectometry(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::vector<double>& wavelengths
    );
    
    MeasurementResult performScatterometry(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& grating_type
    );
    
    // Process control and SPC
    struct ControlLimits {
        double lower_control_limit;
        double upper_control_limit;
        double lower_spec_limit;
        double upper_spec_limit;
        double target_value;
    };
    
    void setControlLimits(MeasurementType type, const ControlLimits& limits);
    bool isWithinControlLimits(const MeasurementResult& result);
    double calculateCpk(const std::vector<MeasurementResult>& results, MeasurementType type);
    
    // Measurement uncertainty analysis
    struct UncertaintyBudget {
        double repeatability;
        double reproducibility;
        double calibration_uncertainty;
        double environmental_uncertainty;
        double combined_uncertainty;
    };
    
    UncertaintyBudget calculateUncertaintyBudget(MeasurementType type);
    
private:
    std::unordered_map<std::string, double> measurement_parameters_;
    std::unordered_map<MeasurementType, ControlLimits> control_limits_;
    std::unordered_map<std::string, double> calibration_factors_;
    
    mutable std::mt19937 rng_;
    
    // Helper functions
    double addMeasurementNoise(double true_value, double noise_level);
    double interpolateGridValue(const Eigen::ArrayXXd& grid, double x, double y);
    std::pair<double, double> convertToGridCoordinates(
        std::shared_ptr<Wafer> wafer, double x, double y
    );
    
    // Instrument-specific measurement functions
    double simulateOpticalThickness(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& material
    );
    
    double simulateElectricalMeasurement(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& measurement_type
    );
    
    double simulatePhysicalMeasurement(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& measurement_type
    );
};

#endif // METROLOGY_MODEL_HPP

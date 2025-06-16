#ifndef DEFECT_INSPECTION_MODEL_HPP
#define DEFECT_INSPECTION_MODEL_HPP

#include "defect_inspection_interface.hpp"
#include "../../core/utils.hpp"
#include <random>
#include <cmath>
#include <algorithm>

class DefectInspectionModel : public DefectInspectionInterface {
public:
    DefectInspectionModel();
    ~DefectInspectionModel() override = default;
    
    // Implement interface methods
    InspectionResult performInspection(
        std::shared_ptr<Wafer> wafer,
        InspectionMethod method,
        const std::vector<std::pair<double, double>>& inspection_areas
    ) override;
    
    std::vector<Defect> detectParticles(
        std::shared_ptr<Wafer> wafer,
        double min_size,
        double max_size
    ) override;
    
    std::vector<Defect> detectPatternDefects(
        std::shared_ptr<Wafer> wafer,
        const std::string& reference_pattern
    ) override;
    
    std::vector<Defect> detectDimensionalDefects(
        std::shared_ptr<Wafer> wafer,
        double tolerance
    ) override;
    
    DefectSeverity classifyDefect(const Defect& defect) override;
    
    std::unordered_map<std::string, int> generateDefectStatistics(
        const std::vector<Defect>& defects
    ) override;
    
    double calculateDefectDensity(
        const std::vector<Defect>& defects,
        double area
    ) override;
    
    double estimateYieldImpact(
        const std::vector<Defect>& defects,
        double die_area
    ) override;
    
    std::vector<std::pair<double, double>> identifyKillDefects(
        const std::vector<Defect>& defects,
        double die_area
    ) override;
    
    void optimizeInspectionParameters(
        InspectionMethod method,
        const std::unordered_map<std::string, double>& requirements
    ) override;
    
    double calculateInspectionSensitivity(
        InspectionMethod method,
        DefectType defect_type
    ) override;
    
    // Advanced inspection techniques
    std::vector<Defect> performAIBasedInspection(
        std::shared_ptr<Wafer> wafer,
        const std::string& model_path
    );
    
    std::vector<Defect> performMultiModalInspection(
        std::shared_ptr<Wafer> wafer,
        const std::vector<InspectionMethod>& methods
    );
    
    // Real-time inspection simulation
    struct InspectionParameters {
        double pixel_size;        // μm
        double scan_speed;        // mm/s
        double sensitivity;       // 0-1
        double false_positive_rate; // 0-1
        double false_negative_rate; // 0-1
        std::unordered_map<std::string, double> method_specific;
    };
    
    void setInspectionParameters(InspectionMethod method, const InspectionParameters& params);
    InspectionParameters getInspectionParameters(InspectionMethod method) const;
    
    // Defect generation for simulation
    void injectDefects(
        std::shared_ptr<Wafer> wafer,
        const std::vector<Defect>& defects
    );
    
    std::vector<Defect> generateRandomDefects(
        double area,
        double defect_density,
        const std::vector<DefectType>& allowed_types
    );
    
    // Statistical analysis
    struct DefectDistribution {
        std::unordered_map<DefectType, double> type_probabilities;
        std::unordered_map<DefectSeverity, double> severity_probabilities;
        double size_mean;
        double size_std_dev;
        std::string distribution_type; // "normal", "lognormal", "exponential"
    };
    
    void setDefectDistribution(const DefectDistribution& distribution);
    DefectDistribution getDefectDistribution() const { return defect_distribution_; }
    
    // Yield modeling
    double calculatePoissonYield(double defect_density, double die_area);
    double calculateNegativeBinomialYield(double defect_density, double die_area, double clustering_factor);
    
    // Process monitoring
    struct ProcessWindow {
        std::unordered_map<DefectType, double> acceptable_densities;
        double total_acceptable_density;
        std::vector<std::string> critical_layers;
    };
    
    void setProcessWindow(const ProcessWindow& window);
    bool isWithinProcessWindow(const std::vector<Defect>& defects, double area);
    
    // Inspection recipe optimization
    struct InspectionRecipe {
        std::vector<InspectionMethod> methods;
        std::vector<std::pair<double, double>> sampling_points;
        std::unordered_map<std::string, double> parameters;
        double expected_throughput; // wafers/hour
        double expected_sensitivity;
    };
    
    InspectionRecipe optimizeInspectionRecipe(
        const std::unordered_map<std::string, double>& requirements
    );
    
private:
    std::unordered_map<InspectionMethod, InspectionParameters> inspection_params_;
    DefectDistribution defect_distribution_;
    ProcessWindow process_window_;
    mutable std::mt19937 rng_;
    
    // Detection probability models
    double calculateDetectionProbability(
        InspectionMethod method,
        const Defect& defect
    );
    
    double calculateOpticalDetectionProbability(
        const Defect& defect,
        double wavelength,
        double numerical_aperture
    );
    
    double calculateEBeamDetectionProbability(
        const Defect& defect,
        double beam_energy,
        double pixel_size
    );
    
    // Signal processing simulation
    double simulateSignalToNoise(
        InspectionMethod method,
        const Defect& defect
    );
    
    std::vector<Defect> applyDetectionThreshold(
        const std::vector<Defect>& candidate_defects,
        double threshold
    );
    
    // Pattern matching algorithms
    double calculatePatternSimilarity(
        const std::vector<std::vector<double>>& pattern1,
        const std::vector<std::vector<double>>& pattern2
    );
    
    std::vector<Defect> performTemplateMatching(
        std::shared_ptr<Wafer> wafer,
        const std::string& template_pattern
    );
    
    // Machine learning simulation
    std::vector<Defect> simulateMLClassification(
        const std::vector<Defect>& candidate_defects,
        double classification_accuracy
    );
    
    // Helper functions
    DefectType stringToDefectType(const std::string& type_str);
    std::string defectTypeToString(DefectType type);
    
    double generateDefectSize(DefectType type);
    std::pair<double, double> generateDefectPosition(double wafer_diameter);
    
    // Coordinate transformations
    std::pair<int, int> physicalToGridCoordinates(
        std::shared_ptr<Wafer> wafer,
        double x, double y
    );
    
    std::pair<double, double> gridToPhysicalCoordinates(
        std::shared_ptr<Wafer> wafer,
        int grid_x, int grid_y
    );
};

#endif // DEFECT_INSPECTION_MODEL_HPP

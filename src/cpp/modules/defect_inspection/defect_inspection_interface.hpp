#ifndef DEFECT_INSPECTION_INTERFACE_HPP
#define DEFECT_INSPECTION_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class DefectInspectionInterface {
public:
    virtual ~DefectInspectionInterface() = default;
    
    // Defect types and classifications
    enum DefectType {
        PARTICLE,
        SCRATCH,
        VOID,
        BRIDGE,
        OPEN_CIRCUIT,
        SHORT_CIRCUIT,
        CONTAMINATION,
        CRYSTAL_DEFECT,
        PATTERN_DEFECT,
        DIMENSIONAL_DEFECT
    };
    
    enum DefectSeverity {
        CRITICAL,
        MAJOR,
        MINOR,
        COSMETIC
    };
    
    // Inspection methods
    enum InspectionMethod {
        OPTICAL_BRIGHTFIELD,
        OPTICAL_DARKFIELD,
        LASER_SCATTERING,
        ELECTRON_BEAM,
        ATOMIC_FORCE_MICROSCOPY,
        SCANNING_TUNNELING_MICROSCOPY,
        X_RAY_TOPOGRAPHY
    };
    
    // Defect structure
    struct Defect {
        DefectType type;
        DefectSeverity severity;
        double x, y, z;  // Position (μm)
        double size;     // Characteristic size (μm)
        double confidence; // Detection confidence (0-1)
        std::string description;
        std::unordered_map<std::string, double> properties;
        
        Defect(DefectType t, DefectSeverity s, double px, double py, double pz, double sz)
            : type(t), severity(s), x(px), y(py), z(pz), size(sz), confidence(1.0) {}
    };
    
    // Inspection results
    struct InspectionResult {
        InspectionMethod method;
        std::vector<Defect> defects;
        double coverage_area;  // μm²
        double inspection_time; // seconds
        std::unordered_map<std::string, double> statistics;
        
        InspectionResult(InspectionMethod m) : method(m), coverage_area(0.0), inspection_time(0.0) {}
    };
    
    // Core inspection functions
    virtual InspectionResult performInspection(
        std::shared_ptr<Wafer> wafer,
        InspectionMethod method,
        const std::vector<std::pair<double, double>>& inspection_areas
    ) = 0;
    
    virtual std::vector<Defect> detectParticles(
        std::shared_ptr<Wafer> wafer,
        double min_size,
        double max_size
    ) = 0;
    
    virtual std::vector<Defect> detectPatternDefects(
        std::shared_ptr<Wafer> wafer,
        const std::string& reference_pattern
    ) = 0;
    
    virtual std::vector<Defect> detectDimensionalDefects(
        std::shared_ptr<Wafer> wafer,
        double tolerance
    ) = 0;
    
    // Classification and analysis
    virtual DefectSeverity classifyDefect(const Defect& defect) = 0;
    
    virtual std::unordered_map<std::string, int> generateDefectStatistics(
        const std::vector<Defect>& defects
    ) = 0;
    
    virtual double calculateDefectDensity(
        const std::vector<Defect>& defects,
        double area
    ) = 0;
    
    // Yield impact analysis
    virtual double estimateYieldImpact(
        const std::vector<Defect>& defects,
        double die_area
    ) = 0;
    
    virtual std::vector<std::pair<double, double>> identifyKillDefects(
        const std::vector<Defect>& defects,
        double die_area
    ) = 0;
    
    // Inspection optimization
    virtual void optimizeInspectionParameters(
        InspectionMethod method,
        const std::unordered_map<std::string, double>& requirements
    ) = 0;
    
    virtual double calculateInspectionSensitivity(
        InspectionMethod method,
        DefectType defect_type
    ) = 0;
};

#endif // DEFECT_INSPECTION_INTERFACE_HPP

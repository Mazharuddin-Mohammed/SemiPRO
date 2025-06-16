#ifndef DRC_INTERFACE_HPP
#define DRC_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class DRCInterface {
public:
    virtual ~DRCInterface() = default;
    
    // Design rule types
    enum RuleType {
        MINIMUM_WIDTH,
        MINIMUM_SPACING,
        MINIMUM_AREA,
        MINIMUM_ENCLOSURE,
        MAXIMUM_WIDTH,
        MAXIMUM_AREA,
        ASPECT_RATIO,
        DENSITY,
        ANTENNA_RATIO,
        VIA_ENCLOSURE,
        OVERLAP,
        EXTENSION,
        NOTCH,
        CORNER_ROUNDING
    };
    
    enum ViolationSeverity {
        ERROR,
        WARNING,
        INFO
    };
    
    // Design rule definition
    struct DesignRule {
        RuleType type;
        std::string layer_name;
        std::string target_layer; // For spacing/enclosure rules
        double value;             // Rule value (μm, ratio, etc.)
        double tolerance;         // Allowed tolerance
        ViolationSeverity severity;
        std::string description;
        
        DesignRule(RuleType t, const std::string& layer, double val)
            : type(t), layer_name(layer), value(val), tolerance(0.0), severity(ERROR) {}
    };
    
    // Violation structure
    struct Violation {
        DesignRule rule;
        double x, y;              // Violation location (μm)
        double measured_value;    // Actual measured value
        double expected_value;    // Expected value from rule
        ViolationSeverity severity;
        std::string description;
        std::vector<std::pair<double, double>> polygon; // Violation area
        
        Violation(const DesignRule& r, double px, double py, double measured, double expected)
            : rule(r), x(px), y(py), measured_value(measured), expected_value(expected), 
              severity(r.severity) {}
    };
    
    // DRC results
    struct DRCResult {
        std::vector<Violation> violations;
        std::unordered_map<RuleType, int> violation_counts;
        std::unordered_map<std::string, int> layer_violation_counts;
        double total_area_checked; // μm²
        double check_time;         // seconds
        bool passed;
        
        DRCResult() : total_area_checked(0.0), check_time(0.0), passed(true) {}
    };
    
    // Core DRC functions
    virtual DRCResult performDRC(
        std::shared_ptr<Wafer> wafer,
        const std::vector<DesignRule>& rules
    ) = 0;
    
    virtual std::vector<Violation> checkMinimumWidth(
        std::shared_ptr<Wafer> wafer,
        const std::string& layer_name,
        double min_width
    ) = 0;
    
    virtual std::vector<Violation> checkMinimumSpacing(
        std::shared_ptr<Wafer> wafer,
        const std::string& layer_name,
        double min_spacing
    ) = 0;
    
    virtual std::vector<Violation> checkMinimumArea(
        std::shared_ptr<Wafer> wafer,
        const std::string& layer_name,
        double min_area
    ) = 0;
    
    virtual std::vector<Violation> checkEnclosure(
        std::shared_ptr<Wafer> wafer,
        const std::string& inner_layer,
        const std::string& outer_layer,
        double min_enclosure
    ) = 0;
    
    virtual std::vector<Violation> checkDensity(
        std::shared_ptr<Wafer> wafer,
        const std::string& layer_name,
        double min_density,
        double max_density,
        double window_size
    ) = 0;
    
    // Advanced DRC functions
    virtual std::vector<Violation> checkAntennaRatio(
        std::shared_ptr<Wafer> wafer,
        const std::string& metal_layer,
        const std::string& gate_layer,
        double max_ratio
    ) = 0;
    
    virtual std::vector<Violation> checkAspectRatio(
        std::shared_ptr<Wafer> wafer,
        const std::string& layer_name,
        double max_aspect_ratio
    ) = 0;
    
    // Rule management
    virtual void loadRuleSet(const std::string& rule_file) = 0;
    virtual void saveRuleSet(const std::string& rule_file, const std::vector<DesignRule>& rules) = 0;
    virtual std::vector<DesignRule> getStandardRules(const std::string& technology_node) = 0;
    
    // Reporting and analysis
    virtual void generateDRCReport(
        const DRCResult& result,
        const std::string& output_file
    ) = 0;
    
    virtual std::unordered_map<std::string, double> analyzeDRCStatistics(
        const DRCResult& result
    ) = 0;
    
    // Waiver and exception handling
    virtual void addWaiver(
        const Violation& violation,
        const std::string& justification
    ) = 0;
    
    virtual bool isWaived(const Violation& violation) = 0;
    
    // Interactive DRC
    virtual std::vector<Violation> checkRegion(
        std::shared_ptr<Wafer> wafer,
        const std::vector<DesignRule>& rules,
        double x1, double y1, double x2, double y2
    ) = 0;
    
    virtual void highlightViolations(
        std::shared_ptr<Wafer> wafer,
        const std::vector<Violation>& violations
    ) = 0;
};

#endif // DRC_INTERFACE_HPP

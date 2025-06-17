// Author: Dr. Mazharuddin Mohammed
#ifndef DRC_MODEL_HPP
#define DRC_MODEL_HPP

#include "drc_interface.hpp"
#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

enum class ViolationType {
    WIDTH,
    SPACING,
    AREA,
    ENCLOSURE,
    DENSITY,
    ANTENNA_RATIO,
    ASPECT_RATIO,
    CORNER_ROUNDING,
    OVERLAP,
    EXTENSION
};

// Use ViolationSeverity from interface
using ViolationSeverity = DRCInterface::ViolationSeverity;

struct DRCRule {
    std::string name;
    ViolationType type;
    std::string layer;
    double min_value;
    double max_value;
    ViolationSeverity severity;
    bool enabled;
    std::string description;
    
    DRCRule(const std::string& rule_name, ViolationType rule_type,
            const std::string& target_layer, double min_val, double max_val = -1.0)
        : name(rule_name), type(rule_type), layer(target_layer),
          min_value(min_val), max_value(max_val), severity(DRCInterface::ERROR),
          enabled(true) {}
};

struct DRCViolation {
    std::string rule_name;
    ViolationType type;
    ViolationSeverity severity;
    std::pair<double, double> location;
    double measured_value;
    double required_value;
    std::string description;
    bool waived;
    
    DRCViolation(const std::string& rule, ViolationType vtype, 
                 const std::pair<double, double>& loc, double measured, double required)
        : rule_name(rule), type(vtype), location(loc), measured_value(measured),
          required_value(required), waived(false) {}
};

class DRCModel : public DRCInterface {
public:
    DRCModel();
    
    // Rule management
    void addRule(const DRCRule& rule);
    void removeRule(const std::string& rule_name);
    void enableRule(const std::string& rule_name, bool enabled);
    void setRuleSeverity(const std::string& rule_name, ViolationSeverity severity);
    
    // Technology node configuration
    void loadTechnologyRules(const std::string& technology_node);
    void configureTechnology(double feature_size, double metal_pitch, double via_size);
    
    // DRC checking
    void runFullDRC(std::shared_ptr<Wafer> wafer);
    void runIncrementalDRC(std::shared_ptr<Wafer> wafer, 
                          const std::pair<double, double>& region_start,
                          const std::pair<double, double>& region_end);
    void runLayerDRC(std::shared_ptr<Wafer> wafer, const std::string& layer);
    
    // Specific rule checks
    void checkWidthRules(std::shared_ptr<Wafer> wafer);
    void checkSpacingRules(std::shared_ptr<Wafer> wafer);
    void checkAreaRules(std::shared_ptr<Wafer> wafer);
    void checkEnclosureRules(std::shared_ptr<Wafer> wafer);
    void checkDensityRules(std::shared_ptr<Wafer> wafer);
    void checkAntennaRules(std::shared_ptr<Wafer> wafer);
    void checkAspectRatioRules(std::shared_ptr<Wafer> wafer);
    void checkCornerRoundingRules(std::shared_ptr<Wafer> wafer);
    
    // Violation management
    void waiveViolation(size_t violation_index, const std::string& reason);
    void unwaiveViolation(size_t violation_index);
    void clearViolations();
    void exportViolations(const std::string& filename) const;
    
    // Analysis and reporting
    void generateDRCReport(const std::string& filename) const;
    void generateViolationSummary() const;
    std::vector<DRCViolation> getViolationsByType(ViolationType type) const;
    std::vector<DRCViolation> getViolationsBySeverity(ViolationSeverity severity) const;
    
    // Interactive DRC
    void highlightViolations(std::shared_ptr<Wafer> wafer);
    void showViolationDetails(size_t violation_index) const;
    
    // Getters
    const std::vector<DRCRule>& getRules() const { return rules_; }
    const std::vector<DRCViolation>& getViolations() const { return violations_; }
    size_t getViolationCount() const { return violations_.size(); }
    size_t getCriticalViolationCount() const;
    size_t getErrorViolationCount() const;
    size_t getWarningViolationCount() const;
    
    // Statistics
    double getDRCCoverage() const;
    double getViolationDensity() const;
    std::unordered_map<std::string, int> getViolationsByLayer() const;

private:
    std::vector<DRCRule> rules_;
    std::vector<DRCViolation> violations_;
    std::string technology_node_;
    double feature_size_;
    double metal_pitch_;
    double via_size_;
    
    // Helper methods
    void initializeDefaultRules();
    void loadTechnologyFile(const std::string& filename);
    bool isPointInRegion(const std::pair<double, double>& point,
                        const std::pair<double, double>& region_start,
                        const std::pair<double, double>& region_end) const;
    
    // Geometric analysis
    std::vector<std::pair<double, double>> extractFeatures(std::shared_ptr<Wafer> wafer,
                                                          const std::string& layer) const;
    double measureWidth(const std::vector<std::pair<double, double>>& features) const;
    double measureSpacing(const std::vector<std::pair<double, double>>& features) const;
    double measureArea(const std::vector<std::pair<double, double>>& features) const;
    
    // Violation detection
    void addViolation(const DRCViolation& violation);
    bool checkRuleCondition(const DRCRule& rule, double measured_value) const;
    ViolationSeverity determineViolationSeverity(const DRCRule& rule, 
                                                 double measured_value) const;
};

#endif // DRC_MODEL_HPP

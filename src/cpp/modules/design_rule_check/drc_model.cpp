// Author: Dr. Mazharuddin Mohammed
#include "drc_model.hpp"
#include "../../core/utils.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>

DRCModel::DRCModel() : technology_node_("generic"), feature_size_(0.1), 
                       metal_pitch_(0.2), via_size_(0.05) {
    initializeDefaultRules();
}

void DRCModel::addRule(const DRCRule& rule) {
    // Check for duplicate rule names
    auto it = std::find_if(rules_.begin(), rules_.end(),
                          [&rule](const DRCRule& r) { return r.name == rule.name; });
    if (it != rules_.end()) {
        throw std::invalid_argument("Rule with name " + rule.name + " already exists");
    }
    
    rules_.push_back(rule);
    Logger::getInstance().log("Added DRC rule: " + rule.name);
}

void DRCModel::removeRule(const std::string& rule_name) {
    auto it = std::find_if(rules_.begin(), rules_.end(),
                          [&rule_name](const DRCRule& r) { return r.name == rule_name; });
    if (it == rules_.end()) {
        throw std::invalid_argument("Rule with name " + rule_name + " not found");
    }
    
    rules_.erase(it);
    Logger::getInstance().log("Removed DRC rule: " + rule_name);
}

void DRCModel::enableRule(const std::string& rule_name, bool enabled) {
    auto it = std::find_if(rules_.begin(), rules_.end(),
                          [&rule_name](DRCRule& r) { return r.name == rule_name; });
    if (it == rules_.end()) {
        throw std::invalid_argument("Rule with name " + rule_name + " not found");
    }
    
    it->enabled = enabled;
    Logger::getInstance().log("Rule " + rule_name + (enabled ? " enabled" : " disabled"));
}

void DRCModel::loadTechnologyRules(const std::string& technology_node) {
    technology_node_ = technology_node;
    
    // Clear existing rules
    rules_.clear();
    
    // Load technology-specific rules
    if (technology_node == "7nm") {
        feature_size_ = 0.007;
        metal_pitch_ = 0.036;
        via_size_ = 0.018;
        
        // 7nm technology rules
        addRule(DRCRule("M1_WIDTH", ViolationType::WIDTH, "metal1", 0.014));
        addRule(DRCRule("M1_SPACING", ViolationType::SPACING, "metal1", 0.014));
        addRule(DRCRule("VIA1_SIZE", ViolationType::WIDTH, "via1", 0.018));
        addRule(DRCRule("POLY_WIDTH", ViolationType::WIDTH, "poly", 0.007));
        addRule(DRCRule("POLY_SPACING", ViolationType::SPACING, "poly", 0.021));
        
    } else if (technology_node == "14nm") {
        feature_size_ = 0.014;
        metal_pitch_ = 0.070;
        via_size_ = 0.035;
        
        // 14nm technology rules
        addRule(DRCRule("M1_WIDTH", ViolationType::WIDTH, "metal1", 0.028));
        addRule(DRCRule("M1_SPACING", ViolationType::SPACING, "metal1", 0.028));
        addRule(DRCRule("VIA1_SIZE", ViolationType::WIDTH, "via1", 0.035));
        addRule(DRCRule("POLY_WIDTH", ViolationType::WIDTH, "poly", 0.014));
        addRule(DRCRule("POLY_SPACING", ViolationType::SPACING, "poly", 0.042));
        
    } else if (technology_node == "28nm") {
        feature_size_ = 0.028;
        metal_pitch_ = 0.090;
        via_size_ = 0.045;
        
        // 28nm technology rules
        addRule(DRCRule("M1_WIDTH", ViolationType::WIDTH, "metal1", 0.056));
        addRule(DRCRule("M1_SPACING", ViolationType::SPACING, "metal1", 0.056));
        addRule(DRCRule("VIA1_SIZE", ViolationType::WIDTH, "via1", 0.045));
        addRule(DRCRule("POLY_WIDTH", ViolationType::WIDTH, "poly", 0.028));
        addRule(DRCRule("POLY_SPACING", ViolationType::SPACING, "poly", 0.084));
        
    } else {
        // Generic rules
        initializeDefaultRules();
    }
    
    Logger::getInstance().log("Loaded technology rules for " + technology_node);
}

void DRCModel::runFullDRC(std::shared_ptr<Wafer> wafer) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    clearViolations();
    
    Logger::getInstance().log("Starting full DRC check");
    
    // Run all enabled rule checks
    checkWidthRules(wafer);
    checkSpacingRules(wafer);
    checkAreaRules(wafer);
    checkEnclosureRules(wafer);
    checkDensityRules(wafer);
    checkAntennaRules(wafer);
    checkAspectRatioRules(wafer);
    checkCornerRoundingRules(wafer);
    
    Logger::getInstance().log("Full DRC check completed. Found " + 
                             std::to_string(violations_.size()) + " violations");
}

void DRCModel::checkWidthRules(std::shared_ptr<Wafer> wafer) {
    const auto& grid = wafer->getGrid();
    const auto& photoresist = wafer->getPhotoresistPattern();
    
    // Check width rules for each enabled width rule
    for (const auto& rule : rules_) {
        if (!rule.enabled || rule.type != ViolationType::WIDTH) continue;
        
        // Extract features for the layer
        auto features = extractFeatures(wafer, rule.layer);
        
        for (const auto& feature : features) {
            double measured_width = measureWidth({feature});
            
            if (!checkRuleCondition(rule, measured_width)) {
                DRCViolation violation(rule.name, ViolationType::WIDTH, feature,
                                     measured_width, rule.min_value);
                violation.severity = determineViolationSeverity(rule, measured_width);
                violation.description = "Width violation: measured " + 
                                      std::to_string(measured_width) + 
                                      " < required " + std::to_string(rule.min_value);
                addViolation(violation);
            }
        }
    }
}

void DRCModel::checkSpacingRules(std::shared_ptr<Wafer> wafer) {
    // Check spacing rules for each enabled spacing rule
    for (const auto& rule : rules_) {
        if (!rule.enabled || rule.type != ViolationType::SPACING) continue;
        
        auto features = extractFeatures(wafer, rule.layer);
        
        // Check spacing between all pairs of features
        for (size_t i = 0; i < features.size(); ++i) {
            for (size_t j = i + 1; j < features.size(); ++j) {
                double dx = features[i].first - features[j].first;
                double dy = features[i].second - features[j].second;
                double spacing = std::sqrt(dx*dx + dy*dy);
                
                if (!checkRuleCondition(rule, spacing)) {
                    DRCViolation violation(rule.name, ViolationType::SPACING,
                                         {(features[i].first + features[j].first) / 2,
                                          (features[i].second + features[j].second) / 2},
                                         spacing, rule.min_value);
                    violation.severity = determineViolationSeverity(rule, spacing);
                    violation.description = "Spacing violation: measured " + 
                                          std::to_string(spacing) + 
                                          " < required " + std::to_string(rule.min_value);
                    addViolation(violation);
                }
            }
        }
    }
}

void DRCModel::checkAreaRules(std::shared_ptr<Wafer> wafer) {
    for (const auto& rule : rules_) {
        if (!rule.enabled || rule.type != ViolationType::AREA) continue;
        
        auto features = extractFeatures(wafer, rule.layer);
        double total_area = measureArea(features);
        
        if (!checkRuleCondition(rule, total_area)) {
            DRCViolation violation(rule.name, ViolationType::AREA,
                                 {0.0, 0.0}, total_area, rule.min_value);
            violation.severity = determineViolationSeverity(rule, total_area);
            violation.description = "Area violation: measured " + 
                                  std::to_string(total_area) + 
                                  " < required " + std::to_string(rule.min_value);
            addViolation(violation);
        }
    }
}

void DRCModel::checkDensityRules(std::shared_ptr<Wafer> wafer) {
    const auto& grid = wafer->getGrid();
    double total_area = grid.rows() * grid.cols();
    
    for (const auto& rule : rules_) {
        if (!rule.enabled || rule.type != ViolationType::DENSITY) continue;
        
        auto features = extractFeatures(wafer, rule.layer);
        double layer_area = measureArea(features);
        double density = layer_area / total_area;
        
        if (!checkRuleCondition(rule, density)) {
            DRCViolation violation(rule.name, ViolationType::DENSITY,
                                 {0.0, 0.0}, density, rule.min_value);
            violation.severity = determineViolationSeverity(rule, density);
            violation.description = "Density violation: measured " + 
                                  std::to_string(density * 100) + "%" +
                                  " outside range [" + std::to_string(rule.min_value * 100) +
                                  "%, " + std::to_string(rule.max_value * 100) + "%]";
            addViolation(violation);
        }
    }
}

void DRCModel::checkAntennaRules(std::shared_ptr<Wafer> wafer) {
    // Simplified antenna rule checking
    const auto& metal_layers = wafer->getMetalLayers();
    
    for (const auto& rule : rules_) {
        if (!rule.enabled || rule.type != ViolationType::ANTENNA_RATIO) continue;
        
        // Calculate antenna ratio (simplified)
        double metal_area = 0.0;
        double gate_area = 1.0; // Simplified assumption
        
        for (const auto& layer : metal_layers) {
            metal_area += layer.first * layer.first; // Approximate area
        }
        
        double antenna_ratio = metal_area / gate_area;
        
        if (!checkRuleCondition(rule, antenna_ratio)) {
            DRCViolation violation(rule.name, ViolationType::ANTENNA_RATIO,
                                 {0.0, 0.0}, antenna_ratio, rule.max_value);
            violation.severity = determineViolationSeverity(rule, antenna_ratio);
            violation.description = "Antenna ratio violation: measured " + 
                                  std::to_string(antenna_ratio) + 
                                  " > allowed " + std::to_string(rule.max_value);
            addViolation(violation);
        }
    }
}

void DRCModel::generateDRCReport(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }
    
    file << "DRC Report\n";
    file << "==========\n\n";
    file << "Technology Node: " << technology_node_ << "\n";
    file << "Feature Size: " << feature_size_ << " um\n";
    file << "Total Rules: " << rules_.size() << "\n";
    file << "Total Violations: " << violations_.size() << "\n\n";
    
    // Violation summary by type
    std::unordered_map<ViolationType, int> violation_counts;
    for (const auto& violation : violations_) {
        violation_counts[violation.type]++;
    }
    
    file << "Violations by Type:\n";
    for (const auto& count : violation_counts) {
        file << "  " << static_cast<int>(count.first) << ": " << count.second << "\n";
    }
    
    file << "\nDetailed Violations:\n";
    for (size_t i = 0; i < violations_.size(); ++i) {
        const auto& v = violations_[i];
        file << i + 1 << ". " << v.rule_name << " at (" 
             << v.location.first << ", " << v.location.second << ")\n";
        file << "   " << v.description << "\n";
        if (v.waived) {
            file << "   [WAIVED]\n";
        }
        file << "\n";
    }
    
    file.close();
    Logger::getInstance().log("DRC report generated: " + filename);
}

void DRCModel::initializeDefaultRules() {
    // Default generic rules
    addRule(DRCRule("MIN_WIDTH", ViolationType::WIDTH, "metal", 0.1));
    addRule(DRCRule("MIN_SPACING", ViolationType::SPACING, "metal", 0.1));
    addRule(DRCRule("MIN_AREA", ViolationType::AREA, "metal", 0.01));
    addRule(DRCRule("MAX_DENSITY", ViolationType::DENSITY, "metal", 0.0, 0.8));
    addRule(DRCRule("ANTENNA_RATIO", ViolationType::ANTENNA_RATIO, "metal", 0.0, 100.0));
}

std::vector<std::pair<double, double>> DRCModel::extractFeatures(
    std::shared_ptr<Wafer> wafer, const std::string& layer) const {
    
    std::vector<std::pair<double, double>> features;
    const auto& grid = wafer->getGrid();
    const auto& photoresist = wafer->getPhotoresistPattern();
    
    // Extract features based on layer type
    if (layer == "metal" || layer == "metal1") {
        // Extract metal features from photoresist pattern
        for (int i = 0; i < photoresist.rows(); ++i) {
            for (int j = 0; j < photoresist.cols(); ++j) {
                if (photoresist(i, j) > 0.5) {
                    features.emplace_back(i, j);
                }
            }
        }
    }
    
    return features;
}

bool DRCModel::checkRuleCondition(const DRCRule& rule, double measured_value) const {
    switch (rule.type) {
        case ViolationType::WIDTH:
        case ViolationType::SPACING:
        case ViolationType::AREA:
            return measured_value >= rule.min_value;
        case ViolationType::DENSITY:
            return measured_value >= rule.min_value && 
                   (rule.max_value < 0 || measured_value <= rule.max_value);
        case ViolationType::ANTENNA_RATIO:
            return rule.max_value < 0 || measured_value <= rule.max_value;
        default:
            return true;
    }
}

void DRCModel::addViolation(const DRCViolation& violation) {
    violations_.push_back(violation);
}

size_t DRCModel::getCriticalViolationCount() const {
    return std::count_if(violations_.begin(), violations_.end(),
                        [](const DRCViolation& v) { 
                            return v.severity == ViolationSeverity::CRITICAL; 
                        });
}

void DRCModel::clearViolations() {
    violations_.clear();
}

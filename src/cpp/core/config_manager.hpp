#pragma once

#include "advanced_logger.hpp"
#include "enhanced_error_handling.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <variant>
#include <optional>
#include <regex>
#include <fstream>
#include <mutex>

namespace SemiPRO {

/**
 * Advanced Configuration Management System for SemiPRO
 * Provides hierarchical configuration, validation, defaults, and optimization
 */

// Configuration value types
using ConfigValue = std::variant<bool, int, double, std::string, std::vector<double>, std::vector<std::string>>;

// Parameter validation function type
using ValidationFunction = std::function<bool(const ConfigValue&, std::string&)>;

// Parameter constraint types
enum class ConstraintType {
    NONE,
    RANGE,           // min <= value <= max
    ENUM,            // value in {allowed_values}
    REGEX,           // string matches pattern
    CUSTOM           // custom validation function
};

// Parameter constraint definition
struct ParameterConstraint {
    ConstraintType type = ConstraintType::NONE;
    ConfigValue min_value;
    ConfigValue max_value;
    std::vector<ConfigValue> allowed_values;
    std::string regex_pattern;
    ValidationFunction custom_validator;
    std::string description;
    
    ParameterConstraint() = default;
    
    // Range constraint constructor
    ParameterConstraint(const ConfigValue& min_val, const ConfigValue& max_val, const std::string& desc = "")
        : type(ConstraintType::RANGE), min_value(min_val), max_value(max_val), description(desc) {}
    
    // Enum constraint constructor
    ParameterConstraint(const std::vector<ConfigValue>& allowed, const std::string& desc = "")
        : type(ConstraintType::ENUM), allowed_values(allowed), description(desc) {}
    
    // Regex constraint constructor
    ParameterConstraint(const std::string& pattern, const std::string& desc = "")
        : type(ConstraintType::REGEX), regex_pattern(pattern), description(desc) {}
    
    // Custom constraint constructor
    ParameterConstraint(ValidationFunction validator, const std::string& desc = "")
        : type(ConstraintType::CUSTOM), custom_validator(validator), description(desc) {}
};

// Parameter definition with metadata
struct ParameterDefinition {
    std::string name;
    std::string description;
    ConfigValue default_value;
    ParameterConstraint constraint;
    bool required = false;
    std::string category = "general";
    std::string units;
    std::vector<std::string> aliases;
    
    ParameterDefinition() = default;
    
    ParameterDefinition(const std::string& param_name, const ConfigValue& default_val,
                       const std::string& desc = "", bool is_required = false)
        : name(param_name), description(desc), default_value(default_val), required(is_required) {}
};

// Configuration section for hierarchical organization
class ConfigSection {
private:
    std::string name_;
    std::unordered_map<std::string, ConfigValue> values_;
    std::unordered_map<std::string, ParameterDefinition> definitions_;
    std::unordered_map<std::string, std::unique_ptr<ConfigSection>> subsections_;
    mutable std::mutex mutex_;
    
public:
    explicit ConfigSection(const std::string& name) : name_(name) {}
    
    // Parameter definition management
    void defineParameter(const ParameterDefinition& def);
    void defineParameter(const std::string& name, const ConfigValue& default_value,
                        const std::string& description = "", bool required = false);
    
    // Value access and modification
    template<typename T>
    std::optional<T> getValue(const std::string& key) const;
    
    template<typename T>
    T getValue(const std::string& key, const T& default_value) const;
    
    void setValue(const std::string& key, const ConfigValue& value);
    bool hasValue(const std::string& key) const;
    void removeValue(const std::string& key);
    
    // Subsection management
    ConfigSection& getOrCreateSubsection(const std::string& name);
    ConfigSection* getSubsection(const std::string& name) const;
    bool hasSubsection(const std::string& name) const;
    
    // Validation
    bool validate(std::vector<std::string>& errors) const;
    bool validateParameter(const std::string& key, const ConfigValue& value, std::string& error) const;
    
    // Utility methods
    std::vector<std::string> getParameterNames() const;
    std::vector<std::string> getSubsectionNames() const;
    void clear();
    
    // Serialization
    std::string toJSON(int indent = 0) const;
    std::string toYAML(int indent = 0) const;
    void fromJSON(const std::string& json);
    
    const std::string& getName() const { return name_; }
};

// Main configuration manager
class ConfigManager {
private:
    static std::unique_ptr<ConfigManager> instance_;
    static std::mutex instance_mutex_;
    
    std::unique_ptr<ConfigSection> root_section_;
    std::string config_file_path_;
    mutable std::mutex config_mutex_;
    
    // Configuration profiles for different scenarios
    std::unordered_map<std::string, std::unique_ptr<ConfigSection>> profiles_;
    std::string active_profile_ = "default";
    
    // Configuration change callbacks
    std::vector<std::function<void(const std::string&, const ConfigValue&)>> change_callbacks_;
    
    ConfigManager();
    
public:
    static ConfigManager& getInstance();
    ~ConfigManager() = default;
    
    // Configuration loading and saving
    bool loadFromFile(const std::string& file_path);
    bool saveToFile(const std::string& file_path = "") const;
    bool loadFromJSON(const std::string& json_content);
    bool loadFromYAML(const std::string& yaml_content);
    
    // Profile management
    void createProfile(const std::string& name);
    void switchProfile(const std::string& name);
    void deleteProfile(const std::string& name);
    std::vector<std::string> getProfileNames() const;
    const std::string& getActiveProfile() const { return active_profile_; }
    
    // Parameter access (with path notation: "section.subsection.parameter")
    template<typename T>
    std::optional<T> getValue(const std::string& path) const;
    
    template<typename T>
    T getValue(const std::string& path, const T& default_value) const;
    
    void setValue(const std::string& path, const ConfigValue& value);
    bool hasValue(const std::string& path) const;
    
    // Parameter definition
    void defineParameter(const std::string& path, const ParameterDefinition& def);
    void defineParameter(const std::string& path, const ConfigValue& default_value,
                        const std::string& description = "", bool required = false);
    
    // Validation
    bool validate(std::vector<std::string>& errors) const;
    bool isValid() const;
    
    // Configuration optimization
    void optimizeForPerformance();
    void optimizeForMemory();
    void optimizeForAccuracy();
    
    // Change notification
    void addChangeCallback(std::function<void(const std::string&, const ConfigValue&)> callback);
    void removeAllChangeCallbacks();
    
    // Utility methods
    void reset();
    void resetToDefaults();
    std::string exportConfiguration(const std::string& format = "json") const;
    
    // Built-in parameter definitions
    void initializeDefaultParameters();
    void initializePhysicsParameters();
    void initializePerformanceParameters();
    
    // Configuration merging
    void mergeConfiguration(const ConfigSection& other);
    void mergeFromFile(const std::string& file_path);
    
    // Environment variable integration
    void loadFromEnvironment(const std::string& prefix = "SEMIPRO_");
    void setEnvironmentOverrides(bool enabled = true);
    
    // Configuration validation and suggestions
    std::vector<std::string> getSuggestions(const std::string& partial_path) const;
    std::string getParameterHelp(const std::string& path) const;
    
private:
    std::vector<std::string> splitPath(const std::string& path) const;
    ConfigSection* navigateToSection(const std::string& path, bool create = false) const;
    std::string getParameterFromPath(const std::string& path) const;
    void notifyChange(const std::string& path, const ConfigValue& value);
    
    // Built-in validators
    static bool validatePositiveNumber(const ConfigValue& value, std::string& error);
    static bool validateTemperature(const ConfigValue& value, std::string& error);
    static bool validateMaterial(const ConfigValue& value, std::string& error);
};

// Template implementations
template<typename T>
std::optional<T> ConfigSection::getValue(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = values_.find(key);
    if (it != values_.end()) {
        try {
            return std::get<T>(it->second);
        } catch (const std::bad_variant_access&) {
            SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::VALIDATION,
                              "Type mismatch for parameter: " + key, "ConfigManager");
            return std::nullopt;
        }
    }
    
    // Try default value
    auto def_it = definitions_.find(key);
    if (def_it != definitions_.end()) {
        try {
            return std::get<T>(def_it->second.default_value);
        } catch (const std::bad_variant_access&) {
            return std::nullopt;
        }
    }
    
    return std::nullopt;
}

template<typename T>
T ConfigSection::getValue(const std::string& key, const T& default_value) const {
    auto result = getValue<T>(key);
    return result.value_or(default_value);
}

template<typename T>
std::optional<T> ConfigManager::getValue(const std::string& path) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto section = navigateToSection(path, false);
    if (!section) {
        return std::nullopt;
    }
    
    std::string param_name = getParameterFromPath(path);
    return section->getValue<T>(param_name);
}

template<typename T>
T ConfigManager::getValue(const std::string& path, const T& default_value) const {
    auto result = getValue<T>(path);
    return result.value_or(default_value);
}

// Utility macros for configuration access
#define CONFIG_GET(path, type, default_val) \
    SemiPRO::ConfigManager::getInstance().getValue<type>(path, default_val)

#define CONFIG_SET(path, value) \
    SemiPRO::ConfigManager::getInstance().setValue(path, value)

#define CONFIG_DEFINE(path, default_val, desc, required) \
    SemiPRO::ConfigManager::getInstance().defineParameter(path, default_val, desc, required)

// JSON parsing helper functions
namespace ConfigUtils {
    ConfigValue parseJSONValue(const std::string& json_str);
    std::string configValueToString(const ConfigValue& value);
    bool isNumericType(const ConfigValue& value);
    bool compareConfigValues(const ConfigValue& a, const ConfigValue& b);
}

} // namespace SemiPRO

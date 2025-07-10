#include "config_manager.hpp"
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <iostream>

namespace SemiPRO {

// Static member definitions
std::unique_ptr<ConfigManager> ConfigManager::instance_ = nullptr;
std::mutex ConfigManager::instance_mutex_;

// ConfigSection implementation
void ConfigSection::defineParameter(const ParameterDefinition& def) {
    std::lock_guard<std::mutex> lock(mutex_);
    definitions_[def.name] = def;
    
    // Set default value if not already set
    if (values_.find(def.name) == values_.end()) {
        values_[def.name] = def.default_value;
    }
    
    SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::VALIDATION,
                      "Defined parameter: " + def.name + " in section: " + name_,
                      "ConfigManager");
}

void ConfigSection::defineParameter(const std::string& name, const ConfigValue& default_value,
                                   const std::string& description, bool required) {
    ParameterDefinition def(name, default_value, description, required);
    defineParameter(def);
}

void ConfigSection::setValue(const std::string& key, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Validate if definition exists
    auto def_it = definitions_.find(key);
    if (def_it != definitions_.end()) {
        std::string error;
        if (!validateParameter(key, value, error)) {
            throw ValidationException("Parameter validation failed for " + key + ": " + error);
        }
    }
    
    values_[key] = value;
    
    SEMIPRO_LOG_MODULE(LogLevel::TRACE, LogCategory::VALIDATION,
                      "Set parameter: " + key + " in section: " + name_,
                      "ConfigManager");
}

bool ConfigSection::hasValue(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return values_.find(key) != values_.end();
}

void ConfigSection::removeValue(const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);
    values_.erase(key);
}

ConfigSection& ConfigSection::getOrCreateSubsection(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = subsections_.find(name);
    if (it == subsections_.end()) {
        subsections_[name] = std::make_unique<ConfigSection>(name);
    }
    
    return *subsections_[name];
}

ConfigSection* ConfigSection::getSubsection(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = subsections_.find(name);
    return (it != subsections_.end()) ? it->second.get() : nullptr;
}

bool ConfigSection::hasSubsection(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return subsections_.find(name) != subsections_.end();
}

bool ConfigSection::validate(std::vector<std::string>& errors) const {
    std::lock_guard<std::mutex> lock(mutex_);
    bool valid = true;
    
    // Check required parameters
    for (const auto& [name, def] : definitions_) {
        if (def.required && values_.find(name) == values_.end()) {
            errors.push_back("Required parameter missing: " + name_ + "." + name);
            valid = false;
        }
    }
    
    // Validate existing values
    for (const auto& [key, value] : values_) {
        std::string error;
        if (!validateParameter(key, value, error)) {
            errors.push_back("Validation failed for " + name_ + "." + key + ": " + error);
            valid = false;
        }
    }
    
    // Validate subsections
    for (const auto& [name, subsection] : subsections_) {
        if (!subsection->validate(errors)) {
            valid = false;
        }
    }
    
    return valid;
}

bool ConfigSection::validateParameter(const std::string& key, const ConfigValue& value, std::string& error) const {
    auto def_it = definitions_.find(key);
    if (def_it == definitions_.end()) {
        return true; // No definition means no validation
    }
    
    const auto& constraint = def_it->second.constraint;
    
    switch (constraint.type) {
        case ConstraintType::NONE:
            return true;
            
        case ConstraintType::RANGE: {
            if (ConfigUtils::isNumericType(value) && ConfigUtils::isNumericType(constraint.min_value) && 
                ConfigUtils::isNumericType(constraint.max_value)) {
                
                double val = std::visit([](const auto& v) -> double {
                    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>) {
                        return static_cast<double>(v);
                    }
                    return 0.0;
                }, value);
                
                double min_val = std::visit([](const auto& v) -> double {
                    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>) {
                        return static_cast<double>(v);
                    }
                    return 0.0;
                }, constraint.min_value);
                
                double max_val = std::visit([](const auto& v) -> double {
                    if constexpr (std::is_arithmetic_v<std::decay_t<decltype(v)>>) {
                        return static_cast<double>(v);
                    }
                    return 0.0;
                }, constraint.max_value);
                
                if (val < min_val || val > max_val) {
                    error = "Value " + std::to_string(val) + " not in range [" + 
                           std::to_string(min_val) + ", " + std::to_string(max_val) + "]";
                    return false;
                }
            }
            return true;
        }
        
        case ConstraintType::ENUM: {
            for (const auto& allowed : constraint.allowed_values) {
                if (ConfigUtils::compareConfigValues(value, allowed)) {
                    return true;
                }
            }
            error = "Value not in allowed set";
            return false;
        }
        
        case ConstraintType::REGEX: {
            if (std::holds_alternative<std::string>(value)) {
                std::regex pattern(constraint.regex_pattern);
                if (!std::regex_match(std::get<std::string>(value), pattern)) {
                    error = "String does not match pattern: " + constraint.regex_pattern;
                    return false;
                }
            }
            return true;
        }
        
        case ConstraintType::CUSTOM: {
            if (constraint.custom_validator) {
                return constraint.custom_validator(value, error);
            }
            return true;
        }
    }
    
    return true;
}

std::vector<std::string> ConfigSection::getParameterNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    
    for (const auto& [name, _] : values_) {
        names.push_back(name);
    }
    
    return names;
}

std::vector<std::string> ConfigSection::getSubsectionNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> names;
    
    for (const auto& [name, _] : subsections_) {
        names.push_back(name);
    }
    
    return names;
}

void ConfigSection::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    values_.clear();
    subsections_.clear();
    definitions_.clear();
}

std::string ConfigSection::toJSON(int indent) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::ostringstream oss;
    std::string indent_str(indent, ' ');
    
    oss << "{\n";
    
    // Add values
    bool first = true;
    for (const auto& [key, value] : values_) {
        if (!first) oss << ",\n";
        oss << indent_str << "  \"" << key << "\": ";
        oss << ConfigUtils::configValueToString(value);
        first = false;
    }
    
    // Add subsections
    for (const auto& [name, subsection] : subsections_) {
        if (!first) oss << ",\n";
        oss << indent_str << "  \"" << name << "\": ";
        oss << subsection->toJSON(indent + 2);
        first = false;
    }
    
    oss << "\n" << indent_str << "}";
    return oss.str();
}

// ConfigManager implementation
ConfigManager::ConfigManager() {
    root_section_ = std::make_unique<ConfigSection>("root");
    initializeDefaultParameters();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::GENERAL,
                      "Configuration Manager initialized", "ConfigManager");
}

ConfigManager& ConfigManager::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<ConfigManager>(new ConfigManager());
    }
    return *instance_;
}

bool ConfigManager::loadFromFile(const std::string& file_path) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    std::ifstream file(file_path);
    if (!file.is_open()) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::SYSTEM,
                          "Failed to open config file: " + file_path, "ConfigManager");
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    file.close();
    
    config_file_path_ = file_path;
    
    // Determine format by extension
    if (file_path.size() >= 5 && file_path.substr(file_path.size() - 5) == ".json") {
        return loadFromJSON(content);
    } else if ((file_path.size() >= 5 && file_path.substr(file_path.size() - 5) == ".yaml") ||
               (file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".yml")) {
        return loadFromYAML(content);
    }
    
    // Try JSON first, then YAML
    if (loadFromJSON(content)) {
        return true;
    }
    
    return loadFromYAML(content);
}

bool ConfigManager::saveToFile(const std::string& file_path) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    std::string path = file_path.empty() ? config_file_path_ : file_path;
    if (path.empty()) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::SYSTEM,
                          "No file path specified for saving config", "ConfigManager");
        return false;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::SYSTEM,
                          "Failed to open config file for writing: " + path, "ConfigManager");
        return false;
    }
    
    file << root_section_->toJSON();
    file.close();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::GENERAL,
                      "Configuration saved to: " + path, "ConfigManager");
    
    return true;
}

void ConfigManager::setValue(const std::string& path, const ConfigValue& value) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto section = navigateToSection(path, true);
    if (section) {
        std::string param_name = getParameterFromPath(path);
        section->setValue(param_name, value);
        notifyChange(path, value);
    }
}

bool ConfigManager::hasValue(const std::string& path) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    
    auto section = navigateToSection(path, false);
    if (!section) {
        return false;
    }
    
    std::string param_name = getParameterFromPath(path);
    return section->hasValue(param_name);
}

void ConfigManager::defineParameter(const std::string& path, const ParameterDefinition& def) {
    std::lock_guard<std::mutex> lock(config_mutex_);

    auto section = navigateToSection(path, true);
    if (section) {
        section->defineParameter(def);
    }
}

void ConfigManager::defineParameter(const std::string& path, const ConfigValue& default_value,
                                   const std::string& description, bool required) {
    ParameterDefinition def(getParameterFromPath(path), default_value, description, required);
    defineParameter(path, def);
}

bool ConfigManager::validate(std::vector<std::string>& errors) const {
    std::lock_guard<std::mutex> lock(config_mutex_);
    return root_section_->validate(errors);
}

bool ConfigManager::isValid() const {
    std::vector<std::string> errors;
    return validate(errors);
}

std::vector<std::string> ConfigManager::splitPath(const std::string& path) const {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;

    while (std::getline(ss, part, '.')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    return parts;
}

ConfigSection* ConfigManager::navigateToSection(const std::string& path, bool create) const {
    auto parts = splitPath(path);
    if (parts.empty()) {
        return root_section_.get();
    }

    ConfigSection* current = root_section_.get();

    // Navigate to the section (all parts except the last one)
    for (size_t i = 0; i < parts.size() - 1; ++i) {
        if (create) {
            current = &current->getOrCreateSubsection(parts[i]);
        } else {
            current = current->getSubsection(parts[i]);
            if (!current) {
                return nullptr;
            }
        }
    }

    return current;
}

std::string ConfigManager::getParameterFromPath(const std::string& path) const {
    auto parts = splitPath(path);
    return parts.empty() ? "" : parts.back();
}

void ConfigManager::notifyChange(const std::string& path, const ConfigValue& value) {
    for (const auto& callback : change_callbacks_) {
        try {
            callback(path, value);
        } catch (const std::exception& e) {
            SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::GENERAL,
                              "Exception in config change callback: " + std::string(e.what()),
                              "ConfigManager");
        }
    }
}

bool ConfigManager::loadFromJSON(const std::string& json_content) {
    // Simple JSON parsing - in a real implementation, use a proper JSON library
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::GENERAL,
                      "Loading configuration from JSON", "ConfigManager");
    return true; // Placeholder
}

bool ConfigManager::loadFromYAML(const std::string& yaml_content) {
    // Simple YAML parsing - in a real implementation, use a proper YAML library
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::GENERAL,
                      "Loading configuration from YAML", "ConfigManager");
    return true; // Placeholder
}

void ConfigManager::addChangeCallback(std::function<void(const std::string&, const ConfigValue&)> callback) {
    std::lock_guard<std::mutex> lock(config_mutex_);
    change_callbacks_.push_back(callback);
}

void ConfigManager::removeAllChangeCallbacks() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    change_callbacks_.clear();
}

void ConfigManager::initializeDefaultParameters() {
    // Physics parameters
    defineParameter("physics.temperature.default", 300.0, "Default temperature in Celsius", false);
    defineParameter("physics.pressure.default", 1.0, "Default pressure in atm", false);
    defineParameter("physics.time_step", 1e-12, "Default simulation time step in seconds", false);

    // Performance parameters
    defineParameter("performance.max_threads", 0, "Maximum number of threads (0 = auto)", false);
    defineParameter("performance.memory_limit", 0, "Memory limit in bytes (0 = unlimited)", false);
    defineParameter("performance.cache_size", 1024*1024, "Cache size in bytes", false);

    // Simulation parameters
    defineParameter("simulation.precision", 1e-6, "Numerical precision tolerance", false);
    defineParameter("simulation.max_iterations", 10000, "Maximum solver iterations", false);
    defineParameter("simulation.convergence_threshold", 1e-8, "Convergence threshold", false);

    SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::GENERAL,
                      "Default parameters initialized", "ConfigManager");
}

// ConfigUtils implementation
namespace ConfigUtils {
    std::string configValueToString(const ConfigValue& value) {
        return std::visit([](const auto& v) -> std::string {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, std::string>) {
                return "\"" + v + "\"";
            } else if constexpr (std::is_same_v<T, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<T>) {
                return std::to_string(v);
            } else {
                return "\"unknown\"";
            }
        }, value);
    }
    
    bool isNumericType(const ConfigValue& value) {
        return std::holds_alternative<int>(value) || std::holds_alternative<double>(value);
    }
    
    bool compareConfigValues(const ConfigValue& a, const ConfigValue& b) {
        return a == b;
    }
}

} // namespace SemiPRO

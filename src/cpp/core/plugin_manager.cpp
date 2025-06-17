// Author: Dr. Mazharuddin Mohammed
#include "plugin_manager.hpp"
#include "utils.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

namespace SemiPRO {

// Static member initialization
std::unordered_map<std::string, std::function<std::shared_ptr<PluginBase>()>> PluginRegistry::registry_;

// PluginManager Implementation
PluginManager::PluginManager() {
    Logger::getInstance().log("Plugin manager initialized");
}

PluginManager::~PluginManager() {
    unloadAllPlugins();
}

void PluginManager::addPluginDirectory(const std::string& directory) {
    if (std::filesystem::exists(directory) && std::filesystem::is_directory(directory)) {
        plugin_directories_.push_back(directory);
        Logger::getInstance().log("Added plugin directory: " + directory);
    } else {
        last_error_ = "Directory does not exist: " + directory;
        Logger::getInstance().log("Error: " + last_error_);
    }
}

void PluginManager::scanPluginDirectories() {
    for (const auto& directory : plugin_directories_) {
        try {
            for (const auto& entry : std::filesystem::directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    std::string file_path = entry.path().string();
                    std::string extension = entry.path().extension().string();
                    
                    // Look for shared libraries (.so on Linux, .dll on Windows, .dylib on macOS)
                    if (extension == ".so" || extension == ".dll" || extension == ".dylib") {
                        if (validatePlugin(file_path)) {
                            loadPlugin(file_path);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            last_error_ = "Error scanning directory " + directory + ": " + e.what();
            Logger::getInstance().log("Error: " + last_error_);
        }
    }
}

bool PluginManager::loadPlugin(const std::string& plugin_path) {
    try {
        void* handle = loadLibrary(plugin_path);
        if (!handle) {
            last_error_ = "Failed to load library: " + plugin_path;
            return false;
        }
        
        // Get plugin entry points
        auto create_func = reinterpret_cast<semipro_create_plugin_t>(
            getSymbol(handle, "semipro_create_plugin"));
        auto get_name_func = reinterpret_cast<semipro_get_plugin_name_t>(
            getSymbol(handle, "semipro_get_plugin_name"));
        auto get_version_func = reinterpret_cast<semipro_get_plugin_version_t>(
            getSymbol(handle, "semipro_get_plugin_version"));
        auto get_description_func = reinterpret_cast<semipro_get_plugin_description_t>(
            getSymbol(handle, "semipro_get_plugin_description"));
        
        if (!create_func || !get_name_func || !get_version_func || !get_description_func) {
            unloadLibrary(handle);
            last_error_ = "Plugin missing required entry points: " + plugin_path;
            return false;
        }
        
        // Create plugin instance
        std::shared_ptr<PluginBase> plugin(create_func());
        if (!plugin) {
            unloadLibrary(handle);
            last_error_ = "Failed to create plugin instance: " + plugin_path;
            return false;
        }
        
        // Initialize plugin
        if (!plugin->initialize()) {
            unloadLibrary(handle);
            last_error_ = "Plugin initialization failed: " + plugin_path;
            return false;
        }
        
        // Store plugin info
        PluginInfo info;
        info.name = get_name_func();
        info.version = get_version_func();
        info.description = get_description_func();
        info.file_path = plugin_path;
        info.handle = handle;
        info.instance = plugin;
        
        loaded_plugins_[info.name] = info;
        
        Logger::getInstance().log("Loaded plugin: " + info.name + " v" + info.version);
        return true;
        
    } catch (const std::exception& e) {
        last_error_ = "Exception loading plugin " + plugin_path + ": " + e.what();
        Logger::getInstance().log("Error: " + last_error_);
        return false;
    }
}

bool PluginManager::unloadPlugin(const std::string& plugin_name) {
    auto it = loaded_plugins_.find(plugin_name);
    if (it == loaded_plugins_.end()) {
        last_error_ = "Plugin not found: " + plugin_name;
        return false;
    }
    
    try {
        // Cleanup plugin
        it->second.instance->cleanup();
        it->second.instance.reset();
        
        // Unload library
        unloadLibrary(it->second.handle);
        
        loaded_plugins_.erase(it);
        
        Logger::getInstance().log("Unloaded plugin: " + plugin_name);
        return true;
        
    } catch (const std::exception& e) {
        last_error_ = "Exception unloading plugin " + plugin_name + ": " + e.what();
        Logger::getInstance().log("Error: " + last_error_);
        return false;
    }
}

void PluginManager::unloadAllPlugins() {
    std::vector<std::string> plugin_names;
    for (const auto& [name, info] : loaded_plugins_) {
        plugin_names.push_back(name);
    }
    
    for (const auto& name : plugin_names) {
        unloadPlugin(name);
    }
}

std::shared_ptr<PluginBase> PluginManager::getPlugin(const std::string& name) {
    auto it = loaded_plugins_.find(name);
    return (it != loaded_plugins_.end()) ? it->second.instance : nullptr;
}

std::shared_ptr<ProcessModulePlugin> PluginManager::getProcessModule(const std::string& name) {
    auto plugin = getPlugin(name);
    return std::dynamic_pointer_cast<ProcessModulePlugin>(plugin);
}

std::shared_ptr<AnalysisPlugin> PluginManager::getAnalysisPlugin(const std::string& name) {
    auto plugin = getPlugin(name);
    return std::dynamic_pointer_cast<AnalysisPlugin>(plugin);
}

std::vector<std::string> PluginManager::getLoadedPluginNames() const {
    std::vector<std::string> names;
    for (const auto& [name, info] : loaded_plugins_) {
        names.push_back(name);
    }
    return names;
}

std::vector<PluginInfo> PluginManager::getLoadedPlugins() const {
    std::vector<PluginInfo> plugins;
    for (const auto& [name, info] : loaded_plugins_) {
        plugins.push_back(info);
    }
    return plugins;
}

bool PluginManager::isPluginLoaded(const std::string& name) const {
    return loaded_plugins_.find(name) != loaded_plugins_.end();
}

bool PluginManager::validatePlugin(const std::string& plugin_path) {
    void* handle = loadLibrary(plugin_path);
    if (!handle) {
        return false;
    }
    
    // Check for required entry points
    bool valid = getSymbol(handle, "semipro_create_plugin") != nullptr &&
                getSymbol(handle, "semipro_destroy_plugin") != nullptr &&
                getSymbol(handle, "semipro_get_plugin_name") != nullptr &&
                getSymbol(handle, "semipro_get_plugin_version") != nullptr &&
                getSymbol(handle, "semipro_get_plugin_description") != nullptr;
    
    unloadLibrary(handle);
    return valid;
}

std::string PluginManager::getPluginError() const {
    return last_error_;
}

void* PluginManager::loadLibrary(const std::string& path) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        last_error_ = dlerror();
    }
    return handle;
}

void PluginManager::unloadLibrary(void* handle) {
    if (handle) {
        dlclose(handle);
    }
}

void* PluginManager::getSymbol(void* handle, const std::string& symbol_name) {
    if (!handle) return nullptr;
    
    dlerror(); // Clear any existing error
    void* symbol = dlsym(handle, symbol_name.c_str());
    
    char* error = dlerror();
    if (error) {
        last_error_ = error;
        return nullptr;
    }
    
    return symbol;
}

// PluginRegistry Implementation
std::shared_ptr<PluginBase> PluginRegistry::createPlugin(const std::string& name) {
    auto it = registry_.find(name);
    return (it != registry_.end()) ? it->second() : nullptr;
}

std::vector<std::string> PluginRegistry::getRegisteredPlugins() {
    std::vector<std::string> names;
    for (const auto& [name, factory] : registry_) {
        names.push_back(name);
    }
    return names;
}

bool PluginRegistry::isRegistered(const std::string& name) {
    return registry_.find(name) != registry_.end();
}

// PluginConfig Implementation
void PluginConfig::loadConfig(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        Logger::getInstance().log("Warning: Could not open plugin config file: " + config_file);
        return;
    }
    
    // Simple key=value format for now
    std::string line;
    std::string current_plugin;
    
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            current_plugin = line.substr(1, line.length() - 2);
        } else if (!current_plugin.empty()) {
            size_t eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = line.substr(0, eq_pos);
                std::string value = line.substr(eq_pos + 1);
                plugin_configs_[current_plugin][key] = value;
            }
        }
    }
}

void PluginConfig::saveConfig(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        Logger::getInstance().log("Error: Could not save plugin config file: " + config_file);
        return;
    }
    
    for (const auto& [plugin_name, config] : plugin_configs_) {
        file << "[" << plugin_name << "]\n";
        for (const auto& [key, value] : config) {
            file << key << "=" << value << "\n";
        }
        file << "\n";
    }
}

void PluginConfig::setPluginConfig(const std::string& plugin_name, 
                                  const std::unordered_map<std::string, std::string>& config) {
    plugin_configs_[plugin_name] = config;
}

std::unordered_map<std::string, std::string> PluginConfig::getPluginConfig(const std::string& plugin_name) {
    auto it = plugin_configs_.find(plugin_name);
    return (it != plugin_configs_.end()) ? it->second : std::unordered_map<std::string, std::string>();
}

void PluginConfig::setPluginParameter(const std::string& plugin_name, 
                                     const std::string& key, const std::string& value) {
    plugin_configs_[plugin_name][key] = value;
}

std::string PluginConfig::getPluginParameter(const std::string& plugin_name, 
                                           const std::string& key, const std::string& default_value) {
    auto plugin_it = plugin_configs_.find(plugin_name);
    if (plugin_it == plugin_configs_.end()) {
        return default_value;
    }
    
    auto param_it = plugin_it->second.find(key);
    return (param_it != plugin_it->second.end()) ? param_it->second : default_value;
}

} // namespace SemiPRO

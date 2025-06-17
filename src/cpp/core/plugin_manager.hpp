// Author: Dr. Mazharuddin Mohammed
#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <dlfcn.h>

namespace SemiPRO {

/**
 * @brief Base class for all plugins
 */
class PluginBase {
public:
    virtual ~PluginBase() = default;
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
};

/**
 * @brief Base class for process module plugins
 */
class ProcessModulePlugin : public PluginBase {
public:
    virtual void execute(class Wafer& wafer) = 0;
    virtual void setParameters(const std::unordered_map<std::string, double>& params) = 0;
    virtual std::unordered_map<std::string, double> getResults() const = 0;
};

/**
 * @brief Base class for analysis plugins
 */
class AnalysisPlugin : public PluginBase {
public:
    virtual std::unordered_map<std::string, double> analyze(const class Wafer& wafer) = 0;
    virtual void generateReport(const std::string& output_file) = 0;
};

/**
 * @brief Plugin information structure
 */
struct PluginInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string file_path;
    void* handle;
    std::shared_ptr<PluginBase> instance;
};

/**
 * @brief Plugin manager for dynamic loading and management
 */
class PluginManager {
private:
    std::unordered_map<std::string, PluginInfo> loaded_plugins_;
    std::vector<std::string> plugin_directories_;
    
    // Function pointer types for plugin entry points
    typedef PluginBase* (*CreatePluginFunc)();
    typedef void (*DestroyPluginFunc)(PluginBase*);
    typedef const char* (*GetPluginNameFunc)();
    typedef const char* (*GetPluginVersionFunc)();
    
public:
    PluginManager();
    ~PluginManager();
    
    // Plugin directory management
    void addPluginDirectory(const std::string& directory);
    void scanPluginDirectories();
    
    // Plugin loading/unloading
    bool loadPlugin(const std::string& plugin_path);
    bool unloadPlugin(const std::string& plugin_name);
    void unloadAllPlugins();
    
    // Plugin access
    std::shared_ptr<PluginBase> getPlugin(const std::string& name);
    std::shared_ptr<ProcessModulePlugin> getProcessModule(const std::string& name);
    std::shared_ptr<AnalysisPlugin> getAnalysisPlugin(const std::string& name);
    
    // Plugin information
    std::vector<std::string> getLoadedPluginNames() const;
    std::vector<PluginInfo> getLoadedPlugins() const;
    bool isPluginLoaded(const std::string& name) const;
    
    // Plugin validation
    bool validatePlugin(const std::string& plugin_path);
    std::string getPluginError() const;
    
private:
    std::string last_error_;
    
    bool loadPluginFromFile(const std::string& file_path);
    void* loadLibrary(const std::string& path);
    void unloadLibrary(void* handle);
    void* getSymbol(void* handle, const std::string& symbol_name);
};

/**
 * @brief Plugin registry for automatic plugin discovery
 */
class PluginRegistry {
private:
    static std::unordered_map<std::string, std::function<std::shared_ptr<PluginBase>()>> registry_;
    
public:
    template<typename T>
    static void registerPlugin(const std::string& name) {
        registry_[name] = []() -> std::shared_ptr<PluginBase> {
            return std::make_shared<T>();
        };
    }
    
    static std::shared_ptr<PluginBase> createPlugin(const std::string& name);
    static std::vector<std::string> getRegisteredPlugins();
    static bool isRegistered(const std::string& name);
};

/**
 * @brief Macro for easy plugin registration
 */
#define REGISTER_PLUGIN(ClassName, PluginName) \
    namespace { \
        struct ClassName##Registrar { \
            ClassName##Registrar() { \
                PluginRegistry::registerPlugin<ClassName>(PluginName); \
            } \
        }; \
        static ClassName##Registrar g_##ClassName##Registrar; \
    }

/**
 * @brief Plugin configuration manager
 */
class PluginConfig {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> plugin_configs_;
    
public:
    void loadConfig(const std::string& config_file);
    void saveConfig(const std::string& config_file);
    
    void setPluginConfig(const std::string& plugin_name, 
                        const std::unordered_map<std::string, std::string>& config);
    std::unordered_map<std::string, std::string> getPluginConfig(const std::string& plugin_name);
    
    void setPluginParameter(const std::string& plugin_name, 
                           const std::string& key, const std::string& value);
    std::string getPluginParameter(const std::string& plugin_name, 
                                  const std::string& key, const std::string& default_value = "");
};

} // namespace SemiPRO

// C-style plugin interface for external plugins
extern "C" {
    typedef SemiPRO::PluginBase* (*semipro_create_plugin_t)();
    typedef void (*semipro_destroy_plugin_t)(SemiPRO::PluginBase*);
    typedef const char* (*semipro_get_plugin_name_t)();
    typedef const char* (*semipro_get_plugin_version_t)();
    typedef const char* (*semipro_get_plugin_description_t)();
}

// Macros for plugin developers
#define SEMIPRO_PLUGIN_EXPORT extern "C"

#define SEMIPRO_PLUGIN_ENTRY_POINTS(PluginClass) \
    SEMIPRO_PLUGIN_EXPORT SemiPRO::PluginBase* semipro_create_plugin() { \
        return new PluginClass(); \
    } \
    SEMIPRO_PLUGIN_EXPORT void semipro_destroy_plugin(SemiPRO::PluginBase* plugin) { \
        delete plugin; \
    } \
    SEMIPRO_PLUGIN_EXPORT const char* semipro_get_plugin_name() { \
        static PluginClass instance; \
        return instance.getName().c_str(); \
    } \
    SEMIPRO_PLUGIN_EXPORT const char* semipro_get_plugin_version() { \
        static PluginClass instance; \
        return instance.getVersion().c_str(); \
    } \
    SEMIPRO_PLUGIN_EXPORT const char* semipro_get_plugin_description() { \
        static PluginClass instance; \
        return instance.getDescription().c_str(); \
    }

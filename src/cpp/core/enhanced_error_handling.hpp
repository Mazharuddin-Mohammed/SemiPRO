#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <exception>
#include <mutex>
#include <atomic>

namespace SemiPRO {

/**
 * Enhanced Error Handling System for SemiPRO
 * Provides comprehensive error management, exception safety, and recovery mechanisms
 */

// Error severity levels
enum class ErrorSeverity {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    CRITICAL = 5,
    FATAL = 6
};

// Error categories for better classification
enum class ErrorCategory {
    SYSTEM,           // System-level errors (memory, file I/O)
    PHYSICS,          // Physics calculation errors
    CONFIGURATION,    // Configuration and parameter errors
    SIMULATION,       // Simulation execution errors
    VALIDATION,       // Data validation errors
    PERFORMANCE,      // Performance-related issues
    NETWORK,          // Network and communication errors
    USER_INPUT        // User input validation errors
};

// Error context information
struct ErrorContext {
    std::string function_name;
    std::string file_name;
    int line_number;
    std::string module_name;
    std::unordered_map<std::string, std::string> additional_data;
    
    ErrorContext(const std::string& func = "", const std::string& file = "", 
                int line = 0, const std::string& module = "")
        : function_name(func), file_name(file), line_number(line), module_name(module) {}
};

// Enhanced error information
class SimulationError {
public:
    ErrorSeverity severity;
    ErrorCategory category;
    std::string message;
    std::string error_code;
    std::chrono::system_clock::time_point timestamp;
    ErrorContext context;
    std::string stack_trace;
    std::vector<std::string> suggested_actions;
    
    SimulationError(ErrorSeverity sev, ErrorCategory cat, const std::string& msg,
                   const std::string& code = "", const ErrorContext& ctx = ErrorContext())
        : severity(sev), category(cat), message(msg), error_code(code),
          timestamp(std::chrono::system_clock::now()), context(ctx) {}
    
    // Utility methods
    std::string getSeverityString() const;
    std::string getCategoryString() const;
    std::string getFormattedMessage() const;
    bool isCritical() const { return severity >= ErrorSeverity::CRITICAL; }
    bool isRecoverable() const { return severity < ErrorSeverity::FATAL; }
};

// Exception classes with enhanced information
class SemiPROException : public std::exception {
protected:
    SimulationError error_;
    
public:
    explicit SemiPROException(const SimulationError& error) : error_(error) {}
    
    const char* what() const noexcept override {
        return error_.message.c_str();
    }
    
    const SimulationError& getError() const { return error_; }
    ErrorSeverity getSeverity() const { return error_.severity; }
    ErrorCategory getCategory() const { return error_.category; }
};

// Specific exception types
class PhysicsException : public SemiPROException {
public:
    PhysicsException(const std::string& msg, const ErrorContext& ctx = ErrorContext())
        : SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::PHYSICS, msg, "PHYS_ERR", ctx)) {}
};

class ConfigurationException : public SemiPROException {
public:
    ConfigurationException(const std::string& msg, const ErrorContext& ctx = ErrorContext())
        : SemiPROException(SimulationError(ErrorSeverity::ERROR, ErrorCategory::CONFIGURATION, msg, "CFG_ERR", ctx)) {}
};

class ValidationException : public SemiPROException {
public:
    ValidationException(const std::string& msg, const ErrorContext& ctx = ErrorContext())
        : SemiPROException(SimulationError(ErrorSeverity::WARNING, ErrorCategory::VALIDATION, msg, "VAL_ERR", ctx)) {}
};

class SystemException : public SemiPROException {
public:
    SystemException(const std::string& msg, const ErrorContext& ctx = ErrorContext())
        : SemiPROException(SimulationError(ErrorSeverity::CRITICAL, ErrorCategory::SYSTEM, msg, "SYS_ERR", ctx)) {}
};

// Error handler callback type
using ErrorHandler = std::function<void(const SimulationError&)>;

// Recovery strategy callback type
using RecoveryStrategy = std::function<bool(const SimulationError&)>;

/**
 * Enhanced Error Manager
 * Centralized error handling, logging, and recovery system
 */
class ErrorManager {
private:
    static std::unique_ptr<ErrorManager> instance_;
    static std::mutex instance_mutex_;
    
    std::vector<SimulationError> error_history_;
    std::vector<ErrorHandler> error_handlers_;
    std::unordered_map<ErrorCategory, RecoveryStrategy> recovery_strategies_;
    
    mutable std::mutex error_mutex_;
    std::atomic<size_t> error_count_{0};
    std::atomic<size_t> critical_error_count_{0};
    
    ErrorSeverity min_log_level_ = ErrorSeverity::INFO;
    size_t max_error_history_ = 1000;
    bool auto_recovery_enabled_ = true;
    
    ErrorManager() = default;
    
public:
    static ErrorManager& getInstance();
    
    // Error reporting
    void reportError(const SimulationError& error);
    void reportError(ErrorSeverity severity, ErrorCategory category, 
                    const std::string& message, const ErrorContext& context = ErrorContext());
    
    // Error handlers
    void addErrorHandler(const ErrorHandler& handler);
    void removeAllErrorHandlers();
    
    // Recovery strategies
    void setRecoveryStrategy(ErrorCategory category, const RecoveryStrategy& strategy);
    bool attemptRecovery(const SimulationError& error);
    
    // Error retrieval and analysis
    std::vector<SimulationError> getErrors(ErrorSeverity min_severity = ErrorSeverity::INFO) const;
    std::vector<SimulationError> getErrorsByCategory(ErrorCategory category) const;
    size_t getErrorCount() const { return error_count_.load(); }
    size_t getCriticalErrorCount() const { return critical_error_count_.load(); }
    
    // Configuration
    void setMinLogLevel(ErrorSeverity level) { min_log_level_ = level; }
    void setMaxErrorHistory(size_t max_size) { max_error_history_ = max_size; }
    void setAutoRecoveryEnabled(bool enabled) { auto_recovery_enabled_ = enabled; }
    
    // Cleanup
    void clearErrors();
    void clearOldErrors(std::chrono::hours max_age = std::chrono::hours(24));
    
    // Statistics
    std::unordered_map<ErrorCategory, size_t> getErrorStatistics() const;
    std::unordered_map<ErrorSeverity, size_t> getSeverityStatistics() const;
};

// Utility macros for error reporting with context
#define SEMIPRO_ERROR_CONTEXT() \
    ErrorContext(__FUNCTION__, __FILE__, __LINE__, "")

#define SEMIPRO_MODULE_ERROR_CONTEXT(module) \
    ErrorContext(__FUNCTION__, __FILE__, __LINE__, module)

#define REPORT_ERROR(severity, category, message) \
    ErrorManager::getInstance().reportError(severity, category, message, SEMIPRO_ERROR_CONTEXT())

#define REPORT_MODULE_ERROR(severity, category, message, module) \
    ErrorManager::getInstance().reportError(severity, category, message, SEMIPRO_MODULE_ERROR_CONTEXT(module))

// Exception safety utilities
template<typename T>
class ExceptionSafeWrapper {
private:
    T value_;
    bool valid_;
    SimulationError last_error_;
    
public:
    ExceptionSafeWrapper() : valid_(false) {}
    ExceptionSafeWrapper(const T& value) : value_(value), valid_(true) {}
    
    bool isValid() const { return valid_; }
    const T& getValue() const { 
        if (!valid_) {
            throw std::runtime_error("Attempting to access invalid value");
        }
        return value_; 
    }
    
    const SimulationError& getLastError() const { return last_error_; }
    
    void setError(const SimulationError& error) {
        valid_ = false;
        last_error_ = error;
    }
    
    void setValue(const T& value) {
        value_ = value;
        valid_ = true;
    }
};

// RAII resource management
template<typename Resource, typename Deleter>
class ResourceGuard {
private:
    Resource resource_;
    Deleter deleter_;
    bool released_;
    
public:
    ResourceGuard(Resource resource, Deleter deleter)
        : resource_(resource), deleter_(deleter), released_(false) {}
    
    ~ResourceGuard() {
        if (!released_) {
            try {
                deleter_(resource_);
            } catch (...) {
                // Log error but don't throw in destructor
                REPORT_ERROR(ErrorSeverity::WARNING, ErrorCategory::SYSTEM, 
                           "Exception in resource cleanup");
            }
        }
    }
    
    Resource get() const { return resource_; }
    
    void release() { released_ = true; }
    
    // Non-copyable, movable
    ResourceGuard(const ResourceGuard&) = delete;
    ResourceGuard& operator=(const ResourceGuard&) = delete;
    ResourceGuard(ResourceGuard&& other) noexcept 
        : resource_(other.resource_), deleter_(std::move(other.deleter_)), released_(other.released_) {
        other.released_ = true;
    }
};

// Factory function for resource guards
template<typename Resource, typename Deleter>
auto makeResourceGuard(Resource resource, Deleter deleter) {
    return ResourceGuard<Resource, Deleter>(resource, deleter);
}

} // namespace SemiPRO

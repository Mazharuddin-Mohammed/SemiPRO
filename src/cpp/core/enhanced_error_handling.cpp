#include "enhanced_error_handling.hpp"
#include "advanced_logger.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iomanip>

namespace SemiPRO {

// Static member definitions
std::unique_ptr<ErrorManager> ErrorManager::instance_ = nullptr;
std::mutex ErrorManager::instance_mutex_;

// SimulationError methods
std::string SimulationError::getSeverityString() const {
    switch (severity) {
        case ErrorSeverity::TRACE: return "TRACE";
        case ErrorSeverity::DEBUG: return "DEBUG";
        case ErrorSeverity::INFO: return "INFO";
        case ErrorSeverity::WARNING: return "WARNING";
        case ErrorSeverity::ERROR: return "ERROR";
        case ErrorSeverity::CRITICAL: return "CRITICAL";
        case ErrorSeverity::FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

std::string SimulationError::getCategoryString() const {
    switch (category) {
        case ErrorCategory::SYSTEM: return "SYSTEM";
        case ErrorCategory::PHYSICS: return "PHYSICS";
        case ErrorCategory::CONFIGURATION: return "CONFIG";
        case ErrorCategory::SIMULATION: return "SIMULATION";
        case ErrorCategory::VALIDATION: return "VALIDATION";
        case ErrorCategory::PERFORMANCE: return "PERFORMANCE";
        case ErrorCategory::NETWORK: return "NETWORK";
        case ErrorCategory::USER_INPUT: return "USER_INPUT";
        default: return "UNKNOWN";
    }
}

std::string SimulationError::getFormattedMessage() const {
    std::ostringstream oss;
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
    
    // Severity and category
    oss << "[" << getSeverityString() << "|" << getCategoryString() << "] ";
    
    // Error code if available
    if (!error_code.empty()) {
        oss << "[" << error_code << "] ";
    }
    
    // Main message
    oss << message;
    
    // Context information
    if (!context.function_name.empty()) {
        oss << " (in " << context.function_name;
        if (!context.file_name.empty()) {
            // Extract just the filename from full path
            size_t pos = context.file_name.find_last_of("/\\");
            std::string filename = (pos != std::string::npos) ? 
                                 context.file_name.substr(pos + 1) : context.file_name;
            oss << " at " << filename << ":" << context.line_number;
        }
        oss << ")";
    }
    
    // Module information
    if (!context.module_name.empty()) {
        oss << " [Module: " << context.module_name << "]";
    }
    
    // Suggested actions
    if (!suggested_actions.empty()) {
        oss << "\n  Suggested actions:";
        for (const auto& action : suggested_actions) {
            oss << "\n    - " << action;
        }
    }
    
    return oss.str();
}

// ErrorManager implementation
ErrorManager& ErrorManager::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<ErrorManager>(new ErrorManager());
    }
    return *instance_;
}

void ErrorManager::reportError(const SimulationError& error) {
    // Check if error meets minimum log level
    if (error.severity < min_log_level_) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(error_mutex_);
        
        // Add to error history
        error_history_.push_back(error);
        
        // Maintain maximum history size
        if (error_history_.size() > max_error_history_) {
            error_history_.erase(error_history_.begin());
        }
        
        // Update counters
        error_count_++;
        if (error.isCritical()) {
            critical_error_count_++;
        }
    }
    
    // Log to advanced logger
    try {
        AdvancedLogger::getInstance().log(
            errorSeverityToLogLevel(error.severity),
            errorCategoryToLogCategory(error.category),
            error.message,
            error.context.function_name,
            error.context.file_name,
            error.context.line_number,
            error.context.module_name
        );
    } catch (...) {
        // Fallback to stderr if logger fails
        std::cerr << "Logger failed: " << error.getFormattedMessage() << std::endl;
    }

    // Call error handlers
    for (const auto& handler : error_handlers_) {
        try {
            handler(error);
        } catch (const std::exception& e) {
            // Prevent infinite recursion by not reporting handler errors
            std::cerr << "Error in error handler: " << e.what() << std::endl;
        }
    }
    
    // Attempt automatic recovery if enabled
    if (auto_recovery_enabled_ && error.isRecoverable()) {
        try {
            attemptRecovery(error);
        } catch (const std::exception& e) {
            // Log recovery failure but don't throw
            std::cerr << "Recovery attempt failed: " << e.what() << std::endl;
        }
    }
    
    // For critical errors, also output to stderr
    if (error.isCritical()) {
        std::cerr << "CRITICAL ERROR: " << error.getFormattedMessage() << std::endl;
    }
}

void ErrorManager::reportError(ErrorSeverity severity, ErrorCategory category, 
                              const std::string& message, const ErrorContext& context) {
    SimulationError error(severity, category, message, "", context);
    
    // Add suggested actions based on category and severity
    switch (category) {
        case ErrorCategory::CONFIGURATION:
            error.suggested_actions.push_back("Check configuration file syntax and parameter values");
            error.suggested_actions.push_back("Verify all required parameters are specified");
            break;
        case ErrorCategory::PHYSICS:
            error.suggested_actions.push_back("Verify input parameters are within physical limits");
            error.suggested_actions.push_back("Check for numerical instabilities or convergence issues");
            break;
        case ErrorCategory::SYSTEM:
            error.suggested_actions.push_back("Check available system resources (memory, disk space)");
            error.suggested_actions.push_back("Verify file permissions and accessibility");
            break;
        case ErrorCategory::VALIDATION:
            error.suggested_actions.push_back("Review input data for correctness and completeness");
            error.suggested_actions.push_back("Check data ranges and units");
            break;
        default:
            break;
    }
    
    reportError(error);
}

void ErrorManager::addErrorHandler(const ErrorHandler& handler) {
    std::lock_guard<std::mutex> lock(error_mutex_);
    error_handlers_.push_back(handler);
}

void ErrorManager::removeAllErrorHandlers() {
    std::lock_guard<std::mutex> lock(error_mutex_);
    error_handlers_.clear();
}

void ErrorManager::setRecoveryStrategy(ErrorCategory category, const RecoveryStrategy& strategy) {
    std::lock_guard<std::mutex> lock(error_mutex_);
    recovery_strategies_[category] = strategy;
}

bool ErrorManager::attemptRecovery(const SimulationError& error) {
    auto it = recovery_strategies_.find(error.category);
    if (it != recovery_strategies_.end()) {
        try {
            return it->second(error);
        } catch (const std::exception& e) {
            std::cerr << "Recovery strategy failed: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

std::vector<SimulationError> ErrorManager::getErrors(ErrorSeverity min_severity) const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    std::vector<SimulationError> filtered_errors;
    
    std::copy_if(error_history_.begin(), error_history_.end(), 
                std::back_inserter(filtered_errors),
                [min_severity](const SimulationError& error) {
                    return error.severity >= min_severity;
                });
    
    return filtered_errors;
}

std::vector<SimulationError> ErrorManager::getErrorsByCategory(ErrorCategory category) const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    std::vector<SimulationError> filtered_errors;
    
    std::copy_if(error_history_.begin(), error_history_.end(), 
                std::back_inserter(filtered_errors),
                [category](const SimulationError& error) {
                    return error.category == category;
                });
    
    return filtered_errors;
}

void ErrorManager::clearErrors() {
    std::lock_guard<std::mutex> lock(error_mutex_);
    error_history_.clear();
    error_count_ = 0;
    critical_error_count_ = 0;
}

void ErrorManager::clearOldErrors(std::chrono::hours max_age) {
    std::lock_guard<std::mutex> lock(error_mutex_);
    auto cutoff_time = std::chrono::system_clock::now() - max_age;
    
    auto new_end = std::remove_if(error_history_.begin(), error_history_.end(),
                                 [cutoff_time](const SimulationError& error) {
                                     return error.timestamp < cutoff_time;
                                 });
    
    error_history_.erase(new_end, error_history_.end());
}

std::unordered_map<ErrorCategory, size_t> ErrorManager::getErrorStatistics() const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    std::unordered_map<ErrorCategory, size_t> stats;
    
    for (const auto& error : error_history_) {
        stats[error.category]++;
    }
    
    return stats;
}

std::unordered_map<ErrorSeverity, size_t> ErrorManager::getSeverityStatistics() const {
    std::lock_guard<std::mutex> lock(error_mutex_);
    std::unordered_map<ErrorSeverity, size_t> stats;
    
    for (const auto& error : error_history_) {
        stats[error.severity]++;
    }
    
    return stats;
}

} // namespace SemiPRO

#include "advanced_logger.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <filesystem>

namespace SemiPRO {

// Static member definitions
std::unique_ptr<AdvancedLogger> AdvancedLogger::instance_ = nullptr;
std::mutex AdvancedLogger::instance_mutex_;

// LogEntry methods
std::string LogEntry::getFormattedMessage() const {
    std::ostringstream oss;
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()) % 1000;
    
    oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(3) << ms.count() << "] ";
    
    // Level
    const char* level_str[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "CRIT", "FATAL"};
    oss << "[" << level_str[static_cast<int>(level)] << "] ";
    
    // Category
    const char* cat_str[] = {"GEN", "PHYS", "PERF", "SYS", "SIM", "VAL", "NET", "UI", "MEM", "THR"};
    oss << "[" << cat_str[static_cast<int>(category)] << "] ";
    
    // Thread ID (shortened)
    oss << "[T:" << thread_id.substr(0, 6) << "] ";
    
    // Module
    if (!module_name.empty()) {
        oss << "[" << module_name << "] ";
    }
    
    // Message
    oss << message;
    
    // Function and location
    if (!function_name.empty()) {
        oss << " (in " << function_name;
        if (!file_name.empty() && line_number > 0) {
            // Extract just filename
            size_t pos = file_name.find_last_of("/\\");
            std::string filename = (pos != std::string::npos) ? 
                                 file_name.substr(pos + 1) : file_name;
            oss << " at " << filename << ":" << line_number;
        }
        oss << ")";
    }
    
    // Performance data
    if (performance_data) {
        oss << " [Duration: " << std::fixed << std::setprecision(3) 
            << performance_data->getDurationMs() << "ms";
        if (performance_data->memory_used > 0) {
            oss << ", Memory: " << performance_data->memory_used << " bytes";
        }
        oss << "]";
    }
    
    // Metadata
    if (!metadata.empty()) {
        oss << " {";
        bool first = true;
        for (const auto& [key, value] : metadata) {
            if (!first) oss << ", ";
            oss << key << "=" << value;
            first = false;
        }
        oss << "}";
    }
    
    return oss.str();
}

std::string LogEntry::getJSONFormat() const {
    std::ostringstream oss;
    oss << "{\n";
    oss << "  \"timestamp\": \"" << std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count() << "\",\n";
    oss << "  \"level\": \"" << static_cast<int>(level) << "\",\n";
    oss << "  \"category\": \"" << static_cast<int>(category) << "\",\n";
    oss << "  \"thread_id\": \"" << thread_id << "\",\n";
    oss << "  \"module\": \"" << module_name << "\",\n";
    oss << "  \"function\": \"" << function_name << "\",\n";
    oss << "  \"file\": \"" << file_name << "\",\n";
    oss << "  \"line\": " << line_number << ",\n";
    oss << "  \"message\": \"" << message << "\"";
    
    if (performance_data) {
        oss << ",\n  \"performance\": {\n";
        oss << "    \"duration_ms\": " << performance_data->getDurationMs() << ",\n";
        oss << "    \"memory_used\": " << performance_data->memory_used << ",\n";
        oss << "    \"operation\": \"" << performance_data->operation_name << "\"\n";
        oss << "  }";
    }
    
    if (!metadata.empty()) {
        oss << ",\n  \"metadata\": {\n";
        bool first = true;
        for (const auto& [key, value] : metadata) {
            if (!first) oss << ",\n";
            oss << "    \"" << key << "\": \"" << value << "\"";
            first = false;
        }
        oss << "\n  }";
    }
    
    oss << "\n}";
    return oss.str();
}

// ConsoleOutput implementation
void ConsoleOutput::write(const LogEntry& entry) {
    if (entry.level < min_level_) return;
    
    std::string output;
    if (colored_output_) {
        output = getColorCode(entry.level) + entry.getFormattedMessage() + getResetCode();
    } else {
        output = entry.getFormattedMessage();
    }
    
    if (entry.level >= LogLevel::ERROR) {
        std::cerr << output << std::endl;
    } else {
        std::cout << output << std::endl;
    }
}

std::string ConsoleOutput::getColorCode(LogLevel level) const {
    switch (level) {
        case LogLevel::TRACE: return "\033[37m";    // White
        case LogLevel::DEBUG: return "\033[36m";    // Cyan
        case LogLevel::INFO: return "\033[32m";     // Green
        case LogLevel::WARNING: return "\033[33m";  // Yellow
        case LogLevel::ERROR: return "\033[31m";    // Red
        case LogLevel::CRITICAL: return "\033[35m"; // Magenta
        case LogLevel::FATAL: return "\033[41m";    // Red background
        default: return "";
    }
}

std::string ConsoleOutput::getResetCode() const {
    return "\033[0m";
}

// FileOutput implementation
void FileOutput::write(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(file_mutex_);
    
    if (!file_.is_open()) return;
    
    std::string formatted = entry.getFormattedMessage();
    file_ << formatted << std::endl;
    current_size_ += formatted.length() + 1;
    
    if (current_size_ > max_file_size_) {
        rotateFile();
    }
}

void FileOutput::flush() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

bool FileOutput::isHealthy() const {
    std::lock_guard<std::mutex> lock(file_mutex_);
    return file_.is_open() && file_.good();
}

void FileOutput::rotateFile() {
    file_.close();
    
    // Rotate backup files
    for (int i = max_backup_files_ - 1; i >= 1; --i) {
        std::string old_name = filename_ + "." + std::to_string(i);
        std::string new_name = filename_ + "." + std::to_string(i + 1);
        
        if (std::filesystem::exists(old_name)) {
            if (i == max_backup_files_ - 1) {
                std::filesystem::remove(new_name);
            }
            std::filesystem::rename(old_name, new_name);
        }
    }
    
    // Move current file to .1
    if (std::filesystem::exists(filename_)) {
        std::filesystem::rename(filename_, filename_ + ".1");
    }
    
    // Open new file
    file_.open(filename_);
    current_size_ = 0;
}

// JSONOutput implementation
void JSONOutput::write(const LogEntry& entry) {
    std::lock_guard<std::mutex> lock(file_mutex_);
    
    if (!file_.is_open()) return;
    
    if (!first_entry_) {
        file_ << ",\n";
    } else {
        first_entry_ = false;
    }
    
    file_ << entry.getJSONFormat();
}

void JSONOutput::flush() {
    std::lock_guard<std::mutex> lock(file_mutex_);
    if (file_.is_open()) {
        file_.flush();
    }
}

bool JSONOutput::isHealthy() const {
    std::lock_guard<std::mutex> lock(file_mutex_);
    return file_.is_open() && file_.good();
}

// PerformanceMonitor implementation
void PerformanceMonitor::recordMetrics(const PerformanceMetrics& metrics) {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    metrics_history_.push_back(metrics);
    
    if (metrics_history_.size() > max_history_size_) {
        metrics_history_.erase(metrics_history_.begin());
    }
}

std::vector<PerformanceMetrics> PerformanceMonitor::getMetrics(const std::string& operation_name) const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    
    if (operation_name.empty()) {
        return metrics_history_;
    }
    
    std::vector<PerformanceMetrics> filtered;
    std::copy_if(metrics_history_.begin(), metrics_history_.end(),
                std::back_inserter(filtered),
                [&operation_name](const PerformanceMetrics& m) {
                    return m.operation_name == operation_name;
                });
    
    return filtered;
}

double PerformanceMonitor::getAverageExecutionTime(const std::string& operation_name) const {
    auto metrics = getMetrics(operation_name);
    if (metrics.empty()) return 0.0;
    
    double total = 0.0;
    for (const auto& m : metrics) {
        total += m.getDurationMs();
    }
    
    return total / metrics.size();
}

// AdvancedLogger implementation
AdvancedLogger::AdvancedLogger() 
    : min_level_(LogLevel::INFO),
      performance_monitor_(std::make_unique<PerformanceMonitor>()),
      worker_thread_(&AdvancedLogger::workerThreadFunction, this) {
}

AdvancedLogger& AdvancedLogger::getInstance() {
    std::lock_guard<std::mutex> lock(instance_mutex_);
    if (!instance_) {
        instance_ = std::unique_ptr<AdvancedLogger>(new AdvancedLogger());
    }
    return *instance_;
}

AdvancedLogger::~AdvancedLogger() {
    shutdown();
}

void AdvancedLogger::shutdown() {
    shutdown_requested_ = true;
    queue_cv_.notify_all();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void AdvancedLogger::workerThreadFunction() {
    while (!shutdown_requested_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !log_queue_.empty() || shutdown_requested_; });
        
        while (!log_queue_.empty()) {
            LogEntry entry = log_queue_.front();
            log_queue_.pop();
            lock.unlock();
            
            // Write to all outputs
            for (auto& output : outputs_) {
                if (output && output->isHealthy()) {
                    output->write(entry);
                }
            }
            
            // Update statistics
            total_log_entries_++;
            if (entry.level >= LogLevel::ERROR) {
                error_count_++;
            } else if (entry.level == LogLevel::WARNING) {
                warning_count_++;
            }
            
            lock.lock();
        }
    }
}

void AdvancedLogger::addOutput(std::unique_ptr<LogOutput> output) {
    if (output) {
        outputs_.push_back(std::move(output));
    }
}

void AdvancedLogger::removeAllOutputs() {
    outputs_.clear();
}

void AdvancedLogger::log(LogLevel level, LogCategory category, const std::string& message,
                        const std::string& function, const std::string& file, int line,
                        const std::string& module) {
    if (level < min_level_) return;

    LogEntry entry(level, category, message);
    entry.function_name = function;
    entry.file_name = file;
    entry.line_number = line;
    entry.module_name = module;

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        log_queue_.push(entry);
    }
    queue_cv_.notify_one();
}

void AdvancedLogger::flush() {
    for (auto& output : outputs_) {
        if (output) {
            output->flush();
        }
    }
}

std::unique_ptr<PerformanceTimer> AdvancedLogger::createTimer(const std::string& operation, const std::string& module) {
    return std::make_unique<PerformanceTimer>(operation, module, performance_monitor_.get());
}

// Convenience methods
void AdvancedLogger::trace(const std::string& message, const std::string& module) {
    log(LogLevel::TRACE, LogCategory::GENERAL, message, "", "", 0, module);
}

void AdvancedLogger::debug(const std::string& message, const std::string& module) {
    log(LogLevel::DEBUG, LogCategory::GENERAL, message, "", "", 0, module);
}

void AdvancedLogger::info(const std::string& message, const std::string& module) {
    log(LogLevel::INFO, LogCategory::GENERAL, message, "", "", 0, module);
}

void AdvancedLogger::warning(const std::string& message, const std::string& module) {
    log(LogLevel::WARNING, LogCategory::GENERAL, message, "", "", 0, module);
}

void AdvancedLogger::error(const std::string& message, const std::string& module) {
    log(LogLevel::ERROR, LogCategory::GENERAL, message, "", "", 0, module);
}

void AdvancedLogger::critical(const std::string& message, const std::string& module) {
    log(LogLevel::CRITICAL, LogCategory::GENERAL, message, "", "", 0, module);
}

void AdvancedLogger::fatal(const std::string& message, const std::string& module) {
    log(LogLevel::FATAL, LogCategory::GENERAL, message, "", "", 0, module);
}

// Helper functions
LogLevel errorSeverityToLogLevel(ErrorSeverity severity) {
    return static_cast<LogLevel>(static_cast<int>(severity));
}

LogCategory errorCategoryToLogCategory(ErrorCategory category) {
    switch (category) {
        case ErrorCategory::PHYSICS: return LogCategory::PHYSICS;
        case ErrorCategory::SYSTEM: return LogCategory::SYSTEM;
        case ErrorCategory::SIMULATION: return LogCategory::SIMULATION;
        case ErrorCategory::VALIDATION: return LogCategory::VALIDATION;
        case ErrorCategory::PERFORMANCE: return LogCategory::PERFORMANCE;
        case ErrorCategory::NETWORK: return LogCategory::NETWORK;
        case ErrorCategory::USER_INPUT: return LogCategory::USER_INTERFACE;
        default: return LogCategory::GENERAL;
    }
}

} // namespace SemiPRO

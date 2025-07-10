#pragma once

#include "enhanced_error_handling.hpp"
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include <atomic>
#include <queue>
#include <thread>
#include <condition_variable>
#include <sstream>
#include <iomanip>

namespace SemiPRO {

/**
 * Advanced Logging System for SemiPRO
 * Provides structured logging, performance metrics, and real-time monitoring
 */

// Log levels (compatible with ErrorSeverity)
enum class LogLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERROR = 4,
    CRITICAL = 5,
    FATAL = 6
};

// Log categories for better organization
enum class LogCategory {
    GENERAL,
    PHYSICS,
    PERFORMANCE,
    SYSTEM,
    SIMULATION,
    VALIDATION,
    NETWORK,
    USER_INTERFACE,
    MEMORY,
    THREADING,
    ADVANCED
};

// Performance metrics structure
struct PerformanceMetrics {
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    std::string operation_name;
    std::string module_name;
    size_t memory_used = 0;
    size_t cpu_cycles = 0;
    std::unordered_map<std::string, double> custom_metrics;
    
    double getDurationMs() const {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
    
    double getDurationSeconds() const {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        return duration.count() / 1000.0;
    }
};

// Log entry structure
struct LogEntry {
    LogLevel level;
    LogCategory category;
    std::string message;
    std::chrono::system_clock::time_point timestamp;
    std::string thread_id;
    std::string module_name;
    std::string function_name;
    std::string file_name;
    int line_number;
    std::unordered_map<std::string, std::string> metadata;
    std::shared_ptr<PerformanceMetrics> performance_data;
    
    LogEntry(LogLevel lvl, LogCategory cat, const std::string& msg)
        : level(lvl), category(cat), message(msg), 
          timestamp(std::chrono::system_clock::now()),
          line_number(0) {
        // Get thread ID
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        thread_id = oss.str();
    }
    
    std::string getFormattedMessage() const;
    std::string getJSONFormat() const;
    std::string getCSVFormat() const;
};

// Log output interface
class LogOutput {
public:
    virtual ~LogOutput() = default;
    virtual void write(const LogEntry& entry) = 0;
    virtual void flush() = 0;
    virtual bool isHealthy() const = 0;
};

// Console output
class ConsoleOutput : public LogOutput {
private:
    bool colored_output_;
    LogLevel min_level_;
    
public:
    ConsoleOutput(bool colored = true, LogLevel min_level = LogLevel::INFO)
        : colored_output_(colored), min_level_(min_level) {}
    
    void write(const LogEntry& entry) override;
    void flush() override {}
    bool isHealthy() const override { return true; }
    
private:
    std::string getColorCode(LogLevel level) const;
    std::string getResetCode() const;
};

// File output
class FileOutput : public LogOutput {
private:
    std::string filename_;
    std::ofstream file_;
    size_t max_file_size_;
    size_t current_size_;
    int max_backup_files_;
    mutable std::mutex file_mutex_;
    
public:
    FileOutput(const std::string& filename, size_t max_size = 100 * 1024 * 1024, int max_backups = 5)
        : filename_(filename), max_file_size_(max_size), current_size_(0), max_backup_files_(max_backups) {
        file_.open(filename_, std::ios::app);
    }
    
    ~FileOutput() {
        if (file_.is_open()) {
            file_.close();
        }
    }
    
    void write(const LogEntry& entry) override;
    void flush() override;
    bool isHealthy() const override;
    
private:
    void rotateFile();
};

// JSON file output
class JSONOutput : public LogOutput {
private:
    std::string filename_;
    std::ofstream file_;
    bool first_entry_;
    mutable std::mutex file_mutex_;
    
public:
    JSONOutput(const std::string& filename) : filename_(filename), first_entry_(true) {
        file_.open(filename_);
        file_ << "[\n";
    }
    
    ~JSONOutput() {
        if (file_.is_open()) {
            file_ << "\n]";
            file_.close();
        }
    }
    
    void write(const LogEntry& entry) override;
    void flush() override;
    bool isHealthy() const override;
};

// Performance monitor
class PerformanceMonitor {
private:
    std::vector<PerformanceMetrics> metrics_history_;
    mutable std::mutex metrics_mutex_;
    size_t max_history_size_;
    
public:
    PerformanceMonitor(size_t max_history = 10000) : max_history_size_(max_history) {}
    
    void recordMetrics(const PerformanceMetrics& metrics);
    std::vector<PerformanceMetrics> getMetrics(const std::string& operation_name = "") const;
    std::unordered_map<std::string, double> getAverageMetrics() const;
    void clearMetrics();
    
    // Statistics
    double getAverageExecutionTime(const std::string& operation_name) const;
    double getMaxExecutionTime(const std::string& operation_name) const;
    double getMinExecutionTime(const std::string& operation_name) const;
    size_t getOperationCount(const std::string& operation_name) const;
};

// RAII performance timer
class PerformanceTimer {
private:
    std::string operation_name_;
    std::string module_name_;
    std::chrono::high_resolution_clock::time_point start_time_;
    PerformanceMonitor* monitor_;
    
public:
    PerformanceTimer(const std::string& operation, const std::string& module, PerformanceMonitor* monitor)
        : operation_name_(operation), module_name_(module), monitor_(monitor),
          start_time_(std::chrono::high_resolution_clock::now()) {}
    
    ~PerformanceTimer() {
        if (monitor_) {
            PerformanceMetrics metrics;
            metrics.operation_name = operation_name_;
            metrics.module_name = module_name_;
            metrics.start_time = start_time_;
            metrics.end_time = std::chrono::high_resolution_clock::now();
            monitor_->recordMetrics(metrics);
        }
    }
    
    // Non-copyable, movable
    PerformanceTimer(const PerformanceTimer&) = delete;
    PerformanceTimer& operator=(const PerformanceTimer&) = delete;
    PerformanceTimer(PerformanceTimer&&) = default;
    PerformanceTimer& operator=(PerformanceTimer&&) = default;
};

// Advanced Logger class
class AdvancedLogger {
private:
    static std::unique_ptr<AdvancedLogger> instance_;
    static std::mutex instance_mutex_;
    
    std::vector<std::unique_ptr<LogOutput>> outputs_;
    std::queue<LogEntry> log_queue_;
    std::thread worker_thread_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> shutdown_requested_{false};
    
    LogLevel min_level_;
    std::unique_ptr<PerformanceMonitor> performance_monitor_;
    
    // Statistics
    std::atomic<size_t> total_log_entries_{0};
    std::atomic<size_t> error_count_{0};
    std::atomic<size_t> warning_count_{0};
    
    AdvancedLogger();
    void workerThreadFunction();
    
public:
    static AdvancedLogger& getInstance();
    ~AdvancedLogger();
    
    // Configuration
    void setMinLevel(LogLevel level) { min_level_ = level; }
    void addOutput(std::unique_ptr<LogOutput> output);
    void removeAllOutputs();
    
    // Logging methods
    void log(LogLevel level, LogCategory category, const std::string& message,
             const std::string& function = "", const std::string& file = "", int line = 0,
             const std::string& module = "");
    
    void logWithMetadata(LogLevel level, LogCategory category, const std::string& message,
                        const std::unordered_map<std::string, std::string>& metadata,
                        const std::string& function = "", const std::string& file = "", int line = 0,
                        const std::string& module = "");
    
    void logPerformance(const PerformanceMetrics& metrics);
    
    // Convenience methods
    void trace(const std::string& message, const std::string& module = "");
    void debug(const std::string& message, const std::string& module = "");
    void info(const std::string& message, const std::string& module = "");
    void warning(const std::string& message, const std::string& module = "");
    void error(const std::string& message, const std::string& module = "");
    void critical(const std::string& message, const std::string& module = "");
    void fatal(const std::string& message, const std::string& module = "");
    
    // Performance monitoring
    PerformanceMonitor& getPerformanceMonitor() { return *performance_monitor_; }
    std::unique_ptr<PerformanceTimer> createTimer(const std::string& operation, const std::string& module);
    
    // Statistics
    size_t getTotalLogEntries() const { return total_log_entries_.load(); }
    size_t getErrorCount() const { return error_count_.load(); }
    size_t getWarningCount() const { return warning_count_.load(); }
    
    // Control
    void flush();
    void shutdown();
};

// Utility macros
#define LOG_CONTEXT() __FUNCTION__, __FILE__, __LINE__

#define SEMIPRO_LOG(level, category, message) \
    SemiPRO::AdvancedLogger::getInstance().log(level, category, message, LOG_CONTEXT())

#define SEMIPRO_LOG_MODULE(level, category, message, module) \
    SemiPRO::AdvancedLogger::getInstance().log(level, category, message, LOG_CONTEXT(), module)

#define SEMIPRO_TRACE(message) \
    SemiPRO::AdvancedLogger::getInstance().trace(message)

#define SEMIPRO_DEBUG(message) \
    SemiPRO::AdvancedLogger::getInstance().debug(message)

#define SEMIPRO_INFO(message) \
    SemiPRO::AdvancedLogger::getInstance().info(message)

#define SEMIPRO_WARNING(message) \
    SemiPRO::AdvancedLogger::getInstance().warning(message)

#define SEMIPRO_ERROR(message) \
    SemiPRO::AdvancedLogger::getInstance().error(message)

#define SEMIPRO_CRITICAL(message) \
    SemiPRO::AdvancedLogger::getInstance().critical(message)

#define SEMIPRO_FATAL(message) \
    SemiPRO::AdvancedLogger::getInstance().fatal(message)

#define SEMIPRO_PERF_TIMER(operation, module) \
    auto timer = SemiPRO::AdvancedLogger::getInstance().createTimer(operation, module)

// Helper function to convert ErrorSeverity to LogLevel
LogLevel errorSeverityToLogLevel(ErrorSeverity severity);

// Helper function to convert ErrorCategory to LogCategory
LogCategory errorCategoryToLogCategory(ErrorCategory category);

} // namespace SemiPRO

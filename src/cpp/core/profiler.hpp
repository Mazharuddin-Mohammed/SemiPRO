#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

class Profiler {
public:
    static Profiler& getInstance();
    
    // Timing functions
    void startTimer(const std::string& name);
    void endTimer(const std::string& name);
    
    // Memory tracking
    void recordMemoryUsage(const std::string& operation, size_t bytes);
    
    // Performance metrics
    double getAverageTime(const std::string& name) const;
    double getTotalTime(const std::string& name) const;
    size_t getCallCount(const std::string& name) const;
    
    // Reporting
    void generateReport() const;
    void exportToFile(const std::string& filename) const;
    void reset();
    
private:
    Profiler() = default;
    
    struct TimingData {
        std::vector<double> times;
        std::chrono::high_resolution_clock::time_point start_time;
        bool is_running = false;
        
        double getTotal() const;
        double getAverage() const;
        double getMin() const;
        double getMax() const;
    };
    
    struct MemoryData {
        std::vector<size_t> allocations;
        size_t total_allocated = 0;
        size_t peak_usage = 0;
    };
    
    mutable std::mutex mutex_;
    std::unordered_map<std::string, TimingData> timing_data_;
    std::unordered_map<std::string, MemoryData> memory_data_;
};

// RAII timer class
class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& name) : name_(name) {
        Profiler::getInstance().startTimer(name_);
    }
    
    ~ScopedTimer() {
        Profiler::getInstance().endTimer(name_);
    }
    
private:
    std::string name_;
};

// Convenience macro
#define PROFILE_SCOPE(name) ScopedTimer _timer(name)
#define PROFILE_FUNCTION() ScopedTimer _timer(__FUNCTION__)

#endif // PROFILER_HPP

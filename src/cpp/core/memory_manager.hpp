#pragma once

#include "advanced_logger.hpp"
#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <condition_variable>

namespace SemiPRO {

// Enhanced memory allocation statistics
struct MemoryStats {
    std::atomic<size_t> total_allocated{0};
    std::atomic<size_t> total_deallocated{0};
    std::atomic<size_t> current_usage{0};
    std::atomic<size_t> peak_usage{0};
    std::atomic<size_t> allocation_count{0};
    std::atomic<size_t> deallocation_count{0};

    size_t getCurrentUsage() const { return current_usage.load(); }
    size_t getPeakUsage() const { return peak_usage.load(); }
    size_t getTotalAllocated() const { return total_allocated.load(); }
    size_t getAllocationCount() const { return allocation_count.load(); }

    void reset() {
        total_allocated = 0;
        total_deallocated = 0;
        current_usage = 0;
        peak_usage = 0;
        allocation_count = 0;
        deallocation_count = 0;
    }
};

// Memory allocation tracking
struct AllocationInfo {
    size_t size;
    std::chrono::system_clock::time_point timestamp;
    std::string context;
    std::thread::id thread_id;

    // Default constructor
    AllocationInfo() : size(0), timestamp(std::chrono::system_clock::now()),
                      context(""), thread_id(std::this_thread::get_id()) {}

    // Parameterized constructor
    AllocationInfo(size_t s, const std::string& ctx = "")
        : size(s), timestamp(std::chrono::system_clock::now()),
          context(ctx), thread_id(std::this_thread::get_id()) {}
};

// Enhanced Memory Manager
class MemoryManager {
public:
    static MemoryManager& getInstance();

    // Enhanced memory pool management
    void* allocate(size_t size, size_t alignment = 16, const std::string& context = "");
    void deallocate(void* ptr);

    // Memory tracking
    void trackAllocation(size_t size, const std::string& context = "");
    void trackDeallocation(size_t size, const std::string& context = "");
    void trackPointer(void* ptr, size_t size, const std::string& context = "");
    void untrackPointer(void* ptr);

    // Enhanced statistics
    const MemoryStats& getStats() const { return stats_; }
    std::vector<AllocationInfo> getAllocations() const;
    size_t getActiveAllocations() const;

    // Legacy compatibility
    size_t getTotalAllocated() const { return stats_.getTotalAllocated(); }
    size_t getPeakUsage() const { return stats_.getPeakUsage(); }
    size_t getCurrentUsage() const { return stats_.getCurrentUsage(); }

    // Enhanced memory optimization
    void defragment();
    void cleanup();
    void performGarbageCollection();
    void optimizeMemoryUsage();
    
    // Configuration
    void setMemoryLimit(size_t limit_bytes) { memory_limit_ = limit_bytes; }
    void setWarningThreshold(double threshold) { warning_threshold_ = threshold; }
    void setAutoCleanupEnabled(bool enabled) { auto_cleanup_enabled_ = enabled; }

    // Memory analysis
    std::unordered_map<std::string, size_t> getMemoryByContext() const;
    std::vector<std::pair<std::string, size_t>> getTopMemoryConsumers(size_t count = 10) const;

    // Emergency functions
    bool isMemoryLimitExceeded() const;
    bool isMemoryWarningTriggered() const;
    void emergencyCleanup();

private:
    MemoryManager();
    ~MemoryManager();

    // Enhanced memory block structure
    struct MemoryBlock {
        void* ptr;
        size_t size;
        bool in_use;
        std::string context;
        std::chrono::system_clock::time_point allocated_time;

        MemoryBlock(void* p, size_t s, const std::string& ctx = "")
            : ptr(p), size(s), in_use(false), context(ctx),
              allocated_time(std::chrono::system_clock::now()) {}
    };

    // Enhanced data members
    MemoryStats stats_;
    std::vector<MemoryBlock> memory_blocks_;
    std::unordered_map<void*, AllocationInfo> allocations_;
    mutable std::mutex mutex_;

    // Memory monitoring
    std::thread monitor_thread_;
    std::atomic<bool> monitoring_enabled_{true};
    std::condition_variable monitor_cv_;
    std::mutex monitor_mutex_;

    // Configuration
    size_t memory_limit_ = 0; // 0 = no limit
    double warning_threshold_ = 0.8; // 80% of limit
    bool auto_cleanup_enabled_ = true;

    static constexpr size_t BLOCK_SIZE = 1024 * 1024; // 1MB blocks

    void monitoringThread();
};

// Enhanced RAII memory wrapper with tracking
template<typename T>
class ManagedArray {
public:
    explicit ManagedArray(size_t count, const std::string& context = "ManagedArray")
        : size_(count), context_(context),
          data_(static_cast<T*>(MemoryManager::getInstance().allocate(sizeof(T) * count, 16, context))) {
        // Initialize elements
        for (size_t i = 0; i < count; ++i) {
            new(data_ + i) T();
        }
        SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::MEMORY,
                          "Created ManagedArray: " + std::to_string(count) + " elements, " +
                          std::to_string(sizeof(T) * count) + " bytes", "MemoryManager");
    }

    ~ManagedArray() {
        // Destroy elements
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        MemoryManager::getInstance().deallocate(data_);
        SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::MEMORY,
                          "Destroyed ManagedArray: " + context_, "MemoryManager");
    }

    // Non-copyable, movable
    ManagedArray(const ManagedArray&) = delete;
    ManagedArray& operator=(const ManagedArray&) = delete;

    ManagedArray(ManagedArray&& other) noexcept
        : size_(other.size_), context_(std::move(other.context_)), data_(other.data_) {
        other.size_ = 0;
        other.data_ = nullptr;
    }

    ManagedArray& operator=(ManagedArray&& other) noexcept {
        if (this != &other) {
            // Clean up current data
            if (data_) {
                for (size_t i = 0; i < size_; ++i) {
                    data_[i].~T();
                }
                MemoryManager::getInstance().deallocate(data_);
            }

            // Move from other
            size_ = other.size_;
            context_ = std::move(other.context_);
            data_ = other.data_;

            other.size_ = 0;
            other.data_ = nullptr;
        }
        return *this;
    }

    // Access operators
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }

    T* data() { return data_; }
    const T* data() const { return data_; }
    size_t size() const { return size_; }

    // Utility methods
    bool empty() const { return size_ == 0; }
    T* begin() { return data_; }
    T* end() { return data_ + size_; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + size_; }

private:
    size_t size_;
    std::string context_;
    T* data_;
};

// RAII memory scope guard
class MemoryScope {
private:
    std::string scope_name_;
    size_t initial_usage_;
    std::chrono::high_resolution_clock::time_point start_time_;

public:
    explicit MemoryScope(const std::string& name)
        : scope_name_(name),
          initial_usage_(MemoryManager::getInstance().getStats().getCurrentUsage()),
          start_time_(std::chrono::high_resolution_clock::now()) {
        SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::MEMORY,
                          "Entering memory scope: " + scope_name_, "MemoryManager");
    }

    ~MemoryScope() {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time_);

        size_t final_usage = MemoryManager::getInstance().getStats().getCurrentUsage();
        long long memory_delta = static_cast<long long>(final_usage) - static_cast<long long>(initial_usage_);

        SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::MEMORY,
                          "Exiting memory scope: " + scope_name_ +
                          ", Duration: " + std::to_string(duration.count()) + "ms" +
                          ", Memory delta: " + std::to_string(memory_delta) + " bytes",
                          "MemoryManager");
    }
};

// Utility macros
#define MEMORY_SCOPE(name) SemiPRO::MemoryScope _scope(name)
#define TRACK_MEMORY(context) SemiPRO::MemoryManager::getInstance().trackAllocation(0, context)

} // namespace SemiPRO

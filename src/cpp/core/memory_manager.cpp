#include "memory_manager.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace SemiPRO {

MemoryManager::MemoryManager() {
    // Initialize monitoring thread
    monitor_thread_ = std::thread(&MemoryManager::monitoringThread, this);
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Enhanced Memory Manager initialized", "MemoryManager");
}

MemoryManager::~MemoryManager() {
    monitoring_enabled_ = false;
    monitor_cv_.notify_all();
    
    if (monitor_thread_.joinable()) {
        monitor_thread_.join();
    }
    
    // Clean up remaining allocations
    cleanup();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Memory Manager destroyed", "MemoryManager");
}

MemoryManager& MemoryManager::getInstance() {
    static MemoryManager instance;
    return instance;
}

void* MemoryManager::allocate(size_t size, size_t alignment, const std::string& context) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check memory limit
    if (memory_limit_ > 0 && stats_.current_usage + size > memory_limit_) {
        if (auto_cleanup_enabled_) {
            performGarbageCollection();
            if (stats_.current_usage + size > memory_limit_) {
                SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::MEMORY,
                                  "Memory allocation failed: limit exceeded", "MemoryManager");
                return nullptr;
            }
        } else {
            SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::MEMORY,
                              "Memory allocation failed: limit exceeded", "MemoryManager");
            return nullptr;
        }
    }
    
    // Find suitable block or allocate new one
    void* ptr = nullptr;
    
    // Try to find existing free block
    for (auto& block : memory_blocks_) {
        if (!block.in_use && block.size >= size) {
            block.in_use = true;
            block.context = context;
            block.allocated_time = std::chrono::system_clock::now();
            ptr = block.ptr;
            break;
        }
    }
    
    // Allocate new block if needed
    if (!ptr) {
        size_t block_size = std::max(size, BLOCK_SIZE);
        ptr = std::aligned_alloc(alignment, block_size);
        
        if (!ptr) {
            SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::MEMORY,
                              "System memory allocation failed", "MemoryManager");
            return nullptr;
        }
        
        memory_blocks_.emplace_back(ptr, block_size, context);
        memory_blocks_.back().in_use = true;
    }
    
    // Update statistics
    stats_.total_allocated += size;
    stats_.current_usage += size;
    stats_.allocation_count++;
    
    if (stats_.current_usage > stats_.peak_usage) {
        stats_.peak_usage = stats_.current_usage.load();
    }
    
    // Track allocation
    allocations_[ptr] = AllocationInfo(size, context);
    
    SEMIPRO_LOG_MODULE(LogLevel::TRACE, LogCategory::MEMORY,
                      "Allocated " + std::to_string(size) + " bytes for: " + context,
                      "MemoryManager");
    
    return ptr;
}

void MemoryManager::deallocate(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Find allocation info
    auto alloc_it = allocations_.find(ptr);
    if (alloc_it == allocations_.end()) {
        SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::MEMORY,
                          "Attempted to deallocate untracked pointer", "MemoryManager");
        return;
    }
    
    size_t size = alloc_it->second.size;
    std::string context = alloc_it->second.context;
    
    // Find and mark block as free
    for (auto& block : memory_blocks_) {
        if (block.ptr == ptr) {
            block.in_use = false;
            block.context.clear();
            break;
        }
    }
    
    // Update statistics
    stats_.total_deallocated += size;
    stats_.current_usage -= size;
    stats_.deallocation_count++;
    
    // Remove from tracking
    allocations_.erase(alloc_it);
    
    SEMIPRO_LOG_MODULE(LogLevel::TRACE, LogCategory::MEMORY,
                      "Deallocated " + std::to_string(size) + " bytes from: " + context,
                      "MemoryManager");
}

void MemoryManager::trackAllocation(size_t size, const std::string& context) {
    stats_.total_allocated += size;
    stats_.current_usage += size;
    stats_.allocation_count++;
    
    if (stats_.current_usage > stats_.peak_usage) {
        stats_.peak_usage = stats_.current_usage.load();
    }
    
    SEMIPRO_LOG_MODULE(LogLevel::TRACE, LogCategory::MEMORY,
                      "Tracked allocation: " + std::to_string(size) + " bytes for: " + context,
                      "MemoryManager");
}

void MemoryManager::trackDeallocation(size_t size, const std::string& context) {
    stats_.total_deallocated += size;
    stats_.current_usage -= size;
    stats_.deallocation_count++;
    
    SEMIPRO_LOG_MODULE(LogLevel::TRACE, LogCategory::MEMORY,
                      "Tracked deallocation: " + std::to_string(size) + " bytes from: " + context,
                      "MemoryManager");
}

std::vector<AllocationInfo> MemoryManager::getAllocations() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<AllocationInfo> result;
    
    for (const auto& [ptr, info] : allocations_) {
        result.push_back(info);
    }
    
    return result;
}

size_t MemoryManager::getActiveAllocations() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return allocations_.size();
}

std::unordered_map<std::string, size_t> MemoryManager::getMemoryByContext() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::unordered_map<std::string, size_t> result;
    
    for (const auto& [ptr, info] : allocations_) {
        result[info.context] += info.size;
    }
    
    return result;
}

std::vector<std::pair<std::string, size_t>> MemoryManager::getTopMemoryConsumers(size_t count) const {
    auto memory_by_context = getMemoryByContext();
    
    std::vector<std::pair<std::string, size_t>> result(memory_by_context.begin(), memory_by_context.end());
    
    std::sort(result.begin(), result.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    if (result.size() > count) {
        result.resize(count);
    }
    
    return result;
}

void MemoryManager::defragment() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Starting memory defragmentation", "MemoryManager");
    
    // Remove unused blocks
    auto it = std::remove_if(memory_blocks_.begin(), memory_blocks_.end(),
                            [](const MemoryBlock& block) {
                                if (!block.in_use) {
                                    std::free(block.ptr);
                                    return true;
                                }
                                return false;
                            });
    
    size_t removed_count = std::distance(it, memory_blocks_.end());
    memory_blocks_.erase(it, memory_blocks_.end());
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Defragmentation complete: removed " + std::to_string(removed_count) + " blocks",
                      "MemoryManager");
}

void MemoryManager::cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Starting memory cleanup", "MemoryManager");
    
    // Free all blocks
    for (const auto& block : memory_blocks_) {
        std::free(block.ptr);
    }
    
    memory_blocks_.clear();
    allocations_.clear();
    stats_.reset();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Memory cleanup complete", "MemoryManager");
}

void MemoryManager::performGarbageCollection() {
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Performing garbage collection", "MemoryManager");
    
    defragment();
    
    // Additional cleanup logic can be added here
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Garbage collection complete", "MemoryManager");
}

void MemoryManager::optimizeMemoryUsage() {
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Optimizing memory usage", "MemoryManager");
    
    performGarbageCollection();
    
    // Additional optimization logic
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::MEMORY,
                      "Memory optimization complete", "MemoryManager");
}

bool MemoryManager::isMemoryLimitExceeded() const {
    return memory_limit_ > 0 && stats_.current_usage > memory_limit_;
}

bool MemoryManager::isMemoryWarningTriggered() const {
    return memory_limit_ > 0 && 
           stats_.current_usage > static_cast<size_t>(memory_limit_ * warning_threshold_);
}

void MemoryManager::emergencyCleanup() {
    SEMIPRO_LOG_MODULE(LogLevel::CRITICAL, LogCategory::MEMORY,
                      "Emergency memory cleanup initiated", "MemoryManager");
    
    performGarbageCollection();
    
    if (isMemoryLimitExceeded()) {
        SEMIPRO_LOG_MODULE(LogLevel::CRITICAL, LogCategory::MEMORY,
                          "Emergency cleanup failed to free sufficient memory", "MemoryManager");
    }
}

void MemoryManager::monitoringThread() {
    while (monitoring_enabled_) {
        std::unique_lock<std::mutex> lock(monitor_mutex_);
        monitor_cv_.wait_for(lock, std::chrono::seconds(10));
        
        if (!monitoring_enabled_) break;
        
        // Check memory status
        if (isMemoryWarningTriggered()) {
            SEMIPRO_LOG_MODULE(LogLevel::WARNING, LogCategory::MEMORY,
                              "Memory usage warning: " + std::to_string(stats_.current_usage) + 
                              " bytes (" + std::to_string(static_cast<double>(stats_.current_usage) / memory_limit_ * 100) + "%)",
                              "MemoryManager");
        }
        
        if (isMemoryLimitExceeded()) {
            SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::MEMORY,
                              "Memory limit exceeded!", "MemoryManager");
            
            if (auto_cleanup_enabled_) {
                emergencyCleanup();
            }
        }
    }
}

} // namespace SemiPRO

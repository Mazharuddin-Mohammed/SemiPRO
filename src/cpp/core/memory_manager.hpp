#ifndef MEMORY_MANAGER_HPP
#define MEMORY_MANAGER_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <unordered_map>

class MemoryManager {
public:
    static MemoryManager& getInstance();
    
    // Memory pool management
    void* allocate(size_t size, size_t alignment = 16);
    void deallocate(void* ptr);
    
    // Statistics
    size_t getTotalAllocated() const;
    size_t getPeakUsage() const;
    size_t getCurrentUsage() const;
    
    // Memory optimization
    void defragment();
    void cleanup();
    
private:
    MemoryManager() = default;
    ~MemoryManager() = default;
    
    struct MemoryBlock {
        void* ptr;
        size_t size;
        bool in_use;
        
        MemoryBlock(void* p, size_t s) : ptr(p), size(s), in_use(false) {}
    };
    
    std::vector<MemoryBlock> memory_blocks_;
    std::unordered_map<void*, size_t> allocations_;
    mutable std::mutex mutex_;
    
    size_t total_allocated_ = 0;
    size_t peak_usage_ = 0;
    size_t current_usage_ = 0;
    
    static constexpr size_t BLOCK_SIZE = 1024 * 1024; // 1MB blocks
};

// RAII memory wrapper
template<typename T>
class ManagedArray {
public:
    explicit ManagedArray(size_t count) 
        : size_(count), data_(static_cast<T*>(MemoryManager::getInstance().allocate(sizeof(T) * count))) {
        // Initialize elements
        for (size_t i = 0; i < count; ++i) {
            new(data_ + i) T();
        }
    }
    
    ~ManagedArray() {
        // Destroy elements
        for (size_t i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        MemoryManager::getInstance().deallocate(data_);
    }
    
    T& operator[](size_t index) { return data_[index]; }
    const T& operator[](size_t index) const { return data_[index]; }
    
    T* data() { return data_; }
    const T* data() const { return data_; }
    size_t size() const { return size_; }
    
private:
    size_t size_;
    T* data_;
};

#endif // MEMORY_MANAGER_HPP

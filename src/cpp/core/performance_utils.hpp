// Author: Dr. Mazharuddin Mohammed
#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <Eigen/Sparse>
#include <Eigen/Dense>

namespace SemiPRO {

/**
 * @brief Optimized solver using sparse matrices for large grids
 */
class OptimizedSolver {
private:
    Eigen::SparseMatrix<double> system_matrix_;
    Eigen::VectorXd solution_vector_;
    int n_rows_;
    bool memory_mapped_;
    
public:
    OptimizedSolver(int rows, int cols);
    ~OptimizedSolver();
    
    void optimize_memory();
    void enable_memory_mapping();
    void reserve_matrix_memory(int expected_nonzeros_per_row = 7);
    void solve_system(const Eigen::VectorXd& rhs);
    
    Eigen::SparseMatrix<double>& get_system_matrix() { return system_matrix_; }
    const Eigen::VectorXd& get_solution() const { return solution_vector_; }
};

/**
 * @brief Simulation result caching system
 */
class SimulationCache {
private:
    std::string cache_dir_;
    std::unordered_map<std::string, std::string> cache_index_;
    size_t max_cache_size_;
    
public:
    SimulationCache(const std::string& cache_dir = "cache", size_t max_size = 1024*1024*1024); // 1GB default
    
    std::string get_cache_key(const std::unordered_map<std::string, double>& params);
    bool has_cached_result(const std::string& cache_key);
    bool load_cached_result(const std::string& cache_key, std::vector<double>& result);
    void save_result(const std::string& cache_key, const std::vector<double>& result);
    void clear_cache();
    size_t get_cache_size();
};

/**
 * @brief Performance profiler for timing operations
 */
class PerformanceProfiler {
private:
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> start_times_;
    std::unordered_map<std::string, double> accumulated_times_;
    std::unordered_map<std::string, int> call_counts_;
    
public:
    void start_timer(const std::string& operation_name);
    void end_timer(const std::string& operation_name);
    double get_elapsed_time(const std::string& operation_name);
    double get_average_time(const std::string& operation_name);
    void print_profile_report();
    void reset_profile();
};

/**
 * @brief Memory pool for efficient allocation/deallocation
 */
template<typename T>
class MemoryPool {
private:
    std::vector<T*> free_blocks_;
    std::vector<std::unique_ptr<T[]>> allocated_chunks_;
    size_t block_size_;
    size_t chunk_size_;
    
public:
    MemoryPool(size_t chunk_size = 1024);
    ~MemoryPool();
    
    T* allocate();
    void deallocate(T* ptr);
    void clear();
    size_t get_allocated_count() const;
};

/**
 * @brief Vectorized operations for numerical computations
 */
class VectorizedOps {
public:
    static void diffusion_step_2d(Eigen::MatrixXd& concentration, 
                                 double diffusivity, double dt, double dx);
    
    static void apply_boundary_conditions(Eigen::MatrixXd& grid, 
                                        const std::string& bc_type,
                                        double bc_value = 0.0);
    
    static double compute_laplacian_at_point(const Eigen::MatrixXd& grid, 
                                           int i, int j, double dx);
    
    static void parallel_matrix_operation(Eigen::MatrixXd& matrix,
                                        std::function<double(double)> operation);
};

/**
 * @brief Adaptive mesh refinement for efficient computation
 */
class AdaptiveMesh {
private:
    struct MeshNode {
        double x, y;
        double value;
        int level;
        std::vector<std::shared_ptr<MeshNode>> children;
    };
    
    std::shared_ptr<MeshNode> root_;
    int max_levels_;
    double refinement_threshold_;
    
public:
    AdaptiveMesh(double x_min, double x_max, double y_min, double y_max, 
                int max_levels = 5, double threshold = 0.1);
    
    void refine_mesh(std::function<double(double, double)> value_function);
    void coarsen_mesh();
    std::vector<std::pair<double, double>> get_mesh_points();
    void interpolate_to_regular_grid(Eigen::MatrixXd& grid);
};

} // namespace SemiPRO

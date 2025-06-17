// Author: Dr. Mazharuddin Mohammed
#include "performance_utils.hpp"
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cmath>
#include <omp.h>

namespace SemiPRO {

// OptimizedSolver Implementation
OptimizedSolver::OptimizedSolver(int rows, int cols) 
    : n_rows_(rows), memory_mapped_(false) {
    system_matrix_.resize(rows, cols);
    solution_vector_.resize(rows);
}

OptimizedSolver::~OptimizedSolver() {
    if (memory_mapped_) {
        // Cleanup memory mapping if needed
    }
}

void OptimizedSolver::optimize_memory() {
    // Reserve memory based on expected non-zeros
    reserve_matrix_memory(7); // Typical for 2D finite difference
    
    // Use compressed storage
    system_matrix_.makeCompressed();
    
    Logger::getInstance().log("Memory optimization applied to solver");
}

void OptimizedSolver::enable_memory_mapping() {
    memory_mapped_ = true;
    Logger::getInstance().log("Memory mapping enabled for large datasets");
}

void OptimizedSolver::reserve_matrix_memory(int expected_nonzeros_per_row) {
    system_matrix_.reserve(Eigen::VectorXi::Constant(n_rows_, expected_nonzeros_per_row));
}

void OptimizedSolver::solve_system(const Eigen::VectorXd& rhs) {
    Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;
    solver.compute(system_matrix_);
    
    if (solver.info() != Eigen::Success) {
        throw std::runtime_error("Matrix decomposition failed");
    }
    
    solution_vector_ = solver.solve(rhs);
    
    if (solver.info() != Eigen::Success) {
        throw std::runtime_error("Solving failed");
    }
}

// SimulationCache Implementation
SimulationCache::SimulationCache(const std::string& cache_dir, size_t max_size)
    : cache_dir_(cache_dir), max_cache_size_(max_size) {
    std::filesystem::create_directories(cache_dir_);
}

std::string SimulationCache::get_cache_key(const std::unordered_map<std::string, double>& params) {
    std::string param_str;
    for (const auto& [key, value] : params) {
        param_str += key + ":" + std::to_string(value) + ";";
    }
    
    // Simple hash function (in production, use proper hash like MD5)
    std::hash<std::string> hasher;
    return std::to_string(hasher(param_str));
}

bool SimulationCache::has_cached_result(const std::string& cache_key) {
    std::string cache_file = cache_dir_ + "/" + cache_key + ".dat";
    return std::filesystem::exists(cache_file);
}

bool SimulationCache::load_cached_result(const std::string& cache_key, std::vector<double>& result) {
    std::string cache_file = cache_dir_ + "/" + cache_key + ".dat";
    
    if (!std::filesystem::exists(cache_file)) {
        return false;
    }
    
    std::ifstream file(cache_file, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    
    result.resize(size);
    file.read(reinterpret_cast<char*>(result.data()), size * sizeof(double));
    
    return true;
}

void SimulationCache::save_result(const std::string& cache_key, const std::vector<double>& result) {
    std::string cache_file = cache_dir_ + "/" + cache_key + ".dat";
    
    std::ofstream file(cache_file, std::ios::binary);
    if (!file.is_open()) {
        return;
    }
    
    size_t size = result.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    file.write(reinterpret_cast<const char*>(result.data()), size * sizeof(double));
    
    cache_index_[cache_key] = cache_file;
}

void SimulationCache::clear_cache() {
    for (const auto& [key, file] : cache_index_) {
        std::filesystem::remove(file);
    }
    cache_index_.clear();
}

size_t SimulationCache::get_cache_size() {
    size_t total_size = 0;
    for (const auto& [key, file] : cache_index_) {
        if (std::filesystem::exists(file)) {
            total_size += std::filesystem::file_size(file);
        }
    }
    return total_size;
}

// PerformanceProfiler Implementation
void PerformanceProfiler::start_timer(const std::string& operation_name) {
    start_times_[operation_name] = std::chrono::high_resolution_clock::now();
}

void PerformanceProfiler::end_timer(const std::string& operation_name) {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto it = start_times_.find(operation_name);
    
    if (it != start_times_.end()) {
        auto duration = std::chrono::duration<double, std::milli>(end_time - it->second).count();
        accumulated_times_[operation_name] += duration;
        call_counts_[operation_name]++;
        start_times_.erase(it);
    }
}

double PerformanceProfiler::get_elapsed_time(const std::string& operation_name) {
    return accumulated_times_[operation_name];
}

double PerformanceProfiler::get_average_time(const std::string& operation_name) {
    int count = call_counts_[operation_name];
    return count > 0 ? accumulated_times_[operation_name] / count : 0.0;
}

void PerformanceProfiler::print_profile_report() {
    std::cout << "\n=== Performance Profile Report ===\n";
    std::cout << "Operation\t\tTotal (ms)\tAvg (ms)\tCalls\n";
    std::cout << "----------------------------------------\n";
    
    for (const auto& [name, total_time] : accumulated_times_) {
        int calls = call_counts_[name];
        double avg_time = calls > 0 ? total_time / calls : 0.0;
        std::cout << name << "\t\t" << total_time << "\t\t" << avg_time << "\t\t" << calls << "\n";
    }
    std::cout << "====================================\n\n";
}

void PerformanceProfiler::reset_profile() {
    start_times_.clear();
    accumulated_times_.clear();
    call_counts_.clear();
}

// VectorizedOps Implementation
void VectorizedOps::diffusion_step_2d(Eigen::MatrixXd& concentration, 
                                     double diffusivity, double dt, double dx) {
    int rows = concentration.rows();
    int cols = concentration.cols();
    Eigen::MatrixXd new_concentration = concentration;
    
    #pragma omp parallel for collapse(2)
    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            double laplacian = (concentration(i+1, j) + concentration(i-1, j) +
                               concentration(i, j+1) + concentration(i, j-1) -
                               4.0 * concentration(i, j)) / (dx * dx);
            
            new_concentration(i, j) = concentration(i, j) + dt * diffusivity * laplacian;
        }
    }
    
    concentration = new_concentration;
}

void VectorizedOps::apply_boundary_conditions(Eigen::MatrixXd& grid, 
                                            const std::string& bc_type,
                                            double bc_value) {
    int rows = grid.rows();
    int cols = grid.cols();
    
    if (bc_type == "dirichlet") {
        // Set boundary values
        grid.row(0).setConstant(bc_value);
        grid.row(rows-1).setConstant(bc_value);
        grid.col(0).setConstant(bc_value);
        grid.col(cols-1).setConstant(bc_value);
    } else if (bc_type == "neumann") {
        // Zero gradient boundary conditions
        grid.row(0) = grid.row(1);
        grid.row(rows-1) = grid.row(rows-2);
        grid.col(0) = grid.col(1);
        grid.col(cols-1) = grid.col(cols-2);
    }
}

double VectorizedOps::compute_laplacian_at_point(const Eigen::MatrixXd& grid, 
                                               int i, int j, double dx) {
    if (i <= 0 || i >= grid.rows()-1 || j <= 0 || j >= grid.cols()-1) {
        return 0.0; // Boundary point
    }
    
    return (grid(i+1, j) + grid(i-1, j) + grid(i, j+1) + grid(i, j-1) - 
            4.0 * grid(i, j)) / (dx * dx);
}

void VectorizedOps::parallel_matrix_operation(Eigen::MatrixXd& matrix,
                                             std::function<double(double)> operation) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < matrix.rows(); ++i) {
        for (int j = 0; j < matrix.cols(); ++j) {
            matrix(i, j) = operation(matrix(i, j));
        }
    }
}

} // namespace SemiPRO

#pragma once

#include "../core/advanced_logger.hpp"
#include "../core/enhanced_error_handling.hpp"
#include "../core/config_manager.hpp"
#include "../core/wafer_enhanced.hpp"
#include "multi_layer_engine.hpp"
#include "temperature_controller.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>

namespace SemiPRO {

/**
 * Advanced Process Optimization Engine
 * Implements sophisticated optimization algorithms including parameter sweeps,
 * genetic algorithms, multi-objective optimization, and automated process tuning
 */

// Optimization algorithm types
enum class OptimizationAlgorithm {
    PARAMETER_SWEEP,      // Exhaustive parameter sweep
    GRID_SEARCH,          // Grid-based search
    RANDOM_SEARCH,        // Random parameter search
    GENETIC_ALGORITHM,    // Genetic algorithm optimization
    PARTICLE_SWARM,       // Particle swarm optimization
    SIMULATED_ANNEALING,  // Simulated annealing
    GRADIENT_DESCENT,     // Gradient descent optimization
    BAYESIAN_OPTIMIZATION, // Bayesian optimization
    MULTI_OBJECTIVE_GA,   // Multi-objective genetic algorithm
    DIFFERENTIAL_EVOLUTION // Differential evolution
};

// Use the OptimizationObjective from multi_layer_engine.hpp to avoid duplication

// Parameter types for optimization
enum class ParameterType {
    CONTINUOUS,           // Continuous parameter (e.g., temperature)
    DISCRETE,             // Discrete parameter (e.g., number of cycles)
    CATEGORICAL,          // Categorical parameter (e.g., material type)
    BOOLEAN,              // Boolean parameter (e.g., enable/disable)
    INTEGER,              // Integer parameter (e.g., grid size)
    ENUMERATED            // Enumerated parameter (e.g., process mode)
};

// Optimization parameter definition
struct OptimizationParameter {
    std::string name;             // Parameter name
    ParameterType type;           // Parameter type
    double min_value;             // Minimum value (for continuous/discrete)
    double max_value;             // Maximum value (for continuous/discrete)
    double current_value;         // Current value
    double step_size;             // Step size for sweeps
    std::vector<std::string> categories; // Categories for categorical parameters
    bool is_active;               // Whether parameter is being optimized
    double weight;                // Parameter importance weight
    
    OptimizationParameter() : type(ParameterType::CONTINUOUS), min_value(0.0), 
                             max_value(1.0), current_value(0.5), step_size(0.1),
                             is_active(true), weight(1.0) {}
};

// Optimization constraint definition
struct OptimizationConstraint {
    std::string name;             // Constraint name
    std::string parameter;        // Parameter being constrained
    double min_limit;             // Minimum allowed value
    double max_limit;             // Maximum allowed value
    bool is_hard_constraint;      // Hard vs soft constraint
    double penalty_weight;        // Penalty weight for violation
    
    OptimizationConstraint() : min_limit(0.0), max_limit(1.0), 
                              is_hard_constraint(true), penalty_weight(1.0) {}
};

// Optimization results for a single evaluation
struct OptimizationEvaluation {
    std::unordered_map<std::string, double> parameters; // Parameter values
    std::unordered_map<std::string, double> objectives; // Objective values
    std::unordered_map<std::string, double> constraints; // Constraint values
    double fitness_score;         // Overall fitness score
    bool is_feasible;             // Whether solution is feasible
    double evaluation_time;       // Time taken for evaluation (seconds)
    std::string evaluation_id;    // Unique evaluation identifier
    
    OptimizationEvaluation() : fitness_score(0.0), is_feasible(true), evaluation_time(0.0) {}
};

// Use the OptimizationResults from multi_layer_engine.hpp to avoid duplication
// Additional fields specific to process optimization
struct ProcessOptimizationResults : public OptimizationResults {
    std::vector<OptimizationEvaluation> evaluations; // All evaluations
    OptimizationEvaluation best_solution;    // Best solution found
    std::vector<OptimizationEvaluation> pareto_front; // Pareto front (multi-objective)
    int total_evaluations;        // Total number of evaluations
    double total_time;            // Total optimization time (seconds)
    double convergence_metric;    // Convergence metric
    bool has_converged;           // Whether optimization has converged
    std::vector<std::string> recommendations; // Optimization recommendations
    std::unordered_map<std::string, double> statistics; // Optimization statistics

    ProcessOptimizationResults() : total_evaluations(0), total_time(0.0),
                                  convergence_metric(0.0), has_converged(false) {}
};

// Genetic algorithm individual
struct GAIndividual {
    std::vector<double> genes;    // Parameter values (genes)
    double fitness;               // Fitness score
    bool is_evaluated;            // Whether fitness has been evaluated
    
    GAIndividual() : fitness(0.0), is_evaluated(false) {}
};

// Genetic algorithm population
struct GAPopulation {
    std::vector<GAIndividual> individuals; // Population individuals
    int generation;               // Current generation
    double best_fitness;          // Best fitness in population
    double average_fitness;       // Average fitness in population
    double diversity_metric;      // Population diversity metric
    
    GAPopulation() : generation(0), best_fitness(0.0), average_fitness(0.0), diversity_metric(0.0) {}
};

// Process optimization engine
class ProcessOptimizer {
private:
    std::unordered_map<std::string, OptimizationParameter> parameters_;
    std::unordered_map<std::string, OptimizationConstraint> constraints_;
    std::unordered_map<std::string, std::function<double(const std::unordered_map<std::string, double>&)>> objective_functions_;
    
    // Optimization state
    OptimizationAlgorithm current_algorithm_;
    std::vector<OptimizationObjective> objectives_;
    std::mt19937 random_generator_;
    
    // Algorithm-specific parameters
    struct GAParameters {
        int population_size = 50;
        int max_generations = 100;
        double mutation_rate = 0.1;
        double crossover_rate = 0.8;
        double elitism_rate = 0.1;
        bool use_tournament_selection = true;
        int tournament_size = 3;
    } ga_params_;
    
    struct PSParameters {
        int swarm_size = 30;
        int max_iterations = 100;
        double inertia_weight = 0.9;
        double cognitive_weight = 2.0;
        double social_weight = 2.0;
    } ps_params_;
    
    // Configuration
    bool enable_parallel_evaluation_ = true;
    bool enable_adaptive_parameters_ = true;
    bool enable_constraint_handling_ = true;
    double convergence_tolerance_ = 1e-6;
    int max_evaluations_ = 1000;
    
public:
    ProcessOptimizer();
    ~ProcessOptimizer() = default;
    
    // Parameter and constraint management
    void addParameter(const std::string& name, const OptimizationParameter& param);
    void addConstraint(const std::string& name, const OptimizationConstraint& constraint);
    void addObjectiveFunction(const std::string& name, 
                             std::function<double(const std::unordered_map<std::string, double>&)> func);
    
    // Optimization execution
    ProcessOptimizationResults optimizeProcess(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        OptimizationAlgorithm algorithm,
        const std::vector<OptimizationObjective>& objectives
    );

    ProcessOptimizationResults parameterSweep(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        const std::vector<std::string>& parameter_names
    );

    ProcessOptimizationResults geneticAlgorithmOptimization(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        const std::vector<OptimizationObjective>& objectives
    );

    ProcessOptimizationResults particleSwarmOptimization(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        const std::vector<OptimizationObjective>& objectives
    );

    ProcessOptimizationResults simulatedAnnealingOptimization(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        OptimizationObjective objective
    );
    
    // Multi-objective optimization
    ProcessOptimizationResults multiObjectiveOptimization(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        const std::vector<OptimizationObjective>& objectives
    );
    
    std::vector<OptimizationEvaluation> calculateParetoFront(
        const std::vector<OptimizationEvaluation>& evaluations
    );
    
    // Process evaluation
    OptimizationEvaluation evaluateProcess(
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type,
        const std::unordered_map<std::string, double>& parameters
    );
    
    double calculateFitness(
        const std::unordered_map<std::string, double>& objectives,
        const std::unordered_map<std::string, double>& constraints
    );
    
    // Genetic algorithm operations
    GAPopulation initializePopulation(int population_size);
    void evaluatePopulation(
        GAPopulation& population,
        std::shared_ptr<WaferEnhanced> wafer,
        const std::string& process_type
    );
    
    GAPopulation selection(const GAPopulation& population);
    GAPopulation crossover(const GAPopulation& parents);
    GAPopulation mutation(const GAPopulation& offspring);
    
    // Analysis and reporting
    std::unordered_map<std::string, double> analyzeOptimizationResults(
        const ProcessOptimizationResults& results
    ) const;

    std::vector<std::string> generateRecommendations(
        const ProcessOptimizationResults& results
    ) const;
    
    // Configuration and tuning
    void setGAParameters(const GAParameters& params);
    void setPSParameters(const PSParameters& params);
    void setConvergenceTolerance(double tolerance);
    void setMaxEvaluations(int max_evals);
    
    void enableFeatures(bool parallel = true, bool adaptive = true, bool constraints = true);
    
    // Validation and diagnostics
    bool validateOptimizationSetup(std::string& error_message) const;
    std::vector<std::string> getDiagnostics() const;
    
    // Utility methods
    std::string algorithmToString(OptimizationAlgorithm algorithm) const;
    std::string objectiveToString(OptimizationObjective objective) const;
    std::string parameterTypeToString(ParameterType type) const;
    
private:
    void initializeOptimizer();
    
    // Utility functions
    std::vector<double> generateRandomParameters();
    std::vector<double> encodeParameters(const std::unordered_map<std::string, double>& params);
    std::unordered_map<std::string, double> decodeParameters(const std::vector<double>& encoded);
    
    // Constraint handling
    bool checkConstraints(const std::unordered_map<std::string, double>& parameters);
    double calculateConstraintPenalty(const std::unordered_map<std::string, double>& parameters);
    
    // Convergence checking
    bool checkConvergence(const std::vector<OptimizationEvaluation>& recent_evaluations);
    double calculateDiversity(const GAPopulation& population);
    
    // Adaptive parameter adjustment
    void adaptAlgorithmParameters(const OptimizationResults& current_results);
};

// Utility functions for optimization analysis
namespace OptimizationUtils {
    // Statistical analysis
    double calculateMean(const std::vector<double>& values);
    double calculateStandardDeviation(const std::vector<double>& values);
    double calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y);
    
    // Pareto analysis
    bool isDominated(const OptimizationEvaluation& a, const OptimizationEvaluation& b);
    std::vector<OptimizationEvaluation> findParetoFront(const std::vector<OptimizationEvaluation>& solutions);
    double calculateHypervolume(const std::vector<OptimizationEvaluation>& pareto_front);
    
    // Optimization utilities
    std::vector<std::string> suggestParameterRanges(const OptimizationResults& results);
    std::unordered_map<std::string, double> calculateSensitivityAnalysis(const OptimizationResults& results);
    OptimizationParameter optimizeParameterRange(const std::string& param_name, const OptimizationResults& results);
}

} // namespace SemiPRO

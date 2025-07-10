#include "process_optimizer.hpp"
#include <algorithm>
#include <cmath>
#include <chrono>
#include <numeric>

namespace SemiPRO {

ProcessOptimizer::ProcessOptimizer() {
    initializeOptimizer();
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Process Optimization Engine initialized", "ProcessOptimizer");
}

void ProcessOptimizer::addParameter(const std::string& name, const OptimizationParameter& param) {
    parameters_[name] = param;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Added optimization parameter: " + name + 
                      " (type: " + parameterTypeToString(param.type) + 
                      ", range: " + std::to_string(param.min_value) + " - " + 
                      std::to_string(param.max_value) + ")",
                      "ProcessOptimizer");
}

void ProcessOptimizer::addConstraint(const std::string& name, const OptimizationConstraint& constraint) {
    constraints_[name] = constraint;
    
    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Added optimization constraint: " + name + 
                      " (parameter: " + constraint.parameter + 
                      ", limits: " + std::to_string(constraint.min_limit) + " - " + 
                      std::to_string(constraint.max_limit) + ")",
                      "ProcessOptimizer");
}

ProcessOptimizationResults ProcessOptimizer::optimizeProcess(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type,
    OptimizationAlgorithm algorithm,
    const std::vector<OptimizationObjective>& objectives) {

    SEMIPRO_PERF_TIMER("process_optimization", "ProcessOptimizer");

    ProcessOptimizationResults results;
    current_algorithm_ = algorithm;
    objectives_ = objectives;
    
    auto start_time = std::chrono::steady_clock::now();
    
    try {
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Starting process optimization: " + process_type + 
                          " using " + algorithmToString(algorithm) + 
                          " with " + std::to_string(objectives.size()) + " objectives",
                          "ProcessOptimizer");
        
        // Route to specific optimization algorithm
        switch (algorithm) {
            case OptimizationAlgorithm::PARAMETER_SWEEP:
            case OptimizationAlgorithm::GRID_SEARCH: {
                std::vector<std::string> param_names;
                for (const auto& param : parameters_) {
                    if (param.second.is_active) {
                        param_names.push_back(param.first);
                    }
                }
                results = parameterSweep(wafer, process_type, param_names);
                break;
            }
            
            case OptimizationAlgorithm::GENETIC_ALGORITHM:
            case OptimizationAlgorithm::MULTI_OBJECTIVE_GA:
                results = geneticAlgorithmOptimization(wafer, process_type, objectives);
                break;
                
            case OptimizationAlgorithm::PARTICLE_SWARM:
                results = particleSwarmOptimization(wafer, process_type, objectives);
                break;
                
            case OptimizationAlgorithm::SIMULATED_ANNEALING:
                if (!objectives.empty()) {
                    results = simulatedAnnealingOptimization(wafer, process_type, objectives[0]);
                }
                break;
                
            default:
                // Default to genetic algorithm
                results = geneticAlgorithmOptimization(wafer, process_type, objectives);
                break;
        }
        
        // Calculate total optimization time
        auto end_time = std::chrono::steady_clock::now();
        results.total_time = std::chrono::duration<double>(end_time - start_time).count();
        
        // Generate recommendations
        results.recommendations = generateRecommendations(results);
        
        // Calculate statistics
        results.statistics = analyzeOptimizationResults(results);
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Process optimization completed: " + 
                          std::to_string(results.total_evaluations) + " evaluations in " + 
                          std::to_string(results.total_time) + " seconds",
                          "ProcessOptimizer");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Process optimization failed: " + std::string(e.what()),
                          "ProcessOptimizer");
    }
    
    return results;
}

ProcessOptimizationResults ProcessOptimizer::parameterSweep(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type,
    const std::vector<std::string>& parameter_names) {

    SEMIPRO_PERF_TIMER("parameter_sweep", "ProcessOptimizer");

    ProcessOptimizationResults results;
    
    try {
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Starting parameter sweep for " + std::to_string(parameter_names.size()) + 
                          " parameters",
                          "ProcessOptimizer");
        
        // Generate parameter combinations
        std::vector<std::unordered_map<std::string, double>> parameter_combinations;
        
        if (parameter_names.size() == 1) {
            // Single parameter sweep
            const std::string& param_name = parameter_names[0];
            const auto& param = parameters_[param_name];
            
            double current = param.min_value;
            while (current <= param.max_value) {
                std::unordered_map<std::string, double> params;
                params[param_name] = current;
                parameter_combinations.push_back(params);
                current += param.step_size;
            }
        } else if (parameter_names.size() == 2) {
            // Two-parameter grid search
            const auto& param1 = parameters_[parameter_names[0]];
            const auto& param2 = parameters_[parameter_names[1]];
            
            double val1 = param1.min_value;
            while (val1 <= param1.max_value) {
                double val2 = param2.min_value;
                while (val2 <= param2.max_value) {
                    std::unordered_map<std::string, double> params;
                    params[parameter_names[0]] = val1;
                    params[parameter_names[1]] = val2;
                    parameter_combinations.push_back(params);
                    val2 += param2.step_size;
                }
                val1 += param1.step_size;
            }
        } else {
            // Multi-parameter random sampling (too many combinations for exhaustive search)
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            
            for (int i = 0; i < 100; ++i) { // Sample 100 random combinations
                std::unordered_map<std::string, double> params;
                for (const std::string& param_name : parameter_names) {
                    const auto& param = parameters_[param_name];
                    double random_val = param.min_value + 
                                       (param.max_value - param.min_value) * dist(random_generator_);
                    params[param_name] = random_val;
                }
                parameter_combinations.push_back(params);
            }
        }
        
        // Evaluate all parameter combinations
        double best_fitness = -std::numeric_limits<double>::infinity();
        
        for (const auto& params : parameter_combinations) {
            auto evaluation = evaluateProcess(wafer, process_type, params);
            results.evaluations.push_back(evaluation);
            
            if (evaluation.fitness_score > best_fitness) {
                best_fitness = evaluation.fitness_score;
                results.best_solution = evaluation;
            }
        }
        
        results.total_evaluations = static_cast<int>(results.evaluations.size());
        results.has_converged = true; // Parameter sweep always "converges"
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Parameter sweep completed: " + std::to_string(results.total_evaluations) + 
                          " evaluations, best fitness: " + std::to_string(best_fitness),
                          "ProcessOptimizer");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Parameter sweep failed: " + std::string(e.what()),
                          "ProcessOptimizer");
    }
    
    return results;
}

ProcessOptimizationResults ProcessOptimizer::geneticAlgorithmOptimization(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type,
    const std::vector<OptimizationObjective>& objectives) {

    SEMIPRO_PERF_TIMER("genetic_algorithm", "ProcessOptimizer");

    ProcessOptimizationResults results;
    
    try {
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Starting genetic algorithm optimization: population=" + 
                          std::to_string(ga_params_.population_size) + 
                          ", generations=" + std::to_string(ga_params_.max_generations),
                          "ProcessOptimizer");
        
        // Initialize population
        GAPopulation population = initializePopulation(ga_params_.population_size);
        
        double best_fitness = -std::numeric_limits<double>::infinity();
        int generations_without_improvement = 0;
        const int max_stagnation = 20; // Stop if no improvement for 20 generations
        
        // Evolution loop
        for (int generation = 0; generation < ga_params_.max_generations; ++generation) {
            population.generation = generation;
            
            // Evaluate population
            evaluatePopulation(population, wafer, process_type);
            
            // Track best solution
            for (const auto& individual : population.individuals) {
                if (individual.fitness > best_fitness) {
                    best_fitness = individual.fitness;
                    generations_without_improvement = 0;
                    
                    // Update best solution
                    results.best_solution.parameters = decodeParameters(individual.genes);
                    results.best_solution.fitness_score = individual.fitness;
                } else {
                    generations_without_improvement++;
                }
            }
            
            // Check for convergence
            if (generations_without_improvement >= max_stagnation) {
                SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                                  "GA converged at generation " + std::to_string(generation),
                                  "ProcessOptimizer");
                results.has_converged = true;
                break;
            }
            
            // Selection, crossover, and mutation
            GAPopulation parents = selection(population);
            GAPopulation offspring = crossover(parents);
            population = mutation(offspring);
            
            // Log progress every 10 generations
            if (generation % 10 == 0) {
                SEMIPRO_LOG_MODULE(LogLevel::DEBUG, LogCategory::ADVANCED,
                                  "GA Generation " + std::to_string(generation) + 
                                  ": best fitness = " + std::to_string(best_fitness),
                                  "ProcessOptimizer");
            }
        }
        
        results.total_evaluations = ga_params_.max_generations * ga_params_.population_size;
        
        SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                          "Genetic algorithm completed: best fitness = " + std::to_string(best_fitness),
                          "ProcessOptimizer");
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Genetic algorithm failed: " + std::string(e.what()),
                          "ProcessOptimizer");
    }
    
    return results;
}

OptimizationEvaluation ProcessOptimizer::evaluateProcess(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type,
    const std::unordered_map<std::string, double>& parameters) {
    
    OptimizationEvaluation evaluation;
    evaluation.parameters = parameters;
    
    auto start_time = std::chrono::steady_clock::now();
    
    try {
        // Use the original wafer for evaluation (no copy needed for this simplified version)
        auto eval_wafer = wafer;
        
        // Simulate the process with given parameters
        // This is a simplified evaluation - in practice, this would call the actual physics engines
        
        // Example objectives calculation (simplified)
        double uniformity = 95.0 + 5.0 * std::sin(parameters.begin()->second); // Mock uniformity
        double yield = 90.0 + 10.0 * std::cos(parameters.begin()->second);     // Mock yield
        double cost = 100.0 + 50.0 * parameters.begin()->second;               // Mock cost
        
        evaluation.objectives["uniformity"] = uniformity;
        evaluation.objectives["yield"] = yield;
        evaluation.objectives["cost"] = cost;
        
        // Check constraints
        evaluation.is_feasible = checkConstraints(parameters);
        
        // Calculate fitness
        evaluation.fitness_score = calculateFitness(evaluation.objectives, evaluation.constraints);
        
        auto end_time = std::chrono::steady_clock::now();
        evaluation.evaluation_time = std::chrono::duration<double>(end_time - start_time).count();
        
    } catch (const std::exception& e) {
        SEMIPRO_LOG_MODULE(LogLevel::ERROR, LogCategory::ADVANCED,
                          "Process evaluation failed: " + std::string(e.what()),
                          "ProcessOptimizer");
        evaluation.fitness_score = -std::numeric_limits<double>::infinity();
        evaluation.is_feasible = false;
    }
    
    return evaluation;
}

double ProcessOptimizer::calculateFitness(
    const std::unordered_map<std::string, double>& objectives,
    const std::unordered_map<std::string, double>& constraints) {
    
    double fitness = 0.0;
    
    // Combine objectives (simplified weighted sum)
    for (const auto& obj : objectives) {
        if (obj.first == "uniformity" || obj.first == "yield") {
            fitness += obj.second; // Higher is better
        } else if (obj.first == "cost") {
            fitness -= obj.second * 0.1; // Lower is better
        }
    }
    
    // Apply constraint penalties (pass the evaluation parameters, not the class member)
    // For now, skip constraint penalty calculation in this simplified version
    // double penalty = calculateConstraintPenalty(evaluation_parameters);
    // fitness -= penalty;
    
    return fitness;
}

GAPopulation ProcessOptimizer::initializePopulation(int population_size) {
    GAPopulation population;
    population.individuals.resize(population_size);
    
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (auto& individual : population.individuals) {
        individual.genes = generateRandomParameters();
        individual.is_evaluated = false;
    }
    
    return population;
}

void ProcessOptimizer::evaluatePopulation(
    GAPopulation& population,
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type) {
    
    double fitness_sum = 0.0;
    
    for (auto& individual : population.individuals) {
        if (!individual.is_evaluated) {
            auto params = decodeParameters(individual.genes);
            auto evaluation = evaluateProcess(wafer, process_type, params);
            individual.fitness = evaluation.fitness_score;
            individual.is_evaluated = true;
        }
        fitness_sum += individual.fitness;
    }
    
    population.average_fitness = fitness_sum / population.individuals.size();
    
    // Find best fitness
    auto best_it = std::max_element(population.individuals.begin(), population.individuals.end(),
        [](const GAIndividual& a, const GAIndividual& b) { return a.fitness < b.fitness; });
    
    if (best_it != population.individuals.end()) {
        population.best_fitness = best_it->fitness;
    }
}

std::vector<double> ProcessOptimizer::generateRandomParameters() {
    std::vector<double> params;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    for (const auto& param : parameters_) {
        if (param.second.is_active) {
            params.push_back(dist(random_generator_));
        }
    }
    
    return params;
}

std::unordered_map<std::string, double> ProcessOptimizer::decodeParameters(const std::vector<double>& encoded) {
    std::unordered_map<std::string, double> decoded;
    
    int index = 0;
    for (const auto& param : parameters_) {
        if (param.second.is_active && index < encoded.size()) {
            double normalized = encoded[index];
            double actual = param.second.min_value + 
                           normalized * (param.second.max_value - param.second.min_value);
            decoded[param.first] = actual;
            index++;
        }
    }
    
    return decoded;
}

bool ProcessOptimizer::checkConstraints(const std::unordered_map<std::string, double>& parameters) {
    for (const auto& constraint : constraints_) {
        auto param_it = parameters.find(constraint.second.parameter);
        if (param_it != parameters.end()) {
            double value = param_it->second;
            if (value < constraint.second.min_limit || value > constraint.second.max_limit) {
                if (constraint.second.is_hard_constraint) {
                    return false;
                }
            }
        }
    }
    return true;
}

double ProcessOptimizer::calculateConstraintPenalty(const std::unordered_map<std::string, double>& parameters) {
    double penalty = 0.0;
    
    for (const auto& constraint : constraints_) {
        auto param_it = parameters.find(constraint.second.parameter);
        if (param_it != parameters.end()) {
            double value = param_it->second;
            if (value < constraint.second.min_limit) {
                penalty += constraint.second.penalty_weight * (constraint.second.min_limit - value);
            } else if (value > constraint.second.max_limit) {
                penalty += constraint.second.penalty_weight * (value - constraint.second.max_limit);
            }
        }
    }
    
    return penalty;
}

void ProcessOptimizer::initializeOptimizer() {
    random_generator_.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    current_algorithm_ = OptimizationAlgorithm::GENETIC_ALGORITHM;
}

std::string ProcessOptimizer::algorithmToString(OptimizationAlgorithm algorithm) const {
    switch (algorithm) {
        case OptimizationAlgorithm::PARAMETER_SWEEP: return "Parameter Sweep";
        case OptimizationAlgorithm::GRID_SEARCH: return "Grid Search";
        case OptimizationAlgorithm::RANDOM_SEARCH: return "Random Search";
        case OptimizationAlgorithm::GENETIC_ALGORITHM: return "Genetic Algorithm";
        case OptimizationAlgorithm::PARTICLE_SWARM: return "Particle Swarm";
        case OptimizationAlgorithm::SIMULATED_ANNEALING: return "Simulated Annealing";
        case OptimizationAlgorithm::MULTI_OBJECTIVE_GA: return "Multi-Objective GA";
        default: return "Unknown";
    }
}

std::string ProcessOptimizer::parameterTypeToString(ParameterType type) const {
    switch (type) {
        case ParameterType::CONTINUOUS: return "Continuous";
        case ParameterType::DISCRETE: return "Discrete";
        case ParameterType::CATEGORICAL: return "Categorical";
        case ParameterType::BOOLEAN: return "Boolean";
        case ParameterType::INTEGER: return "Integer";
        case ParameterType::ENUMERATED: return "Enumerated";
        default: return "Unknown";
    }
}

GAPopulation ProcessOptimizer::selection(const GAPopulation& population) {
    GAPopulation selected;
    selected.individuals.reserve(population.individuals.size());

    if (ga_params_.use_tournament_selection) {
        // Tournament selection
        std::uniform_int_distribution<int> dist(0, population.individuals.size() - 1);

        for (size_t i = 0; i < population.individuals.size(); ++i) {
            GAIndividual best_candidate;
            double best_fitness = -std::numeric_limits<double>::infinity();

            for (int j = 0; j < ga_params_.tournament_size; ++j) {
                int candidate_idx = dist(random_generator_);
                const auto& candidate = population.individuals[candidate_idx];

                if (candidate.fitness > best_fitness) {
                    best_fitness = candidate.fitness;
                    best_candidate = candidate;
                }
            }

            selected.individuals.push_back(best_candidate);
        }
    } else {
        // Roulette wheel selection (simplified)
        selected.individuals = population.individuals;
    }

    return selected;
}

GAPopulation ProcessOptimizer::crossover(const GAPopulation& parents) {
    GAPopulation offspring;
    offspring.individuals.reserve(parents.individuals.size());

    std::uniform_real_distribution<double> crossover_dist(0.0, 1.0);
    std::uniform_int_distribution<int> parent_dist(0, parents.individuals.size() - 1);

    for (size_t i = 0; i < parents.individuals.size(); ++i) {
        if (crossover_dist(random_generator_) < ga_params_.crossover_rate) {
            // Perform crossover
            int parent1_idx = parent_dist(random_generator_);
            int parent2_idx = parent_dist(random_generator_);

            const auto& parent1 = parents.individuals[parent1_idx];
            const auto& parent2 = parents.individuals[parent2_idx];

            GAIndividual child;
            child.genes.resize(parent1.genes.size());

            // Single-point crossover
            std::uniform_int_distribution<int> point_dist(0, parent1.genes.size() - 1);
            int crossover_point = point_dist(random_generator_);

            for (size_t j = 0; j < parent1.genes.size(); ++j) {
                if (j < crossover_point) {
                    child.genes[j] = parent1.genes[j];
                } else {
                    child.genes[j] = parent2.genes[j];
                }
            }

            child.is_evaluated = false;
            offspring.individuals.push_back(child);
        } else {
            // No crossover, copy parent
            offspring.individuals.push_back(parents.individuals[i]);
        }
    }

    return offspring;
}

GAPopulation ProcessOptimizer::mutation(const GAPopulation& offspring) {
    GAPopulation mutated = offspring;

    std::uniform_real_distribution<double> mutation_dist(0.0, 1.0);
    std::normal_distribution<double> gaussian_dist(0.0, 0.1); // Gaussian mutation

    for (auto& individual : mutated.individuals) {
        for (auto& gene : individual.genes) {
            if (mutation_dist(random_generator_) < ga_params_.mutation_rate) {
                // Apply Gaussian mutation
                gene += gaussian_dist(random_generator_);
                gene = std::max(0.0, std::min(1.0, gene)); // Clamp to [0, 1]
                individual.is_evaluated = false;
            }
        }
    }

    return mutated;
}

ProcessOptimizationResults ProcessOptimizer::particleSwarmOptimization(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type,
    const std::vector<OptimizationObjective>& objectives) {

    ProcessOptimizationResults results;

    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Particle Swarm Optimization not fully implemented - using GA instead",
                      "ProcessOptimizer");

    // For now, fall back to genetic algorithm
    return geneticAlgorithmOptimization(wafer, process_type, objectives);
}

ProcessOptimizationResults ProcessOptimizer::simulatedAnnealingOptimization(
    std::shared_ptr<WaferEnhanced> wafer,
    const std::string& process_type,
    OptimizationObjective objective) {

    ProcessOptimizationResults results;

    SEMIPRO_LOG_MODULE(LogLevel::INFO, LogCategory::ADVANCED,
                      "Simulated Annealing not fully implemented - using GA instead",
                      "ProcessOptimizer");

    // For now, fall back to genetic algorithm
    std::vector<OptimizationObjective> objectives = {objective};
    return geneticAlgorithmOptimization(wafer, process_type, objectives);
}

std::vector<std::string> ProcessOptimizer::generateRecommendations(
    const ProcessOptimizationResults& results) const {

    std::vector<std::string> recommendations;

    if (results.evaluations.empty()) {
        recommendations.push_back("No evaluations performed - check optimization setup");
        return recommendations;
    }

    recommendations.push_back("Optimization completed successfully");

    if (results.has_converged) {
        recommendations.push_back("Algorithm converged to optimal solution");
    } else {
        recommendations.push_back("Consider increasing maximum evaluations for better convergence");
    }

    // Analyze best solution
    if (results.best_solution.fitness_score > 0) {
        recommendations.push_back("Good solution found with positive fitness score");
    } else {
        recommendations.push_back("Consider adjusting parameter ranges or constraints");
    }

    // Parameter-specific recommendations
    for (const auto& param : results.best_solution.parameters) {
        recommendations.push_back("Optimal " + param.first + ": " + std::to_string(param.second));
    }

    return recommendations;
}

std::unordered_map<std::string, double> ProcessOptimizer::analyzeOptimizationResults(
    const ProcessOptimizationResults& results) const {

    std::unordered_map<std::string, double> statistics;

    if (results.evaluations.empty()) {
        return statistics;
    }

    // Calculate basic statistics
    std::vector<double> fitness_values;
    for (const auto& eval : results.evaluations) {
        fitness_values.push_back(eval.fitness_score);
    }

    statistics["num_evaluations"] = static_cast<double>(results.evaluations.size());
    statistics["best_fitness"] = results.best_solution.fitness_score;
    statistics["mean_fitness"] = OptimizationUtils::calculateMean(fitness_values);
    statistics["std_fitness"] = OptimizationUtils::calculateStandardDeviation(fitness_values);
    statistics["total_time"] = results.total_time;
    statistics["convergence_rate"] = results.has_converged ? 1.0 : 0.0;

    return statistics;
}

bool ProcessOptimizer::validateOptimizationSetup(std::string& error_message) const {
    if (parameters_.empty()) {
        error_message = "No optimization parameters defined";
        return false;
    }

    bool has_active_params = false;
    for (const auto& param : parameters_) {
        if (param.second.is_active) {
            has_active_params = true;

            if (param.second.min_value >= param.second.max_value) {
                error_message = "Invalid parameter range for " + param.first;
                return false;
            }
        }
    }

    if (!has_active_params) {
        error_message = "No active optimization parameters";
        return false;
    }

    return true;
}

std::vector<std::string> ProcessOptimizer::getDiagnostics() const {
    std::vector<std::string> diagnostics;

    diagnostics.push_back("Process Optimizer Status:");
    diagnostics.push_back("  Algorithm: " + algorithmToString(current_algorithm_));
    diagnostics.push_back("  Parameters: " + std::to_string(parameters_.size()));
    diagnostics.push_back("  Constraints: " + std::to_string(constraints_.size()));
    diagnostics.push_back("  Objectives: " + std::to_string(objectives_.size()));

    diagnostics.push_back("GA Parameters:");
    diagnostics.push_back("  Population Size: " + std::to_string(ga_params_.population_size));
    diagnostics.push_back("  Max Generations: " + std::to_string(ga_params_.max_generations));
    diagnostics.push_back("  Mutation Rate: " + std::to_string(ga_params_.mutation_rate));
    diagnostics.push_back("  Crossover Rate: " + std::to_string(ga_params_.crossover_rate));

    diagnostics.push_back("Configuration:");
    diagnostics.push_back("  Parallel Evaluation: " + std::string(enable_parallel_evaluation_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Adaptive Parameters: " + std::string(enable_adaptive_parameters_ ? "Enabled" : "Disabled"));
    diagnostics.push_back("  Constraint Handling: " + std::string(enable_constraint_handling_ ? "Enabled" : "Disabled"));

    return diagnostics;
}

// Utility functions implementation
namespace OptimizationUtils {

    double calculateMean(const std::vector<double>& values) {
        if (values.empty()) return 0.0;
        return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    }

    double calculateStandardDeviation(const std::vector<double>& values) {
        if (values.size() < 2) return 0.0;

        double mean = calculateMean(values);
        double variance = 0.0;

        for (double value : values) {
            variance += (value - mean) * (value - mean);
        }
        variance /= (values.size() - 1);

        return std::sqrt(variance);
    }

    double calculateCorrelation(const std::vector<double>& x, const std::vector<double>& y) {
        if (x.size() != y.size() || x.size() < 2) return 0.0;

        double mean_x = calculateMean(x);
        double mean_y = calculateMean(y);

        double numerator = 0.0;
        double sum_sq_x = 0.0;
        double sum_sq_y = 0.0;

        for (size_t i = 0; i < x.size(); ++i) {
            double dx = x[i] - mean_x;
            double dy = y[i] - mean_y;
            numerator += dx * dy;
            sum_sq_x += dx * dx;
            sum_sq_y += dy * dy;
        }

        double denominator = std::sqrt(sum_sq_x * sum_sq_y);
        return (denominator > 0.0) ? numerator / denominator : 0.0;
    }

} // namespace OptimizationUtils

} // namespace SemiPRO

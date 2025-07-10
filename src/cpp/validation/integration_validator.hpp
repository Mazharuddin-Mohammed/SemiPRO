#pragma once

#include "../core/advanced_logger.hpp"
#include "../core/enhanced_error_handling.hpp"
#include "../core/config_manager.hpp"
#include "../core/wafer_enhanced.hpp"
#include "../core/performance_utils.hpp"
#include "../physics/enhanced_oxidation.hpp"
#include "../physics/enhanced_deposition.hpp"
#include "../physics/enhanced_doping.hpp"
#include "../physics/enhanced_etching.hpp"
#include "../advanced/multi_layer_engine.hpp"
#include "../advanced/temperature_controller.hpp"
#include "../advanced/process_optimizer.hpp"
#include "../advanced/process_integrator.hpp"
#include "../ui/visualization_engine.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>

namespace SemiPRO {

/**
 * Comprehensive Integration & Validation System
 * Implements system-wide testing, performance benchmarking, and scientific validation
 */

// Validation test types
enum class ValidationTestType {
    UNIT_TEST,              // Individual component testing
    INTEGRATION_TEST,       // Component integration testing
    SYSTEM_TEST,            // Full system testing
    PERFORMANCE_TEST,       // Performance benchmarking
    STRESS_TEST,            // Stress and load testing
    REGRESSION_TEST,        // Regression testing
    SCIENTIFIC_VALIDATION,  // Scientific accuracy validation
    MEMORY_TEST,            // Memory leak and usage testing
    CONCURRENCY_TEST,       // Multi-threading testing
    COMPATIBILITY_TEST      // Cross-platform compatibility
};

// Test execution status
enum class TestStatus {
    NOT_STARTED,            // Test not yet started
    RUNNING,                // Test currently running
    PASSED,                 // Test passed successfully
    FAILED,                 // Test failed
    SKIPPED,                // Test skipped
    TIMEOUT,                // Test timed out
    ERROR                   // Test encountered error
};

// Use PerformanceMetrics from advanced_logger.hpp to avoid duplication
// Additional validation-specific metrics
struct ValidationMetrics {
    double execution_time;          // Total execution time (seconds)
    double cpu_usage;               // CPU usage percentage
    size_t memory_usage;            // Memory usage (bytes)
    size_t peak_memory;             // Peak memory usage (bytes)
    double throughput;              // Operations per second
    int cache_hits;                 // Cache hit count
    int cache_misses;               // Cache miss count
    double accuracy;                // Numerical accuracy

    ValidationMetrics() : execution_time(0.0), cpu_usage(0.0), memory_usage(0),
                         peak_memory(0), throughput(0.0), cache_hits(0),
                         cache_misses(0), accuracy(0.0) {}
};

// Benchmark configuration
struct BenchmarkConfig {
    int num_iterations;             // Number of benchmark iterations
    int num_threads;                // Number of threads to use
    bool enable_profiling;          // Enable detailed profiling
    bool enable_memory_tracking;    // Enable memory tracking
    double timeout_seconds;         // Benchmark timeout
    std::vector<std::string> metrics_to_collect; // Metrics to collect
    
    BenchmarkConfig() : num_iterations(100), num_threads(1), enable_profiling(true),
                       enable_memory_tracking(true), timeout_seconds(300.0) {}
};

// Validation test result
struct ValidationTestResult {
    std::string test_id;            // Unique test identifier
    std::string test_name;          // Human-readable test name
    ValidationTestType test_type;   // Type of test
    TestStatus status;              // Test execution status
    double execution_time;          // Test execution time (seconds)
    ValidationMetrics metrics;      // Validation metrics
    std::vector<std::string> errors; // Error messages
    std::vector<std::string> warnings; // Warning messages
    std::unordered_map<std::string, double> results; // Test results
    std::unordered_map<std::string, std::string> metadata; // Test metadata
    
    ValidationTestResult() : test_type(ValidationTestType::UNIT_TEST),
                            status(TestStatus::NOT_STARTED), execution_time(0.0) {}
};

// Validation suite results
struct ValidationSuiteResults {
    std::string suite_id;           // Suite identifier
    std::string suite_name;         // Suite name
    std::vector<ValidationTestResult> test_results; // Individual test results
    int total_tests;                // Total number of tests
    int passed_tests;               // Number of passed tests
    int failed_tests;               // Number of failed tests
    int skipped_tests;              // Number of skipped tests
    double total_execution_time;    // Total execution time
    ValidationMetrics overall_metrics; // Overall validation metrics
    std::vector<std::string> recommendations; // Validation recommendations
    
    ValidationSuiteResults() : total_tests(0), passed_tests(0), failed_tests(0),
                              skipped_tests(0), total_execution_time(0.0) {}
};

// Scientific validation data
struct ScientificValidationData {
    std::string experiment_name;    // Experiment name
    std::vector<double> experimental_data; // Experimental reference data
    std::vector<double> simulation_data;   // Simulation results
    double correlation_coefficient; // Correlation with experimental data
    double mean_absolute_error;     // Mean absolute error
    double root_mean_square_error;  // Root mean square error
    double relative_error;          // Relative error percentage
    bool validation_passed;         // Whether validation passed
    
    ScientificValidationData() : correlation_coefficient(0.0), mean_absolute_error(0.0),
                                root_mean_square_error(0.0), relative_error(0.0),
                                validation_passed(false) {}
};

// Integration validator class
class IntegrationValidator {
private:
    std::unordered_map<std::string, ValidationTestResult> test_results_;
    std::unordered_map<std::string, ValidationSuiteResults> suite_results_;
    std::unordered_map<std::string, ScientificValidationData> scientific_data_;
    
    // Test configuration
    BenchmarkConfig benchmark_config_;
    bool enable_detailed_logging_ = true;
    bool enable_performance_profiling_ = true;
    bool enable_memory_validation_ = true;
    double validation_tolerance_ = 1e-6;
    
    // Component references
    std::unique_ptr<MultiLayerEngine> multi_layer_engine_;
    std::unique_ptr<AdvancedTemperatureController> temperature_controller_;
    std::unique_ptr<ProcessOptimizer> process_optimizer_;
    std::unique_ptr<ProcessIntegrator> process_integrator_;
    std::unique_ptr<VisualizationEngine> visualization_engine_;
    
public:
    IntegrationValidator();
    ~IntegrationValidator() = default;
    
    // Test suite execution
    ValidationSuiteResults runFullValidationSuite();
    ValidationSuiteResults runUnitTestSuite();
    ValidationSuiteResults runIntegrationTestSuite();
    ValidationSuiteResults runPerformanceBenchmarks();
    ValidationSuiteResults runScientificValidation();
    ValidationSuiteResults runStressTests();
    
    // Individual test categories
    ValidationTestResult testPhysicsEngines();
    ValidationTestResult testMultiLayerProcessing();
    ValidationTestResult testTemperatureControl();
    ValidationTestResult testProcessOptimization();
    ValidationTestResult testProcessIntegration();
    ValidationTestResult testVisualizationEngine();
    
    // Performance benchmarking
    ValidationTestResult benchmarkOxidationPerformance();
    ValidationTestResult benchmarkDepositionPerformance();
    ValidationTestResult benchmarkDopingPerformance();
    ValidationTestResult benchmarkEtchingPerformance();
    ValidationTestResult benchmarkSystemThroughput();
    
    // Scientific validation
    ScientificValidationData validateOxidationKinetics();
    ScientificValidationData validateDopingProfiles();
    ScientificValidationData validateDepositionRates();
    ScientificValidationData validateEtchingSelectivity();
    
    // Memory and resource validation
    ValidationTestResult validateMemoryUsage();
    ValidationTestResult validateResourceLeaks();
    ValidationTestResult validateThreadSafety();
    
    // Cross-component integration tests
    ValidationTestResult testPhysicsIntegration();
    ValidationTestResult testAdvancedFeaturesIntegration();
    ValidationTestResult testVisualizationIntegration();
    ValidationTestResult testEndToEndWorkflow();
    
    // Regression testing
    ValidationTestResult runRegressionTests();
    ValidationTestResult compareWithBaseline();
    
    // Configuration and setup
    void setBenchmarkConfig(const BenchmarkConfig& config);
    void setValidationTolerance(double tolerance);
    void enableDetailedLogging(bool enable);
    void enablePerformanceProfiling(bool enable);
    void enableMemoryValidation(bool enable);
    
    // Results analysis
    std::unordered_map<std::string, double> analyzePerformanceMetrics(
        const ValidationSuiteResults& results
    ) const;
    
    std::vector<std::string> generateValidationReport(
        const ValidationSuiteResults& results
    ) const;
    
    std::vector<std::string> identifyPerformanceBottlenecks(
        const ValidationSuiteResults& results
    ) const;
    
    // Data management
    void addTestResult(const std::string& test_id, const ValidationTestResult& result);
    ValidationTestResult getTestResult(const std::string& test_id) const;
    std::vector<std::string> getAvailableTests() const;
    
    // Utility functions
    bool validateNumericalAccuracy(
        const std::vector<double>& expected,
        const std::vector<double>& actual,
        double tolerance = 1e-6
    ) const;
    
    ValidationMetrics measurePerformance(
        std::function<void()> test_function,
        int iterations = 1
    ) const;
    
    double calculateCorrelation(
        const std::vector<double>& x,
        const std::vector<double>& y
    ) const;
    
    // Diagnostics and reporting
    std::vector<std::string> getDiagnostics() const;
    std::string generatePerformanceReport() const;
    std::string generateValidationSummary() const;
    
    // Export and import
    bool exportResults(const std::string& file_path) const;
    bool importBaseline(const std::string& file_path);
    
private:
    void initializeValidator();
    
    // Test execution helpers
    ValidationTestResult executeTest(
        const std::string& test_name,
        std::function<bool()> test_function,
        ValidationTestType test_type = ValidationTestType::UNIT_TEST
    );
    
    // Performance measurement helpers
    std::chrono::steady_clock::time_point startTimer() const;
    double stopTimer(const std::chrono::steady_clock::time_point& start) const;
    size_t getCurrentMemoryUsage() const;
    double getCurrentCPUUsage() const;
    
    // Scientific validation helpers
    double calculateMeanAbsoluteError(
        const std::vector<double>& expected,
        const std::vector<double>& actual
    ) const;
    
    double calculateRootMeanSquareError(
        const std::vector<double>& expected,
        const std::vector<double>& actual
    ) const;
    
    // Test data generation
    std::vector<double> generateTestData(const std::string& data_type, int size) const;
    std::shared_ptr<WaferEnhanced> createTestWafer() const;
    
    // Stress testing helpers
    ValidationTestResult stressTestMemory();
    ValidationTestResult stressTestCPU();
    ValidationTestResult stressTestConcurrency();
};

// Utility functions for validation
namespace ValidationUtils {
    // Statistical analysis
    double calculateMean(const std::vector<double>& data);
    double calculateStandardDeviation(const std::vector<double>& data);
    double calculatePercentile(const std::vector<double>& data, double percentile);
    
    // Performance analysis
    std::unordered_map<std::string, double> analyzePerformanceTrends(
        const std::vector<PerformanceMetrics>& metrics
    );
    
    std::vector<std::string> identifyAnomalies(
        const std::vector<PerformanceMetrics>& metrics
    );
    
    // Validation utilities
    bool isWithinTolerance(double expected, double actual, double tolerance);
    double calculateRelativeError(double expected, double actual);
    
    // Benchmark utilities
    std::vector<double> generateBenchmarkWorkload(int size);
    ValidationMetrics aggregateMetrics(const std::vector<ValidationMetrics>& metrics);
    
    // Report generation
    std::string formatPerformanceMetrics(const ValidationMetrics& metrics);
    std::string formatTestResults(const ValidationTestResult& result);
    std::string formatSuiteResults(const ValidationSuiteResults& results);
}

} // namespace SemiPRO

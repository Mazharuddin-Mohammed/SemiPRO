#ifndef WAFER_ENHANCED_HPP
#define WAFER_ENHANCED_HPP

#include "wafer.hpp"
#include "memory_manager.hpp"
#include "profiler.hpp"
#include <unordered_set>
#include <atomic>
#include <thread>

// Enhanced wafer with better performance and features
class WaferEnhanced : public Wafer {
public:
    WaferEnhanced(double diameter, double thickness, const std::string& material);
    virtual ~WaferEnhanced() = default;

    // Enhanced grid operations with parallel processing
    void initializeGridParallel(int rows, int cols);
    void updateGridParallel(const Eigen::ArrayXXd& new_grid);
    
    // Multi-layer support
    struct Layer {
        std::string material;
        double thickness;
        Eigen::ArrayXXd composition;
        std::unordered_map<std::string, double> properties;
        
        Layer(const std::string& mat, double thick, int rows, int cols) 
            : material(mat), thickness(thick), composition(Eigen::ArrayXXd::Zero(rows, cols)) {}
    };
    
    void addLayer(const std::string& material, double thickness);
    void removeLayer(size_t index);
    const std::vector<Layer>& getLayers() const { return layers_; }
    
    // Advanced doping profiles
    void setDopantProfileMultiLayer(const std::vector<Eigen::ArrayXXd>& profiles);
    Eigen::ArrayXXd getDopantProfileAtDepth(double depth) const;
    
    // Stress and strain analysis
    void calculateStress();
    void calculateStrain();
    const Eigen::ArrayXXd& getStressField() const { return stress_field_; }
    const Eigen::ArrayXXd& getStrainField() const { return strain_field_; }
    
    // Defect tracking
    struct Defect {
        enum Type { VACANCY, INTERSTITIAL, DISLOCATION, GRAIN_BOUNDARY };
        Type type;
        double x, y, z;  // Position
        double energy;   // Formation energy
        
        Defect(Type t, double px, double py, double pz, double e) 
            : type(t), x(px), y(py), z(pz), energy(e) {}
    };
    
    void addDefect(const Defect& defect);
    void removeDefect(size_t index);
    const std::vector<Defect>& getDefects() const { return defects_; }
    
    // Crystal structure information
    enum CrystalStructure { DIAMOND, FCC, BCC, HCP };
    void setCrystalStructure(CrystalStructure structure) { crystal_structure_ = structure; }
    CrystalStructure getCrystalStructure() const { return crystal_structure_; }
    
    // Temperature distribution
    void setTemperatureField(const Eigen::ArrayXXd& temperature);
    const Eigen::ArrayXXd& getTemperatureField() const { return temperature_field_; }
    
    // Process history tracking
    struct ProcessStep {
        std::string operation;
        std::chrono::system_clock::time_point timestamp;
        std::unordered_map<std::string, double> parameters;
        
        ProcessStep(const std::string& op) 
            : operation(op), timestamp(std::chrono::system_clock::now()) {}
    };
    
    void recordProcessStep(const ProcessStep& step);
    const std::vector<ProcessStep>& getProcessHistory() const { return process_history_; }
    
    // Validation and integrity checks
    bool validateIntegrity() const;
    std::vector<std::string> getValidationErrors() const;
    
    // Serialization
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    
    // Performance monitoring
    void enableProfiling(bool enable) { profiling_enabled_ = enable; }
    bool isProfilingEnabled() const { return profiling_enabled_; }
    
private:
    std::vector<Layer> layers_;
    std::vector<Defect> defects_;
    std::vector<ProcessStep> process_history_;
    
    Eigen::ArrayXXd stress_field_;
    Eigen::ArrayXXd strain_field_;
    Eigen::ArrayXXd temperature_field_;
    
    CrystalStructure crystal_structure_ = DIAMOND;
    
    // Thread safety
    mutable std::mutex data_mutex_;
    std::atomic<bool> profiling_enabled_{false};
    
    // Helper functions
    void updateStressFromLayers();
    void updateStrainFromStress();
    double calculateLayerStress(const Layer& layer) const;
    
    // Parallel processing helpers
    void processGridChunk(int start_row, int end_row, int start_col, int end_col,
                         std::function<void(int, int)> operation);
};

#endif // WAFER_ENHANCED_HPP

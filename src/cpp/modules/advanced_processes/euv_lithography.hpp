// Author: Dr. Mazharuddin Mohammed
#ifndef EUV_LITHOGRAPHY_HPP
#define EUV_LITHOGRAPHY_HPP

#include "../core/wafer.hpp"
#include <memory>
#include <vector>
#include <complex>
#include <Eigen/Dense>

/**
 * Advanced EUV (Extreme Ultraviolet) Lithography Simulation
 * Implements state-of-the-art 13.5nm wavelength lithography for sub-10nm nodes
 */
class EUVLithography {
public:
    EUVLithography();
    ~EUVLithography() = default;
    
    // EUV exposure simulation
    bool simulateEUVExposure(std::shared_ptr<Wafer> wafer, 
                            const std::vector<std::vector<int>>& mask_pattern,
                            double numerical_aperture = 0.33,
                            double dose = 20.0,  // mJ/cm²
                            double defocus = 0.0); // nm
    
    // Advanced EUV features
    void setMultipleExposure(bool enable, int exposures = 4);
    void enableStochasticEffects(bool enable);
    void setResistParameters(double thickness, double sensitivity, const std::string& type);
    void enableOPCCorrection(bool enable);
    
    // EUV-specific calculations
    double calculateResolution(double numerical_aperture) const;
    double calculateDepthOfFocus(double numerical_aperture) const;
    Eigen::ArrayXXd simulateStochasticEffects(const Eigen::ArrayXXd& aerial_image) const;
    
    // Performance metrics
    struct EUVMetrics {
        double resolution;
        double depth_of_focus;
        double line_edge_roughness;
        double critical_dimension_uniformity;
        double stochastic_defect_density;
        double throughput; // wafers per hour
    };
    
    EUVMetrics getPerformanceMetrics() const { return metrics_; }

private:
    // EUV wavelength (13.5 nm)
    static constexpr double EUV_WAVELENGTH = 13.5e-9; // meters
    
    // EUV simulation parameters
    bool multiple_exposure_enabled_;
    int exposure_count_;
    bool stochastic_effects_enabled_;
    bool opc_correction_enabled_;
    
    // Resist parameters
    double resist_thickness_;
    double resist_sensitivity_;
    std::string resist_type_;
    
    // Performance tracking
    mutable EUVMetrics metrics_;
    
    // Advanced simulation methods
    Eigen::ArrayXXd calculateEUVAerialImage(const std::vector<std::vector<int>>& mask,
                                           double na, double defocus, int rows, int cols) const;
    Eigen::ArrayXXd simulateResistResponse(const Eigen::ArrayXXd& aerial_image, double dose) const;
    Eigen::ArrayXXd applyOPCCorrection(const Eigen::ArrayXXd& pattern) const;
    Eigen::ArrayXXd simulateMultipleExposure(const std::vector<std::vector<int>>& mask,
                                           double na, double dose, int rows, int cols) const;
    
    // EUV-specific physics
    double calculateAbsorption(const std::string& material, double thickness) const;
    double calculateScattering(double feature_size) const;
    std::complex<double> calculateComplexRefractiveIndex(const std::string& material) const;
    
    // Stochastic modeling
    double calculatePhotonShotNoise(double dose, double area) const;
    double calculateLineEdgeRoughness(const Eigen::ArrayXXd& pattern) const;
    double calculateCriticalDimensionUniformity(const Eigen::ArrayXXd& pattern) const;
    
    // Performance optimization
    void updateMetrics(const Eigen::ArrayXXd& final_pattern) const;
};

// Factory functions for different EUV systems
std::unique_ptr<EUVLithography> createASMLTwinscan(double numerical_aperture = 0.33);
std::unique_ptr<EUVLithography> createCanonFPA(double numerical_aperture = 0.30);

// Utility functions
bool validateEUVMask(const std::vector<std::vector<int>>& mask);
double calculateEUVThroughput(double dose, double field_size, double scan_speed);
std::vector<std::vector<int>> generateEUVTestPattern(int size, double feature_size);

#endif // EUV_LITHOGRAPHY_HPP

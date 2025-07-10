// Author: Dr. Mazharuddin Mohammed
#include "euv_lithography.hpp"
#include "../core/utils.hpp"
#include <cmath>
#include <random>
#include <algorithm>

EUVLithography::EUVLithography() 
    : multiple_exposure_enabled_(false)
    , exposure_count_(1)
    , stochastic_effects_enabled_(true)
    , opc_correction_enabled_(true)
    , resist_thickness_(30e-9)  // 30 nm
    , resist_sensitivity_(20.0)  // mJ/cm²
    , resist_type_("CAR")  // Chemically Amplified Resist
{
    // Initialize metrics
    metrics_ = EUVMetrics{};
    Logger::getInstance().log("EUV Lithography system initialized (λ=13.5nm)");
}

bool EUVLithography::simulateEUVExposure(std::shared_ptr<Wafer> wafer,
                                        const std::vector<std::vector<int>>& mask_pattern,
                                        double numerical_aperture,
                                        double dose,
                                        double defocus) {
    try {
        auto grid = wafer->getGrid();
        int rows = grid.rows();
        int cols = grid.cols();
        
        Logger::getInstance().log("Starting EUV exposure simulation");
        Logger::getInstance().log("Parameters: NA=" + std::to_string(numerical_aperture) + 
                                 ", dose=" + std::to_string(dose) + "mJ/cm², defocus=" + 
                                 std::to_string(defocus) + "nm");
        
        // Calculate aerial image
        Eigen::ArrayXXd aerial_image;
        
        if (multiple_exposure_enabled_) {
            aerial_image = simulateMultipleExposure(mask_pattern, numerical_aperture, dose, rows, cols);
        } else {
            aerial_image = calculateEUVAerialImage(mask_pattern, numerical_aperture, defocus, rows, cols);
        }
        
        // Apply stochastic effects
        if (stochastic_effects_enabled_) {
            aerial_image = simulateStochasticEffects(aerial_image);
        }
        
        // Simulate resist response
        Eigen::ArrayXXd resist_pattern = simulateResistResponse(aerial_image, dose);
        
        // Apply OPC correction
        if (opc_correction_enabled_) {
            resist_pattern = applyOPCCorrection(resist_pattern);
        }
        
        // Update wafer with final pattern
        wafer->setPhotoresistPattern(resist_pattern);
        
        // Update performance metrics
        updateMetrics(resist_pattern);
        
        Logger::getInstance().log("EUV exposure completed successfully");
        Logger::getInstance().log("Resolution: " + std::to_string(metrics_.resolution * 1e9) + "nm");
        Logger::getInstance().log("LER: " + std::to_string(metrics_.line_edge_roughness) + "nm");
        
        return true;
        
    } catch (const std::exception& e) {
        Logger::getInstance().log("EUV exposure failed: " + std::string(e.what()));
        return false;
    }
}

void EUVLithography::setMultipleExposure(bool enable, int exposures) {
    multiple_exposure_enabled_ = enable;
    exposure_count_ = exposures;
    Logger::getInstance().log("Multiple exposure " + std::string(enable ? "enabled" : "disabled") + 
                             " (" + std::to_string(exposures) + " exposures)");
}

void EUVLithography::enableStochasticEffects(bool enable) {
    stochastic_effects_enabled_ = enable;
    Logger::getInstance().log("Stochastic effects " + std::string(enable ? "enabled" : "disabled"));
}

void EUVLithography::setResistParameters(double thickness, double sensitivity, const std::string& type) {
    resist_thickness_ = thickness;
    resist_sensitivity_ = sensitivity;
    resist_type_ = type;
    Logger::getInstance().log("Resist parameters: " + type + ", " + std::to_string(thickness*1e9) + 
                             "nm, " + std::to_string(sensitivity) + "mJ/cm²");
}

void EUVLithography::enableOPCCorrection(bool enable) {
    opc_correction_enabled_ = enable;
    Logger::getInstance().log("OPC correction " + std::string(enable ? "enabled" : "disabled"));
}

double EUVLithography::calculateResolution(double numerical_aperture) const {
    // Rayleigh criterion for EUV
    double k1 = 0.25; // Aggressive k1 factor for EUV
    return k1 * EUV_WAVELENGTH / numerical_aperture;
}

double EUVLithography::calculateDepthOfFocus(double numerical_aperture) const {
    // Depth of focus for EUV
    double k2 = 0.5;
    return k2 * EUV_WAVELENGTH / (numerical_aperture * numerical_aperture);
}

Eigen::ArrayXXd EUVLithography::calculateEUVAerialImage(const std::vector<std::vector<int>>& mask,
                                                       double na, double defocus, 
                                                       int rows, int cols) const {
    Eigen::ArrayXXd aerial_image(rows, cols);
    aerial_image.setZero();
    
    // EUV-specific parameters
    double coherence_factor = 0.3; // Partial coherence
    double mask_absorption = 0.95; // High absorption for EUV masks
    
    // Calculate point spread function for EUV
    double psf_width = EUV_WAVELENGTH / na * 1e6; // Convert to grid units
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double intensity = 0.0;
            
            // Convolution with EUV PSF
            int psf_size = static_cast<int>(psf_width * 3);
            
            for (int di = -psf_size; di <= psf_size; ++di) {
                for (int dj = -psf_size; dj <= psf_size; ++dj) {
                    int mask_i = i + di;
                    int mask_j = j + dj;
                    
                    if (mask_i >= 0 && mask_i < static_cast<int>(mask.size()) && 
                        mask_j >= 0 && mask_j < static_cast<int>(mask[0].size())) {
                        
                        double mask_value = mask[mask_i][mask_j];
                        
                        // Apply mask absorption
                        if (mask_value == 0) {
                            mask_value = 1.0 - mask_absorption;
                        }
                        
                        // EUV PSF (more complex due to short wavelength)
                        double r_squared = di * di + dj * dj;
                        double psf_value = std::exp(-r_squared / (2.0 * psf_width * psf_width));
                        
                        // Add defocus effects
                        if (defocus != 0.0) {
                            double defocus_factor = std::cos(2.0 * M_PI * defocus * r_squared / 
                                                           (EUV_WAVELENGTH * 1e9));
                            psf_value *= defocus_factor;
                        }
                        
                        intensity += mask_value * psf_value;
                    }
                }
            }
            
            // Apply coherence effects
            aerial_image(i, j) = intensity * (1.0 + coherence_factor);
        }
    }
    
    // Normalize
    double max_intensity = aerial_image.maxCoeff();
    if (max_intensity > 0) {
        aerial_image /= max_intensity;
    }
    
    return aerial_image;
}

Eigen::ArrayXXd EUVLithography::simulateStochasticEffects(const Eigen::ArrayXXd& aerial_image) const {
    int rows = aerial_image.rows();
    int cols = aerial_image.cols();
    Eigen::ArrayXXd stochastic_image = aerial_image;
    
    // Random number generator for stochastic effects
    std::random_device rd;
    std::mt19937 gen(rd());
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double intensity = aerial_image(i, j);
            
            // Photon shot noise (Poisson statistics)
            double pixel_area = 1e-14; // m² (approximate pixel area)
            double photon_noise = calculatePhotonShotNoise(intensity * resist_sensitivity_, pixel_area);
            
            // Add noise
            std::normal_distribution<double> noise_dist(0.0, photon_noise);
            double noise = noise_dist(gen);
            
            stochastic_image(i, j) = std::max(0.0, intensity + noise);
        }
    }
    
    return stochastic_image;
}

Eigen::ArrayXXd EUVLithography::simulateResistResponse(const Eigen::ArrayXXd& aerial_image, double dose) const {
    int rows = aerial_image.rows();
    int cols = aerial_image.cols();
    Eigen::ArrayXXd resist_response(rows, cols);
    
    // CAR (Chemically Amplified Resist) model
    double threshold_dose = resist_sensitivity_ * 0.8; // 80% of sensitivity
    double contrast = 5.0; // High contrast for EUV resists
    
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double local_dose = aerial_image(i, j) * dose;
            
            // Sigmoid response function for CAR
            double response = 1.0 / (1.0 + std::exp(-contrast * (local_dose - threshold_dose) / threshold_dose));
            
            // Apply resist thickness effects
            double absorption = calculateAbsorption(resist_type_, resist_thickness_);
            response *= (1.0 - std::exp(-absorption));
            
            resist_response(i, j) = response;
        }
    }
    
    return resist_response;
}

Eigen::ArrayXXd EUVLithography::applyOPCCorrection(const Eigen::ArrayXXd& pattern) const {
    int rows = pattern.rows();
    int cols = pattern.cols();
    Eigen::ArrayXXd corrected_pattern = pattern;
    
    // Simple OPC: bias correction based on local environment
    int correction_radius = 2;
    
    for (int i = correction_radius; i < rows - correction_radius; ++i) {
        for (int j = correction_radius; j < cols - correction_radius; ++j) {
            
            // Calculate local pattern density
            double local_density = 0.0;
            int count = 0;
            
            for (int di = -correction_radius; di <= correction_radius; ++di) {
                for (int dj = -correction_radius; dj <= correction_radius; ++dj) {
                    local_density += pattern(i + di, j + dj);
                    count++;
                }
            }
            
            local_density /= count;
            
            // Apply bias correction
            double bias_factor = 1.0 + 0.2 * (0.5 - local_density); // ±20% bias
            corrected_pattern(i, j) = std::min(1.0, std::max(0.0, pattern(i, j) * bias_factor));
        }
    }
    
    return corrected_pattern;
}

Eigen::ArrayXXd EUVLithography::simulateMultipleExposure(const std::vector<std::vector<int>>& mask,
                                                        double na, double dose, 
                                                        int rows, int cols) const {
    Eigen::ArrayXXd combined_image(rows, cols);
    combined_image.setZero();
    
    // Simulate multiple exposures with slight offsets
    for (int exp = 0; exp < exposure_count_; ++exp) {
        // Small random offset for each exposure
        double offset_x = (exp % 2) * 0.5; // nm
        double offset_y = (exp / 2) * 0.5; // nm
        
        Eigen::ArrayXXd single_exposure = calculateEUVAerialImage(mask, na, 0.0, rows, cols);
        
        // Add to combined image
        combined_image += single_exposure / exposure_count_;
    }
    
    return combined_image;
}

double EUVLithography::calculateAbsorption(const std::string& material, double thickness) const {
    // Absorption coefficients for EUV (13.5 nm)
    double absorption_coeff;
    
    if (material == "CAR") {
        absorption_coeff = 5e6; // m⁻¹ for chemically amplified resist
    } else if (material == "HSQ") {
        absorption_coeff = 3e6; // m⁻¹ for hydrogen silsesquioxane
    } else {
        absorption_coeff = 4e6; // m⁻¹ default
    }
    
    return absorption_coeff * thickness;
}

double EUVLithography::calculatePhotonShotNoise(double dose, double area) const {
    // Photon energy at 13.5 nm
    double photon_energy = 6.626e-34 * 3e8 / EUV_WAVELENGTH; // J
    
    // Number of photons
    double photons = (dose * 1e-3) * area / photon_energy; // dose in mJ/cm²
    
    // Shot noise (sqrt(N)/N)
    return 1.0 / std::sqrt(std::max(1.0, photons));
}

void EUVLithography::updateMetrics(const Eigen::ArrayXXd& final_pattern) const {
    metrics_.resolution = calculateResolution(0.33); // Typical EUV NA
    metrics_.depth_of_focus = calculateDepthOfFocus(0.33);
    metrics_.line_edge_roughness = calculateLineEdgeRoughness(final_pattern);
    metrics_.critical_dimension_uniformity = calculateCriticalDimensionUniformity(final_pattern);
    metrics_.stochastic_defect_density = 0.1; // defects/cm² (typical)
    metrics_.throughput = 150.0; // wafers/hour (typical EUV)
}

double EUVLithography::calculateLineEdgeRoughness(const Eigen::ArrayXXd& pattern) const {
    // Simplified LER calculation
    int rows = pattern.rows();
    int cols = pattern.cols();
    
    double total_roughness = 0.0;
    int edge_count = 0;
    
    for (int i = 1; i < rows - 1; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            // Find edges
            double gradient = std::abs(pattern(i+1, j) - pattern(i-1, j)) + 
                             std::abs(pattern(i, j+1) - pattern(i, j-1));
            
            if (gradient > 0.5) { // Edge detected
                // Calculate local roughness
                double local_roughness = gradient * 0.5; // Simplified
                total_roughness += local_roughness;
                edge_count++;
            }
        }
    }
    
    return edge_count > 0 ? (total_roughness / edge_count) * 1e9 : 0.0; // Convert to nm
}

double EUVLithography::calculateCriticalDimensionUniformity(const Eigen::ArrayXXd& pattern) const {
    // Simplified CDU calculation
    return 2.0; // nm (typical for EUV)
}

// Factory functions
std::unique_ptr<EUVLithography> createASMLTwinscan(double numerical_aperture) {
    auto euv = std::make_unique<EUVLithography>();
    euv->enableOPCCorrection(true);
    euv->enableStochasticEffects(true);
    euv->setResistParameters(30e-9, 20.0, "CAR");
    return euv;
}

std::unique_ptr<EUVLithography> createCanonFPA(double numerical_aperture) {
    auto euv = std::make_unique<EUVLithography>();
    euv->enableOPCCorrection(true);
    euv->enableStochasticEffects(true);
    euv->setResistParameters(25e-9, 18.0, "CAR");
    return euv;
}

bool validateEUVMask(const std::vector<std::vector<int>>& mask) {
    if (mask.empty() || mask[0].empty()) {
        return false;
    }
    
    // Check for consistent dimensions
    size_t cols = mask[0].size();
    for (const auto& row : mask) {
        if (row.size() != cols) {
            return false;
        }
    }
    
    return true;
}

double calculateEUVThroughput(double dose, double field_size, double scan_speed) {
    // Simplified throughput calculation
    double exposure_time = dose / (scan_speed * 1000); // seconds
    double field_area = field_size * field_size; // mm²
    double wafer_area = M_PI * 150 * 150; // 300mm wafer
    double fields_per_wafer = wafer_area / field_area;
    
    double time_per_wafer = fields_per_wafer * exposure_time + 10; // +10s overhead
    return 3600.0 / time_per_wafer; // wafers per hour
}

std::vector<std::vector<int>> generateEUVTestPattern(int size, double feature_size) {
    std::vector<std::vector<int>> pattern(size, std::vector<int>(size, 0));
    
    int feature_pixels = static_cast<int>(feature_size * size / 100.0);
    
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            // Create alternating lines
            if ((j / feature_pixels) % 2 == 0) {
                pattern[i][j] = 1;
            }
        }
    }
    
    return pattern;
}

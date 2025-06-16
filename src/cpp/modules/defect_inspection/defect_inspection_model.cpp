// Author: Dr. Mazharuddin Mohammed
#include "defect_inspection_model.hpp"
#include <algorithm>
#include <cmath>

DefectInspectionModel::DefectInspectionModel() : rng_(std::random_device{}()) {
    // Initialize default inspection parameters
    InspectionParameters default_params;
    default_params.pixel_size = 0.1;  // μm
    default_params.scan_speed = 10.0;  // mm/s
    default_params.sensitivity = 0.9;
    default_params.false_positive_rate = 0.01;
    default_params.false_negative_rate = 0.05;
    
    // Set for all inspection methods
    for (int method = OPTICAL_BRIGHTFIELD; method <= X_RAY_TOPOGRAPHY; ++method) {
        inspection_params_[static_cast<InspectionMethod>(method)] = default_params;
    }
    
    // Initialize defect distribution
    defect_distribution_.type_probabilities[PARTICLE] = 0.4;
    defect_distribution_.type_probabilities[SCRATCH] = 0.2;
    defect_distribution_.type_probabilities[VOID] = 0.15;
    defect_distribution_.type_probabilities[CONTAMINATION] = 0.15;
    defect_distribution_.type_probabilities[PATTERN_DEFECT] = 0.1;
    
    defect_distribution_.severity_probabilities[MINOR] = 0.6;
    defect_distribution_.severity_probabilities[MAJOR] = 0.3;
    defect_distribution_.severity_probabilities[CRITICAL] = 0.1;
    
    defect_distribution_.size_mean = 0.5;  // μm
    defect_distribution_.size_std_dev = 0.2;
    defect_distribution_.distribution_type = "lognormal";
    
    Logger::getInstance().log("DefectInspectionModel initialized");
}

DefectInspectionModel::InspectionResult DefectInspectionModel::performInspection(
    std::shared_ptr<Wafer> wafer,
    InspectionMethod method,
    const std::vector<std::pair<double, double>>& inspection_areas) {
    
    InspectionResult result(method);
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Calculate total inspection area
    double total_area = 0.0;
    for (const auto& area : inspection_areas) {
        total_area += area.first * area.second;
    }
    result.coverage_area = total_area;
    
    // Generate candidate defects based on area and expected density
    double expected_density = 0.1;  // defects per cm²
    std::vector<DefectType> allowed_types = {PARTICLE, SCRATCH, VOID, CONTAMINATION, PATTERN_DEFECT};
    std::vector<Defect> candidate_defects = generateRandomDefects(
        total_area, expected_density, allowed_types);
    
    // Apply detection probability based on method
    for (const auto& defect : candidate_defects) {
        double detection_prob = calculateDetectionProbability(method, defect);
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        
        if (prob_dist(rng_) < detection_prob) {
            result.defects.push_back(defect);
        }
    }
    
    // Calculate inspection time
    auto end_time = std::chrono::high_resolution_clock::now();
    result.inspection_time = std::chrono::duration<double>(end_time - start_time).count();
    
    // Generate statistics
    result.statistics = generateDefectStatistics(result.defects);
    
    Logger::getInstance().log("Inspection completed: " + std::to_string(result.defects.size()) + 
                            " defects found in " + std::to_string(result.inspection_time) + "s");
    
    return result;
}

std::vector<DefectInspectionModel::Defect> DefectInspectionModel::detectParticles(
    std::shared_ptr<Wafer> wafer,
    double min_size,
    double max_size) {
    
    std::vector<Defect> particles;
    
    // Simulate particle detection based on wafer surface
    double wafer_area = M_PI * std::pow(wafer->getDiameter() / 2.0, 2);  // mm²
    double particle_density = 0.05;  // particles per mm²
    
    int num_particles = static_cast<int>(wafer_area * particle_density);
    
    for (int i = 0; i < num_particles; ++i) {
        auto position = generateDefectPosition(wafer->getDiameter());
        double size = generateDefectSize(PARTICLE);
        
        if (size >= min_size && size <= max_size) {
            Defect particle(PARTICLE, MINOR, position.first, position.second, 0.0, size);
            particle.confidence = 0.9;
            particle.description = "Particle detected";
            particles.push_back(particle);
        }
    }
    
    return particles;
}

std::vector<DefectInspectionModel::Defect> DefectInspectionModel::detectPatternDefects(
    std::shared_ptr<Wafer> wafer,
    const std::string& reference_pattern) {
    
    std::vector<Defect> pattern_defects;
    
    // Simulate pattern defect detection
    if (wafer->getGrid().rows() > 0 && wafer->getGrid().cols() > 0) {
        const auto& grid = wafer->getGrid();
        
        for (int i = 1; i < grid.rows() - 1; ++i) {
            for (int j = 1; j < grid.cols() - 1; ++j) {
                // Simple edge detection for pattern defects
                double gradient = std::abs(grid(i+1, j) - grid(i-1, j)) + 
                                std::abs(grid(i, j+1) - grid(i, j-1));
                
                if (gradient > 0.5) {  // Threshold for defect detection
                    auto physical_coords = gridToPhysicalCoordinates(wafer, i, j);
                    Defect defect(PATTERN_DEFECT, MAJOR, 
                                physical_coords.first, physical_coords.second, 0.0, 0.1);
                    defect.confidence = 0.8;
                    defect.description = "Pattern irregularity";
                    pattern_defects.push_back(defect);
                }
            }
        }
    }
    
    return pattern_defects;
}

std::vector<DefectInspectionModel::Defect> DefectInspectionModel::detectDimensionalDefects(
    std::shared_ptr<Wafer> wafer,
    double tolerance) {
    
    std::vector<Defect> dimensional_defects;
    
    // Check layer thicknesses against tolerance
    const auto& layers = wafer->getFilmLayers();
    for (size_t i = 0; i < layers.size(); ++i) {
        double thickness = layers[i].first;
        double expected_thickness = 0.5;  // Example expected thickness
        
        if (std::abs(thickness - expected_thickness) > tolerance) {
            auto position = generateDefectPosition(wafer->getDiameter());
            Defect defect(DIMENSIONAL_DEFECT, MAJOR, 
                        position.first, position.second, 0.0, 
                        std::abs(thickness - expected_thickness));
            defect.confidence = 0.95;
            defect.description = "Thickness out of tolerance";
            dimensional_defects.push_back(defect);
        }
    }
    
    return dimensional_defects;
}

DefectInspectionModel::DefectSeverity DefectInspectionModel::classifyDefect(const Defect& defect) {
    // Classification based on defect type and size
    if (defect.type == PARTICLE) {
        if (defect.size > 1.0) return CRITICAL;
        if (defect.size > 0.5) return MAJOR;
        return MINOR;
    } else if (defect.type == VOID || defect.type == BRIDGE) {
        return CRITICAL;
    } else if (defect.type == PATTERN_DEFECT) {
        if (defect.size > 0.1) return MAJOR;
        return MINOR;
    }
    
    return MINOR;
}

std::unordered_map<std::string, int> DefectInspectionModel::generateDefectStatistics(
    const std::vector<Defect>& defects) {
    
    std::unordered_map<std::string, int> stats;
    
    // Count by type
    for (const auto& defect : defects) {
        std::string type_name = defectTypeToString(defect.type);
        stats[type_name]++;
        
        std::string severity_name;
        switch (defect.severity) {
            case CRITICAL: severity_name = "critical"; break;
            case MAJOR: severity_name = "major"; break;
            case MINOR: severity_name = "minor"; break;
            case COSMETIC: severity_name = "cosmetic"; break;
        }
        stats[severity_name]++;
    }
    
    stats["total"] = static_cast<int>(defects.size());
    
    return stats;
}

double DefectInspectionModel::calculateDefectDensity(
    const std::vector<Defect>& defects,
    double area) {
    
    if (area <= 0.0) return 0.0;
    return static_cast<double>(defects.size()) / area;
}

double DefectInspectionModel::estimateYieldImpact(
    const std::vector<Defect>& defects,
    double die_area) {
    
    // Simple Poisson yield model
    double critical_defect_density = 0.0;
    
    for (const auto& defect : defects) {
        if (defect.severity == CRITICAL) {
            critical_defect_density += 1.0;
        }
    }
    
    critical_defect_density /= die_area;
    
    // Poisson yield = exp(-defect_density * die_area)
    return std::exp(-critical_defect_density * die_area);
}

std::vector<std::pair<double, double>> DefectInspectionModel::identifyKillDefects(
    const std::vector<Defect>& defects,
    double die_area) {
    
    std::vector<std::pair<double, double>> kill_defects;
    
    for (const auto& defect : defects) {
        if (defect.severity == CRITICAL) {
            kill_defects.emplace_back(defect.x, defect.y);
        }
    }
    
    return kill_defects;
}

void DefectInspectionModel::optimizeInspectionParameters(
    InspectionMethod method,
    const std::unordered_map<std::string, double>& requirements) {
    
    auto& params = inspection_params_[method];
    
    if (requirements.find("sensitivity") != requirements.end()) {
        params.sensitivity = requirements.at("sensitivity");
    }
    
    if (requirements.find("throughput") != requirements.end()) {
        // Adjust scan speed based on throughput requirement
        double target_throughput = requirements.at("throughput");
        params.scan_speed = target_throughput * 10.0;  // Simplified relationship
    }
    
    Logger::getInstance().log("Optimized inspection parameters for method " + 
                            std::to_string(static_cast<int>(method)));
}

double DefectInspectionModel::calculateInspectionSensitivity(
    InspectionMethod method,
    DefectType defect_type) {
    
    double base_sensitivity = inspection_params_[method].sensitivity;
    
    // Adjust based on defect type and method compatibility
    if (method == OPTICAL_BRIGHTFIELD || method == OPTICAL_DARKFIELD) {
        if (defect_type == PARTICLE) return base_sensitivity * 0.9;
        if (defect_type == SCRATCH) return base_sensitivity * 0.8;
        if (defect_type == PATTERN_DEFECT) return base_sensitivity * 0.7;
    } else if (method == ELECTRON_BEAM) {
        if (defect_type == PATTERN_DEFECT) return base_sensitivity * 0.95;
        if (defect_type == DIMENSIONAL_DEFECT) return base_sensitivity * 0.9;
    }
    
    return base_sensitivity;
}

// Helper function implementations
double DefectInspectionModel::calculateDetectionProbability(
    InspectionMethod method,
    const Defect& defect) {
    
    double base_prob = calculateInspectionSensitivity(method, defect.type);
    
    // Size-dependent detection probability
    double size_factor = std::min(1.0, defect.size / 0.1);  // Normalize to 0.1 μm
    
    return base_prob * size_factor;
}

std::vector<DefectInspectionModel::Defect> DefectInspectionModel::generateRandomDefects(
    double area,
    double defect_density,
    const std::vector<DefectType>& allowed_types) {
    
    std::vector<Defect> defects;
    
    int num_defects = static_cast<int>(area * defect_density);
    std::uniform_int_distribution<int> type_dist(0, allowed_types.size() - 1);
    
    for (int i = 0; i < num_defects; ++i) {
        DefectType type = allowed_types[type_dist(rng_)];
        auto position = generateDefectPosition(300.0);  // Assume 300mm wafer
        double size = generateDefectSize(type);
        DefectSeverity severity = classifyDefect(Defect(type, MINOR, 0, 0, 0, size));
        
        Defect defect(type, severity, position.first, position.second, 0.0, size);
        defect.confidence = 0.8 + 0.2 * std::uniform_real_distribution<double>(0.0, 1.0)(rng_);
        defects.push_back(defect);
    }
    
    return defects;
}

std::string DefectInspectionModel::defectTypeToString(DefectType type) {
    switch (type) {
        case PARTICLE: return "particle";
        case SCRATCH: return "scratch";
        case VOID: return "void";
        case BRIDGE: return "bridge";
        case OPEN_CIRCUIT: return "open_circuit";
        case SHORT_CIRCUIT: return "short_circuit";
        case CONTAMINATION: return "contamination";
        case CRYSTAL_DEFECT: return "crystal_defect";
        case PATTERN_DEFECT: return "pattern_defect";
        case DIMENSIONAL_DEFECT: return "dimensional_defect";
        default: return "unknown";
    }
}

double DefectInspectionModel::generateDefectSize(DefectType type) {
    std::lognormal_distribution<double> size_dist(
        std::log(defect_distribution_.size_mean),
        defect_distribution_.size_std_dev
    );
    
    double size = size_dist(rng_);
    
    // Type-specific size adjustments
    if (type == PARTICLE) {
        size *= 0.5;  // Particles tend to be smaller
    } else if (type == SCRATCH) {
        size *= 2.0;  // Scratches tend to be larger
    }
    
    return std::max(0.01, size);  // Minimum 0.01 μm
}

std::pair<double, double> DefectInspectionModel::generateDefectPosition(double wafer_diameter) {
    std::uniform_real_distribution<double> angle_dist(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<double> radius_dist(0.0, wafer_diameter / 2.0);
    
    double angle = angle_dist(rng_);
    double radius = radius_dist(rng_);
    
    double x = radius * std::cos(angle);
    double y = radius * std::sin(angle);
    
    return {x, y};
}

std::pair<double, double> DefectInspectionModel::gridToPhysicalCoordinates(
    std::shared_ptr<Wafer> wafer,
    int grid_x, int grid_y) {
    
    double diameter = wafer->getDiameter();
    double x = (static_cast<double>(grid_x) / wafer->getGrid().rows() - 0.5) * diameter;
    double y = (static_cast<double>(grid_y) / wafer->getGrid().cols() - 0.5) * diameter;
    
    return {x, y};
}

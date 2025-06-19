// Author: Dr. Mazharuddin Mohammed
#include "wafer_enhanced.hpp"
#include "utils.hpp"
#include <algorithm>
#include <fstream>
#include <thread>
#include <future>
#include <execution>

WaferEnhanced::WaferEnhanced(double diameter, double thickness, const std::string& material)
    : Wafer(diameter, thickness, material) {
    
    // Initialize enhanced features
    crystal_structure_ = DIAMOND;  // Default for silicon
    
    Logger::getInstance().log("Enhanced wafer created: " + std::to_string(diameter) + 
                             "mm diameter, " + std::to_string(thickness) + "μm thickness");
}

void WaferEnhanced::initializeGridParallel(int rows, int cols) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Call base class initialization
    Wafer::initializeGrid(rows, cols);
    
    // Initialize enhanced fields
    stress_field_ = Eigen::ArrayXXd::Zero(rows, cols);
    strain_field_ = Eigen::ArrayXXd::Zero(rows, cols);
    temperature_field_ = Eigen::ArrayXXd::Constant(rows, cols, 300.0);  // Room temperature
    
    Logger::getInstance().log("Enhanced grid initialized: " + std::to_string(rows) + 
                             "x" + std::to_string(cols));
}

void WaferEnhanced::updateGridParallel(const Eigen::ArrayXXd& new_grid) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    if (new_grid.rows() != grid_.rows() || new_grid.cols() != grid_.cols()) {
        throw std::invalid_argument("Grid dimensions mismatch");
    }
    
    // Parallel update using OpenMP if available
    #pragma omp parallel for
    for (int i = 0; i < new_grid.rows(); ++i) {
        for (int j = 0; j < new_grid.cols(); ++j) {
            grid_(i, j) = new_grid(i, j);
        }
    }
    
    // Update dependent fields
    calculateStress();
    calculateStrain();
}

void WaferEnhanced::addLayer(const std::string& material, double thickness) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    int rows = grid_.rows();
    int cols = grid_.cols();
    
    layers_.emplace_back(material, thickness, rows, cols);
    
    // Update stress field
    updateStressFromLayers();
    
    Logger::getInstance().log("Added layer: " + material + ", thickness: " + 
                             std::to_string(thickness) + "μm");
}

void WaferEnhanced::removeLayer(size_t index) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    if (index >= layers_.size()) {
        throw std::out_of_range("Layer index out of range");
    }
    
    std::string material = layers_[index].material;
    layers_.erase(layers_.begin() + index);
    
    // Update stress field
    updateStressFromLayers();
    
    Logger::getInstance().log("Removed layer: " + material);
}

void WaferEnhanced::setDopantProfileMultiLayer(const std::vector<Eigen::ArrayXXd>& profiles) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    if (profiles.size() != layers_.size()) {
        throw std::invalid_argument("Number of profiles must match number of layers");
    }
    
    for (size_t i = 0; i < profiles.size(); ++i) {
        if (profiles[i].rows() != grid_.rows() || profiles[i].cols() != grid_.cols()) {
            throw std::invalid_argument("Profile dimensions must match grid dimensions");
        }
        layers_[i].composition = profiles[i];
    }
    
    Logger::getInstance().log("Updated multi-layer dopant profiles");
}

Eigen::ArrayXXd WaferEnhanced::getDopantProfileAtDepth(double depth) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Find the layer at the specified depth
    double current_depth = 0.0;
    for (const auto& layer : layers_) {
        if (depth <= current_depth + layer.thickness) {
            return layer.composition;
        }
        current_depth += layer.thickness;
    }
    
    // Return zero profile if depth exceeds all layers
    return Eigen::ArrayXXd::Zero(grid_.rows(), grid_.cols());
}

void WaferEnhanced::calculateStress() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    stress_field_.setZero();
    
    // Calculate stress from thermal expansion
    const double reference_temp = 300.0;  // Room temperature
    const double thermal_expansion_coeff = 2.6e-6;  // Silicon
    
    #pragma omp parallel for
    for (int i = 0; i < stress_field_.rows(); ++i) {
        for (int j = 0; j < stress_field_.cols(); ++j) {
            double temp_diff = temperature_field_(i, j) - reference_temp;
            stress_field_(i, j) = thermal_expansion_coeff * temp_diff * 1e9;  // Convert to Pa
        }
    }
    
    // Add stress from layers
    updateStressFromLayers();
}

void WaferEnhanced::calculateStrain() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Calculate strain from stress using elastic modulus
    const double elastic_modulus = 130e9;  // Silicon elastic modulus in Pa
    
    strain_field_ = stress_field_ / elastic_modulus;
}

void WaferEnhanced::addDefect(const Defect& defect) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    defects_.push_back(defect);
    
    Logger::getInstance().log("Added defect at (" + std::to_string(defect.x) + 
                             ", " + std::to_string(defect.y) + ", " + std::to_string(defect.z) + ")");
}

void WaferEnhanced::removeDefect(size_t index) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    if (index >= defects_.size()) {
        throw std::out_of_range("Defect index out of range");
    }
    
    defects_.erase(defects_.begin() + index);
    Logger::getInstance().log("Removed defect at index " + std::to_string(index));
}

void WaferEnhanced::setTemperatureField(const Eigen::ArrayXXd& temperature) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    if (temperature.rows() != grid_.rows() || temperature.cols() != grid_.cols()) {
        throw std::invalid_argument("Temperature field dimensions must match grid dimensions");
    }
    
    temperature_field_ = temperature;
    
    // Recalculate stress due to temperature change
    calculateStress();
    calculateStrain();
    
    Logger::getInstance().log("Updated temperature field");
}

void WaferEnhanced::recordProcessStep(const ProcessStep& step) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    process_history_.push_back(step);
    
    Logger::getInstance().log("Recorded process step: " + step.operation);
}

bool WaferEnhanced::validateIntegrity() const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Check grid consistency
    if (grid_.rows() <= 0 || grid_.cols() <= 0) {
        return false;
    }
    
    // Check field dimensions
    if (stress_field_.rows() != grid_.rows() || stress_field_.cols() != grid_.cols()) {
        return false;
    }
    
    if (strain_field_.rows() != grid_.rows() || strain_field_.cols() != grid_.cols()) {
        return false;
    }
    
    if (temperature_field_.rows() != grid_.rows() || temperature_field_.cols() != grid_.cols()) {
        return false;
    }
    
    // Check layer consistency
    for (const auto& layer : layers_) {
        if (layer.composition.rows() != grid_.rows() || layer.composition.cols() != grid_.cols()) {
            return false;
        }
        if (layer.thickness <= 0.0) {
            return false;
        }
    }
    
    // Check defect positions
    for (const auto& defect : defects_) {
        if (defect.x < 0 || defect.x >= getDiameter() ||
            defect.y < 0 || defect.y >= getDiameter() ||
            defect.z < 0 || defect.z >= getThickness()) {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> WaferEnhanced::getValidationErrors() const {
    std::vector<std::string> errors;
    
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    // Check grid
    if (grid_.rows() <= 0 || grid_.cols() <= 0) {
        errors.push_back("Invalid grid dimensions");
    }
    
    // Check fields
    if (stress_field_.rows() != grid_.rows() || stress_field_.cols() != grid_.cols()) {
        errors.push_back("Stress field dimension mismatch");
    }
    
    if (strain_field_.rows() != grid_.rows() || strain_field_.cols() != grid_.cols()) {
        errors.push_back("Strain field dimension mismatch");
    }
    
    if (temperature_field_.rows() != grid_.rows() || temperature_field_.cols() != grid_.cols()) {
        errors.push_back("Temperature field dimension mismatch");
    }
    
    // Check layers
    for (size_t i = 0; i < layers_.size(); ++i) {
        const auto& layer = layers_[i];
        if (layer.composition.rows() != grid_.rows() || layer.composition.cols() != grid_.cols()) {
            errors.push_back("Layer " + std::to_string(i) + " composition dimension mismatch");
        }
        if (layer.thickness <= 0.0) {
            errors.push_back("Layer " + std::to_string(i) + " has invalid thickness");
        }
    }
    
    // Check defects
    for (size_t i = 0; i < defects_.size(); ++i) {
        const auto& defect = defects_[i];
        if (defect.x < 0 || defect.x >= getDiameter() ||
            defect.y < 0 || defect.y >= getDiameter() ||
            defect.z < 0 || defect.z >= getThickness()) {
            errors.push_back("Defect " + std::to_string(i) + " has invalid position");
        }
    }
    
    return errors;
}

void WaferEnhanced::saveToFile(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }
    
    // Save basic wafer properties
    file.write(reinterpret_cast<const char*>(&diameter_), sizeof(diameter_));
    file.write(reinterpret_cast<const char*>(&thickness_), sizeof(thickness_));
    
    // Save material name
    size_t material_size = material_id_.size();
    file.write(reinterpret_cast<const char*>(&material_size), sizeof(material_size));
    file.write(material_id_.c_str(), material_size);
    
    // Save grid dimensions
    int rows = grid_.rows();
    int cols = grid_.cols();
    file.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
    file.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
    
    // Save grid data
    file.write(reinterpret_cast<const char*>(grid_.data()), rows * cols * sizeof(double));
    
    // Save enhanced fields
    file.write(reinterpret_cast<const char*>(stress_field_.data()), rows * cols * sizeof(double));
    file.write(reinterpret_cast<const char*>(strain_field_.data()), rows * cols * sizeof(double));
    file.write(reinterpret_cast<const char*>(temperature_field_.data()), rows * cols * sizeof(double));
    
    // Save crystal structure
    file.write(reinterpret_cast<const char*>(&crystal_structure_), sizeof(crystal_structure_));
    
    // Save layers
    size_t layer_count = layers_.size();
    file.write(reinterpret_cast<const char*>(&layer_count), sizeof(layer_count));
    
    for (const auto& layer : layers_) {
        // Save layer material
        size_t mat_size = layer.material.size();
        file.write(reinterpret_cast<const char*>(&mat_size), sizeof(mat_size));
        file.write(layer.material.c_str(), mat_size);
        
        // Save layer thickness
        file.write(reinterpret_cast<const char*>(&layer.thickness), sizeof(layer.thickness));
        
        // Save layer composition
        file.write(reinterpret_cast<const char*>(layer.composition.data()), rows * cols * sizeof(double));
    }
    
    // Save defects
    size_t defect_count = defects_.size();
    file.write(reinterpret_cast<const char*>(&defect_count), sizeof(defect_count));
    
    for (const auto& defect : defects_) {
        file.write(reinterpret_cast<const char*>(&defect), sizeof(defect));
    }
    
    Logger::getInstance().log("Wafer saved to file: " + filename);
}

void WaferEnhanced::loadFromFile(const std::string& filename) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file for reading: " + filename);
    }
    
    // Load basic wafer properties
    file.read(reinterpret_cast<char*>(&diameter_), sizeof(diameter_));
    file.read(reinterpret_cast<char*>(&thickness_), sizeof(thickness_));
    
    // Load material name
    size_t material_size;
    file.read(reinterpret_cast<char*>(&material_size), sizeof(material_size));
    material_id_.resize(material_size);
    file.read(&material_id_[0], material_size);
    
    // Load grid dimensions
    int rows, cols;
    file.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    file.read(reinterpret_cast<char*>(&cols), sizeof(cols));
    
    // Resize arrays
    grid_.resize(rows, cols);
    stress_field_.resize(rows, cols);
    strain_field_.resize(rows, cols);
    temperature_field_.resize(rows, cols);
    
    // Load grid data
    file.read(reinterpret_cast<char*>(grid_.data()), rows * cols * sizeof(double));
    
    // Load enhanced fields
    file.read(reinterpret_cast<char*>(stress_field_.data()), rows * cols * sizeof(double));
    file.read(reinterpret_cast<char*>(strain_field_.data()), rows * cols * sizeof(double));
    file.read(reinterpret_cast<char*>(temperature_field_.data()), rows * cols * sizeof(double));
    
    // Load crystal structure
    file.read(reinterpret_cast<char*>(&crystal_structure_), sizeof(crystal_structure_));
    
    // Load layers
    size_t layer_count;
    file.read(reinterpret_cast<char*>(&layer_count), sizeof(layer_count));
    
    layers_.clear();
    layers_.reserve(layer_count);
    
    for (size_t i = 0; i < layer_count; ++i) {
        // Load layer material
        size_t mat_size;
        file.read(reinterpret_cast<char*>(&mat_size), sizeof(mat_size));
        std::string material(mat_size, '\0');
        file.read(&material[0], mat_size);
        
        // Load layer thickness
        double thickness;
        file.read(reinterpret_cast<char*>(&thickness), sizeof(thickness));
        
        // Create layer
        layers_.emplace_back(material, thickness, rows, cols);
        
        // Load layer composition
        file.read(reinterpret_cast<char*>(layers_.back().composition.data()), rows * cols * sizeof(double));
    }
    
    // Load defects
    size_t defect_count;
    file.read(reinterpret_cast<char*>(&defect_count), sizeof(defect_count));
    
    defects_.clear();
    defects_.reserve(defect_count);
    
    for (size_t i = 0; i < defect_count; ++i) {
        Defect defect(Defect::VACANCY, 0, 0, 0, 0);
        file.read(reinterpret_cast<char*>(&defect), sizeof(defect));
        defects_.push_back(defect);
    }
    
    Logger::getInstance().log("Wafer loaded from file: " + filename);
}

void WaferEnhanced::updateStressFromLayers() {
    // Calculate stress contribution from each layer
    for (const auto& layer : layers_) {
        double layer_stress = calculateLayerStress(layer);
        
        // Add to stress field (simplified model)
        #pragma omp parallel for
        for (int i = 0; i < stress_field_.rows(); ++i) {
            for (int j = 0; j < stress_field_.cols(); ++j) {
                stress_field_(i, j) += layer_stress;
            }
        }
    }
}

void WaferEnhanced::updateStrainFromStress() {
    calculateStrain();
}

double WaferEnhanced::calculateLayerStress(const Layer& layer) const {
    // Simplified stress calculation based on material properties
    // In reality, this would involve complex material models
    
    if (layer.material == "SiO2") {
        return -300e6;  // Compressive stress in Pa
    } else if (layer.material == "Si3N4") {
        return 1000e6;  // Tensile stress in Pa
    } else if (layer.material == "Al") {
        return 50e6;    // Low stress in Pa
    }
    
    return 0.0;  // Default no stress
}

void WaferEnhanced::processGridChunk(int start_row, int end_row, int start_col, int end_col,
                                    std::function<void(int, int)> operation) {
    for (int i = start_row; i < end_row; ++i) {
        for (int j = start_col; j < end_col; ++j) {
            operation(i, j);
        }
    }
}

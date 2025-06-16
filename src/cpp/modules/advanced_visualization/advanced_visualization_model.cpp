// Author: Dr. Mazharuddin Mohammed
#include "advanced_visualization_model.hpp"
#include "../../core/utils.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <chrono>

AdvancedVisualizationModel::AdvancedVisualizationModel() 
    : frame_rate_(60.0f), triangle_count_(0), render_time_(0.0f), memory_usage_(0),
      volumetric_enabled_(false), particle_system_enabled_(false),
      measurement_tools_enabled_(false), adaptive_lod_enabled_(true) {
    
    // Initialize layer visibility
    layer_visibility_[VisualizationLayer::SUBSTRATE] = true;
    layer_visibility_[VisualizationLayer::OXIDE] = true;
    layer_visibility_[VisualizationLayer::METAL] = true;
    layer_visibility_[VisualizationLayer::PHOTORESIST] = true;
    layer_visibility_[VisualizationLayer::DOPANT] = false;
    layer_visibility_[VisualizationLayer::TEMPERATURE] = false;
    layer_visibility_[VisualizationLayer::STRESS] = false;
    layer_visibility_[VisualizationLayer::DEFECTS] = false;
    
    // Initialize layer transparency
    for (auto& pair : layer_visibility_) {
        layer_transparency_[pair.first] = 1.0f;
    }
    
    initializeResources();
    Logger::getInstance().log("Advanced visualization model initialized");
}

void AdvancedVisualizationModel::setRenderingMode(RenderingMode mode) {
    rendering_params_.mode = mode;
    
    // Adjust parameters based on mode
    switch (mode) {
        case RenderingMode::WIREFRAME:
            rendering_params_.transparency = 1.0f;
            rendering_params_.enable_shadows = false;
            break;
        case RenderingMode::SOLID:
            rendering_params_.transparency = 1.0f;
            rendering_params_.enable_shadows = true;
            break;
        case RenderingMode::TRANSPARENT:
            rendering_params_.transparency = 0.7f;
            rendering_params_.enable_shadows = true;
            break;
        case RenderingMode::PBR:
            rendering_params_.enable_shadows = true;
            rendering_params_.enable_reflections = true;
            break;
        case RenderingMode::RAY_TRACED:
            rendering_params_.enable_shadows = true;
            rendering_params_.enable_reflections = true;
            rendering_params_.samples_per_pixel = 16;
            break;
        case RenderingMode::VOLUMETRIC:
            volumetric_enabled_ = true;
            break;
    }
    
    Logger::getInstance().log("Rendering mode set to " + std::to_string(static_cast<int>(mode)));
}

void AdvancedVisualizationModel::setCameraPosition(float x, float y, float z) {
    camera_params_.position = {x, y, z};
    Logger::getInstance().log("Camera position set to (" + std::to_string(x) + 
                             ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

void AdvancedVisualizationModel::setCameraTarget(float x, float y, float z) {
    camera_params_.target = {x, y, z};
    Logger::getInstance().log("Camera target set to (" + std::to_string(x) + 
                             ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
}

void AdvancedVisualizationModel::orbitCamera(float theta, float phi, float radius) {
    float x = radius * std::sin(phi) * std::cos(theta);
    float y = radius * std::cos(phi);
    float z = radius * std::sin(phi) * std::sin(theta);
    
    setCameraPosition(x, y, z);
}

void AdvancedVisualizationModel::addLight(const std::array<float, 3>& position,
                                         const std::array<float, 3>& color, float intensity) {
    lighting_params_.light_positions.push_back(position);
    lighting_params_.light_colors.push_back(color);
    lighting_params_.light_intensities.push_back(intensity);
    
    Logger::getInstance().log("Light added at (" + std::to_string(position[0]) + 
                             ", " + std::to_string(position[1]) + 
                             ", " + std::to_string(position[2]) + ")");
}

void AdvancedVisualizationModel::enableLayer(VisualizationLayer layer, bool enabled) {
    layer_visibility_[layer] = enabled;
    Logger::getInstance().log("Layer " + std::to_string(static_cast<int>(layer)) + 
                             (enabled ? " enabled" : " disabled"));
}

void AdvancedVisualizationModel::setLayerTransparency(VisualizationLayer layer, float transparency) {
    layer_transparency_[layer] = std::clamp(transparency, 0.0f, 1.0f);
    Logger::getInstance().log("Layer " + std::to_string(static_cast<int>(layer)) + 
                             " transparency set to " + std::to_string(transparency));
}

void AdvancedVisualizationModel::startAnimation() {
    animation_params_.current_time = 0.0f;
    Logger::getInstance().log("Animation started");
}

void AdvancedVisualizationModel::setAnimationTime(float time) {
    animation_params_.current_time = std::clamp(time, 0.0f, animation_params_.duration);
}

void AdvancedVisualizationModel::renderTemperatureField(std::shared_ptr<Wafer> wafer) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    const auto& temperature = wafer->getTemperatureProfile();
    
    // Enable volumetric rendering for temperature visualization
    volumetric_enabled_ = true;
    
    // Generate temperature-based color mapping
    float min_temp = temperature.minCoeff();
    float max_temp = temperature.maxCoeff();
    
    Logger::getInstance().log("Rendering temperature field: " + 
                             std::to_string(min_temp) + "K to " + 
                             std::to_string(max_temp) + "K");
}

void AdvancedVisualizationModel::renderDopantDistribution(std::shared_ptr<Wafer> wafer) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    const auto& dopant = wafer->getDopantProfile();
    
    // Enable volumetric rendering for dopant visualization
    volumetric_enabled_ = true;
    
    Logger::getInstance().log("Rendering dopant distribution");
}

void AdvancedVisualizationModel::measureDistance(const std::array<float, 3>& point1,
                                                const std::array<float, 3>& point2) {
    float dx = point2[0] - point1[0];
    float dy = point2[1] - point1[1];
    float dz = point2[2] - point1[2];
    float distance = std::sqrt(dx*dx + dy*dy + dz*dz);
    
    Logger::getInstance().log("Distance measured: " + std::to_string(distance) + " units");
}

void AdvancedVisualizationModel::exportImage(const std::string& filename, int width, int height) {
    // Simplified image export - in real implementation would use Vulkan/OpenGL
    Logger::getInstance().log("Exporting image: " + filename + " (" + 
                             std::to_string(width) + "x" + std::to_string(height) + ")");
}

void AdvancedVisualizationModel::exportSTL(std::shared_ptr<Wafer> wafer, const std::string& filename) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename);
    }
    
    file << "solid wafer\n";
    
    // Generate triangles for wafer geometry
    const auto& grid = wafer->getGrid();
    int rows = grid.rows();
    int cols = grid.cols();
    
    for (int i = 0; i < rows - 1; ++i) {
        for (int j = 0; j < cols - 1; ++j) {
            // Create two triangles for each grid cell
            float x1 = static_cast<float>(i);
            float y1 = static_cast<float>(j);
            float z1 = static_cast<float>(grid(i, j));
            
            float x2 = static_cast<float>(i + 1);
            float y2 = static_cast<float>(j);
            float z2 = static_cast<float>(grid(i + 1, j));
            
            float x3 = static_cast<float>(i);
            float y3 = static_cast<float>(j + 1);
            float z3 = static_cast<float>(grid(i, j + 1));
            
            float x4 = static_cast<float>(i + 1);
            float y4 = static_cast<float>(j + 1);
            float z4 = static_cast<float>(grid(i + 1, j + 1));
            
            // Triangle 1
            file << "  facet normal 0.0 0.0 1.0\n";
            file << "    outer loop\n";
            file << "      vertex " << x1 << " " << y1 << " " << z1 << "\n";
            file << "      vertex " << x2 << " " << y2 << " " << z2 << "\n";
            file << "      vertex " << x3 << " " << y3 << " " << z3 << "\n";
            file << "    endloop\n";
            file << "  endfacet\n";
            
            // Triangle 2
            file << "  facet normal 0.0 0.0 1.0\n";
            file << "    outer loop\n";
            file << "      vertex " << x2 << " " << y2 << " " << z2 << "\n";
            file << "      vertex " << x4 << " " << y4 << " " << z4 << "\n";
            file << "      vertex " << x3 << " " << y3 << " " << z3 << "\n";
            file << "    endloop\n";
            file << "  endfacet\n";
        }
    }
    
    file << "endsolid wafer\n";
    file.close();
    
    Logger::getInstance().log("STL file exported: " + filename);
}

void AdvancedVisualizationModel::render(std::shared_ptr<Wafer> wafer) {
    if (!wafer) {
        throw std::invalid_argument("Wafer pointer is null");
    }
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Update animation if active
    if (animation_params_.type != AnimationType::NONE) {
        updateAnimation(1.0f / 60.0f); // Assume 60 FPS
    }
    
    // Update LOD if enabled
    if (adaptive_lod_enabled_) {
        updateLOD(camera_params_);
    }
    
    // Render each visible layer
    triangle_count_ = 0;
    for (const auto& layer_vis : layer_visibility_) {
        if (layer_vis.second) {
            renderLayer(wafer, layer_vis.first);
        }
    }
    
    // Render particles if enabled
    if (particle_system_enabled_) {
        renderParticles();
    }
    
    // Render measurements if enabled
    if (measurement_tools_enabled_) {
        renderMeasurements();
    }
    
    // Apply post-processing effects
    applyPostProcessing();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    render_time_ = duration.count() / 1000.0f; // Convert to milliseconds
    
    frame_rate_ = 1000.0f / render_time_; // FPS
    
    Logger::getInstance().log("Rendered frame in " + std::to_string(render_time_) + " ms");
}

void AdvancedVisualizationModel::renderLayer(std::shared_ptr<Wafer> wafer, VisualizationLayer layer) {
    // Generate mesh for the layer
    generateMesh(wafer, layer);
    
    // Apply material properties
    applyMaterialProperties(layer);
    
    // Increment triangle count (simplified)
    triangle_count_ += 1000; // Placeholder
}

void AdvancedVisualizationModel::updateAnimation(float delta_time) {
    animation_params_.current_time += delta_time * animation_params_.speed_multiplier;
    
    if (animation_params_.current_time >= animation_params_.duration) {
        if (animation_params_.loop) {
            animation_params_.current_time = 0.0f;
        } else {
            animation_params_.current_time = animation_params_.duration;
        }
    }
}

void AdvancedVisualizationModel::generateMesh(std::shared_ptr<Wafer> wafer, VisualizationLayer layer) {
    // Simplified mesh generation - real implementation would create vertex/index buffers
    const auto& grid = wafer->getGrid();
    
    switch (layer) {
        case VisualizationLayer::SUBSTRATE:
            // Generate substrate mesh
            break;
        case VisualizationLayer::METAL:
            // Generate metal layer mesh
            break;
        case VisualizationLayer::TEMPERATURE:
            renderTemperatureField(wafer);
            break;
        case VisualizationLayer::DOPANT:
            renderDopantDistribution(wafer);
            break;
        default:
            break;
    }
}

void AdvancedVisualizationModel::applyMaterialProperties(VisualizationLayer layer) {
    // Set material properties based on layer type
    switch (layer) {
        case VisualizationLayer::SUBSTRATE:
            rendering_params_.metallic = 0.0f;
            rendering_params_.roughness = 0.8f;
            break;
        case VisualizationLayer::METAL:
            rendering_params_.metallic = 1.0f;
            rendering_params_.roughness = 0.1f;
            break;
        case VisualizationLayer::OXIDE:
            rendering_params_.metallic = 0.0f;
            rendering_params_.roughness = 0.9f;
            rendering_params_.transparency = 0.8f;
            break;
        default:
            break;
    }
}

void AdvancedVisualizationModel::initializeResources() {
    // Initialize Vulkan resources, shaders, etc.
    Logger::getInstance().log("Initializing visualization resources");
}

void AdvancedVisualizationModel::renderParticles() {
    // Render particle systems for process visualization
}

void AdvancedVisualizationModel::renderMeasurements() {
    // Render measurement overlays
}

void AdvancedVisualizationModel::applyPostProcessing() {
    // Apply post-processing effects like bloom, SSAO, etc.
}

void AdvancedVisualizationModel::updateLOD(const CameraParameters& camera) {
    // Update level of detail based on camera distance
}

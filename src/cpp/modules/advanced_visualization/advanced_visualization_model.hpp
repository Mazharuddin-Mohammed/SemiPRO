// Author: Dr. Mazharuddin Mohammed
#ifndef ADVANCED_VISUALIZATION_MODEL_HPP
#define ADVANCED_VISUALIZATION_MODEL_HPP

#include "advanced_visualization_interface.hpp"
#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

enum class RenderingMode {
    WIREFRAME,
    SOLID,
    TRANSPARENT,
    PBR,
    RAY_TRACED,
    VOLUMETRIC
};

enum class VisualizationLayer {
    SUBSTRATE,
    OXIDE,
    METAL,
    PHOTORESIST,
    DOPANT,
    TEMPERATURE,
    STRESS,
    DEFECTS,
    ALL_LAYERS
};

enum class AnimationType {
    NONE,
    ROTATION,
    PROCESS_FLOW,
    TIME_SERIES,
    CROSS_SECTION,
    LAYER_PEEL
};

struct RenderingParameters {
    RenderingMode mode;
    float transparency;
    float metallic;
    float roughness;
    float emission;
    bool enable_shadows;
    bool enable_reflections;
    bool enable_subsurface_scattering;
    int samples_per_pixel;
    
    RenderingParameters() : mode(RenderingMode::SOLID), transparency(1.0f),
                           metallic(0.0f), roughness(0.5f), emission(0.0f),
                           enable_shadows(true), enable_reflections(false),
                           enable_subsurface_scattering(false), samples_per_pixel(1) {}
};

struct CameraParameters {
    std::array<float, 3> position;
    std::array<float, 3> target;
    std::array<float, 3> up;
    float fov;
    float near_plane;
    float far_plane;
    
    CameraParameters() : position({0.0f, 0.0f, 5.0f}), target({0.0f, 0.0f, 0.0f}),
                        up({0.0f, 1.0f, 0.0f}), fov(45.0f), near_plane(0.1f),
                        far_plane(100.0f) {}
};

struct LightingParameters {
    std::array<float, 3> ambient_color;
    float ambient_intensity;
    std::vector<std::array<float, 3>> light_positions;
    std::vector<std::array<float, 3>> light_colors;
    std::vector<float> light_intensities;
    bool enable_ibl; // Image-based lighting
    std::string hdri_environment;
    
    LightingParameters() : ambient_color({0.1f, 0.1f, 0.1f}), ambient_intensity(0.2f),
                          enable_ibl(false) {}
};

struct AnimationParameters {
    AnimationType type;
    float duration;
    float current_time;
    bool loop;
    float speed_multiplier;
    std::vector<float> keyframes;
    
    AnimationParameters() : type(AnimationType::NONE), duration(1.0f),
                           current_time(0.0f), loop(false), speed_multiplier(1.0f) {}
};

class AdvancedVisualizationModel : public AdvancedVisualizationInterface {
public:
    AdvancedVisualizationModel();
    
    // Rendering mode control
    void setRenderingMode(RenderingMode mode);
    void setRenderingParameters(const RenderingParameters& params);
    void enableLayer(VisualizationLayer layer, bool enabled);
    void setLayerTransparency(VisualizationLayer layer, float transparency);
    
    // Camera control
    void setCameraParameters(const CameraParameters& params);
    void setCameraPosition(float x, float y, float z);
    void setCameraTarget(float x, float y, float z);
    void orbitCamera(float theta, float phi, float radius);
    void panCamera(float dx, float dy);
    void zoomCamera(float factor);
    
    // Lighting control
    void setLightingParameters(const LightingParameters& params);
    void addLight(const std::array<float, 3>& position, 
                  const std::array<float, 3>& color, float intensity);
    void removeLight(size_t light_index);
    void enableImageBasedLighting(const std::string& hdri_path);
    
    // Animation control
    void setAnimationParameters(const AnimationParameters& params);
    void startAnimation();
    void stopAnimation();
    void pauseAnimation();
    void setAnimationTime(float time);
    void addKeyframe(float time);
    
    // Volumetric rendering
    void enableVolumetricRendering(bool enabled);
    void setVolumetricDensity(float density);
    void setVolumetricScattering(float scattering);
    void renderTemperatureField(std::shared_ptr<Wafer> wafer);
    void renderDopantDistribution(std::shared_ptr<Wafer> wafer);
    void renderStressField(std::shared_ptr<Wafer> wafer);
    
    // Particle systems
    void enableParticleSystem(bool enabled);
    void addParticleEmitter(const std::array<float, 3>& position,
                           const std::string& particle_type);
    void simulateIonImplantation(std::shared_ptr<Wafer> wafer);
    void simulateEtchingProcess(std::shared_ptr<Wafer> wafer);
    
    // Interactive features
    void enableMeasurementTools(bool enabled);
    void measureDistance(const std::array<float, 3>& point1,
                        const std::array<float, 3>& point2);
    void measureArea(const std::vector<std::array<float, 3>>& points);
    void measureVolume(const std::vector<std::array<float, 3>>& points);
    void highlightRegion(const std::array<float, 3>& center, float radius);
    
    // Export capabilities
    void exportImage(const std::string& filename, int width, int height);
    void exportVideo(const std::string& filename, int width, int height,
                    float duration, int fps);
    void exportSTL(std::shared_ptr<Wafer> wafer, const std::string& filename);
    void export3DModel(std::shared_ptr<Wafer> wafer, const std::string& filename,
                      const std::string& format);
    
    // Level of detail
    void setLODParameters(float near_distance, float far_distance,
                         int max_detail_level);
    void enableAdaptiveLOD(bool enabled);
    
    // Post-processing effects
    void enableBloom(bool enabled, float threshold, float intensity);
    void enableSSAO(bool enabled, float radius, float intensity);
    void enableMotionBlur(bool enabled, float strength);
    void enableDepthOfField(bool enabled, float focal_distance, float aperture);
    void enableToneMapping(bool enabled, float exposure, float gamma);
    
    // Performance optimization
    void enableFrustumCulling(bool enabled);
    void enableOcclusionCulling(bool enabled);
    void setRenderingQuality(float quality); // 0.0 to 1.0
    void enableTemporalUpsampling(bool enabled);
    
    // Render the wafer
    void render(std::shared_ptr<Wafer> wafer) override;
    void renderLayer(std::shared_ptr<Wafer> wafer, VisualizationLayer layer);
    void renderCrossSection(std::shared_ptr<Wafer> wafer, 
                           const std::array<float, 3>& plane_normal,
                           float plane_distance);
    
    // Getters
    const RenderingParameters& getRenderingParameters() const { return rendering_params_; }
    const CameraParameters& getCameraParameters() const { return camera_params_; }
    const LightingParameters& getLightingParameters() const { return lighting_params_; }
    const AnimationParameters& getAnimationParameters() const { return animation_params_; }
    
    // Statistics
    float getFrameRate() const { return frame_rate_; }
    int getTriangleCount() const { return triangle_count_; }
    float getRenderTime() const { return render_time_; }
    size_t getMemoryUsage() const { return memory_usage_; }

private:
    RenderingParameters rendering_params_;
    CameraParameters camera_params_;
    LightingParameters lighting_params_;
    AnimationParameters animation_params_;
    
    std::unordered_map<VisualizationLayer, bool> layer_visibility_;
    std::unordered_map<VisualizationLayer, float> layer_transparency_;
    
    // Performance metrics
    float frame_rate_;
    int triangle_count_;
    float render_time_;
    size_t memory_usage_;
    
    // Rendering state
    bool volumetric_enabled_;
    bool particle_system_enabled_;
    bool measurement_tools_enabled_;
    bool adaptive_lod_enabled_;
    
    // Helper methods
    void updateAnimation(float delta_time);
    void updateLOD(const CameraParameters& camera);
    void generateMesh(std::shared_ptr<Wafer> wafer, VisualizationLayer layer);
    void applyMaterialProperties(VisualizationLayer layer);
    void setupLighting();
    void renderParticles();
    void renderMeasurements();
    void applyPostProcessing();
    
    // Shader management
    void loadShaders();
    void updateShaderUniforms();
    void compileShader(const std::string& shader_path);
    
    // Resource management
    void initializeResources();
    void cleanupResources();
    void optimizeMemoryUsage();

    // Vulkan-specific methods
    void initializeVulkan();
    void createRenderPass();
    void createGraphicsPipeline();
    void createCommandBuffers();
    void recordCommandBuffer();
    void submitCommandBuffer();
};

#endif // ADVANCED_VISUALIZATION_MODEL_HPP

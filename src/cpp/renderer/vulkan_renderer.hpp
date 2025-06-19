// Author: Dr. Mazharuddin Mohammed
#pragma once
#include "../../core/wafer.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <vector>
#include <string>
#include <chrono>
#include <memory>

struct Vertex {
    std::array<float, 2> position;
    std::array<float, 3> color;
};

class VulkanRenderer {
public:
    VulkanRenderer(uint32_t width, uint32_t height);
    ~VulkanRenderer();
    void initialize();
    void render(const std::shared_ptr<Wafer>& wafer, bool show_temp_overlay = false,
                const std::string& reliability_metric = ""); // MODIFIED
    GLFWwindow* getWindow() const;

    // Enhanced visualization features
    enum RenderingMode {
        SURFACE_RENDERING,
        VOLUMETRIC_RENDERING,
        CROSS_SECTION,
        TEMPERATURE_FIELD,
        DOPANT_DISTRIBUTION,
        STRESS_ANALYSIS
    };

    void setRenderingMode(RenderingMode mode) { rendering_mode_ = mode; }
    void enableBloom(bool enable) { bloom_enabled_ = enable; }
    void enableAntiAliasing(bool enable) { anti_aliasing_enabled_ = enable; }
    void setQualityLevel(float quality) { quality_level_ = quality; }
    void enableVolumetricRendering(bool enable) { volumetric_enabled_ = enable; }

    // Performance monitoring
    float getFrameRate() const { return frame_rate_; }
    float getRenderTime() const { return last_render_time_; }
    void exportImage(const std::string& filename, int width, int height);
    void exportSTL(const std::shared_ptr<Wafer>& wafer, const std::string& filename);

private:
    void createInstance();
    void setupDevice();
    void createSwapchain();
    void createRenderPass();
    void createPipeline();
    void createFramebuffers();
    void createCommandBuffers();
    void createVertexBuffer(const Eigen::ArrayXXd& grid, const Eigen::ArrayXd& dopant_profile,
                           const Eigen::ArrayXXd& photoresist_pattern,
                           const std::vector<std::pair<double, std::string>>& film_layers,
                           const std::vector<std::pair<double, std::string>>& metal_layers,
                           const std::pair<double, std::string>& packaging_substrate,
                           const std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>>& wire_bonds,
                           const std::vector<std::pair<std::string, double>>& electrical_properties,
                           const Eigen::ArrayXXd& temperature_profile,
                           const Eigen::ArrayXXd& electromigration_mttf,
                           const Eigen::ArrayXXd& thermal_stress,
                           const Eigen::ArrayXXd& dielectric_field,
                           const std::string& material_id,
                           bool show_temp_overlay,
                           const std::string& reliability_metric); // MODIFIED
    void loadShaders();
    void drawFrame();

    uint32_t width_;
    uint32_t height_;
    GLFWwindow* window_;
    vk::Instance instance_;
    vk::PhysicalDevice physical_device_;
    vk::Device device_;
    vk::Queue graphics_queue_;
    vk::SurfaceKHR surface_;
    vk::SwapchainKHR swapchain_;
    std::vector<vk::Image> swapchain_images_;
    std::vector<vk::ImageView> swapchain_image_views_;
    vk::RenderPass render_pass_;
    vk::PipelineLayout pipeline_layout_;
    vk::Pipeline pipeline_;
    std::vector<vk::Framebuffer> framebuffers_;
    vk::CommandPool command_pool_;
    std::vector<vk::CommandBuffer> command_buffers_;
    vk::Buffer vertex_buffer_;
    vk::DeviceMemory vertex_buffer_memory_;
    vk::Semaphore image_available_semaphore_;
    vk::Semaphore render_finished_semaphore_;
    vk::Fence in_flight_fence_;

    // Enhanced rendering features
    RenderingMode rendering_mode_ = SURFACE_RENDERING;
    bool bloom_enabled_ = false;
    bool anti_aliasing_enabled_ = true;
    bool volumetric_enabled_ = false;
    float quality_level_ = 1.0f;

    // Performance tracking
    mutable float frame_rate_ = 0.0f;
    mutable float last_render_time_ = 0.0f;
    mutable uint32_t frame_count_ = 0;
    mutable std::chrono::high_resolution_clock::time_point last_frame_time_;

    // Vertex structure
    struct Vertex {
        float pos[2];
        float color[3];
    };

    // Enhanced rendering methods
    void updatePerformanceMetrics() const;
    void beginFrame();
    void endFrame();
    void renderSurface(std::shared_ptr<Wafer> wafer);
    void renderVolumetric(std::shared_ptr<Wafer> wafer);
    void renderCrossSection(std::shared_ptr<Wafer> wafer);
    void renderTemperatureField(std::shared_ptr<Wafer> wafer);
    void renderDopantDistribution(std::shared_ptr<Wafer> wafer);
    void applyBloomEffect();
    void applyAntiAliasing();
    void updateWaferData(std::shared_ptr<Wafer> wafer);
};
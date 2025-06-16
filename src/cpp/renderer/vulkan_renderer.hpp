// Author: Dr. Mazharuddin Mohammed
#pragma once
#include "../../core/wafer.hpp"
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <Eigen/Dense>
#include <vector>
#include <string>

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
};
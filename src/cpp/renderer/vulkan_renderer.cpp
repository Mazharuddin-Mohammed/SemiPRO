// Author: Dr. Mazharuddin Mohammed
#include "vulkan_renderer.hpp"
#include <stdexcept>
#include <array>
#include <algorithm>
#include <string>
#include <iostream>
#include <cstring>

VulkanRenderer::VulkanRenderer(uint32_t width, uint32_t height) : width_(width), height_(height) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width, height, "Semiconductor Simulator", nullptr, nullptr);
}

VulkanRenderer::~VulkanRenderer() {
    device_.destroySemaphore(image_available_semaphore_);
    device_.destroySemaphore(render_finished_semaphore_);
    device_.destroyFence(in_flight_fence_);
    device_.destroyCommandPool(command_pool_);
    for (auto framebuffer : framebuffers_) device_.destroyFramebuffer(framebuffer);
    device_.destroyPipeline(pipeline_);
    device_.destroyPipelineLayout(pipeline_layout_);
    device_.destroyRenderPass(render_pass_);
    for (auto image_view : swapchain_image_views_) device_.destroyImageView(image_view);
    device_.destroySwapchainKHR(swapchain_);
    instance_.destroySurfaceKHR(surface_);
    device_.destroy();
    instance_.destroy();
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void VulkanRenderer::initialize() {
    createInstance();
    setupDevice();
    createSwapchain();
    createRenderPass();
    createPipeline();
    createFramebuffers();
    createCommandBuffers();
    image_available_semaphore_ = device_.createSemaphore({});
    render_finished_semaphore_ = device_.createSemaphore({});
    in_flight_fence_ = device_.createFence({vk::FenceCreateFlagBits::eSignaled});
}

void VulkanRenderer::createInstance() {
    vk::ApplicationInfo app_info("Semiconductor Simulator", 1, "Simulator Engine", 1, VK_API_VERSION_1_0);
    std::vector<const char*> extensions = {"VK_KHR_surface", "VK_KHR_xcb_surface"};
    vk::InstanceCreateInfo create_info({}, &app_info, 0, nullptr, extensions.size(), extensions.data());
    instance_ = vk::createInstance(create_info);
}

void VulkanRenderer::setupDevice() {
    physical_device_ = instance_.enumeratePhysicalDevices().front();
    auto queue_props = physical_device_.getQueueFamilyProperties();
    uint32_t queue_family_index = 0;
    for (size_t i = 0; i < queue_props.size(); ++i) {
        if (queue_props[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            queue_family_index = i;
            break;
        }
    }
    float queue_priority = 1.0f;
    vk::DeviceQueueCreateInfo queue_info({}, queue_family_index, 1, &queue_priority);
    vk::DeviceCreateInfo device_info({}, 1, &queue_info);
    device_ = physical_device_.createDevice(device_info);
    graphics_queue_ = device_.getQueue(queue_family_index, 0);
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance_), window_, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
    surface_ = vk::SurfaceKHR(surface);
}

void VulkanRenderer::createSwapchain() {
    vk::SwapchainCreateInfoKHR swapchain_info({}, surface_, 2, vk::Format::eB8G8R8A8Unorm,
                                              vk::ColorSpaceKHR::eSrgbNonlinear, {width_, height_}, 1,
                                              vk::ImageUsageFlagBits::eColorAttachment);
    swapchain_ = device_.createSwapchainKHR(swapchain_info);
    swapchain_images_ = device_.getSwapchainImagesKHR(swapchain_);
    swapchain_image_views_.resize(swapchain_images_.size());
    for (size_t i = 0; i < swapchain_images_.size(); ++i) {
        vk::ImageViewCreateInfo view_info({}, swapchain_images_[i], vk::ImageViewType::e2D, vk::Format::eB8G8R8A8Unorm);
        swapchain_image_views_[i] = device_.createImageView(view_info);
    }
}

void VulkanRenderer::createRenderPass() {
    vk::AttachmentDescription color_attachment({}, vk::Format::eB8G8R8A8Unorm, vk::SampleCountFlagBits::e1,
                                               vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                               vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                                               vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
    vk::AttachmentReference color_ref(0, vk::ImageLayout::eColorAttachmentOptimal);
    vk::SubpassDescription subpass({}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &color_ref);
    vk::RenderPassCreateInfo render_pass_info({}, 1, &color_attachment, 1, &subpass);
    render_pass_ = device_.createRenderPass(render_pass_info);
}

void VulkanRenderer::createPipeline() {
    loadShaders();
    vk::PipelineLayoutCreateInfo layout_info;
    pipeline_layout_ = device_.createPipelineLayout(layout_info);
    // Create a minimal pipeline for now - this needs proper shader stages
    vk::PipelineVertexInputStateCreateInfo vertex_input_info;
    vk::PipelineInputAssemblyStateCreateInfo input_assembly({}, vk::PrimitiveTopology::eTriangleList);
    vk::Viewport viewport(0.0f, 0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f, 1.0f);
    vk::Rect2D scissor({0, 0}, {width_, height_});
    vk::PipelineViewportStateCreateInfo viewport_state({}, 1, &viewport, 1, &scissor);
    vk::PipelineRasterizationStateCreateInfo rasterizer({}, false, false, vk::PolygonMode::eFill,
                                                        vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
                                                        false, 0.0f, 0.0f, 0.0f, 1.0f);
    vk::PipelineMultisampleStateCreateInfo multisampling({}, vk::SampleCountFlagBits::e1);
    vk::PipelineColorBlendAttachmentState color_blend_attachment(false);
    color_blend_attachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                           vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    vk::PipelineColorBlendStateCreateInfo color_blending({}, false, vk::LogicOp::eCopy, 1, &color_blend_attachment);

    vk::GraphicsPipelineCreateInfo pipeline_info({}, {}, &vertex_input_info, &input_assembly, nullptr,
                                                &viewport_state, &rasterizer, &multisampling, nullptr,
                                                &color_blending, nullptr, pipeline_layout_, render_pass_, 0);
    auto result = device_.createGraphicsPipeline({}, pipeline_info);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }
    pipeline_ = result.value;
}

void VulkanRenderer::createFramebuffers() {
    framebuffers_.resize(swapchain_image_views_.size());
    for (size_t i = 0; i < swapchain_image_views_.size(); ++i) {
        vk::FramebufferCreateInfo fb_info({}, render_pass_, 1, &swapchain_image_views_[i], width_, height_, 1);
        framebuffers_[i] = device_.createFramebuffer(fb_info);
    }
}

void VulkanRenderer::createCommandBuffers() {
    command_pool_ = device_.createCommandPool({{}, 0});
    vk::CommandBufferAllocateInfo alloc_info(command_pool_, vk::CommandBufferLevel::ePrimary, framebuffers_.size());
    command_buffers_ = device_.allocateCommandBuffers(alloc_info);
}

void VulkanRenderer::loadShaders() {
    // Placeholder: Load SPIR-V shaders (wafer.vert, wafer.frag)
}

void VulkanRenderer::createVertexBuffer(const Eigen::ArrayXXd& grid, const Eigen::ArrayXd& dopant_profile,
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
                                       const std::string& reliability_metric) {
    std::vector<Vertex> vertices;
    float base_color[3] = {1.0f, 0.0f, 0.0f}; // Red for silicon
    if (material_id == "oxide") {
        base_color[0] = 0.0f; base_color[1] = 1.0f; base_color[2] = 0.0f; // Green for oxide
    }

    double max_concentration = dopant_profile.maxCoeff();
    double min_concentration = dopant_profile.minCoeff();
    double range = max_concentration - min_concentration > 0 ? max_concentration - min_concentration : 1.0;

    double max_resistance = 0.0;
    for (const auto& prop : electrical_properties) {
        if (prop.first == "Resistance") max_resistance = std::max(max_resistance, prop.second);
    }
    max_resistance = max_resistance > 0 ? max_resistance : 1.0;

    double T_min = 300.0; // K
    double T_max = 400.0; // K
    double T_range = T_max - T_min;

    double MTTF_threshold = 3.15e8; // 10 years in seconds
    double stress_max = 500.0; // MPa
    double E_bd = 1e7; // Breakdown field (V/cm) for SiO2

    for (int i = 0; i < grid.rows(); ++i) {
        for (int j = 0; j < grid.cols(); ++j) {
            float x = (2.0f * j / grid.cols()) - 1.0f;
            float y = (2.0f * i / grid.rows()) - 1.0f;
            float doping_intensity = 0.0f;
            if (i < dopant_profile.size()) {
                doping_intensity = static_cast<float>((dopant_profile[i] - min_concentration) / range);
            }

            float color[3];
            bool is_wire = false;
            for (const auto& wire : wire_bonds) {
                if ((wire.first.first == i && wire.first.second == j) || 
                    (wire.second.first == i && wire.second.second == j)) {
                    color[0] = 1.0f; color[1] = 0.84f; color[2] = 0.0f; // Gold for wire bonds
                    is_wire = true;
                    break;
                }
            }

            if (!is_wire) {
                if (reliability_metric == "Electromigration" && 
                    electromigration_mttf.rows() > i && electromigration_mttf.cols() > j &&
                    electromigration_mttf(i, j) > 0 && electromigration_mttf(i, j) < MTTF_threshold) {
                    float mttf_norm = std::min(1.0f, static_cast<float>(MTTF_threshold / electromigration_mttf(i, j)));
                    color[0] = mttf_norm; color[1] = 0.0f; color[2] = 0.0f; // Red for low MTTF
                } else if (reliability_metric == "Thermal Stress" && 
                           thermal_stress.rows() > i && thermal_stress.cols() > j &&
                           thermal_stress(i, j) > 0) {
                    float stress_norm = std::min(1.0f, static_cast<float>(thermal_stress(i, j) / stress_max));
                    color[0] = stress_norm; color[1] = 0.0f; color[2] = 1.0f - stress_norm; // Blue to red
                } else if (reliability_metric == "Dielectric Breakdown" && 
                           dielectric_field.rows() > i && dielectric_field.cols() > j &&
                           dielectric_field(i, j) > 0.9 * E_bd) {
                    color[0] = 1.0f; color[1] = 1.0f; color[2] = 0.0f; // Yellow for breakdown risk
                } else if (show_temp_overlay && temperature_profile.rows() > i && temperature_profile.cols() > j &&
                           temperature_profile(i, j) > T_min) {
                    float T_norm = std::min(std::max((temperature_profile(i, j) - T_min) / T_range, 0.0), 1.0);
                    color[0] = T_norm; color[1] = 0.0f; color[2] = 1.0f - T_norm; // Blue to red
                } else if (grid(i, j) < 0.0) {
                    color[0] = 0.5f; color[1] = 0.5f; color[2] = 0.5f; // Gray for etched
                } else if (packaging_substrate.first > 0) {
                    color[0] = 0.6f; color[1] = 0.4f; color[2] = 0.2f; // Brown for substrate
                } else if (!metal_layers.empty()) {
                    color[0] = 0.8f; color[1] = 0.8f; color[2] = 0.8f; // Silver/gray for metal
                    for (const auto& prop : electrical_properties) {
                        if (prop.first == "Resistance") {
                            float intensity = static_cast<float>(prop.second / max_resistance);
                            color[0] = 0.8f * (1.0f - intensity) + intensity; // Red tint
                        }
                    }
                } else if (!film_layers.empty()) {
                    color[0] = 0.0f; color[1] = 0.0f; color[2] = 1.0f; // Blue for dielectric
                } else if (i < photoresist_pattern.rows() && j < photoresist_pattern.cols() && 
                           photoresist_pattern(i, j) > 0.5) {
                    color[0] = 1.0f; color[1] = 1.0f; color[2] = 1.0f; // White for photoresist
                } else {
                    color[0] = base_color[0] * (1.0f - doping_intensity);
                    color[1] = base_color[1] * (1.0f - doping_intensity);
                    color[2] = doping_intensity; // Blue for doping
                }
            }
            vertices.push_back({{x, y}, {color[0], color[1], color[2]}});

            // Log reliability metrics for console-based text overlay
            if (!reliability_metric.empty() && 
                (electromigration_mttf(i, j) > 0 || thermal_stress(i, j) > 0 || dielectric_field(i, j) > 0)) {
                std::cout << "At (" << i << "," << j << "): ";
                if (reliability_metric == "Electromigration") {
                    std::cout << "MTTF=" << electromigration_mttf(i, j) << " s";
                } else if (reliability_metric == "Thermal Stress") {
                    std::cout << "Stress=" << thermal_stress(i, j) << " MPa";
                } else if (reliability_metric == "Dielectric Breakdown") {
                    std::cout << "Field=" << dielectric_field(i, j) << " V/cm";
                }
                std::cout << "\n";
            }
        }
    }

    vk::BufferCreateInfo buffer_info({}, vertices.size() * sizeof(Vertex), vk::BufferUsageFlagBits::eVertexBuffer);
    vertex_buffer_ = device_.createBuffer(buffer_info);
    vk::MemoryRequirements mem_reqs = device_.getBufferMemoryRequirements(vertex_buffer_);
    vk::MemoryAllocateInfo alloc_info(mem_reqs.size, 0);
    vertex_buffer_memory_ = device_.allocateMemory(alloc_info);
    device_.bindBufferMemory(vertex_buffer_, vertex_buffer_memory_, 0);
    void* data = device_.mapMemory(vertex_buffer_memory_, 0, mem_reqs.size);
    memcpy(data, vertices.data(), vertices.size() * sizeof(Vertex));
    device_.unmapMemory(vertex_buffer_memory_);
}

void VulkanRenderer::drawFrame() {
    auto wait_result = device_.waitForFences(in_flight_fence_, true, UINT64_MAX);
    if (wait_result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to wait for fences");
    }
    device_.resetFences(in_flight_fence_);
    uint32_t image_index = device_.acquireNextImageKHR(swapchain_, UINT64_MAX, image_available_semaphore_, {}).value;
    vk::SubmitInfo submit_info(1, &image_available_semaphore_, nullptr, 1, &command_buffers_[image_index], 
                               1, &render_finished_semaphore_);
    graphics_queue_.submit(submit_info, in_flight_fence_);
    vk::PresentInfoKHR present_info(1, &render_finished_semaphore_, 1, &swapchain_, &image_index);
    auto present_result = graphics_queue_.presentKHR(present_info);
    if (present_result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to present");
    }
}

void VulkanRenderer::render(const std::shared_ptr<Wafer>& wafer, bool show_temp_overlay, 
                           const std::string& reliability_metric) {
    createVertexBuffer(wafer->getGrid(), wafer->getDopantProfile(), wafer->getPhotoresistPattern(),
                      wafer->getFilmLayers(), wafer->getMetalLayers(), wafer->getPackagingSubstrate(),
                      wafer->getWireBonds(), wafer->getElectricalProperties(), wafer->getTemperatureProfile(),
                      wafer->getElectromigrationMTTF(), wafer->getThermalStress(), wafer->getDielectricField(),
                      wafer->getMaterialId(), show_temp_overlay, reliability_metric);
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        drawFrame();
    }
}

GLFWwindow* VulkanRenderer::getWindow() const { return window_; }
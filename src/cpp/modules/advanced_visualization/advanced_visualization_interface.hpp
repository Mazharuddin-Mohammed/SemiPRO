#ifndef ADVANCED_VISUALIZATION_INTERFACE_HPP
#define ADVANCED_VISUALIZATION_INTERFACE_HPP

#include "../../core/wafer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

class AdvancedVisualizationInterface {
public:
    virtual ~AdvancedVisualizationInterface() = default;
    
    // Visualization types
    enum VisualizationType {
        CROSS_SECTION_2D,
        ISOMETRIC_3D,
        VOLUMETRIC_3D,
        LAYER_STACK,
        PROCESS_FLOW,
        DEFECT_MAP,
        THERMAL_MAP,
        STRESS_MAP,
        ELECTRICAL_FIELD,
        PARTICLE_SIMULATION
    };
    
    enum RenderingMode {
        WIREFRAME,
        SOLID,
        TRANSPARENT,
        TEXTURED,
        PHYSICALLY_BASED,
        RAY_TRACED
    };
    
    enum ColorScheme {
        MATERIAL_BASED,
        LAYER_BASED,
        PROPERTY_BASED,
        CUSTOM,
        SCIENTIFIC,
        RAINBOW,
        GRAYSCALE
    };
    
    // Visualization parameters
    struct VisualizationParams {
        VisualizationType type;
        RenderingMode mode;
        ColorScheme color_scheme;
        double scale_factor;
        std::vector<double> camera_position;
        std::vector<double> camera_target;
        std::vector<std::string> visible_layers;
        std::unordered_map<std::string, double> material_properties;
        bool enable_lighting;
        bool enable_shadows;
        bool enable_reflections;
        
        VisualizationParams() : type(ISOMETRIC_3D), mode(SOLID), color_scheme(MATERIAL_BASED),
                               scale_factor(1.0), enable_lighting(true), enable_shadows(false),
                               enable_reflections(false) {}
    };
    
    // Animation and time-based visualization
    struct AnimationFrame {
        double timestamp;
        std::shared_ptr<Wafer> wafer_state;
        std::string description;
        
        AnimationFrame(double t, std::shared_ptr<Wafer> w, const std::string& desc)
            : timestamp(t), wafer_state(w), description(desc) {}
    };
    
    // Core visualization functions
    virtual void initializeRenderer(
        int window_width,
        int window_height,
        const std::string& title
    ) = 0;
    
    virtual void renderWafer(
        std::shared_ptr<Wafer> wafer,
        const VisualizationParams& params
    ) = 0;
    
    virtual void renderCrossSection(
        std::shared_ptr<Wafer> wafer,
        double x, double y,
        const std::string& direction,
        const VisualizationParams& params
    ) = 0;
    
    virtual void renderLayerStack(
        std::shared_ptr<Wafer> wafer,
        const VisualizationParams& params
    ) = 0;
    
    // Advanced rendering techniques
    virtual void renderVolumetric(
        std::shared_ptr<Wafer> wafer,
        const std::string& property_name,
        const VisualizationParams& params
    ) = 0;
    
    virtual void renderParticleSystem(
        const std::vector<std::vector<double>>& particles,
        const VisualizationParams& params
    ) = 0;
    
    virtual void renderFieldVisualization(
        std::shared_ptr<Wafer> wafer,
        const std::string& field_type,
        const VisualizationParams& params
    ) = 0;
    
    // Interactive features
    virtual void enableInteractiveMode(bool enable) = 0;
    virtual void setCamera(const std::vector<double>& position, const std::vector<double>& target) = 0;
    virtual void zoomToFit(std::shared_ptr<Wafer> wafer) = 0;
    virtual void highlightRegion(double x1, double y1, double x2, double y2) = 0;
    
    // Animation and time-series
    virtual void createAnimation(
        const std::vector<AnimationFrame>& frames,
        const std::string& output_file
    ) = 0;
    
    virtual void playAnimation(
        const std::vector<AnimationFrame>& frames,
        double frame_rate
    ) = 0;
    
    virtual void renderProcessFlow(
        const std::vector<std::shared_ptr<Wafer>>& process_steps,
        const std::vector<std::string>& step_names
    ) = 0;
    
    // Data visualization overlays
    virtual void addDataOverlay(
        const std::string& overlay_name,
        const std::vector<std::vector<double>>& data,
        const VisualizationParams& params
    ) = 0;
    
    virtual void removeDataOverlay(const std::string& overlay_name) = 0;
    
    virtual void updateDataOverlay(
        const std::string& overlay_name,
        const std::vector<std::vector<double>>& new_data
    ) = 0;
    
    // Export and output
    virtual void exportImage(
        const std::string& filename,
        int width, int height,
        const std::string& format
    ) = 0;
    
    virtual void exportVideo(
        const std::vector<AnimationFrame>& frames,
        const std::string& filename,
        int width, int height,
        double frame_rate
    ) = 0;
    
    virtual void exportSTL(
        std::shared_ptr<Wafer> wafer,
        const std::string& filename
    ) = 0;
    
    // Measurement and analysis tools
    virtual double measureDistance(
        const std::vector<double>& point1,
        const std::vector<double>& point2
    ) = 0;
    
    virtual double measureArea(
        const std::vector<std::vector<double>>& polygon
    ) = 0;
    
    virtual std::vector<double> getPointCoordinates(int screen_x, int screen_y) = 0;
    
    // Shader and material management
    virtual void loadShader(
        const std::string& shader_name,
        const std::string& vertex_shader_path,
        const std::string& fragment_shader_path
    ) = 0;
    
    virtual void setMaterialProperties(
        const std::string& material_name,
        const std::unordered_map<std::string, double>& properties
    ) = 0;
    
    virtual void enablePhysicallyBasedRendering(bool enable) = 0;
    
    // Performance and quality settings
    virtual void setRenderQuality(const std::string& quality_level) = 0;
    virtual void enableAntiAliasing(bool enable, int samples = 4) = 0;
    virtual void setLevelOfDetail(bool enable, double distance_threshold = 100.0) = 0;
    
    // Event handling
    virtual void setMouseCallback(std::function<void(int, int, int)> callback) = 0;
    virtual void setKeyboardCallback(std::function<void(int, int)> callback) = 0;
    virtual void setResizeCallback(std::function<void(int, int)> callback) = 0;
    
    // Cleanup
    virtual void cleanup() = 0;
};

#endif // ADVANCED_VISUALIZATION_INTERFACE_HPP

# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/advanced_visualization/advanced_visualization_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.array cimport array
from geometry cimport PyWafer

cdef extern from "../cpp/modules/advanced_visualization/advanced_visualization_model.hpp":
    cdef enum RenderingMode:
        WIREFRAME
        SOLID
        TRANSPARENT
        PBR
        RAY_TRACED
        VOLUMETRIC
    
    cdef enum VisualizationLayer:
        SUBSTRATE
        OXIDE
        METAL
        PHOTORESIST
        DOPANT
        TEMPERATURE
        STRESS
        DEFECTS
        ALL_LAYERS
    
    cdef enum AnimationType:
        NONE
        ROTATION
        PROCESS_FLOW
        TIME_SERIES
        CROSS_SECTION
        LAYER_PEEL
    
    cdef cppclass RenderingParameters:
        RenderingMode mode
        float transparency
        float metallic
        float roughness
        float emission
        bint enable_shadows
        bint enable_reflections
        bint enable_subsurface_scattering
        int samples_per_pixel
        
        RenderingParameters()
    
    cdef cppclass CameraParameters:
        array[float, 3] position
        array[float, 3] target
        array[float, 3] up
        float fov
        float near_plane
        float far_plane
        
        CameraParameters()
    
    cdef cppclass LightingParameters:
        array[float, 3] ambient_color
        float ambient_intensity
        vector[array[float, 3]] light_positions
        vector[array[float, 3]] light_colors
        vector[float] light_intensities
        bint enable_ibl
        string hdri_environment
        
        LightingParameters()
    
    cdef cppclass AnimationParameters:
        AnimationType type
        float duration
        float current_time
        bint loop
        float speed_multiplier
        vector[float] keyframes
        
        AnimationParameters()
    
    cdef cppclass AdvancedVisualizationModel:
        AdvancedVisualizationModel() except +
        
        # Rendering mode control
        void setRenderingMode(RenderingMode mode) except +
        void setRenderingParameters(const RenderingParameters& params) except +
        void enableLayer(VisualizationLayer layer, bint enabled) except +
        void setLayerTransparency(VisualizationLayer layer, float transparency) except +
        
        # Camera control
        void setCameraParameters(const CameraParameters& params) except +
        void setCameraPosition(float x, float y, float z) except +
        void setCameraTarget(float x, float y, float z) except +
        void orbitCamera(float theta, float phi, float radius) except +
        void panCamera(float dx, float dy) except +
        void zoomCamera(float factor) except +
        
        # Lighting control
        void setLightingParameters(const LightingParameters& params) except +
        void addLight(const array[float, 3]& position, 
                      const array[float, 3]& color, float intensity) except +
        void removeLight(size_t light_index) except +
        void enableImageBasedLighting(const string& hdri_path) except +
        
        # Animation control
        void setAnimationParameters(const AnimationParameters& params) except +
        void startAnimation() except +
        void stopAnimation() except +
        void pauseAnimation() except +
        void setAnimationTime(float time) except +
        void addKeyframe(float time) except +
        
        # Volumetric rendering
        void enableVolumetricRendering(bint enabled) except +
        void setVolumetricDensity(float density) except +
        void setVolumetricScattering(float scattering) except +
        void renderTemperatureField(shared_ptr[Wafer] wafer) except +
        void renderDopantDistribution(shared_ptr[Wafer] wafer) except +
        void renderStressField(shared_ptr[Wafer] wafer) except +
        
        # Interactive features
        void enableMeasurementTools(bint enabled) except +
        void measureDistance(const array[float, 3]& point1,
                            const array[float, 3]& point2) except +
        void measureArea(const vector[array[float, 3]]& points) except +
        void measureVolume(const vector[array[float, 3]]& points) except +
        void highlightRegion(const array[float, 3]& center, float radius) except +
        
        # Export capabilities
        void exportImage(const string& filename, int width, int height) except +
        void exportVideo(const string& filename, int width, int height,
                        float duration, int fps) except +
        void exportSTL(shared_ptr[Wafer] wafer, const string& filename) except +
        void export3DModel(shared_ptr[Wafer] wafer, const string& filename,
                          const string& format) except +
        
        # Post-processing effects
        void enableBloom(bint enabled, float threshold, float intensity) except +
        void enableSSAO(bint enabled, float radius, float intensity) except +
        void enableMotionBlur(bint enabled, float strength) except +
        void enableDepthOfField(bint enabled, float focal_distance, float aperture) except +
        void enableToneMapping(bint enabled, float exposure, float gamma) except +
        
        # Performance optimization
        void enableFrustumCulling(bint enabled) except +
        void enableOcclusionCulling(bint enabled) except +
        void setRenderingQuality(float quality) except +
        void enableTemporalUpsampling(bint enabled) except +
        
        # Render the wafer
        void render(shared_ptr[Wafer] wafer) except +
        void renderLayer(shared_ptr[Wafer] wafer, VisualizationLayer layer) except +
        void renderCrossSection(shared_ptr[Wafer] wafer, 
                               const array[float, 3]& plane_normal,
                               float plane_distance) except +
        
        # Getters
        const RenderingParameters& getRenderingParameters() const
        const CameraParameters& getCameraParameters() const
        const LightingParameters& getLightingParameters() const
        const AnimationParameters& getAnimationParameters() const
        
        # Statistics
        float getFrameRate() const
        int getTriangleCount() const
        float getRenderTime() const
        size_t getMemoryUsage() const

cdef class PyRenderingParameters:
    cdef RenderingParameters* thisptr
    
    def __cinit__(self):
        self.thisptr = new RenderingParameters()
    
    def __dealloc__(self):
        del self.thisptr
    
    @property
    def mode(self):
        return <int>self.thisptr.mode
    
    @mode.setter
    def mode(self, int value):
        self.thisptr.mode = <RenderingMode>value
    
    @property
    def transparency(self):
        return self.thisptr.transparency
    
    @transparency.setter
    def transparency(self, float value):
        self.thisptr.transparency = value
    
    @property
    def metallic(self):
        return self.thisptr.metallic
    
    @metallic.setter
    def metallic(self, float value):
        self.thisptr.metallic = value
    
    @property
    def roughness(self):
        return self.thisptr.roughness
    
    @roughness.setter
    def roughness(self, float value):
        self.thisptr.roughness = value
    
    @property
    def enable_shadows(self):
        return self.thisptr.enable_shadows
    
    @enable_shadows.setter
    def enable_shadows(self, bint value):
        self.thisptr.enable_shadows = value

cdef class PyCameraParameters:
    cdef CameraParameters* thisptr
    
    def __cinit__(self):
        self.thisptr = new CameraParameters()
    
    def __dealloc__(self):
        del self.thisptr
    
    @property
    def position(self):
        return [self.thisptr.position[0], self.thisptr.position[1], self.thisptr.position[2]]
    
    @position.setter
    def position(self, list value):
        self.thisptr.position[0] = value[0]
        self.thisptr.position[1] = value[1]
        self.thisptr.position[2] = value[2]
    
    @property
    def target(self):
        return [self.thisptr.target[0], self.thisptr.target[1], self.thisptr.target[2]]
    
    @target.setter
    def target(self, list value):
        self.thisptr.target[0] = value[0]
        self.thisptr.target[1] = value[1]
        self.thisptr.target[2] = value[2]
    
    @property
    def fov(self):
        return self.thisptr.fov
    
    @fov.setter
    def fov(self, float value):
        self.thisptr.fov = value

cdef class PyAdvancedVisualizationModel:
    cdef AdvancedVisualizationModel* thisptr
    
    def __cinit__(self):
        self.thisptr = new AdvancedVisualizationModel()
    
    def __dealloc__(self):
        del self.thisptr
    
    def set_rendering_mode(self, int mode):
        self.thisptr.setRenderingMode(<RenderingMode>mode)
    
    def set_rendering_parameters(self, PyRenderingParameters params):
        self.thisptr.setRenderingParameters(params.thisptr[0])
    
    def enable_layer(self, int layer, bint enabled):
        self.thisptr.enableLayer(<VisualizationLayer>layer, enabled)
    
    def set_layer_transparency(self, int layer, float transparency):
        self.thisptr.setLayerTransparency(<VisualizationLayer>layer, transparency)
    
    def set_camera_parameters(self, PyCameraParameters params):
        self.thisptr.setCameraParameters(params.thisptr[0])
    
    def set_camera_position(self, float x, float y, float z):
        self.thisptr.setCameraPosition(x, y, z)
    
    def set_camera_target(self, float x, float y, float z):
        self.thisptr.setCameraTarget(x, y, z)
    
    def orbit_camera(self, float theta, float phi, float radius):
        self.thisptr.orbitCamera(theta, phi, radius)
    
    def pan_camera(self, float dx, float dy):
        self.thisptr.panCamera(dx, dy)
    
    def zoom_camera(self, float factor):
        self.thisptr.zoomCamera(factor)
    
    def add_light(self, list position, list color, float intensity):
        cdef array[float, 3] pos
        cdef array[float, 3] col
        pos[0] = position[0]
        pos[1] = position[1]
        pos[2] = position[2]
        col[0] = color[0]
        col[1] = color[1]
        col[2] = color[2]
        self.thisptr.addLight(pos, col, intensity)
    
    def start_animation(self):
        self.thisptr.startAnimation()
    
    def stop_animation(self):
        self.thisptr.stopAnimation()
    
    def pause_animation(self):
        self.thisptr.pauseAnimation()
    
    def set_animation_time(self, float time):
        self.thisptr.setAnimationTime(time)
    
    def enable_volumetric_rendering(self, bint enabled):
        self.thisptr.enableVolumetricRendering(enabled)
    
    def render_temperature_field(self, PyWafer wafer):
        self.thisptr.renderTemperatureField(wafer.thisptr)
    
    def render_dopant_distribution(self, PyWafer wafer):
        self.thisptr.renderDopantDistribution(wafer.thisptr)
    
    def render_stress_field(self, PyWafer wafer):
        self.thisptr.renderStressField(wafer.thisptr)
    
    def enable_measurement_tools(self, bint enabled):
        self.thisptr.enableMeasurementTools(enabled)
    
    def measure_distance(self, list point1, list point2):
        cdef array[float, 3] p1, p2
        p1[0] = point1[0]; p1[1] = point1[1]; p1[2] = point1[2]
        p2[0] = point2[0]; p2[1] = point2[1]; p2[2] = point2[2]
        self.thisptr.measureDistance(p1, p2)
    
    def export_image(self, str filename, int width, int height):
        self.thisptr.exportImage(filename.encode('utf-8'), width, height)
    
    def export_stl(self, PyWafer wafer, str filename):
        self.thisptr.exportSTL(wafer.thisptr, filename.encode('utf-8'))
    
    def export_3d_model(self, PyWafer wafer, str filename, str format):
        self.thisptr.export3DModel(wafer.thisptr, filename.encode('utf-8'), format.encode('utf-8'))
    
    def enable_bloom(self, bint enabled, float threshold=1.0, float intensity=1.0):
        self.thisptr.enableBloom(enabled, threshold, intensity)
    
    def enable_ssao(self, bint enabled, float radius=1.0, float intensity=1.0):
        self.thisptr.enableSSAO(enabled, radius, intensity)
    
    def set_rendering_quality(self, float quality):
        self.thisptr.setRenderingQuality(quality)
    
    def render(self, PyWafer wafer):
        self.thisptr.render(wafer.thisptr)
    
    def render_layer(self, PyWafer wafer, int layer):
        self.thisptr.renderLayer(wafer.thisptr, <VisualizationLayer>layer)
    
    def get_frame_rate(self):
        return self.thisptr.getFrameRate()
    
    def get_triangle_count(self):
        return self.thisptr.getTriangleCount()
    
    def get_render_time(self):
        return self.thisptr.getRenderTime()
    
    def get_memory_usage(self):
        return self.thisptr.getMemoryUsage()

# Constants for Python interface
RENDERING_MODE_WIREFRAME = <int>WIREFRAME
RENDERING_MODE_SOLID = <int>SOLID
RENDERING_MODE_TRANSPARENT = <int>TRANSPARENT
RENDERING_MODE_PBR = <int>PBR
RENDERING_MODE_RAY_TRACED = <int>RAY_TRACED
RENDERING_MODE_VOLUMETRIC = <int>VOLUMETRIC

LAYER_SUBSTRATE = <int>SUBSTRATE
LAYER_OXIDE = <int>OXIDE
LAYER_METAL = <int>METAL
LAYER_PHOTORESIST = <int>PHOTORESIST
LAYER_DOPANT = <int>DOPANT
LAYER_TEMPERATURE = <int>TEMPERATURE
LAYER_STRESS = <int>STRESS
LAYER_DEFECTS = <int>DEFECTS
LAYER_ALL = <int>ALL_LAYERS

ANIMATION_NONE = <int>NONE
ANIMATION_ROTATION = <int>ROTATION
ANIMATION_PROCESS_FLOW = <int>PROCESS_FLOW
ANIMATION_TIME_SERIES = <int>TIME_SERIES
ANIMATION_CROSS_SECTION = <int>CROSS_SECTION
ANIMATION_LAYER_PEEL = <int>LAYER_PEEL

# Author: Dr. Mazharuddin Mohammed
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, 
                               QPushButton, QLabel, QComboBox, QSpinBox, 
                               QDoubleSpinBox, QSlider, QCheckBox,
                               QGroupBox, QFormLayout, QTabWidget,
                               QFileDialog, QColorDialog, QFrame)
from PySide6.QtCore import Signal, Qt, QTimer
from PySide6.QtGui import QColor, QPalette
from ..simulator import Simulator
from ..advanced_visualization import (RENDERING_MODE_WIREFRAME, RENDERING_MODE_SOLID, 
                                     RENDERING_MODE_TRANSPARENT, RENDERING_MODE_PBR,
                                     RENDERING_MODE_RAY_TRACED, RENDERING_MODE_VOLUMETRIC,
                                     LAYER_SUBSTRATE, LAYER_OXIDE, LAYER_METAL, 
                                     LAYER_PHOTORESIST, LAYER_DOPANT, LAYER_TEMPERATURE,
                                     LAYER_STRESS, LAYER_DEFECTS,
                                     ANIMATION_NONE, ANIMATION_ROTATION, ANIMATION_PROCESS_FLOW)

class ColorButton(QPushButton):
    colorChanged = Signal(QColor)
    
    def __init__(self, color=QColor(255, 255, 255)):
        super().__init__()
        self.color = color
        self.setFixedSize(30, 30)
        self.update_color()
        self.clicked.connect(self.choose_color)
        
    def update_color(self):
        self.setStyleSheet(f"background-color: {self.color.name()}")
        
    def choose_color(self):
        color = QColorDialog.getColor(self.color, self)
        if color.isValid():
            self.color = color
            self.update_color()
            self.colorChanged.emit(color)

class AdvancedVisualizationPanel(QWidget):
    visualization_updated = Signal(object)
    
    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        self.animation_timer = QTimer()
        self.animation_timer.timeout.connect(self.update_animation)
        self.setup_ui()
        
    def setup_ui(self):
        layout = QVBoxLayout(self)
        
        # Create tab widget
        tab_widget = QTabWidget()
        
        # Rendering Tab
        rendering_tab = self.create_rendering_tab()
        tab_widget.addTab(rendering_tab, "Rendering")
        
        # Camera Tab
        camera_tab = self.create_camera_tab()
        tab_widget.addTab(camera_tab, "Camera")
        
        # Lighting Tab
        lighting_tab = self.create_lighting_tab()
        tab_widget.addTab(lighting_tab, "Lighting")
        
        # Animation Tab
        animation_tab = self.create_animation_tab()
        tab_widget.addTab(animation_tab, "Animation")
        
        # Effects Tab
        effects_tab = self.create_effects_tab()
        tab_widget.addTab(effects_tab, "Effects")
        
        # Export Tab
        export_tab = self.create_export_tab()
        tab_widget.addTab(export_tab, "Export")
        
        layout.addWidget(tab_widget)
        
        # Render Button
        render_button = QPushButton("Render")
        render_button.clicked.connect(self.render_scene)
        render_button.setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }")
        layout.addWidget(render_button)
        
    def create_rendering_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Rendering Mode
        mode_group = QGroupBox("Rendering Mode")
        mode_layout = QFormLayout(mode_group)
        
        self.rendering_mode = QComboBox()
        self.rendering_mode.addItems(["Wireframe", "Solid", "Transparent", "PBR", "Ray Traced", "Volumetric"])
        self.rendering_mode.currentIndexChanged.connect(self.update_rendering_mode)
        mode_layout.addRow("Mode:", self.rendering_mode)
        
        layout.addWidget(mode_group)
        
        # Layer Visibility
        layers_group = QGroupBox("Layer Visibility")
        layers_layout = QVBoxLayout(layers_group)
        
        self.layer_checkboxes = {}
        layer_names = ["Substrate", "Oxide", "Metal", "Photoresist", "Dopant", "Temperature", "Stress", "Defects"]
        layer_constants = [LAYER_SUBSTRATE, LAYER_OXIDE, LAYER_METAL, LAYER_PHOTORESIST, 
                          LAYER_DOPANT, LAYER_TEMPERATURE, LAYER_STRESS, LAYER_DEFECTS]
        
        for name, constant in zip(layer_names, layer_constants):
            checkbox = QCheckBox(name)
            checkbox.setChecked(True)
            checkbox.stateChanged.connect(lambda state, layer=constant: self.toggle_layer(layer, state == Qt.Checked))
            self.layer_checkboxes[name] = checkbox
            layers_layout.addWidget(checkbox)
            
        layout.addWidget(layers_group)
        
        # Transparency Controls
        transparency_group = QGroupBox("Transparency")
        transparency_layout = QVBoxLayout(transparency_group)
        
        for name in layer_names:
            layer_layout = QHBoxLayout()
            layer_layout.addWidget(QLabel(f"{name}:"))
            
            slider = QSlider(Qt.Horizontal)
            slider.setRange(0, 100)
            slider.setValue(100)
            slider.valueChanged.connect(lambda value, layer=name: self.set_layer_transparency(layer, value/100.0))
            
            label = QLabel("100%")
            slider.valueChanged.connect(lambda value, lbl=label: lbl.setText(f"{value}%"))
            
            layer_layout.addWidget(slider)
            layer_layout.addWidget(label)
            transparency_layout.addLayout(layer_layout)
            
        layout.addWidget(transparency_group)
        
        # Quality Settings
        quality_group = QGroupBox("Quality Settings")
        quality_layout = QFormLayout(quality_group)
        
        self.quality_slider = QSlider(Qt.Horizontal)
        self.quality_slider.setRange(1, 100)
        self.quality_slider.setValue(50)
        self.quality_slider.valueChanged.connect(self.update_quality)
        quality_layout.addRow("Rendering Quality:", self.quality_slider)
        
        self.samples_spinbox = QSpinBox()
        self.samples_spinbox.setRange(1, 64)
        self.samples_spinbox.setValue(1)
        quality_layout.addRow("Samples per Pixel:", self.samples_spinbox)
        
        layout.addWidget(quality_group)
        
        return widget
        
    def create_camera_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Camera Position
        position_group = QGroupBox("Camera Position")
        position_layout = QFormLayout(position_group)
        
        self.camera_x = QDoubleSpinBox()
        self.camera_x.setRange(-100.0, 100.0)
        self.camera_x.setValue(0.0)
        self.camera_x.valueChanged.connect(self.update_camera_position)
        
        self.camera_y = QDoubleSpinBox()
        self.camera_y.setRange(-100.0, 100.0)
        self.camera_y.setValue(0.0)
        self.camera_y.valueChanged.connect(self.update_camera_position)
        
        self.camera_z = QDoubleSpinBox()
        self.camera_z.setRange(-100.0, 100.0)
        self.camera_z.setValue(5.0)
        self.camera_z.valueChanged.connect(self.update_camera_position)
        
        position_layout.addRow("X:", self.camera_x)
        position_layout.addRow("Y:", self.camera_y)
        position_layout.addRow("Z:", self.camera_z)
        
        layout.addWidget(position_group)
        
        # Camera Target
        target_group = QGroupBox("Camera Target")
        target_layout = QFormLayout(target_group)
        
        self.target_x = QDoubleSpinBox()
        self.target_x.setRange(-100.0, 100.0)
        self.target_x.setValue(0.0)
        self.target_x.valueChanged.connect(self.update_camera_target)
        
        self.target_y = QDoubleSpinBox()
        self.target_y.setRange(-100.0, 100.0)
        self.target_y.setValue(0.0)
        self.target_y.valueChanged.connect(self.update_camera_target)
        
        self.target_z = QDoubleSpinBox()
        self.target_z.setRange(-100.0, 100.0)
        self.target_z.setValue(0.0)
        self.target_z.valueChanged.connect(self.update_camera_target)
        
        target_layout.addRow("X:", self.target_x)
        target_layout.addRow("Y:", self.target_y)
        target_layout.addRow("Z:", self.target_z)
        
        layout.addWidget(target_group)
        
        # Orbit Controls
        orbit_group = QGroupBox("Orbit Controls")
        orbit_layout = QFormLayout(orbit_group)
        
        self.orbit_theta = QSlider(Qt.Horizontal)
        self.orbit_theta.setRange(0, 360)
        self.orbit_theta.setValue(0)
        self.orbit_theta.valueChanged.connect(self.update_orbit)
        
        self.orbit_phi = QSlider(Qt.Horizontal)
        self.orbit_phi.setRange(0, 180)
        self.orbit_phi.setValue(90)
        self.orbit_phi.valueChanged.connect(self.update_orbit)
        
        self.orbit_radius = QDoubleSpinBox()
        self.orbit_radius.setRange(0.1, 100.0)
        self.orbit_radius.setValue(5.0)
        self.orbit_radius.valueChanged.connect(self.update_orbit)
        
        orbit_layout.addRow("Theta (°):", self.orbit_theta)
        orbit_layout.addRow("Phi (°):", self.orbit_phi)
        orbit_layout.addRow("Radius:", self.orbit_radius)
        
        layout.addWidget(orbit_group)
        
        # Camera Presets
        presets_group = QGroupBox("Camera Presets")
        presets_layout = QHBoxLayout(presets_group)
        
        front_button = QPushButton("Front")
        front_button.clicked.connect(lambda: self.set_camera_preset("front"))
        presets_layout.addWidget(front_button)
        
        top_button = QPushButton("Top")
        top_button.clicked.connect(lambda: self.set_camera_preset("top"))
        presets_layout.addWidget(top_button)
        
        side_button = QPushButton("Side")
        side_button.clicked.connect(lambda: self.set_camera_preset("side"))
        presets_layout.addWidget(side_button)
        
        iso_button = QPushButton("Isometric")
        iso_button.clicked.connect(lambda: self.set_camera_preset("iso"))
        presets_layout.addWidget(iso_button)
        
        layout.addWidget(presets_group)
        
        return widget
        
    def create_lighting_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Ambient Lighting
        ambient_group = QGroupBox("Ambient Lighting")
        ambient_layout = QFormLayout(ambient_group)
        
        self.ambient_color_button = ColorButton(QColor(50, 50, 50))
        self.ambient_color_button.colorChanged.connect(self.update_ambient_color)
        
        self.ambient_intensity = QSlider(Qt.Horizontal)
        self.ambient_intensity.setRange(0, 100)
        self.ambient_intensity.setValue(20)
        self.ambient_intensity.valueChanged.connect(self.update_ambient_intensity)
        
        ambient_layout.addRow("Color:", self.ambient_color_button)
        ambient_layout.addRow("Intensity:", self.ambient_intensity)
        
        layout.addWidget(ambient_group)
        
        # Light Sources
        lights_group = QGroupBox("Light Sources")
        lights_layout = QVBoxLayout(lights_group)
        
        # Add Light Controls
        add_light_layout = QHBoxLayout()
        
        add_light_button = QPushButton("Add Light")
        add_light_button.clicked.connect(self.add_light)
        add_light_layout.addWidget(add_light_button)
        
        remove_light_button = QPushButton("Remove Light")
        remove_light_button.clicked.connect(self.remove_light)
        add_light_layout.addWidget(remove_light_button)
        
        lights_layout.addLayout(add_light_layout)
        
        # Light Position Controls
        light_pos_layout = QFormLayout()
        
        self.light_x = QDoubleSpinBox()
        self.light_x.setRange(-100.0, 100.0)
        self.light_x.setValue(5.0)
        
        self.light_y = QDoubleSpinBox()
        self.light_y.setRange(-100.0, 100.0)
        self.light_y.setValue(5.0)
        
        self.light_z = QDoubleSpinBox()
        self.light_z.setRange(-100.0, 100.0)
        self.light_z.setValue(5.0)
        
        self.light_color_button = ColorButton(QColor(255, 255, 255))
        
        self.light_intensity = QSlider(Qt.Horizontal)
        self.light_intensity.setRange(0, 100)
        self.light_intensity.setValue(100)
        
        light_pos_layout.addRow("Light X:", self.light_x)
        light_pos_layout.addRow("Light Y:", self.light_y)
        light_pos_layout.addRow("Light Z:", self.light_z)
        light_pos_layout.addRow("Color:", self.light_color_button)
        light_pos_layout.addRow("Intensity:", self.light_intensity)
        
        lights_layout.addLayout(light_pos_layout)
        
        layout.addWidget(lights_group)
        
        return widget
        
    def create_animation_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Animation Type
        type_group = QGroupBox("Animation Type")
        type_layout = QFormLayout(type_group)
        
        self.animation_type = QComboBox()
        self.animation_type.addItems(["None", "Rotation", "Process Flow", "Time Series", "Cross Section", "Layer Peel"])
        type_layout.addRow("Type:", self.animation_type)
        
        layout.addWidget(type_group)
        
        # Animation Controls
        controls_group = QGroupBox("Animation Controls")
        controls_layout = QVBoxLayout(controls_group)
        
        # Playback Controls
        playback_layout = QHBoxLayout()
        
        self.play_button = QPushButton("Play")
        self.play_button.clicked.connect(self.play_animation)
        playback_layout.addWidget(self.play_button)
        
        self.pause_button = QPushButton("Pause")
        self.pause_button.clicked.connect(self.pause_animation)
        playback_layout.addWidget(self.pause_button)
        
        self.stop_button = QPushButton("Stop")
        self.stop_button.clicked.connect(self.stop_animation)
        playback_layout.addWidget(self.stop_button)
        
        controls_layout.addLayout(playback_layout)
        
        # Animation Parameters
        params_layout = QFormLayout()
        
        self.animation_duration = QDoubleSpinBox()
        self.animation_duration.setRange(0.1, 60.0)
        self.animation_duration.setValue(5.0)
        self.animation_duration.setSuffix(" s")
        
        self.animation_speed = QSlider(Qt.Horizontal)
        self.animation_speed.setRange(1, 500)
        self.animation_speed.setValue(100)
        
        self.animation_loop = QCheckBox("Loop")
        
        params_layout.addRow("Duration:", self.animation_duration)
        params_layout.addRow("Speed:", self.animation_speed)
        params_layout.addRow("", self.animation_loop)
        
        controls_layout.addLayout(params_layout)
        
        layout.addWidget(controls_group)
        
        return widget
        
    def create_effects_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Post-Processing Effects
        effects_group = QGroupBox("Post-Processing Effects")
        effects_layout = QFormLayout(effects_group)
        
        # Bloom
        self.bloom_enabled = QCheckBox("Enable Bloom")
        self.bloom_threshold = QSlider(Qt.Horizontal)
        self.bloom_threshold.setRange(0, 100)
        self.bloom_threshold.setValue(80)
        self.bloom_intensity = QSlider(Qt.Horizontal)
        self.bloom_intensity.setRange(0, 100)
        self.bloom_intensity.setValue(50)
        
        effects_layout.addRow("", self.bloom_enabled)
        effects_layout.addRow("Bloom Threshold:", self.bloom_threshold)
        effects_layout.addRow("Bloom Intensity:", self.bloom_intensity)
        
        # SSAO
        self.ssao_enabled = QCheckBox("Enable SSAO")
        self.ssao_radius = QSlider(Qt.Horizontal)
        self.ssao_radius.setRange(1, 100)
        self.ssao_radius.setValue(50)
        self.ssao_intensity = QSlider(Qt.Horizontal)
        self.ssao_intensity.setRange(0, 100)
        self.ssao_intensity.setValue(50)
        
        effects_layout.addRow("", self.ssao_enabled)
        effects_layout.addRow("SSAO Radius:", self.ssao_radius)
        effects_layout.addRow("SSAO Intensity:", self.ssao_intensity)
        
        layout.addWidget(effects_group)
        
        # Volumetric Rendering
        volumetric_group = QGroupBox("Volumetric Rendering")
        volumetric_layout = QFormLayout(volumetric_group)
        
        self.volumetric_enabled = QCheckBox("Enable Volumetric Rendering")
        self.volumetric_density = QSlider(Qt.Horizontal)
        self.volumetric_density.setRange(0, 100)
        self.volumetric_density.setValue(50)
        
        volumetric_layout.addRow("", self.volumetric_enabled)
        volumetric_layout.addRow("Density:", self.volumetric_density)
        
        # Volumetric Data Buttons
        volumetric_buttons_layout = QHBoxLayout()
        
        temp_button = QPushButton("Temperature Field")
        temp_button.clicked.connect(self.render_temperature_field)
        volumetric_buttons_layout.addWidget(temp_button)
        
        dopant_button = QPushButton("Dopant Distribution")
        dopant_button.clicked.connect(self.render_dopant_distribution)
        volumetric_buttons_layout.addWidget(dopant_button)
        
        stress_button = QPushButton("Stress Field")
        stress_button.clicked.connect(self.render_stress_field)
        volumetric_buttons_layout.addWidget(stress_button)
        
        volumetric_layout.addRow("Visualize:", volumetric_buttons_layout)
        
        layout.addWidget(volumetric_group)
        
        return widget
        
    def create_export_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Image Export
        image_group = QGroupBox("Image Export")
        image_layout = QFormLayout(image_group)
        
        self.image_width = QSpinBox()
        self.image_width.setRange(100, 8192)
        self.image_width.setValue(1920)
        
        self.image_height = QSpinBox()
        self.image_height.setRange(100, 8192)
        self.image_height.setValue(1080)
        
        image_layout.addRow("Width:", self.image_width)
        image_layout.addRow("Height:", self.image_height)
        
        export_image_button = QPushButton("Export Image")
        export_image_button.clicked.connect(self.export_image)
        image_layout.addRow(export_image_button)
        
        layout.addWidget(image_group)
        
        # 3D Model Export
        model_group = QGroupBox("3D Model Export")
        model_layout = QFormLayout(model_group)
        
        self.model_format = QComboBox()
        self.model_format.addItems(["STL", "OBJ", "PLY", "GLTF"])
        
        model_layout.addRow("Format:", self.model_format)
        
        export_stl_button = QPushButton("Export STL")
        export_stl_button.clicked.connect(self.export_stl)
        model_layout.addRow(export_stl_button)
        
        export_model_button = QPushButton("Export 3D Model")
        export_model_button.clicked.connect(self.export_3d_model)
        model_layout.addRow(export_model_button)
        
        layout.addWidget(model_group)
        
        return widget
        
    # Event handlers
    def update_rendering_mode(self):
        mode = self.rendering_mode.currentIndex()
        self.simulator.set_rendering_mode(mode)
        
    def toggle_layer(self, layer, enabled):
        self.simulator.enable_layer(layer, enabled)
        
    def set_layer_transparency(self, layer_name, transparency):
        # Map layer name to constant
        layer_map = {
            "Substrate": LAYER_SUBSTRATE,
            "Oxide": LAYER_OXIDE,
            "Metal": LAYER_METAL,
            "Photoresist": LAYER_PHOTORESIST,
            "Dopant": LAYER_DOPANT,
            "Temperature": LAYER_TEMPERATURE,
            "Stress": LAYER_STRESS,
            "Defects": LAYER_DEFECTS
        }
        if layer_name in layer_map:
            self.simulator.set_layer_transparency(layer_map[layer_name], transparency)
            
    def update_quality(self):
        quality = self.quality_slider.value() / 100.0
        self.simulator.set_rendering_quality(quality)
        
    def update_camera_position(self):
        x = self.camera_x.value()
        y = self.camera_y.value()
        z = self.camera_z.value()
        self.simulator.set_camera_position(x, y, z)
        
    def update_camera_target(self):
        x = self.target_x.value()
        y = self.target_y.value()
        z = self.target_z.value()
        self.simulator.set_camera_target(x, y, z)
        
    def update_orbit(self):
        theta = self.orbit_theta.value() * 3.14159 / 180.0
        phi = self.orbit_phi.value() * 3.14159 / 180.0
        radius = self.orbit_radius.value()
        self.simulator.orbit_camera(theta, phi, radius)
        
    def set_camera_preset(self, preset):
        if preset == "front":
            self.camera_x.setValue(0)
            self.camera_y.setValue(0)
            self.camera_z.setValue(5)
        elif preset == "top":
            self.camera_x.setValue(0)
            self.camera_y.setValue(5)
            self.camera_z.setValue(0)
        elif preset == "side":
            self.camera_x.setValue(5)
            self.camera_y.setValue(0)
            self.camera_z.setValue(0)
        elif preset == "iso":
            self.camera_x.setValue(3)
            self.camera_y.setValue(3)
            self.camera_z.setValue(3)
            
    def add_light(self):
        position = [self.light_x.value(), self.light_y.value(), self.light_z.value()]
        color = [self.light_color_button.color.redF(), 
                self.light_color_button.color.greenF(), 
                self.light_color_button.color.blueF()]
        intensity = self.light_intensity.value() / 100.0
        self.simulator.add_light(position, color, intensity)
        
    def remove_light(self):
        # This would remove the last added light
        pass
        
    def play_animation(self):
        self.simulator.start_animation()
        self.animation_timer.start(50)  # 20 FPS
        
    def pause_animation(self):
        self.simulator.pause_animation()
        self.animation_timer.stop()
        
    def stop_animation(self):
        self.simulator.stop_animation()
        self.animation_timer.stop()
        
    def update_animation(self):
        # This would update the animation frame
        pass
        
    def render_temperature_field(self):
        self.simulator.enable_volumetric_rendering(True)
        self.simulator.render_temperature_field()
        
    def render_dopant_distribution(self):
        self.simulator.enable_volumetric_rendering(True)
        self.simulator.render_dopant_distribution()
        
    def render_stress_field(self):
        self.simulator.enable_volumetric_rendering(True)
        self.simulator.render_stress_field()
        
    def export_image(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Export Image", "render.png", "PNG Files (*.png)")
        if filename:
            width = self.image_width.value()
            height = self.image_height.value()
            self.simulator.export_image(filename, width, height)
            
    def export_stl(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Export STL", "wafer.stl", "STL Files (*.stl)")
        if filename:
            self.simulator.export_stl(filename)
            
    def export_3d_model(self):
        format_ext = self.model_format.currentText().lower()
        filename, _ = QFileDialog.getSaveFileName(self, "Export 3D Model", f"wafer.{format_ext}", 
                                                 f"{format_ext.upper()} Files (*.{format_ext})")
        if filename:
            self.simulator.export_3d_model(filename, format_ext)
            
    def render_scene(self):
        self.simulator.render_advanced()
        self.visualization_updated.emit(self.simulator.get_wafer())
        
    def update_ambient_color(self, color):
        # This would update ambient lighting color
        pass
        
    def update_ambient_intensity(self):
        # This would update ambient lighting intensity
        pass

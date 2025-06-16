# Author: Dr. Mazharuddin Mohammed
from .geometry import PyGeometryManager, PyWafer
from .oxidation import PyOxidationModel
from .doping import PyDopingManager
from .photolithography import PyLithographyModel
from .deposition import PyDepositionModel
from .etching import PyEtchingModel
from .metallization import PyMetallizationModel
from .packaging import PyPackagingModel
from .thermal import PyThermalSimulationModel
from .reliability import PyReliabilityModel
from .renderer import PyVulkanRenderer
from .multi_die import PyMultiDieModel, PyDie, PyInterconnect
from .drc import PyDRCModel, PyDRCRule
from .advanced_visualization import PyAdvancedVisualizationModel

class Simulator:
    def __init__(self):
        self.geometry_manager = PyGeometryManager()
        self.oxidation_model = PyOxidationModel()
        self.doping_manager = PyDopingManager()
        self.lithography_model = PyLithographyModel()
        self.deposition_model = PyDepositionModel()
        self.etching_model = PyEtchingModel()
        self.metallization_model = PyMetallizationModel()
        self.packaging_model = PyPackagingModel()
        self.thermal_model = PyThermalSimulationModel()
        self.reliability_model = PyReliabilityModel()
        self.multi_die_model = PyMultiDieModel()
        self.drc_model = PyDRCModel()
        self.advanced_visualization = PyAdvancedVisualizationModel()
        self.wafer = PyWafer(300.0, 775.0, "silicon")
        self.renderer = None

    def initialize_geometry(self, x_dim: int, y_dim: int):
        self.geometry_manager.initialize_grid(self.wafer, x_dim, y_dim)

    def apply_layer(self, thickness: float, material_id: str):
        self.geometry_manager.apply_layer(self.wafer, thickness, material_id)

    def run_oxidation(self, temperature: float, time: float):
        self.oxidation_model.simulate_oxidation(self.wafer, temperature, time)

    def run_ion_implantation(self, energy: float, dose: float):
        self.doping_manager.simulate_ion_implantation(self.wafer, energy, dose)

    def run_diffusion(self, temperature: float, time: float):
        self.doping_manager.simulate_diffusion(self.wafer, temperature, time)

    def run_exposure(self, wavelength: float, na: float, mask: list):
        self.lithography_model.simulate_exposure(self.wafer, wavelength, na, mask)

    def run_multi_patterning(self, wavelength: float, na: float, masks: list):
        self.lithography_model.simulate_multi_patterning(self.wafer, wavelength, na, masks)

    def run_deposition(self, thickness: float, material: str, type: str):
        self.deposition_model.simulate_deposition(self.wafer, thickness, material, type)

    def run_etching(self, depth: float, type: str):
        self.etching_model.simulate_etching(self.wafer, depth, type)

    def run_metallization(self, thickness: float, metal: str, method: str):
        self.metallization_model.simulate_metallization(self.wafer, thickness, metal, method)

    def simulate_packaging(self, substrate_thickness: float, substrate_material: str, num_wires: int):
        self.packaging_model.simulate_packaging(self.wafer, substrate_thickness, substrate_material, num_wires)

    def perform_electrical_test(self, test_type: str):
        self.packaging_model.perform_electrical_test(self.wafer, test_type)

    def simulate_thermal(self, ambient_temperature: float, current: float):
        self.thermal_model.simulate_thermal(self.wafer, ambient_temperature, current)

    def perform_reliability_test(self, current: float, voltage: float):
        self.reliability_model.perform_reliability_test(self.wafer, current, voltage)

    def initialize_renderer(self, width: int, height: int):
        self.renderer = PyVulkanRenderer(width, height)
        self.renderer.initialize()

    # Multi-die integration methods
    def add_die(self, die_id: str, die_type: int, width: float, height: float, thickness: float):
        die = PyDie(die_id, die_type, width, height, thickness)
        self.multi_die_model.add_die(die)
        return die

    def position_die(self, die_id: str, x: float, y: float):
        self.multi_die_model.position_die(die_id, x, y)

    def perform_wire_bonding(self, die1: str, die2: str, bonds: list):
        self.multi_die_model.perform_wire_bonding(self.wafer, die1, die2, bonds)

    def perform_flip_chip_bonding(self, die1: str, die2: str, bump_pitch: float, bump_diameter: float):
        self.multi_die_model.perform_flip_chip_bonding(self.wafer, die1, die2, bump_pitch, bump_diameter)

    def perform_tsv_integration(self, die_id: str, tsv_positions: list, tsv_diameter: float, tsv_depth: float):
        self.multi_die_model.perform_tsv_integration(self.wafer, die_id, tsv_positions, tsv_diameter, tsv_depth)

    def analyze_electrical_performance(self):
        self.multi_die_model.analyze_electrical_performance(self.wafer)

    def analyze_thermal_performance(self):
        self.multi_die_model.analyze_thermal_performance(self.wafer)

    def get_system_metrics(self):
        return self.multi_die_model.get_system_metrics()

    # Design Rule Check methods
    def add_drc_rule(self, rule_name: str, rule_type: int, layer: str, min_value: float, max_value: float = -1.0):
        rule = PyDRCRule(rule_name, rule_type, layer, min_value, max_value)
        self.drc_model.add_rule(rule)
        return rule

    def load_technology_rules(self, technology_node: str):
        self.drc_model.load_technology_rules(technology_node)

    def run_full_drc(self):
        self.drc_model.run_full_drc(self.wafer)

    def run_layer_drc(self, layer: str):
        self.drc_model.run_layer_drc(self.wafer, layer)

    def get_violation_count(self):
        return self.drc_model.get_violation_count()

    def get_critical_violation_count(self):
        return self.drc_model.get_critical_violation_count()

    def generate_drc_report(self, filename: str):
        self.drc_model.generate_drc_report(filename)

    def get_violations_by_layer(self):
        return self.drc_model.get_violations_by_layer()

    # Advanced visualization methods
    def set_rendering_mode(self, mode: int):
        self.advanced_visualization.set_rendering_mode(mode)

    def enable_layer(self, layer: int, enabled: bool):
        self.advanced_visualization.enable_layer(layer, enabled)

    def set_camera_position(self, x: float, y: float, z: float):
        self.advanced_visualization.set_camera_position(x, y, z)

    def set_camera_target(self, x: float, y: float, z: float):
        self.advanced_visualization.set_camera_target(x, y, z)

    def orbit_camera(self, theta: float, phi: float, radius: float):
        self.advanced_visualization.orbit_camera(theta, phi, radius)

    def add_light(self, position: list, color: list, intensity: float):
        self.advanced_visualization.add_light(position, color, intensity)

    def start_animation(self):
        self.advanced_visualization.start_animation()

    def stop_animation(self):
        self.advanced_visualization.stop_animation()

    def enable_volumetric_rendering(self, enabled: bool):
        self.advanced_visualization.enable_volumetric_rendering(enabled)

    def render_temperature_field(self):
        self.advanced_visualization.render_temperature_field(self.wafer)

    def render_dopant_distribution(self):
        self.advanced_visualization.render_dopant_distribution(self.wafer)

    def export_image(self, filename: str, width: int, height: int):
        self.advanced_visualization.export_image(filename, width, height)

    def export_stl(self, filename: str):
        self.advanced_visualization.export_stl(self.wafer, filename)

    def enable_bloom(self, enabled: bool, threshold: float = 1.0, intensity: float = 1.0):
        self.advanced_visualization.enable_bloom(enabled, threshold, intensity)

    def set_rendering_quality(self, quality: float):
        self.advanced_visualization.set_rendering_quality(quality)

    def render_advanced(self):
        self.advanced_visualization.render(self.wafer)

    def get_frame_rate(self):
        return self.advanced_visualization.get_frame_rate()

    def get_render_time(self):
        return self.advanced_visualization.get_render_time()

    def render(self, wafer):
        if self.renderer:
            self.renderer.render(wafer)

    def get_wafer(self):
        return self.wafer

    def get_renderer_window(self):
        return self.renderer.get_window()
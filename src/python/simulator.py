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

    def render(self, wafer):
        if self.renderer:
            self.renderer.render(wafer)

    def get_wafer(self):
        return self.wafer

    def get_renderer_window(self):
        return self.renderer.get_window()
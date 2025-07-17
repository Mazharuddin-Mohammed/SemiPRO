# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/deposition/deposition_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer
import numpy as np
cimport numpy as cnp

# Enhanced deposition structures
cdef extern from "../cpp/modules/deposition/deposition_model.hpp":
    cdef enum DepositionTechnique:
        CVD
        LPCVD
        PECVD
        MOCVD
        PVD_SPUTTERING
        PVD_EVAPORATION
        ALD
        ELECTROPLATING
        ELECTROLESS_PLATING

    cdef enum MaterialType:
        MATERIAL_SILICON
        MATERIAL_SILICON_DIOXIDE
        MATERIAL_SILICON_NITRIDE
        MATERIAL_POLYSILICON
        MATERIAL_ALUMINUM
        MATERIAL_COPPER
        MATERIAL_TUNGSTEN
        MATERIAL_TITANIUM
        MATERIAL_TITANIUM_NITRIDE
        MATERIAL_TANTALUM
        MATERIAL_HAFNIUM_OXIDE

    cdef struct DepositionConditions:
        DepositionTechnique technique
        MaterialType material
        double target_thickness
        double temperature
        double pressure
        double gas_flow_rate
        string precursor_gas
        double rf_power
        double dc_bias
        double substrate_bias
        bint enable_stress_modeling
        bint enable_grain_modeling
        bint enable_interface_effects

    cdef struct DepositionResults:
        double final_thickness
        double deposition_rate
        double uniformity
        double step_coverage
        double conformality
        double stress_level
        double grain_size
        vector[double] thickness_profile
        unordered_map[string, double] quality_metrics
        unordered_map[string, double] process_parameters

    cppclass DepositionModel:
        DepositionModel() except +
        void simulateDeposition(shared_ptr[Wafer], double, string, string) except +
        DepositionResults simulateEnhancedDeposition(shared_ptr[Wafer], const DepositionConditions&) except +

# Python wrapper classes
cdef class PyDepositionConditions:
    cdef DepositionConditions conditions

    def __init__(self):
        self.conditions.technique = CVD
        self.conditions.material = MATERIAL_SILICON_DIOXIDE
        self.conditions.target_thickness = 100.0  # nm
        self.conditions.temperature = 400.0       # °C
        self.conditions.pressure = 1.0            # Torr
        self.conditions.gas_flow_rate = 100.0     # sccm
        self.conditions.precursor_gas = b"SiH4"
        self.conditions.rf_power = 100.0          # W
        self.conditions.dc_bias = 0.0             # V
        self.conditions.substrate_bias = 0.0      # V
        self.conditions.enable_stress_modeling = True
        self.conditions.enable_grain_modeling = True
        self.conditions.enable_interface_effects = True

    @property
    def technique(self):
        return self.conditions.technique

    @technique.setter
    def technique(self, value):
        self.conditions.technique = value

    @property
    def material(self):
        return self.conditions.material

    @material.setter
    def material(self, value):
        self.conditions.material = value

    @property
    def target_thickness(self):
        return self.conditions.target_thickness

    @target_thickness.setter
    def target_thickness(self, value):
        self.conditions.target_thickness = value

    @property
    def temperature(self):
        return self.conditions.temperature

    @temperature.setter
    def temperature(self, value):
        self.conditions.temperature = value

    @property
    def pressure(self):
        return self.conditions.pressure

    @pressure.setter
    def pressure(self, value):
        self.conditions.pressure = value

cdef class PyDepositionResults:
    cdef DepositionResults results

    @property
    def final_thickness(self):
        return self.results.final_thickness

    @property
    def deposition_rate(self):
        return self.results.deposition_rate

    @property
    def uniformity(self):
        return self.results.uniformity

    @property
    def step_coverage(self):
        return self.results.step_coverage

    @property
    def conformality(self):
        return self.results.conformality

    @property
    def stress_level(self):
        return self.results.stress_level

    @property
    def grain_size(self):
        return self.results.grain_size

    @property
    def thickness_profile(self):
        return np.array(self.results.thickness_profile)

    @property
    def quality_metrics(self):
        return dict(self.results.quality_metrics)

    @property
    def process_parameters(self):
        return dict(self.results.process_parameters)

cdef class PyDepositionModel:
    cdef DepositionModel* thisptr

    def __cinit__(self):
        self.thisptr = new DepositionModel()

    def __dealloc__(self):
        del self.thisptr

    def simulate_deposition(self, wafer: PyWafer, thickness: float, material: str, type: str):
        """Basic deposition simulation"""
        self.thisptr.simulateDeposition(wafer.thisptr, thickness, material.encode('utf-8'), type.encode('utf-8'))

    def simulate_enhanced_deposition(self, wafer: PyWafer, conditions: PyDepositionConditions):
        """Enhanced deposition simulation with detailed parameters"""
        cdef DepositionResults cpp_results = self.thisptr.simulateEnhancedDeposition(wafer.thisptr, conditions.conditions)

        py_results = PyDepositionResults()
        py_results.results = cpp_results
        return py_results

# Constants for Python use
CVD_TECHNIQUE = CVD
LPCVD_TECHNIQUE = LPCVD
PECVD_TECHNIQUE = PECVD
MOCVD_TECHNIQUE = MOCVD
PVD_SPUTTERING_TECHNIQUE = PVD_SPUTTERING
PVD_EVAPORATION_TECHNIQUE = PVD_EVAPORATION
ALD_TECHNIQUE = ALD
ELECTROPLATING_TECHNIQUE = ELECTROPLATING
ELECTROLESS_PLATING_TECHNIQUE = ELECTROLESS_PLATING

MATERIAL_SILICON_TYPE = MATERIAL_SILICON
MATERIAL_SILICON_DIOXIDE_TYPE = MATERIAL_SILICON_DIOXIDE
MATERIAL_SILICON_NITRIDE_TYPE = MATERIAL_SILICON_NITRIDE
MATERIAL_POLYSILICON_TYPE = MATERIAL_POLYSILICON
MATERIAL_ALUMINUM_TYPE = MATERIAL_ALUMINUM
MATERIAL_COPPER_TYPE = MATERIAL_COPPER
MATERIAL_TUNGSTEN_TYPE = MATERIAL_TUNGSTEN
MATERIAL_TITANIUM_TYPE = MATERIAL_TITANIUM
MATERIAL_TITANIUM_NITRIDE_TYPE = MATERIAL_TITANIUM_NITRIDE
MATERIAL_TANTALUM_TYPE = MATERIAL_TANTALUM
MATERIAL_HAFNIUM_OXIDE_TYPE = MATERIAL_HAFNIUM_OXIDE
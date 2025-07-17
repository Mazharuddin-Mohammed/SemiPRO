# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/oxidation/oxidation_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer

# Enhanced oxidation conditions structure
cdef extern from "../cpp/modules/oxidation/oxidation_model.hpp":
    cdef enum OxidationAtmosphere:
        DRY_OXYGEN
        WET_OXYGEN
        STEAM
        PYROGENIC

    cdef enum CrystalOrientation:
        ORIENTATION_100
        ORIENTATION_110
        ORIENTATION_111

    cdef struct OxidationConditions:
        double temperature
        double time
        double pressure
        OxidationAtmosphere atmosphere
        CrystalOrientation orientation
        double dopant_concentration
        string dopant_type
        double initial_oxide_thickness
        bint enable_stress_effects
        bint enable_orientation_effects
        bint enable_dopant_effects

    cdef struct OxidationResults:
        double final_thickness
        double growth_rate
        double interface_roughness
        double stress_level
        double uniformity
        vector[double] thickness_profile
        unordered_map[string, double] quality_metrics

    cppclass OxidationModel:
        OxidationModel() except +
        void simulateOxidation(shared_ptr[Wafer], double, double) except +
        OxidationResults simulateEnhancedOxidation(shared_ptr[Wafer], const OxidationConditions&) except +

# Python wrapper classes
cdef class PyOxidationConditions:
    cdef OxidationConditions conditions

    def __init__(self):
        self.conditions.temperature = 1000.0
        self.conditions.time = 1.0
        self.conditions.pressure = 1.0
        self.conditions.atmosphere = DRY_OXYGEN
        self.conditions.orientation = ORIENTATION_100
        self.conditions.dopant_concentration = 0.0
        self.conditions.dopant_type = b"none"
        self.conditions.initial_oxide_thickness = 0.0
        self.conditions.enable_stress_effects = True
        self.conditions.enable_orientation_effects = True
        self.conditions.enable_dopant_effects = True

    @property
    def temperature(self):
        return self.conditions.temperature

    @temperature.setter
    def temperature(self, value):
        self.conditions.temperature = value

    @property
    def time(self):
        return self.conditions.time

    @time.setter
    def time(self, value):
        self.conditions.time = value

    @property
    def pressure(self):
        return self.conditions.pressure

    @pressure.setter
    def pressure(self, value):
        self.conditions.pressure = value

    @property
    def atmosphere(self):
        return self.conditions.atmosphere

    @atmosphere.setter
    def atmosphere(self, value):
        self.conditions.atmosphere = value

    @property
    def orientation(self):
        return self.conditions.orientation

    @orientation.setter
    def orientation(self, value):
        self.conditions.orientation = value

cdef class PyOxidationResults:
    cdef OxidationResults results

    @property
    def final_thickness(self):
        return self.results.final_thickness

    @property
    def growth_rate(self):
        return self.results.growth_rate

    @property
    def interface_roughness(self):
        return self.results.interface_roughness

    @property
    def stress_level(self):
        return self.results.stress_level

    @property
    def uniformity(self):
        return self.results.uniformity

    @property
    def thickness_profile(self):
        return list(self.results.thickness_profile)

    @property
    def quality_metrics(self):
        return dict(self.results.quality_metrics)

cdef class PyOxidationModel:
    cdef OxidationModel* thisptr

    def __cinit__(self):
        self.thisptr = new OxidationModel()

    def __dealloc__(self):
        del self.thisptr

    def simulate_oxidation(self, wafer: PyWafer, temperature: float, time: float):
        """Basic oxidation simulation"""
        self.thisptr.simulateOxidation(wafer.thisptr, temperature, time)

    def simulate_enhanced_oxidation(self, wafer: PyWafer, conditions: PyOxidationConditions):
        """Enhanced oxidation simulation with detailed parameters"""
        cdef OxidationResults cpp_results = self.thisptr.simulateEnhancedOxidation(wafer.thisptr, conditions.conditions)

        py_results = PyOxidationResults()
        py_results.results = cpp_results
        return py_results

# Constants for Python use
DRY_OXYGEN_ATMOSPHERE = DRY_OXYGEN
WET_OXYGEN_ATMOSPHERE = WET_OXYGEN
STEAM_ATMOSPHERE = STEAM
PYROGENIC_ATMOSPHERE = PYROGENIC

ORIENTATION_100_CRYSTAL = ORIENTATION_100
ORIENTATION_110_CRYSTAL = ORIENTATION_110
ORIENTATION_111_CRYSTAL = ORIENTATION_111
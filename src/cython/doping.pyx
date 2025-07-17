# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/doping/monte_carlo_solver.cpp ../cpp/modules/doping/diffusion_solver.cpp ../cpp/modules/doping/doping_manager.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer
import numpy as np
cimport numpy as cnp

# Enhanced doping structures
cdef extern from "../cpp/modules/doping/doping_manager.hpp":
    cdef enum IonSpecies:
        ION_BORON_11
        ION_PHOSPHORUS_31
        ION_ARSENIC_75
        ION_ANTIMONY_121
        ION_INDIUM_115
        ION_GALLIUM_69

    cdef enum ImplantationTechnique:
        STANDARD_IMPLANT
        HIGH_CURRENT_IMPLANT
        MEDIUM_CURRENT_IMPLANT
        HIGH_ENERGY_IMPLANT
        PLASMA_IMMERSION

    cdef struct ImplantationConditions:
        IonSpecies species
        double energy
        double dose
        double tilt_angle
        double twist_angle
        double beam_current
        ImplantationTechnique technique
        double substrate_temperature
        bint enable_channeling
        bint enable_sputtering
        bint enable_damage_modeling

    cdef struct DiffusionConditions:
        double temperature
        double time
        double ambient_pressure
        string ambient_gas
        bint enable_segregation
        bint enable_clustering
        double initial_concentration

    cdef struct DopingResults:
        vector[double] concentration_profile
        double peak_concentration
        double junction_depth
        double sheet_resistance
        double projected_range
        double range_straggling
        double lateral_straggling
        double channeling_fraction
        double sputtering_yield
        unordered_map[string, double] electrical_properties

    cppclass DopingManager:
        DopingManager() except +
        void simulateIonImplantation(shared_ptr[Wafer], double, double) except +
        void simulateDiffusion(shared_ptr[Wafer], double, double) except +
        DopingResults simulateEnhancedImplantation(shared_ptr[Wafer], const ImplantationConditions&) except +
        DopingResults simulateEnhancedDiffusion(shared_ptr[Wafer], const DiffusionConditions&) except +

# Python wrapper classes
cdef class PyImplantationConditions:
    cdef ImplantationConditions conditions

    def __init__(self):
        self.conditions.species = ION_BORON_11
        self.conditions.energy = 50000.0  # 50 keV
        self.conditions.dose = 1e15       # cm^-2
        self.conditions.tilt_angle = 7.0  # degrees
        self.conditions.twist_angle = 0.0
        self.conditions.beam_current = 1e-3  # mA
        self.conditions.technique = STANDARD_IMPLANT
        self.conditions.substrate_temperature = 300.0  # K
        self.conditions.enable_channeling = True
        self.conditions.enable_sputtering = True
        self.conditions.enable_damage_modeling = False

    @property
    def species(self):
        return self.conditions.species

    @species.setter
    def species(self, value):
        self.conditions.species = value

    @property
    def energy(self):
        return self.conditions.energy

    @energy.setter
    def energy(self, value):
        self.conditions.energy = value

    @property
    def dose(self):
        return self.conditions.dose

    @dose.setter
    def dose(self, value):
        self.conditions.dose = value

    @property
    def tilt_angle(self):
        return self.conditions.tilt_angle

    @tilt_angle.setter
    def tilt_angle(self, value):
        self.conditions.tilt_angle = value

cdef class PyDiffusionConditions:
    cdef DiffusionConditions conditions

    def __init__(self):
        self.conditions.temperature = 1000.0  # °C
        self.conditions.time = 3600.0         # seconds
        self.conditions.ambient_pressure = 1.0  # atm
        self.conditions.ambient_gas = b"N2"
        self.conditions.enable_segregation = True
        self.conditions.enable_clustering = False
        self.conditions.initial_concentration = 1e18  # cm^-3

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

cdef class PyDopingResults:
    cdef DopingResults results

    @property
    def concentration_profile(self):
        return np.array(self.results.concentration_profile)

    @property
    def peak_concentration(self):
        return self.results.peak_concentration

    @property
    def junction_depth(self):
        return self.results.junction_depth

    @property
    def sheet_resistance(self):
        return self.results.sheet_resistance

    @property
    def projected_range(self):
        return self.results.projected_range

    @property
    def range_straggling(self):
        return self.results.range_straggling

    @property
    def electrical_properties(self):
        return dict(self.results.electrical_properties)

cdef class PyDopingManager:
    cdef DopingManager* thisptr

    def __cinit__(self):
        self.thisptr = new DopingManager()

    def __dealloc__(self):
        del self.thisptr

    def simulate_ion_implantation(self, wafer: PyWafer, energy: float, dose: float):
        """Basic ion implantation simulation"""
        self.thisptr.simulateIonImplantation(wafer.thisptr, energy, dose)

    def simulate_diffusion(self, wafer: PyWafer, temperature: float, time: float):
        """Basic diffusion simulation"""
        self.thisptr.simulateDiffusion(wafer.thisptr, temperature, time)

    def simulate_enhanced_implantation(self, wafer: PyWafer, conditions: PyImplantationConditions):
        """Enhanced ion implantation with detailed parameters"""
        cdef DopingResults cpp_results = self.thisptr.simulateEnhancedImplantation(wafer.thisptr, conditions.conditions)

        py_results = PyDopingResults()
        py_results.results = cpp_results
        return py_results

    def simulate_enhanced_diffusion(self, wafer: PyWafer, conditions: PyDiffusionConditions):
        """Enhanced diffusion simulation with detailed parameters"""
        cdef DopingResults cpp_results = self.thisptr.simulateEnhancedDiffusion(wafer.thisptr, conditions.conditions)

        py_results = PyDopingResults()
        py_results.results = cpp_results
        return py_results

# Constants for Python use
ION_BORON = ION_BORON_11
ION_PHOSPHORUS = ION_PHOSPHORUS_31
ION_ARSENIC = ION_ARSENIC_75
ION_ANTIMONY = ION_ANTIMONY_121
ION_INDIUM = ION_INDIUM_115
ION_GALLIUM = ION_GALLIUM_69

STANDARD_IMPLANTATION = STANDARD_IMPLANT
HIGH_CURRENT_IMPLANTATION = HIGH_CURRENT_IMPLANT
MEDIUM_CURRENT_IMPLANTATION = MEDIUM_CURRENT_IMPLANT
HIGH_ENERGY_IMPLANTATION = HIGH_ENERGY_IMPLANT
PLASMA_IMMERSION_IMPLANTATION = PLASMA_IMMERSION
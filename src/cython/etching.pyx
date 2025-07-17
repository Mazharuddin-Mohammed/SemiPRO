# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/etching/etching_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer
import numpy as np
cimport numpy as cnp

# Enhanced etching structures
cdef extern from "../cpp/modules/etching/etching_model.hpp":
    cdef enum EtchingTechnique:
        WET_ETCHING
        DRY_ETCHING_RIE
        DRY_ETCHING_ICP
        DRY_ETCHING_CCP
        DEEP_RIE
        ATOMIC_LAYER_ETCHING
        PLASMA_ETCHING
        ION_BEAM_ETCHING

    cdef enum EtchingChemistry:
        CHEMISTRY_SF6_O2
        CHEMISTRY_CF4_O2
        CHEMISTRY_CHF3_O2
        CHEMISTRY_C4F8_SF6
        CHEMISTRY_HBR_CL2
        CHEMISTRY_BCL3_CL2
        CHEMISTRY_KOH
        CHEMISTRY_TMAH
        CHEMISTRY_HF
        CHEMISTRY_BUFFERED_HF

    cdef struct EtchingConditions:
        EtchingTechnique technique
        EtchingChemistry chemistry
        double target_depth
        double pressure
        double temperature
        double rf_power
        double bias_power
        double gas_flow_rate
        double selectivity_ratio
        double aspect_ratio_limit
        bint enable_loading_effects
        bint enable_microloading
        bint enable_aspect_ratio_effects
        bint enable_sidewall_passivation

    cdef struct EtchingResults:
        double final_depth
        double etch_rate
        double selectivity
        double anisotropy
        double sidewall_angle
        double surface_roughness
        double critical_dimension_bias
        vector[double] depth_profile
        unordered_map[string, double] uniformity_metrics
        unordered_map[string, double] process_metrics

    cppclass EtchingModel:
        EtchingModel() except +
        void simulateEtching(shared_ptr[Wafer], double, string) except +
        EtchingResults simulateEnhancedEtching(shared_ptr[Wafer], const EtchingConditions&) except +

# Python wrapper classes
cdef class PyEtchingConditions:
    cdef EtchingConditions conditions

    def __init__(self):
        self.conditions.technique = DRY_ETCHING_RIE
        self.conditions.chemistry = CHEMISTRY_SF6_O2
        self.conditions.target_depth = 1000.0    # nm
        self.conditions.pressure = 10.0          # mTorr
        self.conditions.temperature = 20.0       # °C
        self.conditions.rf_power = 100.0         # W
        self.conditions.bias_power = 50.0        # W
        self.conditions.gas_flow_rate = 50.0     # sccm
        self.conditions.selectivity_ratio = 10.0
        self.conditions.aspect_ratio_limit = 20.0
        self.conditions.enable_loading_effects = True
        self.conditions.enable_microloading = True
        self.conditions.enable_aspect_ratio_effects = True
        self.conditions.enable_sidewall_passivation = True

    @property
    def technique(self):
        return self.conditions.technique

    @technique.setter
    def technique(self, value):
        self.conditions.technique = value

    @property
    def chemistry(self):
        return self.conditions.chemistry

    @chemistry.setter
    def chemistry(self, value):
        self.conditions.chemistry = value

    @property
    def target_depth(self):
        return self.conditions.target_depth

    @target_depth.setter
    def target_depth(self, value):
        self.conditions.target_depth = value

    @property
    def pressure(self):
        return self.conditions.pressure

    @pressure.setter
    def pressure(self, value):
        self.conditions.pressure = value

    @property
    def temperature(self):
        return self.conditions.temperature

    @temperature.setter
    def temperature(self, value):
        self.conditions.temperature = value

cdef class PyEtchingResults:
    cdef EtchingResults results

    @property
    def final_depth(self):
        return self.results.final_depth

    @property
    def etch_rate(self):
        return self.results.etch_rate

    @property
    def selectivity(self):
        return self.results.selectivity

    @property
    def anisotropy(self):
        return self.results.anisotropy

    @property
    def sidewall_angle(self):
        return self.results.sidewall_angle

    @property
    def surface_roughness(self):
        return self.results.surface_roughness

    @property
    def critical_dimension_bias(self):
        return self.results.critical_dimension_bias

    @property
    def depth_profile(self):
        return np.array(self.results.depth_profile)

    @property
    def uniformity_metrics(self):
        return dict(self.results.uniformity_metrics)

    @property
    def process_metrics(self):
        return dict(self.results.process_metrics)

cdef class PyEtchingModel:
    cdef EtchingModel* thisptr

    def __cinit__(self):
        self.thisptr = new EtchingModel()

    def __dealloc__(self):
        del self.thisptr

    def simulate_etching(self, wafer: PyWafer, depth: float, type: str):
        """Basic etching simulation"""
        self.thisptr.simulateEtching(wafer.thisptr, depth, type.encode('utf-8'))

    def simulate_enhanced_etching(self, wafer: PyWafer, conditions: PyEtchingConditions):
        """Enhanced etching simulation with detailed parameters"""
        cdef EtchingResults cpp_results = self.thisptr.simulateEnhancedEtching(wafer.thisptr, conditions.conditions)

        py_results = PyEtchingResults()
        py_results.results = cpp_results
        return py_results

# Constants for Python use
WET_ETCHING_TECHNIQUE = WET_ETCHING
DRY_ETCHING_RIE_TECHNIQUE = DRY_ETCHING_RIE
DRY_ETCHING_ICP_TECHNIQUE = DRY_ETCHING_ICP
DRY_ETCHING_CCP_TECHNIQUE = DRY_ETCHING_CCP
DEEP_RIE_TECHNIQUE = DEEP_RIE
ATOMIC_LAYER_ETCHING_TECHNIQUE = ATOMIC_LAYER_ETCHING
PLASMA_ETCHING_TECHNIQUE = PLASMA_ETCHING
ION_BEAM_ETCHING_TECHNIQUE = ION_BEAM_ETCHING

SF6_O2_CHEMISTRY = CHEMISTRY_SF6_O2
CF4_O2_CHEMISTRY = CHEMISTRY_CF4_O2
CHF3_O2_CHEMISTRY = CHEMISTRY_CHF3_O2
C4F8_SF6_CHEMISTRY = CHEMISTRY_C4F8_SF6
HBR_CL2_CHEMISTRY = CHEMISTRY_HBR_CL2
BCL3_CL2_CHEMISTRY = CHEMISTRY_BCL3_CL2
KOH_CHEMISTRY = CHEMISTRY_KOH
TMAH_CHEMISTRY = CHEMISTRY_TMAH
HF_CHEMISTRY = CHEMISTRY_HF
BUFFERED_HF_CHEMISTRY = CHEMISTRY_BUFFERED_HF
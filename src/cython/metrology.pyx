# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/metrology/metrology_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/metrology/metrology_model.hpp":
    cdef enum MetrologyTechnique:
        OPTICAL_MICROSCOPY
        PROFILOMETRY
        ELLIPSOMETRY
        XRD
        XRF
        SEM
        TEM
        AFM
        SIMS
        FTIR
    
    cdef enum MeasurementType:
        THICKNESS
        COMPOSITION
        ROUGHNESS
        STRESS
        RESISTIVITY
        DOPANT_PROFILE
        CRYSTAL_STRUCTURE
        OPTICAL_PROPERTIES
    
    cdef struct MetrologyParameters:
        MetrologyTechnique technique
        MeasurementType measurement_type
        double resolution
        double accuracy
        double precision
        double scan_area
        double scan_time
        bint destructive
    
    cdef struct MetrologyResult:
        double measured_value
        double uncertainty
        double min_value
        double max_value
        double signal_to_noise
        vector[pair[double, double]] profile_data
        string units
        string summary
    
    cdef cppclass MetrologyModel:
        MetrologyModel() except +
        
        # Main measurement methods
        MetrologyResult measureThickness(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        MetrologyResult measureComposition(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        MetrologyResult measureRoughness(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        MetrologyResult measureStress(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        MetrologyResult measureResistivity(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        MetrologyResult measureDopantProfile(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        MetrologyResult measureCrystalStructure(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        MetrologyResult measureOpticalProperties(shared_ptr[Wafer] wafer, MetrologyTechnique technique) except +
        
        # Generic measurement method
        MetrologyResult performMeasurement(shared_ptr[Wafer] wafer, const MetrologyParameters& params) except +
        
        # Configuration methods
        void setMeasurementParameters(MetrologyTechnique technique, double resolution, double accuracy) except +
        void enableNoiseSimulation(bint enable) except +
        void setCalibrationFactor(double factor) except +
        
        # Analysis methods
        void generateMetrologyReport(const string& filename, const vector[MetrologyResult]& results) except +
        double calculateMeasurementUncertainty(MetrologyTechnique technique, MeasurementType type) except +
        
        # Utility methods
        string getTechniqueName(MetrologyTechnique technique) except +
        string getMeasurementTypeName(MeasurementType type) except +
        string getUnits(MeasurementType type) except +

cdef class PyMetrologyParameters:
    cdef MetrologyParameters params
    
    def __cinit__(self, int technique, int measurement_type, double resolution=0.001, 
                 double accuracy=0.99, double precision=0.001, double scan_area=1.0,
                 double scan_time=60.0, bint destructive=False):
        self.params.technique = <MetrologyTechnique>technique
        self.params.measurement_type = <MeasurementType>measurement_type
        self.params.resolution = resolution
        self.params.accuracy = accuracy
        self.params.precision = precision
        self.params.scan_area = scan_area
        self.params.scan_time = scan_time
        self.params.destructive = destructive
    
    @property
    def technique(self):
        return <int>self.params.technique
    
    @technique.setter
    def technique(self, int value):
        self.params.technique = <MetrologyTechnique>value
    
    @property
    def measurement_type(self):
        return <int>self.params.measurement_type
    
    @measurement_type.setter
    def measurement_type(self, int value):
        self.params.measurement_type = <MeasurementType>value
    
    @property
    def resolution(self):
        return self.params.resolution
    
    @resolution.setter
    def resolution(self, double value):
        self.params.resolution = value
    
    @property
    def accuracy(self):
        return self.params.accuracy
    
    @accuracy.setter
    def accuracy(self, double value):
        self.params.accuracy = value
    
    @property
    def precision(self):
        return self.params.precision
    
    @precision.setter
    def precision(self, double value):
        self.params.precision = value
    
    @property
    def scan_area(self):
        return self.params.scan_area
    
    @scan_area.setter
    def scan_area(self, double value):
        self.params.scan_area = value
    
    @property
    def scan_time(self):
        return self.params.scan_time
    
    @scan_time.setter
    def scan_time(self, double value):
        self.params.scan_time = value
    
    @property
    def destructive(self):
        return self.params.destructive
    
    @destructive.setter
    def destructive(self, bint value):
        self.params.destructive = value

cdef class PyMetrologyResult:
    cdef MetrologyResult result

    def __cinit__(self):
        pass
    
    @property
    def measured_value(self):
        return self.result.measured_value

    @property
    def uncertainty(self):
        return self.result.uncertainty

    @property
    def min_value(self):
        return self.result.min_value

    @property
    def max_value(self):
        return self.result.max_value

    @property
    def signal_to_noise(self):
        return self.result.signal_to_noise

    @property
    def units(self):
        return self.result.units.decode('utf-8')

    @property
    def summary(self):
        return self.result.summary.decode('utf-8')

    def get_profile_data(self):
        profile = []
        for i in range(self.result.profile_data.size()):
            profile.append((self.result.profile_data[i].first, self.result.profile_data[i].second))
        return profile

cdef class PyMetrologyModel:
    cdef MetrologyModel* thisptr
    
    def __cinit__(self):
        self.thisptr = new MetrologyModel()
    
    def __dealloc__(self):
        del self.thisptr
    
    def measure_thickness(self, PyWafer wafer, int technique):
        cdef MetrologyResult result = self.thisptr.measureThickness(wafer.thisptr, <MetrologyTechnique>technique)
        
        py_result = PyMetrologyResult()
        py_result.result = result
        return py_result
    
    def measure_composition(self, PyWafer wafer, int technique):
        cdef MetrologyResult result = self.thisptr.measureComposition(wafer.thisptr, <MetrologyTechnique>technique)
        
        py_result = PyMetrologyResult()
        py_result.result = result
        return py_result
    
    def measure_roughness(self, PyWafer wafer, int technique):
        cdef MetrologyResult result = self.thisptr.measureRoughness(wafer.thisptr, <MetrologyTechnique>technique)
        
        py_result = PyMetrologyResult()
        py_result.result = result
        return py_result
    
    def measure_stress(self, PyWafer wafer, int technique):
        cdef MetrologyResult result = self.thisptr.measureStress(wafer.thisptr, <MetrologyTechnique>technique)
        
        py_result = PyMetrologyResult()
        py_result.result = result
        return py_result
    
    def measure_resistivity(self, PyWafer wafer, int technique):
        cdef MetrologyResult result = self.thisptr.measureResistivity(wafer.thisptr, <MetrologyTechnique>technique)
        
        py_result = PyMetrologyResult()
        py_result.result = result
        return py_result
    
    def measure_dopant_profile(self, PyWafer wafer, int technique):
        cdef MetrologyResult result = self.thisptr.measureDopantProfile(wafer.thisptr, <MetrologyTechnique>technique)
        
        py_result = PyMetrologyResult()
        py_result.result = result
        return py_result
    
    def perform_measurement(self, PyWafer wafer, PyMetrologyParameters params):
        cdef MetrologyResult result = self.thisptr.performMeasurement(wafer.thisptr, params.params)
        
        py_result = PyMetrologyResult()
        py_result.result = result
        return py_result
    
    def set_measurement_parameters(self, int technique, double resolution, double accuracy):
        self.thisptr.setMeasurementParameters(<MetrologyTechnique>technique, resolution, accuracy)
    
    def enable_noise_simulation(self, bint enable):
        self.thisptr.enableNoiseSimulation(enable)
    
    def set_calibration_factor(self, double factor):
        self.thisptr.setCalibrationFactor(factor)
    
    def generate_metrology_report(self, str filename, list results):
        cdef vector[MetrologyResult] cpp_results
        for result in results:
            if isinstance(result, PyMetrologyResult):
                cpp_results.push_back((<PyMetrologyResult>result).result)
        
        self.thisptr.generateMetrologyReport(filename.encode('utf-8'), cpp_results)
    
    def calculate_measurement_uncertainty(self, int technique, int measurement_type):
        return self.thisptr.calculateMeasurementUncertainty(
            <MetrologyTechnique>technique, <MeasurementType>measurement_type)
    
    def get_technique_name(self, int technique):
        return self.thisptr.getTechniqueName(<MetrologyTechnique>technique).decode('utf-8')
    
    def get_measurement_type_name(self, int measurement_type):
        return self.thisptr.getMeasurementTypeName(<MeasurementType>measurement_type).decode('utf-8')
    
    def get_units(self, int measurement_type):
        return self.thisptr.getUnits(<MeasurementType>measurement_type).decode('utf-8')

# Constants for Python interface
TECHNIQUE_OPTICAL_MICROSCOPY = <int>OPTICAL_MICROSCOPY
TECHNIQUE_PROFILOMETRY = <int>PROFILOMETRY
TECHNIQUE_ELLIPSOMETRY = <int>ELLIPSOMETRY
TECHNIQUE_XRD = <int>XRD
TECHNIQUE_XRF = <int>XRF
TECHNIQUE_SEM = <int>SEM
TECHNIQUE_TEM = <int>TEM
TECHNIQUE_AFM = <int>AFM
TECHNIQUE_SIMS = <int>SIMS
TECHNIQUE_FTIR = <int>FTIR

MEASUREMENT_THICKNESS = <int>THICKNESS
MEASUREMENT_COMPOSITION = <int>COMPOSITION
MEASUREMENT_ROUGHNESS = <int>ROUGHNESS
MEASUREMENT_STRESS = <int>STRESS
MEASUREMENT_RESISTIVITY = <int>RESISTIVITY
MEASUREMENT_DOPANT_PROFILE = <int>DOPANT_PROFILE
MEASUREMENT_CRYSTAL_STRUCTURE = <int>CRYSTAL_STRUCTURE
MEASUREMENT_OPTICAL_PROPERTIES = <int>OPTICAL_PROPERTIES

# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/defect_inspection/defect_inspection_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/defect_inspection/defect_inspection_model.hpp":
    cdef enum InspectionMethod:
        OPTICAL_BRIGHTFIELD
        OPTICAL_DARKFIELD
        SEM_INSPECTION
        AFM_INSPECTION
        X_RAY_TOPOGRAPHY
    
    cdef enum DefectType:
        PARTICLE
        SCRATCH
        VOID
        CONTAMINATION
        PATTERN_DEFECT
        DIMENSIONAL_DEFECT
        ELECTRICAL_DEFECT
    
    cdef enum DefectSeverity:
        MINOR
        MAJOR
        CRITICAL
    
    cdef cppclass Defect:
        DefectType type
        DefectSeverity severity
        double x
        double y
        double z
        double size
        double confidence
        string description
        
        Defect(DefectType t, DefectSeverity s, double x_pos, double y_pos, double z_pos, double defect_size)
    
    cdef cppclass InspectionResult:
        InspectionMethod method
        vector[Defect] defects
        double coverage_area
        double inspection_time
        double false_positive_rate
        double false_negative_rate
        string summary
    
    cdef cppclass DefectInspectionModel:
        DefectInspectionModel() except +
        
        # Main inspection methods
        InspectionResult performInspection(shared_ptr[Wafer] wafer, InspectionMethod method) except +
        vector[Defect] performDimensionalInspection(shared_ptr[Wafer] wafer, double tolerance) except +
        vector[Defect] performElectricalInspection(shared_ptr[Wafer] wafer, double voltage_threshold) except +
        
        # Configuration methods
        void setInspectionParameters(InspectionMethod method, double pixel_size, 
                                   double scan_speed, double sensitivity) except +
        void enableDefectClassification(bint enable) except +
        void setDefectSizeThreshold(double min_size) except +
        
        # Analysis methods
        void generateInspectionReport(const string& filename, const InspectionResult& result) except +
        double calculateDefectDensity(const vector[Defect]& defects, double area) except +
        vector[Defect] filterDefectsBySeverity(const vector[Defect]& defects, DefectSeverity min_severity) except +

cdef class PyDefect:
    cdef Defect* thisptr
    
    def __cinit__(self, int defect_type, int severity, double x, double y, double z, double size):
        self.thisptr = new Defect(<DefectType>defect_type, <DefectSeverity>severity, x, y, z, size)
    
    def __dealloc__(self):
        del self.thisptr
    
    @property
    def type(self):
        return <int>self.thisptr.type
    
    @property
    def severity(self):
        return <int>self.thisptr.severity
    
    @property
    def x(self):
        return self.thisptr.x
    
    @property
    def y(self):
        return self.thisptr.y
    
    @property
    def z(self):
        return self.thisptr.z
    
    @property
    def size(self):
        return self.thisptr.size
    
    @property
    def confidence(self):
        return self.thisptr.confidence
    
    @property
    def description(self):
        return self.thisptr.description.decode('utf-8')

cdef class PyInspectionResult:
    cdef InspectionResult* thisptr
    cdef bint owner
    
    def __cinit__(self):
        self.thisptr = new InspectionResult()
        self.owner = True
    
    def __dealloc__(self):
        if self.owner:
            del self.thisptr
    
    @property
    def method(self):
        return <int>self.thisptr.method
    
    @property
    def defect_count(self):
        return self.thisptr.defects.size()
    
    @property
    def coverage_area(self):
        return self.thisptr.coverage_area
    
    @property
    def inspection_time(self):
        return self.thisptr.inspection_time
    
    @property
    def false_positive_rate(self):
        return self.thisptr.false_positive_rate
    
    @property
    def false_negative_rate(self):
        return self.thisptr.false_negative_rate
    
    @property
    def summary(self):
        return self.thisptr.summary.decode('utf-8')
    
    def get_defects(self):
        defects = []
        for i in range(self.thisptr.defects.size()):
            defect = PyDefect(
                <int>self.thisptr.defects[i].type,
                <int>self.thisptr.defects[i].severity,
                self.thisptr.defects[i].x,
                self.thisptr.defects[i].y,
                self.thisptr.defects[i].z,
                self.thisptr.defects[i].size
            )
            defects.append(defect)
        return defects

cdef class PyDefectInspectionModel:
    cdef DefectInspectionModel* thisptr
    
    def __cinit__(self):
        self.thisptr = new DefectInspectionModel()
    
    def __dealloc__(self):
        del self.thisptr
    
    def perform_inspection(self, PyWafer wafer, int method):
        cdef InspectionResult result = self.thisptr.performInspection(wafer.thisptr, <InspectionMethod>method)
        
        py_result = PyInspectionResult()
        py_result.thisptr[0] = result
        return py_result
    
    def perform_dimensional_inspection(self, PyWafer wafer, double tolerance):
        cdef vector[Defect] defects = self.thisptr.performDimensionalInspection(wafer.thisptr, tolerance)
        
        py_defects = []
        for i in range(defects.size()):
            defect = PyDefect(
                <int>defects[i].type,
                <int>defects[i].severity,
                defects[i].x,
                defects[i].y,
                defects[i].z,
                defects[i].size
            )
            py_defects.append(defect)
        return py_defects
    
    def perform_electrical_inspection(self, PyWafer wafer, double voltage_threshold):
        cdef vector[Defect] defects = self.thisptr.performElectricalInspection(wafer.thisptr, voltage_threshold)
        
        py_defects = []
        for i in range(defects.size()):
            defect = PyDefect(
                <int>defects[i].type,
                <int>defects[i].severity,
                defects[i].x,
                defects[i].y,
                defects[i].z,
                defects[i].size
            )
            py_defects.append(defect)
        return py_defects
    
    def set_inspection_parameters(self, int method, double pixel_size, double scan_speed, double sensitivity):
        self.thisptr.setInspectionParameters(<InspectionMethod>method, pixel_size, scan_speed, sensitivity)
    
    def enable_defect_classification(self, bint enable):
        self.thisptr.enableDefectClassification(enable)
    
    def set_defect_size_threshold(self, double min_size):
        self.thisptr.setDefectSizeThreshold(min_size)
    
    def generate_inspection_report(self, str filename, PyInspectionResult result):
        self.thisptr.generateInspectionReport(filename.encode('utf-8'), result.thisptr[0])
    
    def calculate_defect_density(self, list defects, double area):
        cdef vector[Defect] cpp_defects
        for defect in defects:
            if isinstance(defect, PyDefect):
                cpp_defects.push_back((<PyDefect>defect).thisptr[0])
        return self.thisptr.calculateDefectDensity(cpp_defects, area)

# Constants for Python interface
INSPECTION_OPTICAL_BRIGHTFIELD = <int>OPTICAL_BRIGHTFIELD
INSPECTION_OPTICAL_DARKFIELD = <int>OPTICAL_DARKFIELD
INSPECTION_SEM = <int>SEM_INSPECTION
INSPECTION_AFM = <int>AFM_INSPECTION
INSPECTION_X_RAY = <int>X_RAY_TOPOGRAPHY

DEFECT_PARTICLE = <int>PARTICLE
DEFECT_SCRATCH = <int>SCRATCH
DEFECT_VOID = <int>VOID
DEFECT_CONTAMINATION = <int>CONTAMINATION
DEFECT_PATTERN = <int>PATTERN_DEFECT
DEFECT_DIMENSIONAL = <int>DIMENSIONAL_DEFECT
DEFECT_ELECTRICAL = <int>ELECTRICAL_DEFECT

SEVERITY_MINOR = <int>MINOR
SEVERITY_MAJOR = <int>MAJOR
SEVERITY_CRITICAL = <int>CRITICAL

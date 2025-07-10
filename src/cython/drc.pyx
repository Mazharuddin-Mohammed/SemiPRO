# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/design_rule_check/drc_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/design_rule_check/drc_model.hpp":
    cdef enum ViolationType:
        WIDTH
        SPACING
        AREA
        ENCLOSURE
        DENSITY
        ANTENNA_RATIO
        ASPECT_RATIO
        CORNER_ROUNDING
        OVERLAP
        EXTENSION
    
    cdef enum ViolationSeverity:
        CRITICAL
        ERROR
        WARNING
        INFO
    
    cdef cppclass DRCRule:
        string name
        ViolationType type
        string layer
        double min_value
        double max_value
        ViolationSeverity severity
        bint enabled
        string description
        
        DRCRule(const string& rule_name, ViolationType rule_type, 
                const string& target_layer, double min_val, double max_val)
    
    cdef cppclass DRCViolation:
        string rule_name
        ViolationType type
        ViolationSeverity severity
        pair[double, double] location
        double measured_value
        double required_value
        string description
        bint waived
        
        DRCViolation(const string& rule, ViolationType vtype, 
                     const pair[double, double]& loc, double measured, double required)
    
    cdef cppclass DRCModel:
        DRCModel() except +
        
        # Rule management
        void addRule(const DRCRule& rule) except +
        void removeRule(const string& rule_name) except +
        void enableRule(const string& rule_name, bint enabled) except +
        void setRuleSeverity(const string& rule_name, ViolationSeverity severity) except +
        
        # Technology node configuration
        void loadTechnologyRules(const string& technology_node) except +
        void configureTechnology(double feature_size, double metal_pitch, double via_size) except +
        
        # DRC checking
        void runFullDRC(shared_ptr[Wafer] wafer) except +
        void runIncrementalDRC(shared_ptr[Wafer] wafer, 
                              const pair[double, double]& region_start,
                              const pair[double, double]& region_end) except +
        void runLayerDRC(shared_ptr[Wafer] wafer, const string& layer) except +
        
        # Specific rule checks
        void checkWidthRules(shared_ptr[Wafer] wafer) except +
        void checkSpacingRules(shared_ptr[Wafer] wafer) except +
        void checkAreaRules(shared_ptr[Wafer] wafer) except +
        void checkDensityRules(shared_ptr[Wafer] wafer) except +
        void checkAntennaRules(shared_ptr[Wafer] wafer) except +
        
        # Violation management
        void waiveViolation(size_t violation_index, const string& reason) except +
        void unwaiveViolation(size_t violation_index) except +
        void clearViolations() except +
        void exportViolations(const string& filename) except +
        
        # Analysis and reporting
        void generateDRCReport(const string& filename) except +
        void generateViolationSummary() except +
        vector[DRCViolation] getViolationsByType(ViolationType type) except +
        vector[DRCViolation] getViolationsBySeverity(ViolationSeverity severity) except +
        
        # Getters
        const vector[DRCRule]& getRules() const
        const vector[DRCViolation]& getViolations() const
        size_t getViolationCount() const
        size_t getCriticalViolationCount() const
        size_t getErrorViolationCount() const
        size_t getWarningViolationCount() const
        
        # Statistics
        double getDRCCoverage() const
        double getViolationDensity() const
        unordered_map[string, int] getViolationsByLayer() const

cdef class PyDRCRule:
    cdef DRCRule* thisptr
    
    def __cinit__(self, str rule_name, int rule_type, str target_layer, 
                  double min_val, double max_val = -1.0):
        self.thisptr = new DRCRule(rule_name.encode('utf-8'), 
                                  <ViolationType>rule_type,
                                  target_layer.encode('utf-8'), 
                                  min_val, max_val)
    
    def __dealloc__(self):
        del self.thisptr
    
    @property
    def name(self):
        return self.thisptr.name.decode('utf-8')
    
    @property
    def type(self):
        return <int>self.thisptr.type
    
    @property
    def layer(self):
        return self.thisptr.layer.decode('utf-8')
    
    @property
    def min_value(self):
        return self.thisptr.min_value
    
    @property
    def max_value(self):
        return self.thisptr.max_value
    
    @property
    def enabled(self):
        return self.thisptr.enabled
    
    @enabled.setter
    def enabled(self, bint value):
        self.thisptr.enabled = value
    
    @property
    def severity(self):
        return <int>self.thisptr.severity
    
    @severity.setter
    def severity(self, int value):
        self.thisptr.severity = <ViolationSeverity>value

cdef class PyDRCViolation:
    cdef DRCViolation* thisptr
    cdef bint owner
    
    def __cinit__(self):
        self.thisptr = NULL
        self.owner = False
    
    def __dealloc__(self):
        if self.owner and self.thisptr != NULL:
            del self.thisptr
    
    @staticmethod
    cdef PyDRCViolation from_ptr(DRCViolation* ptr, bint owner=False):
        cdef PyDRCViolation wrapper = PyDRCViolation()
        wrapper.thisptr = ptr
        wrapper.owner = owner
        return wrapper
    
    @property
    def rule_name(self):
        return self.thisptr.rule_name.decode('utf-8')
    
    @property
    def type(self):
        return <int>self.thisptr.type
    
    @property
    def severity(self):
        return <int>self.thisptr.severity
    
    @property
    def location(self):
        return (self.thisptr.location.first, self.thisptr.location.second)
    
    @property
    def measured_value(self):
        return self.thisptr.measured_value
    
    @property
    def required_value(self):
        return self.thisptr.required_value
    
    @property
    def description(self):
        return self.thisptr.description.decode('utf-8')
    
    @property
    def waived(self):
        return self.thisptr.waived

cdef class PyDRCModel:
    cdef DRCModel* thisptr
    
    def __cinit__(self):
        self.thisptr = new DRCModel()
    
    def __dealloc__(self):
        del self.thisptr
    
    def add_rule(self, PyDRCRule rule):
        self.thisptr.addRule(rule.thisptr[0])
    
    def remove_rule(self, str rule_name):
        self.thisptr.removeRule(rule_name.encode('utf-8'))
    
    def enable_rule(self, str rule_name, bint enabled):
        self.thisptr.enableRule(rule_name.encode('utf-8'), enabled)
    
    def set_rule_severity(self, str rule_name, int severity):
        self.thisptr.setRuleSeverity(rule_name.encode('utf-8'), <ViolationSeverity>severity)
    
    def load_technology_rules(self, str technology_node):
        self.thisptr.loadTechnologyRules(technology_node.encode('utf-8'))
    
    def configure_technology(self, double feature_size, double metal_pitch, double via_size):
        self.thisptr.configureTechnology(feature_size, metal_pitch, via_size)
    
    def run_full_drc(self, PyWafer wafer):
        self.thisptr.runFullDRC(wafer.thisptr)
    
    def run_incremental_drc(self, PyWafer wafer, tuple region_start, tuple region_end):
        cdef pair[double, double] start = pair[double, double](region_start[0], region_start[1])
        cdef pair[double, double] end = pair[double, double](region_end[0], region_end[1])
        self.thisptr.runIncrementalDRC(wafer.thisptr, start, end)
    
    def run_layer_drc(self, PyWafer wafer, str layer):
        self.thisptr.runLayerDRC(wafer.thisptr, layer.encode('utf-8'))
    
    def check_width_rules(self, PyWafer wafer):
        self.thisptr.checkWidthRules(wafer.thisptr)
    
    def check_spacing_rules(self, PyWafer wafer):
        self.thisptr.checkSpacingRules(wafer.thisptr)
    
    def check_area_rules(self, PyWafer wafer):
        self.thisptr.checkAreaRules(wafer.thisptr)
    
    def check_density_rules(self, PyWafer wafer):
        self.thisptr.checkDensityRules(wafer.thisptr)
    
    def check_antenna_rules(self, PyWafer wafer):
        self.thisptr.checkAntennaRules(wafer.thisptr)
    
    def waive_violation(self, size_t violation_index, str reason):
        self.thisptr.waiveViolation(violation_index, reason.encode('utf-8'))
    
    def unwaive_violation(self, size_t violation_index):
        self.thisptr.unwaiveViolation(violation_index)
    
    def clear_violations(self):
        self.thisptr.clearViolations()
    
    def export_violations(self, str filename):
        self.thisptr.exportViolations(filename.encode('utf-8'))
    
    def generate_drc_report(self, str filename):
        self.thisptr.generateDRCReport(filename.encode('utf-8'))
    
    def generate_violation_summary(self):
        self.thisptr.generateViolationSummary()
    
    def get_violations_by_type(self, int violation_type):
        cdef vector[DRCViolation] violations = self.thisptr.getViolationsByType(<ViolationType>violation_type)
        result = []
        for i in range(violations.size()):
            result.append(PyDRCViolation.from_ptr(&violations[i]))
        return result
    
    def get_violations_by_severity(self, int severity):
        cdef vector[DRCViolation] violations = self.thisptr.getViolationsBySeverity(<ViolationSeverity>severity)
        result = []
        for i in range(violations.size()):
            result.append(PyDRCViolation.from_ptr(&violations[i]))
        return result
    
    def get_violation_count(self):
        return self.thisptr.getViolationCount()
    
    def get_critical_violation_count(self):
        return self.thisptr.getCriticalViolationCount()
    
    def get_error_violation_count(self):
        return self.thisptr.getErrorViolationCount()
    
    def get_warning_violation_count(self):
        return self.thisptr.getWarningViolationCount()
    
    def get_drc_coverage(self):
        return self.thisptr.getDRCCoverage()
    
    def get_violation_density(self):
        return self.thisptr.getViolationDensity()
    
    def get_violations_by_layer(self):
        cdef unordered_map[string, int] violations = self.thisptr.getViolationsByLayer()
        result = {}
        for item in violations:
            result[item.first.decode('utf-8')] = item.second
        return result

# Constants for Python interface
VIOLATION_TYPE_WIDTH = <int>WIDTH
VIOLATION_TYPE_SPACING = <int>SPACING
VIOLATION_TYPE_AREA = <int>AREA
VIOLATION_TYPE_ENCLOSURE = <int>ENCLOSURE
VIOLATION_TYPE_DENSITY = <int>DENSITY
VIOLATION_TYPE_ANTENNA_RATIO = <int>ANTENNA_RATIO
VIOLATION_TYPE_ASPECT_RATIO = <int>ASPECT_RATIO
VIOLATION_TYPE_CORNER_ROUNDING = <int>CORNER_ROUNDING
VIOLATION_TYPE_OVERLAP = <int>OVERLAP
VIOLATION_TYPE_EXTENSION = <int>EXTENSION

VIOLATION_SEVERITY_CRITICAL = <int>CRITICAL
VIOLATION_SEVERITY_ERROR = <int>ERROR
VIOLATION_SEVERITY_WARNING = <int>WARNING
VIOLATION_SEVERITY_INFO = <int>INFO

# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/interconnect/damascene_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/interconnect/damascene_model.hpp":
    cdef enum DamasceneType:
        SINGLE_DAMASCENE
        DUAL_DAMASCENE
        TRIPLE_DAMASCENE
    
    cdef enum BarrierMaterial:
        TANTALUM
        TITANIUM
        TITANIUM_NITRIDE
        TANTALUM_NITRIDE
        RUTHENIUM
        COBALT
    
    cdef enum MetalMaterial:
        COPPER
        ALUMINUM
        TUNGSTEN
        COBALT_METAL
        RUTHENIUM_METAL
    
    cdef struct DamasceneParameters:
        DamasceneType type
        double line_width
        double line_spacing
        double metal_thickness
        double barrier_thickness
        double dielectric_thickness
        BarrierMaterial barrier_material
        MetalMaterial metal_material
        bint cmp_enabled
        double cmp_pressure
        double cmp_time
    
    cdef struct DamasceneResult:
        double final_thickness
        double resistance
        double capacitance
        double rc_delay
        double barrier_coverage
        double void_probability
        double electromigration_lifetime
        string quality_summary
    
    cdef cppclass DamasceneModel:
        DamasceneModel() except +
        
        # Main process methods
        DamasceneResult simulateDamascene(shared_ptr[Wafer] wafer, const DamasceneParameters& params) except +
        bint simulateBarrierDeposition(shared_ptr[Wafer] wafer, BarrierMaterial material, double thickness) except +
        bint simulateSeedDeposition(shared_ptr[Wafer] wafer, MetalMaterial material, double thickness) except +
        bint simulateElectroplating(shared_ptr[Wafer] wafer, MetalMaterial material, double thickness) except +
        bint simulateCMP(shared_ptr[Wafer] wafer, double pressure, double time) except +
        
        # Analysis methods
        double calculateLineResistance(double width, double thickness, MetalMaterial material) except +
        double calculateLineCapacitance(double width, double spacing, double thickness) except +
        double calculateRCDelay(double resistance, double capacitance) except +
        double calculateElectromigrationLifetime(double current_density, double temperature) except +
        
        # Configuration methods
        void setProcessTemperature(double temperature) except +
        void enableAdvancedModels(bint enable) except +
        void setBarrierConformality(double conformality) except +
        
        # Utility methods
        string getBarrierMaterialName(BarrierMaterial material) except +
        string getMetalMaterialName(MetalMaterial material) except +
        string getDamasceneTypeName(DamasceneType type) except +

cdef class PyDamasceneParameters:
    cdef DamasceneParameters params
    
    def __cinit__(self, int damascene_type, double line_width, double line_spacing, 
                 double metal_thickness, double barrier_thickness, double dielectric_thickness,
                 int barrier_material, int metal_material, bint cmp_enabled=True,
                 double cmp_pressure=5.0, double cmp_time=60.0):
        self.params.type = <DamasceneType>damascene_type
        self.params.line_width = line_width
        self.params.line_spacing = line_spacing
        self.params.metal_thickness = metal_thickness
        self.params.barrier_thickness = barrier_thickness
        self.params.dielectric_thickness = dielectric_thickness
        self.params.barrier_material = <BarrierMaterial>barrier_material
        self.params.metal_material = <MetalMaterial>metal_material
        self.params.cmp_enabled = cmp_enabled
        self.params.cmp_pressure = cmp_pressure
        self.params.cmp_time = cmp_time
    
    @property
    def damascene_type(self):
        return <int>self.params.type
    
    @damascene_type.setter
    def damascene_type(self, int value):
        self.params.type = <DamasceneType>value
    
    @property
    def line_width(self):
        return self.params.line_width
    
    @line_width.setter
    def line_width(self, double value):
        self.params.line_width = value
    
    @property
    def line_spacing(self):
        return self.params.line_spacing
    
    @line_spacing.setter
    def line_spacing(self, double value):
        self.params.line_spacing = value
    
    @property
    def metal_thickness(self):
        return self.params.metal_thickness
    
    @metal_thickness.setter
    def metal_thickness(self, double value):
        self.params.metal_thickness = value
    
    @property
    def barrier_thickness(self):
        return self.params.barrier_thickness
    
    @barrier_thickness.setter
    def barrier_thickness(self, double value):
        self.params.barrier_thickness = value
    
    @property
    def dielectric_thickness(self):
        return self.params.dielectric_thickness
    
    @dielectric_thickness.setter
    def dielectric_thickness(self, double value):
        self.params.dielectric_thickness = value
    
    @property
    def barrier_material(self):
        return <int>self.params.barrier_material
    
    @barrier_material.setter
    def barrier_material(self, int value):
        self.params.barrier_material = <BarrierMaterial>value
    
    @property
    def metal_material(self):
        return <int>self.params.metal_material
    
    @metal_material.setter
    def metal_material(self, int value):
        self.params.metal_material = <MetalMaterial>value
    
    @property
    def cmp_enabled(self):
        return self.params.cmp_enabled
    
    @cmp_enabled.setter
    def cmp_enabled(self, bint value):
        self.params.cmp_enabled = value
    
    @property
    def cmp_pressure(self):
        return self.params.cmp_pressure
    
    @cmp_pressure.setter
    def cmp_pressure(self, double value):
        self.params.cmp_pressure = value
    
    @property
    def cmp_time(self):
        return self.params.cmp_time
    
    @cmp_time.setter
    def cmp_time(self, double value):
        self.params.cmp_time = value

cdef class PyDamasceneResult:
    cdef DamasceneResult* thisptr
    cdef bint owner
    
    def __cinit__(self):
        self.thisptr = new DamasceneResult()
        self.owner = True
    
    def __dealloc__(self):
        if self.owner:
            del self.thisptr
    
    @property
    def final_thickness(self):
        return self.thisptr.final_thickness
    
    @property
    def resistance(self):
        return self.thisptr.resistance
    
    @property
    def capacitance(self):
        return self.thisptr.capacitance
    
    @property
    def rc_delay(self):
        return self.thisptr.rc_delay
    
    @property
    def barrier_coverage(self):
        return self.thisptr.barrier_coverage
    
    @property
    def void_probability(self):
        return self.thisptr.void_probability
    
    @property
    def electromigration_lifetime(self):
        return self.thisptr.electromigration_lifetime
    
    @property
    def quality_summary(self):
        return self.thisptr.quality_summary.decode('utf-8')

cdef class PyDamasceneModel:
    cdef DamasceneModel* thisptr
    
    def __cinit__(self):
        self.thisptr = new DamasceneModel()
    
    def __dealloc__(self):
        del self.thisptr
    
    def simulate_damascene(self, PyWafer wafer, PyDamasceneParameters params):
        cdef DamasceneResult result = self.thisptr.simulateDamascene(wafer.thisptr, params.params)
        
        py_result = PyDamasceneResult()
        py_result.thisptr[0] = result
        return py_result
    
    def simulate_barrier_deposition(self, PyWafer wafer, int material, double thickness):
        return self.thisptr.simulateBarrierDeposition(wafer.thisptr, <BarrierMaterial>material, thickness)
    
    def simulate_seed_deposition(self, PyWafer wafer, int material, double thickness):
        return self.thisptr.simulateSeedDeposition(wafer.thisptr, <MetalMaterial>material, thickness)
    
    def simulate_electroplating(self, PyWafer wafer, int material, double thickness):
        return self.thisptr.simulateElectroplating(wafer.thisptr, <MetalMaterial>material, thickness)
    
    def simulate_cmp(self, PyWafer wafer, double pressure, double time):
        return self.thisptr.simulateCMP(wafer.thisptr, pressure, time)
    
    def calculate_line_resistance(self, double width, double thickness, int material):
        return self.thisptr.calculateLineResistance(width, thickness, <MetalMaterial>material)
    
    def calculate_line_capacitance(self, double width, double spacing, double thickness):
        return self.thisptr.calculateLineCapacitance(width, spacing, thickness)
    
    def calculate_rc_delay(self, double resistance, double capacitance):
        return self.thisptr.calculateRCDelay(resistance, capacitance)
    
    def calculate_electromigration_lifetime(self, double current_density, double temperature):
        return self.thisptr.calculateElectromigrationLifetime(current_density, temperature)
    
    def set_process_temperature(self, double temperature):
        self.thisptr.setProcessTemperature(temperature)
    
    def enable_advanced_models(self, bint enable):
        self.thisptr.enableAdvancedModels(enable)
    
    def set_barrier_conformality(self, double conformality):
        self.thisptr.setBarrierConformality(conformality)
    
    def get_barrier_material_name(self, int material):
        return self.thisptr.getBarrierMaterialName(<BarrierMaterial>material).decode('utf-8')
    
    def get_metal_material_name(self, int material):
        return self.thisptr.getMetalMaterialName(<MetalMaterial>material).decode('utf-8')
    
    def get_damascene_type_name(self, int damascene_type):
        return self.thisptr.getDamasceneTypeName(<DamasceneType>damascene_type).decode('utf-8')

# Constants for Python interface
DAMASCENE_SINGLE = <int>SINGLE_DAMASCENE
DAMASCENE_DUAL = <int>DUAL_DAMASCENE
DAMASCENE_TRIPLE = <int>TRIPLE_DAMASCENE

BARRIER_TANTALUM = <int>TANTALUM
BARRIER_TITANIUM = <int>TITANIUM
BARRIER_TITANIUM_NITRIDE = <int>TITANIUM_NITRIDE
BARRIER_TANTALUM_NITRIDE = <int>TANTALUM_NITRIDE
BARRIER_RUTHENIUM = <int>RUTHENIUM
BARRIER_COBALT = <int>COBALT

METAL_COPPER = <int>COPPER
METAL_ALUMINUM = <int>ALUMINUM
METAL_TUNGSTEN = <int>TUNGSTEN
METAL_COBALT = <int>COBALT_METAL
METAL_RUTHENIUM = <int>RUTHENIUM_METAL

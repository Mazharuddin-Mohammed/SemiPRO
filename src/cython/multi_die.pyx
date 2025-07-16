# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/multi_die/multi_die_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.unordered_map cimport unordered_map
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/multi_die/multi_die_model.hpp":
    cdef enum DieType:
        LOGIC
        MEMORY
        ANALOG
        RF
        SENSOR
        POWER
    
    cdef enum IntegrationMethod:
        WIRE_BONDING
        FLIP_CHIP
        TSV
        WLP
        CHIPLET
    
    cdef cppclass Die:
        string id
        DieType type
        double width
        double height
        double thickness
        pair[double, double] position
        string material
        vector[pair[string, double]] electrical_properties
        double thermal_conductivity
        double power_consumption
        
        Die(const string& die_id, DieType die_type, double w, double h, double t)
    
    cdef cppclass Interconnect:
        string from_die
        string to_die
        string signal_type
        double resistance
        double capacitance
        double inductance
        double delay
        
        Interconnect(const string& from_node, const string& to_node, const string& type)
    
    cdef cppclass MultiDieModel:
        MultiDieModel() except +
        
        # Die management
        void addDie(const Die& die) except +
        void removeDie(const string& die_id) except +
        void positionDie(const string& die_id, double x, double y) except +
        
        # Integration methods
        void performWireBonding(shared_ptr[Wafer] wafer, 
                               const string& die1, const string& die2,
                               const vector[pair[pair[int, int], pair[int, int]]]& bonds) except +
        
        void performFlipChipBonding(shared_ptr[Wafer] wafer,
                                   const string& die1, const string& die2,
                                   double bump_pitch, double bump_diameter) except +
        
        void performTSVIntegration(shared_ptr[Wafer] wafer,
                                  const string& die_id,
                                  const vector[pair[double, double]]& tsv_positions,
                                  double tsv_diameter, double tsv_depth) except +
        
        # Analysis methods
        void analyzeElectricalPerformance(shared_ptr[Wafer] wafer) except +
        void analyzeThermalPerformance(shared_ptr[Wafer] wafer) except +
        
        # Interconnect modeling
        void addInterconnect(const Interconnect& interconnect) except +
        void calculateInterconnectParameters() except +
        
        # Getters
        const vector[Die]& getDies() const
        const vector[Interconnect]& getInterconnects() const
        const unordered_map[string, double]& getSystemMetrics() const

cdef class PyDie:
    cdef Die* thisptr
    
    def __cinit__(self, str die_id, int die_type, double width, double height, double thickness):
        self.thisptr = new Die(die_id.encode('utf-8'), <DieType>die_type, width, height, thickness)
    
    def __dealloc__(self):
        del self.thisptr
    
    @property
    def id(self):
        return self.thisptr.id.decode('utf-8')
    
    @property
    def type(self):
        return <int>self.thisptr.type
    
    @property
    def width(self):
        return self.thisptr.width
    
    @property
    def height(self):
        return self.thisptr.height
    
    @property
    def thickness(self):
        return self.thisptr.thickness
    
    @property
    def position(self):
        return (self.thisptr.position.first, self.thisptr.position.second)
    
    def set_position(self, double x, double y):
        self.thisptr.position.first = x
        self.thisptr.position.second = y
    
    @property
    def power_consumption(self):
        return self.thisptr.power_consumption
    
    @power_consumption.setter
    def power_consumption(self, double value):
        self.thisptr.power_consumption = value

cdef class PyInterconnect:
    cdef Interconnect* thisptr
    
    def __cinit__(self, str from_die, str to_die, str signal_type):
        self.thisptr = new Interconnect(from_die.encode('utf-8'), 
                                       to_die.encode('utf-8'), 
                                       signal_type.encode('utf-8'))
    
    def __dealloc__(self):
        del self.thisptr
    
    @property
    def from_die(self):
        return self.thisptr.from_die.decode('utf-8')
    
    @property
    def to_die(self):
        return self.thisptr.to_die.decode('utf-8')
    
    @property
    def signal_type(self):
        return self.thisptr.signal_type.decode('utf-8')
    
    @property
    def resistance(self):
        return self.thisptr.resistance
    
    @property
    def capacitance(self):
        return self.thisptr.capacitance
    
    @property
    def delay(self):
        return self.thisptr.delay

cdef class PyMultiDieModel:
    cdef MultiDieModel* thisptr
    
    def __cinit__(self):
        self.thisptr = new MultiDieModel()
    
    def __dealloc__(self):
        del self.thisptr
    
    def add_die(self, PyDie die):
        self.thisptr.addDie(die.thisptr[0])
    
    def remove_die(self, str die_id):
        self.thisptr.removeDie(die_id.encode('utf-8'))
    
    def position_die(self, str die_id, double x, double y):
        self.thisptr.positionDie(die_id.encode('utf-8'), x, y)
    
    def perform_wire_bonding(self, PyWafer wafer, str die1, str die2, list bonds):
        cdef vector[pair[pair[int, int], pair[int, int]]] cpp_bonds
        for bond in bonds:
            cpp_bonds.push_back(pair[pair[int, int], pair[int, int]](
                pair[int, int](bond[0][0], bond[0][1]),
                pair[int, int](bond[1][0], bond[1][1])
            ))
        self.thisptr.performWireBonding(wafer.thisptr, 
                                       die1.encode('utf-8'), 
                                       die2.encode('utf-8'), 
                                       cpp_bonds)
    
    def perform_flip_chip_bonding(self, PyWafer wafer, str die1, str die2, 
                                 double bump_pitch, double bump_diameter):
        self.thisptr.performFlipChipBonding(wafer.thisptr, 
                                           die1.encode('utf-8'), 
                                           die2.encode('utf-8'), 
                                           bump_pitch, bump_diameter)
    
    def perform_tsv_integration(self, PyWafer wafer, str die_id, list tsv_positions,
                               double tsv_diameter, double tsv_depth):
        cdef vector[pair[double, double]] cpp_positions
        for pos in tsv_positions:
            cpp_positions.push_back(pair[double, double](pos[0], pos[1]))
        self.thisptr.performTSVIntegration(wafer.thisptr, 
                                          die_id.encode('utf-8'), 
                                          cpp_positions, 
                                          tsv_diameter, tsv_depth)
    
    def analyze_electrical_performance(self, PyWafer wafer):
        self.thisptr.analyzeElectricalPerformance(wafer.thisptr)
    
    def analyze_thermal_performance(self, PyWafer wafer):
        self.thisptr.analyzeThermalPerformance(wafer.thisptr)
    
    def add_interconnect(self, PyInterconnect interconnect):
        self.thisptr.addInterconnect(interconnect.thisptr[0])
    
    def calculate_interconnect_parameters(self):
        self.thisptr.calculateInterconnectParameters()
    
    def get_system_metrics(self):
        cdef unordered_map[string, double] metrics = self.thisptr.getSystemMetrics()
        result = {}
        for item in metrics:
            result[item.first.decode('utf-8')] = item.second
        return result
    
    def get_die_count(self):
        return self.thisptr.getDies().size()
    
    def get_interconnect_count(self):
        return self.thisptr.getInterconnects().size()

# Constants for Python interface
DIE_TYPE_LOGIC = <int>LOGIC
DIE_TYPE_MEMORY = <int>MEMORY
DIE_TYPE_ANALOG = <int>ANALOG
DIE_TYPE_RF = <int>RF
DIE_TYPE_SENSOR = <int>SENSOR
DIE_TYPE_POWER = <int>POWER

INTEGRATION_WIRE_BONDING = <int>WIRE_BONDING
INTEGRATION_FLIP_CHIP = <int>FLIP_CHIP
INTEGRATION_TSV = <int>TSV
INTEGRATION_WLP = <int>WLP
INTEGRATION_CHIPLET = <int>CHIPLET

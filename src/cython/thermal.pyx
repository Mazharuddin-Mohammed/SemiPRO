# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/thermal/thermal_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from geometry cimport PyWafer

cdef extern from "../cpp/modules/thermal/thermal_model.hpp":
    cppclass ThermalSimulationModel:
        ThermalSimulationModel() except +
        void simulateThermal(shared_ptr[Wafer], double, double) except +

cdef class PyThermalSimulationModel:
    cdef ThermalSimulationModel* thisptr
    def __cinit__(self):
        self.thisptr = new ThermalSimulationModel()
    def __dealloc__(self):
        del self.thisptr
    def simulate_thermal(self, wafer: PyWafer, ambient_temperature: float, current: float):
        self.thisptr.simulateThermal(wafer.thisptr, ambient_temperature, current)
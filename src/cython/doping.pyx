# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/doping/monte_carlo_solver.cpp ../cpp/modules/doping/diffusion_solver.cpp ../cpp/modules/doping/doping_manager.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from geometry cimport PyWafer

cdef extern from "../cpp/modules/doping/doping_manager.hpp":
    cppclass DopingManager:
        DopingManager() except +
        void simulateIonImplantation(shared_ptr[Wafer], double, double) except +
        void simulateDiffusion(shared_ptr[Wafer], double, double) except +

cdef class PyDopingManager:
    cdef DopingManager* thisptr
    def __cinit__(self):
        self.thisptr = new DopingManager()
    def __dealloc__(self):
        del self.thisptr
    def simulate_ion_implantation(self, wafer: PyWafer, energy: float, dose: float):
        self.thisptr.simulateIonImplantation(wafer.thisptr, energy, dose)
    def simulate_diffusion(self, wafer: PyWafer, temperature: float, time: float):
        self.thisptr.simulateDiffusion(wafer.thisptr, temperature, time)
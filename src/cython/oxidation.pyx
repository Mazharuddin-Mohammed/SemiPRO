# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/oxidation/oxidation_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/oxidation/oxidation_model.hpp":
    cppclass OxidationModel:
        OxidationModel() except +
        void simulateOxidation(shared_ptr[Wafer], double, double) except +

cdef class PyOxidationModel:
    cdef OxidationModel* thisptr
    def __cinit__(self):
        self.thisptr = new OxidationModel()
    def __dealloc__(self):
        del self.thisptr
    def simulate_oxidation(self, wafer: PyWafer, temperature: float, time: float):
        self.thisptr.simulateOxidation(wafer.thisptr, temperature, time)
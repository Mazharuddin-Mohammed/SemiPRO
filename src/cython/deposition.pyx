# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/deposition/deposition_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/deposition/deposition_model.hpp":
    cppclass DepositionModel:
        DepositionModel() except +
        void simulateDeposition(shared_ptr[Wafer], double, string, string) except +

cdef class PyDepositionModel:
    cdef DepositionModel* thisptr
    def __cinit__(self):
        self.thisptr = new DepositionModel()
    def __dealloc__(self):
        del self.thisptr
    def simulate_deposition(self, wafer: PyWafer, thickness: float, material: str, type: str):
        self.thisptr.simulateDeposition(wafer.thisptr, thickness, material.encode('utf-8'), type.encode('utf-8'))
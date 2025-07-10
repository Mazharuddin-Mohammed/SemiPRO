# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/etching/etching_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/etching/etching_model.hpp":
    cppclass EtchingModel:
        EtchingModel() except +
        void simulateEtching(shared_ptr[Wafer], double, string) except +

cdef class PyEtchingModel:
    cdef EtchingModel* thisptr
    def __cinit__(self):
        self.thisptr = new EtchingModel()
    def __dealloc__(self):
        del self.thisptr
    def simulate_etching(self, wafer: PyWafer, depth: float, type: str):
        self.thisptr.simulateEtching(wafer.thisptr, depth, type.encode('utf-8'))
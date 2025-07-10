# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/reliability/reliability_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/reliability/reliability_model.hpp":
    cppclass ReliabilityModel:
        ReliabilityModel() except +
        void performReliabilityTest(shared_ptr[Wafer], double, double) except +

cdef class PyReliabilityModel:
    cdef ReliabilityModel* thisptr
    def __cinit__(self):
        self.thisptr = new ReliabilityModel()
    def __dealloc__(self):
        del self.thisptr
    def perform_reliability_test(self, wafer: PyWafer, current: float, voltage: float):
        self.thisptr.performReliabilityTest(wafer.thisptr, current, voltage)
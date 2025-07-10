# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/metallization/metallization_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from geometry cimport PyWafer, Wafer

cdef extern from "../cpp/modules/metallization/metallization_model.hpp":
    cppclass MetallizationModel:
        MetallizationModel() except +
        void simulateMetallization(shared_ptr[Wafer], double, string, string) except +

cdef class PyMetallizationModel:
    cdef MetallizationModel* thisptr
    def __cinit__(self):
        self.thisptr = new MetallizationModel()
    def __dealloc__(self):
        del self.thisptr
    def simulate_metallization(self, wafer: PyWafer, thickness: float, metal: str, method: str):
        self.thisptr.simulateMetallization(wafer.thisptr, thickness, metal.encode('utf-8'), method.encode('utf-8'))
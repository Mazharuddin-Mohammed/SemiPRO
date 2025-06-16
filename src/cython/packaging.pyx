# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/packaging/packaging_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from geometry cimport PyWafer

cdef extern from "../cpp/modules/packaging/packaging_model.hpp":
    cppclass PackagingModel:
        PackagingModel() except +
        void simulatePackaging(shared_ptr[Wafer], double, string, int) except +
        void performElectricalTest(shared_ptr[Wafer], string) except +

cdef class PyPackagingModel:
    cdef PackagingModel* thisptr
    def __cinit__(self):
        self.thisptr = new PackagingModel()
    def __dealloc__(self):
        del self.thisptr
    def simulate_packaging(self, wafer: PyWafer, substrate_thickness: float, substrate_material: str, num_wires: int):
        self.thisptr.simulatePackaging(wafer.thisptr, substrate_thickness, substrate_material.encode('utf-8'), num_wires)
    def perform_electrical_test(self, wafer: PyWafer, test_type: str):
        self.thisptr.performElectricalTest(wafer.thisptr, test_type.encode('utf-8'))
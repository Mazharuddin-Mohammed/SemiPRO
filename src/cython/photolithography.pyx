# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/modules/photolithography/lithography_model.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from geometry cimport PyWafer

cdef extern from "../cpp/modules/photolithography/lithography_model.hpp":
    cppclass LithographyModel:
        LithographyModel() except +
        void simulateExposure(shared_ptr[Wafer], double, double, vector[vector[int]]) except +
        void simulateMultiPatterning(shared_ptr[Wafer], double, double, vector[vector[vector[int]]]) except +

cdef class PyLithographyModel:
    cdef LithographyModel* thisptr
    def __cinit__(self):
        self.thisptr = new LithographyModel()
    def __dealloc__(self):
        del self.thisptr
    def simulate_exposure(self, wafer: PyWafer, wavelength: float, na: float, mask: list):
        cdef vector[vector[int]] cpp_mask
        for row in mask:
            cpp_mask.push_back(row)
        self.thisptr.simulateExposure(wafer.thisptr, wavelength, na, cpp_mask)
    def simulate_multi_patterning(self, wafer: PyWafer, wavelength: float, na: float, masks: list):
        cdef vector[vector[vector[int]]] cpp_masks
        for mask in masks:
            cdef vector[vector[int]] cpp_mask
            for row in mask:
                cpp_mask.push_back(row)
            cpp_masks.push_back(cpp_mask)
        self.thisptr.simulateMultiPatterning(wafer.thisptr, wavelength, na, cpp_masks)
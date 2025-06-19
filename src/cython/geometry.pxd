# Author: Dr. Mazharuddin Mohammed
# Cython declaration file for geometry module

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
cimport numpy as np

cdef extern from "../cpp/core/wafer.hpp":
    cppclass Wafer:
        Wafer(double, double, string) except +
        void initializeGrid(int, int) except +
        void applyLayer(double, string) except +
        void setDopantProfile(const double*) except +
        void setPhotoresistPattern(const double*, int, int) except +
        void addFilmLayer(double, string) except +
        void addMetalLayer(double, string) except +
        void addPackaging(double, string, vector[pair[pair[int, int], pair[int, int]]]) except +
        void setElectricalProperties(vector[pair[string, double]]) except +
        void setTemperatureProfile(const double*, int, int) except +
        void setThermalConductivity(const double*, int, int) except +
        void setElectromigrationMTTF(const double*, int, int) except +
        void setThermalStress(const double*, int, int) except +
        void setDielectricField(const double*, int, int) except +
        void updateGrid(const double*, int, int) except +
        double* getGrid(int*, int*) except +
        double* getDopantProfile(int*) except +
        double* getPhotoresistPattern(int*, int*) except +
        vector[pair[double, string]]& getFilmLayers() except +
        vector[pair[double, string]]& getMetalLayers() except +
        pair[double, string] getPackagingSubstrate() except +
        const vector[pair[pair[int, int], pair[int, int]]]& getWireBonds() except +
        const vector[pair[string, double]]& getElectricalProperties() except +
        double* getTemperatureProfile(int*, int*) except +
        double* getThermalConductivity(int*, int*) except +
        double* getElectromigrationMTTF(int*, int*) except +
        double* getThermalStress(int*, int*) except +
        double* getDielectricField(int*, int*) except +
        string getMaterialId() except +
        double getDiameter() except +
        double getThickness() except +

cdef extern from "../cpp/modules/geometry/geometry_manager.hpp":
    cppclass GeometryManager:
        GeometryManager() except +
        void initializeGrid(shared_ptr[Wafer], int, int) except +
        void applyLayer(shared_ptr[Wafer], double, string) except +

cdef class PyWafer:
    cdef shared_ptr[Wafer] thisptr

cdef class PyGeometryManager:
    cdef GeometryManager* thisptr

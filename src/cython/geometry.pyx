# Author: Dr. Mazharuddin Mohammed
# distutils: language = c++
# distutils: sources = ../cpp/core/wafer.cpp ../cpp/core/utils.cpp

from libcpp.memory cimport shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
cimport numpy as np
import numpy as np

# Import declarations from .pxd file
from geometry cimport PyWafer, PyGeometryManager, Wafer, GeometryManager

# Implementation of PyWafer class (declaration is in .pxd file)
cdef class PyWafer:
    def __cinit__(self, diameter: float, thickness: float, material_id: str):
        self.thisptr = shared_ptr[Wafer](new Wafer(diameter, thickness, material_id.encode('utf-8')))

    def initialize_grid(self, x_dim: int, y_dim: int):
        self.thisptr.get().initializeGrid(x_dim, y_dim)

    def apply_layer(self, thickness: float, material_id: str):
        self.thisptr.get().applyLayer(thickness, material_id.encode('utf-8'))

    def set_dopant_profile(self, np.ndarray[np.float64_t, ndim=1] profile):
        self.thisptr.get().setDopantProfile(&profile[0])

    def set_photoresist_pattern(self, np.ndarray[np.float64_t, ndim=2] pattern):
        self.thisptr.get().setPhotoresistPattern(&pattern[0, 0], pattern.shape[0], pattern.shape[1])

    def add_film_layer(self, thickness: float, material: str):
        self.thisptr.get().addFilmLayer(thickness, material.encode('utf-8'))

    def add_metal_layer(self, thickness: float, metal: str):
        self.thisptr.get().addMetalLayer(thickness, metal.encode('utf-8'))

    def add_packaging(self, substrate_thickness: float, substrate_material: str, wire_bonds: list):
        cdef vector[pair[pair[int, int], pair[int, int]]] cpp_bonds
        for bond in wire_bonds:
            x1, y1, x2, y2 = bond
            cpp_bonds.push_back(pair[pair[int, int], pair[int, int]](pair[int, int](x1, y1), pair[int, int](x2, y2)))
        self.thisptr.get().addPackaging(substrate_thickness, substrate_material.encode('utf-8'), cpp_bonds)

    def set_electrical_properties(self, properties: list):
        cdef vector[pair[string, double]] cpp_props
        for prop in properties:
            name, value = prop
            cpp_props.push_back(pair[string, double](name.encode('utf-8'), value))
        self.thisptr.get().setElectricalProperties(cpp_props)

    def set_temperature_profile(self, np.ndarray[np.float64_t, ndim=2] profile):
        self.thisptr.get().setTemperatureProfile(&profile[0, 0], profile.shape[0], profile.shape[1])

    def set_thermal_conductivity(self, np.ndarray[np.float64_t, ndim=2] conductivity):
        self.thisptr.get().setThermalConductivity(&conductivity[0, 0], conductivity.shape[0], conductivity.shape[1])

    def set_electromigration_mttf(self, np.ndarray[np.float64_t, ndim=2] mttf):
        self.thisptr.get().setElectromigrationMTTF(&mttf[0, 0], mttf.shape[0], mttf.shape[1])

    def set_thermal_stress(self, np.ndarray[np.float64_t, ndim=2] stress):
        self.thisptr.get().setThermalStress(&stress[0, 0], stress.shape[0], stress.shape[1])

    def set_dielectric_field(self, np.ndarray[np.float64_t, ndim=2] field):
        self.thisptr.get().setDielectricField(&field[0, 0], field.shape[0], field.shape[1])

    def update_grid(self, np.ndarray[np.float64_t, ndim=2] grid):
        self.thisptr.get().updateGrid(&grid[0, 0], grid.shape[0], grid.shape[1])

    def get_grid(self):
        cdef int rows, cols
        cdef double* grid_ptr = self.thisptr.get().getGrid(&rows, &cols)
        return np.asarray(<np.float64_t[:rows, :cols]> grid_ptr)

    def get_dopant_profile(self):
        cdef int size
        cdef double* profile_ptr = self.thisptr.get().getDopantProfile(&size)
        return np.asarray(<np.float64_t[:size]> profile_ptr)

    def get_photoresist_pattern(self):
        cdef int rows, cols
        cdef double* pattern_ptr = self.thisptr.get().getPhotoresistPattern(&rows, &cols)
        return np.asarray(<np.float64_t[:rows, :cols]> pattern_ptr)
    def get_film_layers(self):
        layers = self.thisptr.get().getFilmLayers()
        return [(layer.first, layer.second.decode('utf-8')) for layer in layers]
    def get_metal_layers(self):
        layers = self.thisptr.get().getMetalLayers()
        return [(layer.first, layer.second.decode('utf-8')) for layer in layers]
    def get_packaging_substrate(self):
        substrate = self.thisptr.get().getPackagingSubstrate()
        return (substrate.first, substrate.second.decode('utf-8'))
    def get_wire_bonds(self):
        bonds = self.thisptr.get().getWireBonds()
        return [((bond.first.first, bond.first.second), (bond.second.first, bond.second.second)) for bond in bonds]
    def get_electrical_properties(self):
        props = self.thisptr.get().getElectricalProperties()
        return [(prop.first.decode('utf-8'), prop.second) for prop in props]
    def get_temperature_profile(self):
        cdef int rows, cols
        cdef double* profile_ptr = self.thisptr.get().getTemperatureProfile(&rows, &cols)
        return np.asarray(<np.float64_t[:rows, :cols]> profile_ptr)

    def get_thermal_conductivity(self):
        cdef int rows, cols
        cdef double* conductivity_ptr = self.thisptr.get().getThermalConductivity(&rows, &cols)
        return np.asarray(<np.float64_t[:rows, :cols]> conductivity_ptr)

    def get_electromigration_mttf(self):
        cdef int rows, cols
        cdef double* mttf_ptr = self.thisptr.get().getElectromigrationMTTF(&rows, &cols)
        return np.asarray(<np.float64_t[:rows, :cols]> mttf_ptr)

    def get_thermal_stress(self):
        cdef int rows, cols
        cdef double* stress_ptr = self.thisptr.get().getThermalStress(&rows, &cols)
        return np.asarray(<np.float64_t[:rows, :cols]> stress_ptr)

    def get_dielectric_field(self):
        cdef int rows, cols
        cdef double* field_ptr = self.thisptr.get().getDielectricField(&rows, &cols)
        return np.asarray(<np.float64_t[:rows, :cols]> field_ptr)
    def get_material_id(self):
        return self.thisptr.get().getMaterialId().decode('utf-8')
    def get_diameter(self):
        return self.thisptr.get().getDiameter()
    def get_thickness(self):
        return self.thisptr.get().getThickness()

cdef class PyGeometryManager:
    def __cinit__(self):
        self.thisptr = new GeometryManager()
    def __dealloc__(self):
        del self.thisptr
    def initialize_grid(self, PyWafer wafer, x_dim: int, y_dim: int):
        self.thisptr.initializeGrid(wafer.thisptr, x_dim, y_dim)
    def apply_layer(self, PyWafer wafer, thickness: float, material_id: str):
        self.thisptr.applyLayer(wafer.thisptr, thickness, material_id.encode('utf-8'))
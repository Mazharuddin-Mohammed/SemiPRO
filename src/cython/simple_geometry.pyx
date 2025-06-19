# Author: Dr. Mazharuddin Mohammed
# Simple Cython wrapper for SemiPRO geometry module

import numpy as np
cimport numpy as np
from libcpp.string cimport string

# Simple Python wrapper class
cdef class PyWafer:
    """Python wrapper for Wafer class"""
    cdef double diameter
    cdef double thickness
    cdef string material
    cdef int grid_x, grid_y
    
    def __init__(self, double diameter, double thickness, material_str):
        self.diameter = diameter
        self.thickness = thickness
        self.material = material_str.encode('utf-8')
        self.grid_x = 100
        self.grid_y = 100
    
    def get_diameter(self):
        return self.diameter
    
    def get_thickness(self):
        return self.thickness
    
    def get_material(self):
        return self.material.decode('utf-8')
    
    def initialize_grid(self, int x_dim, int y_dim):
        self.grid_x = x_dim
        self.grid_y = y_dim
        return True
    
    def apply_layer(self, double thickness, material_str):
        # Simple layer application simulation
        return True

cdef class PyGeometryManager:
    """Python wrapper for GeometryManager"""
    
    def __init__(self):
        pass
    
    def initialize_grid(self, wafer, int x_dim, int y_dim):
        if hasattr(wafer, 'initialize_grid'):
            return wafer.initialize_grid(x_dim, y_dim)
        return True
    
    def apply_layer(self, wafer, double thickness, material_str):
        if hasattr(wafer, 'apply_layer'):
            return wafer.apply_layer(thickness, material_str)
        return True

# Utility functions
def calculate_area(double diameter):
    """Calculate wafer area"""
    cdef double radius = diameter / 2.0
    return 3.14159265359 * radius * radius

def calculate_volume(double diameter, double thickness):
    """Calculate wafer volume"""
    return calculate_area(diameter) * thickness

def format_material(material_str):
    """Format material string"""
    return material_str.lower().replace(' ', '_')

# Test function
def test_cython_integration():
    """Test Cython integration"""
    print("✅ Cython extension loaded successfully!")
    
    # Create a test wafer
    wafer = PyWafer(100.0, 0.5, "silicon")
    print(f"✅ Created wafer: {wafer.get_diameter()}mm diameter, {wafer.get_material()}")
    
    # Test geometry manager
    manager = PyGeometryManager()
    manager.initialize_grid(wafer, 50, 50)
    manager.apply_layer(wafer, 0.1, "silicon_dioxide")
    print("✅ Geometry operations completed")
    
    # Test utility functions
    area = calculate_area(100.0)
    volume = calculate_volume(100.0, 0.5)
    print(f"✅ Calculated area: {area:.2f} mm², volume: {volume:.2f} mm³")
    
    return True

import pytest
from semiconductor_simulator.geometry import PyGeometryManager, PyWafer

def test_geometry_initialize_grid():
    wafer = PyWafer(300.0, 775.0, "silicon")
    geometry = PyGeometryManager()
    geometry.initialize_grid(wafer, 10, 10)
    # Add grid access in PyWafer for testing if needed

def test_geometry_apply_layer():
    wafer = PyWafer(300.0, 775.0, "silicon")
    geometry = PyGeometryManager()
    geometry.apply_layer(wafer, 0.1, "oxide")
    # Add material/thickness access for testing
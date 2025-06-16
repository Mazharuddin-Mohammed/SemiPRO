import pytest
import numpy as np
from semiconductor_simulator import Simulator

def test_pvd_metallization():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    initial_grid_sum = simulator.get_wafer().get_grid().sum()
    simulator.run_pvdmetallization(0.5, "Cu", "pvd")
    final_grid_sum = simulator.get_wafer().get_grid().sum()
    assert final_grid_sum > initial_grid_sum, "Grid sum should increase after PVD metallization"

def test_electroplating_metallization():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    initial_grid_sum = simulator.get_wafer().get_grid().sum()
    simulator.run_electroplatingmetallization(0.5, "metalCu", "electroplating")
    final_grid_sum = simulator.get_wafer().get_grid().sum()
    assert final_grid_sum > initial_grid_sum, "Grid sum should increase after electroplating"
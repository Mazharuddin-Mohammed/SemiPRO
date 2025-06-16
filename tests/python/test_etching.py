import pytest
from semiconductor_simulator.simulator import Simulator

def test_isotropic_etching():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    initial_grid_sum = simulator.get_wafer().get_grid().sum()
    simulator.run_etching(0.05, "isotropic")
    final_grid_sum = simulator.get_wafer().get_grid().sum()
    assert final_grid_sum < initial_grid_sum, "Grid sum should decrease after isotropic etching"

def test_anisotropic_etching():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    initial_grid_sum = simulator.get_wafer().get_grid().sum()
    simulator.run_etching(0.05, "anisotropic")
    final_grid_sum = simulator.get_wafer().get_grid().sum()
    assert final_grid_sum < initial_grid_sum, "Grid sum should decrease after anisotropic etching"
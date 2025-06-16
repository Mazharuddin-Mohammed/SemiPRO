import pytest
from semiconductor_simulator.simulator import Simulator

def test_uniform_deposition():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    initial_grid_sum = simulator.get_wafer().get_grid().sum()
    simulator.run_deposition(0.1, "SiO2", "uniform")
    final_grid_sum = simulator.get_wafer().get_grid().sum()
    assert final_grid_sum > initial_grid_sum, "Grid sum should increase after deposition"
    # Note: PyWafer needs film layer access for deeper testing if implemented

def test_conformal_deposition():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    initial_grid_sum = simulator.get_wafer().get_grid().sum()
    simulator.run_deposition(0.1, "SiN", "conformal")
    final_grid_sum = simulator.get_wafer().get_grid().sum()
    assert final_grid_sum > initial_grid_sum, "Grid sum should increase after conformal deposition"
import pytest
import numpy as np
from semiconductor_simulator import Simulator

def test_thermal_simulation():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.perform_electrical_test("resistance")
    simulator.simulate_thermal(300.0, 0.001)
    wafer = simulator.get_wafer()
    temp_profile = wafer.get_temperature_profile()
    assert temp_profile.shape == (10, 10), "Temperature profile should match grid size"
    assert temp_profile.min() >= 300.0, "Minimum temperature should be ambient or higher"

def test_thermal_no_metal():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    simulator.simulate_thermal(300.0, 0.001)
    wafer = simulator.get_wafer()
    temp_profile = wafer.get_temperature_profile()
    assert np.all(temp_profile == 300.0), "No metal layers should result in uniform ambient temperature"
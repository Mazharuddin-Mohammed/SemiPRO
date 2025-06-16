import pytest
from semiconductor_simulator.simulator import Simulator

def test_exposure_simulation():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    # Add pattern access in PyWafer for testing if needed
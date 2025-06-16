import pytest
from semiconductor_simulator.simulator import Simulator

def test_doping_simulation():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    simulator.run_ion_implantation(50.0, 1e15)
    simulator.run_diffusion(1000.0, 3600.0)
    # Add dopant profile access in PyWafer for testing if needed
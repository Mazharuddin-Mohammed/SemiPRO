import pytest
from semiconductor_simulator.simulator import Simulator
from semiconductor_simulator.oxidation import PyOxidationModel
from semiconductor_simulator.geometry import PyWafer

def test_oxidation_simulation():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    initial_thickness = simulator.get_wafer().thisptr.get().getThickness()
    simulator.run_oxidation(1000.0, 3600.0)
    final_thickness = simulator.get_wafer().thisptr.get().getThickness()
    assert final_thickness > initial_thickness
    # Add material access in PyWafer for oxide check if needed
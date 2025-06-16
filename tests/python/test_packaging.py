import pytest
import numpy as np
from semiconductor_simulator import Simulator

def test_packaging():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.simulate_packaging(1000.0, "Ceramic", 2)
    wafer = simulator.get_wafer()
    substrate_thickness, substrate_material = wafer.get_packaging_substrate()
    wire_bonds = wafer.get_wire_bonds()
    assert substrate_thickness == 1000.0, "Substrate thickness should be 1000 um"
    assert substrate_material == "Ceramic", "Substrate material should be Ceramic"
    assert len(wire_bonds) <= 2, "Number of wire bonds should not exceed requested"

def test_resistance_test():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.perform_electrical_test("resistance")
    wafer = simulator.get_wafer()
    properties = wafer.get_electrical_properties()
    assert len(properties) > 0, "Electrical properties should be set"
    assert properties[0][0] == "Resistance", "Property should be Resistance"
    assert properties[0][1] > 0.0, "Resistance should be positive"

def test_capacitance_test():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = [[1, 0, 1], [0, 1, 0], [1, 0, 1]]
    simulator.run_exposure(13.5, 0.33, mask)
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.perform_electrical_test("capacitance")
    wafer = simulator.get_wafer()
    properties = wafer.get_electrical_properties()
    assert len(properties) > 0, "Electrical properties should be set"
    assert properties[0][0] == "Capacitance", "Property should be Capacitance"
    assert properties[0][1] > 0.0, "Capacitance should be positive"
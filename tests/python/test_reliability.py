import pytest
import numpy as np
from semiconductor_simulator import Simulator

def test_reliability_simulation():
    simulator = Simulator()
    simulator.initialize_geometry(10, 10)
    mask = np.zeros((10, 10))
    mask[2:5, 2:5] = 0.1
    simulator.run_exposure(13.5, 0.33, mask.tolist())
    simulator.run_metallization(0.5, "Cu", "pvd")
    simulator.run_deposition(0.1, "SiO2", "cvd")
    simulator.perform_electrical_test("resistance")
    simulator.simulate_thermal(300.0, 0.001)
    simulator.perform_reliability_test(0.001, 5.0)
    
    wafer = simulator.get_wafer()
    mttf = wafer.get_electromigration_mttf()
    stress = wafer.get_thermal_stress()
    field = wafer.get_dielectric_field()
    
    assert mttf.max() > 0, "MTTF should have non-zero values"
    assert mttf[mttf > 0].min() > 1e6, "MTTF should be > 1e6 s"
    assert stress.max() > 0, "Stress should have non-zero values"
    assert stress.max() < 1000, "Stress should be < 1000 MPa"
    assert field.max() > 0, "Field should have non-zero values"
    assert field.max() < 1e7, "Field should be < 10 MV/cm"

def test_reliability_gui():
    from PySide6.QtWidgets import QApplication
    from semiconductor_simulator.gui.reliability_panel import ReliabilityPanel
    app = QApplication([])
    
    panel = ReliabilityPanel()
    panel.current_input.setText("0.001")
    panel.voltage_input.setText("5.0")
    
    # Mock wafer setup
    panel.simulator.initialize_geometry(10, 10)
    mask = np.zeros((10, 10))
    mask[2:5, 2:5] = 0.1
    panel.simulator.run_exposure(13.5, 0.33, mask.tolist())
    panel.simulator.run_metallization(0.5, "Cu", "pvd")
    panel.simulator.run_deposition(0.1, "SiO2", "cvd")
    panel.simulator.perform_electrical_test("resistance")
    panel.simulator.simulate_thermal(300.0, 0.001)
    
    panel.run_test()
    assert "Min MTTF" in panel.result_label.text(), "Result should include MTTF"
    assert "Max Stress" in panel.result_label.text(), "Result should include stress"
    assert "Max Field" in panel.result_label.text(), "Result should include field"
    assert panel.error_label.text() == "", "No errors should occur"
    
    # Test metric selector
    panel.metric_selector.setCurrentText("Electromigration")
    assert panel.metric_selector.currentText() == "Electromigration"
    panel.metric_selector.setCurrentText("Thermal Stress")
    assert panel.metric_selector.currentText() == "Thermal Stress"
    panel.metric_selector.setCurrentText("Dielectric Breakdown")
    assert panel.metric_selector.currentText() == "Dielectric Breakdown"
    panel.metric_selector.setCurrentText("None")
    assert panel.metric_selector.currentText() == "None"
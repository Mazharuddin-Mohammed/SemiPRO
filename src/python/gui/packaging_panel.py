from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton, QComboBox, QLabel, QSpinBox
from PySide6.QtCore import Signal
from ..simulator import Simulator

class PackagingPanel(QWidget):
    packaging_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.substrate_thickness_input = QLineEdit("1000.0") # um
        self.substrate_material_input = QLineEdit("Ceramic")
        self.num_wires_input = QSpinBox()
        self.num_wires_input.setRange(0, 100)
        self.num_wires_input.setValue(5)
        self.test_type_combo = QComboBox()
        self.test_type_combo.addItems(["Resistance", "Capacitance"])
        self.error_label = QLabel("")
        self.result_label = QLabel("") # NEW
        packaging_button = QPushButton("Run Packaging")
        test_button = QPushButton("Perform Electrical Test")

        layout.addWidget(QLabel("Substrate Thickness (um):"))
        layout.addWidget(self.substrate_thickness_input)
        layout.addWidget(QLabel("Substrate Material:"))
        layout.addWidget(self.substrate_material_input)
        layout.addWidget(QLabel("Number of Wire Bonds:"))
        layout.addWidget(self.num_wires_input)
        layout.addWidget(self.error_label)
        layout.addWidget(packaging_button)
        layout.addWidget(QLabel("Electrical Test Type:"))
        layout.addWidget(self.test_type_combo)
        layout.addWidget(test_button)
        layout.addWidget(QLabel("Test Result:"))
        layout.addWidget(self.result_label)

        packaging_button.clicked.connect(self.run_packaging)
        test_button.clicked.connect(self.run_test)

    def run_packaging(self):
        try:
            substrate_thickness = float(self.substrate_thickness_input.text())
            substrate_material = self.substrate_material_input.text()
            num_wires = self.num_wires_input.value()
            if substrate_thickness <= 0:
                self.error_label.setText("Error: Thickness must be positive")
                return
            self.error_label.setText("")
            self.simulator.simulate_packaging(substrate_thickness, substrate_material, num_wires)
            self.packaging_updated.emit(self.simulator.get_wafer())
        except ValueError:
            self.error_label.setText("Error: Invalid thickness")

    def run_test(self):
        try:
            test_type = self.test_type_combo.currentText().lower()
            self.error_label.setText("")
            self.simulator.perform_electrical_test(test_type)
            wafer = self.simulator.get_wafer()
            properties = wafer.get_electrical_properties()
            if properties:
                prop_name, prop_value = properties[0]
                unit = "Ohms" if prop_name == "Resistance" else "pF"
                self.result_label.setText(f"{prop_name}: {prop_value:.2e} {unit}")
            self.packaging_updated.emit(wafer)
        except Exception as e:
            self.error_label.setText(f"Error: {str(e)}")
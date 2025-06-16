from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton, QComboBox, QLabel
from PySide6.QtCore import Signal
from ..simulator import Simulator

class DepositionPanel(QWidget):
    deposition_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.thickness_input = QLineEdit("0.1") # um
        self.material_input = QLineEdit("SiO2")
        self.type_combo = QComboBox()
        self.type_combo.addItems(["Uniform", "Conformal"])
        self.error_label = QLabel("")
        deposit_button = QPushButton("Run Deposition")

        layout.addWidget(QLabel("Thickness (um):"))
        layout.addWidget(self.thickness_input)
        layout.addWidget(QLabel("Material:"))
        layout.addWidget(self.material_input)
        layout.addWidget(QLabel("Deposition Type:"))
        layout.addWidget(self.type_combo)
        layout.addWidget(self.error_label)
        layout.addWidget(deposit_button)

        deposit_button.clicked.connect(self.run_deposition)

    def run_deposition(self):
        try:
            thickness = float(self.thickness_input.text())
            material = self.material_input.text()
            dep_type = self.type_combo.currentText().lower()
            if thickness <= 0:
                self.error_label.setText("Error: Thickness must be positive")
                return
            self.error_label.setText("")
            self.simulator.run_deposition(thickness, material, dep_type)
            self.deposition_updated.emit(self.simulator.get_wafer())
        except ValueError:
            self.error_label.setText("Error: Invalid thickness")
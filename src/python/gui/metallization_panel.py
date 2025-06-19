from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton, QComboBox, QLabel
from PySide6.QtCore import Signal
from ..simulator import Simulator

class MetallizationPanel(QWidget):
    metallization_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.thickness_input = QLineEdit("0.5") # μm thickness
        self.metal_input = QLineEdit("Cu")
        self.method_combo = QComboBox()
        self.method_combo.addItems(["PVD", "Electroplating"])
        self.error_label = QLabel("")
        metallize_button = QPushButton("Run Metallization")

        layout.addWidget(QLabel("Thickness (um):"))
        layout.addWidget(self.thickness_input)
        layout.addWidget(QLabel("Metal:"))
        layout.addWidget(self.metal_input)
        layout.addWidget(QLabel("Method:"))
        layout.addWidget(self.method_combo)
        layout.addWidget(self.error_label)
        layout.addWidget(metallize_button)

        metallize_button.clicked.connect(self.run_metallization)

    def run_metallization(self):
        try:
            thickness = float(self.thickness_input.text())
            metal = self.metal_input.text()
            method = self.method_combo.currentText().lower()
            if thickness <= 0:
                self.error_label.setText("Error: Thickness must be positive")
                return
            self.error_label.setText("")
            self.simulator.run_metallization(thickness, metal, method)
            self.metallization_updated.emit(self.simulator.get_wafer())
        except ValueError:
            self.error_label.setText("Error: Invalid thickness")
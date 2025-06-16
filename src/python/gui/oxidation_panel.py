from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton
from PySide6.QtCore import Signal
from ..simulator import Simulator

class OxidationPanel(QWidget):
    oxidation_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.temperature_input = QLineEdit("1000")
        self.time_input = QLineEdit("3600")
        oxidize_button = QPushButton("Run Oxidation")

        layout.addWidget(self.temperature_input)
        layout.addWidget(self.time_input)
        layout.addWidget(oxidize_button)

        oxidize_button.clicked.connect(self.run_oxidation)

    def run_oxidation(self):
        temperature = float(self.temperature_input.text())
        time = float(self.time_input.text())
        self.simulator.run_oxidation(temperature, time)
        self.oxidation_updated.emit(self.simulator.get_wafer())
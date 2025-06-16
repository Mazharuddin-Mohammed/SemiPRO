from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton
from PySide6.QtCore import Signal
from ..simulator import Simulator

class DopingPanel(QWidget):
    doping_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.energy_input = QLineEdit("50")    # keV
        self.dose_input = QLineEdit("1e15")    # cm^-2
        self.temp_input = QLineEdit("1000")    # K
        self.time_input = QLineEdit("3600")    # s
        implant_button = QPushButton("Run Implantation")
        diffuse_button = QPushButton("Run Diffusion")

        layout.addWidget(self.energy_input)
        layout.addWidget(self.dose_input)
        layout.addWidget(self.temp_input)
        layout.addWidget(self.time_input)
        layout.addWidget(implant_button)
        layout.addWidget(diffuse_button)

        implant_button.clicked.connect(self.run_implantation)
        diffuse_button.clicked.connect(self.run_diffusion)

    def run_implantation(self):
        energy = float(self.energy_input.text())
        dose = float(self.dose_input.text())
        self.simulator.run_ion_implantation(energy, dose)
        self.doping_updated.emit(self.simulator.get_wafer())

    def run_diffusion(self):
        temperature = float(self.temp_input.text())
        time = float(self.time_input.text())
        self.simulator.run_diffusion(temperature, time)
        self.doping_updated.emit(self.simulator.get_wafer())
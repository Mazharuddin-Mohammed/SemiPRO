from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton, QLabel, QCheckBox
from PySide6.QtCore import Signal
from ..simulator import Simulator

class ThermalPanel(QWidget):
    thermal_updated = Signal(object)
    visualization_option_changed = Signal(bool)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.ambient_temp_input = QLineEdit("300.0") # K
        self.current_input = QLineEdit("0.001") # A
        self.overlay_checkbox = QCheckBox("Show Temperature Overlay")
        self.error_label = QLabel("")
        self.result_label = QLabel("")
        simulation_button = QPushButton("Run Thermal Simulation")

        layout.addWidget(QLabel("Ambient Temperature (K):"))
        layout.addWidget(self.ambient_temp_input)
        layout.addWidget(QLabel("Current (A):"))
        layout.addWidget(self.current_input)
        layout.addWidget(self.overlay_checkbox)
        layout.addWidget(self.error_label)
        layout.addWidget(simulation_button)
        layout.addWidget(QLabel("Max Temperature:"))
        layout.addWidget(self.result_label)

        simulation_button.clicked.connect(self.run_simulation)
        self.overlay_checkbox.stateChanged.connect(self.toggle_overlay)

    def run_simulation(self):
        try:
            ambient_temp = float(self.ambient_temp_input.text())
            current = float(self.current_input.text())
            if ambient_temp <= 0:
                self.error_label.setText("Error: Temperature must be positive")
                return
            if current < 0:
                self.error_label.setText("Error: Current must be non-negative")
                return
            self.error_label.setText("")
            self.simulator.simulate_thermal(ambient_temp, current)
            wafer = self.simulator.get_wafer()
            temp_profile = wafer.get_temperature_profile()
            max_temp = temp_profile.max()
            self.result_label.setText(f"{max_temp:.2f} K")
            self.thermal_updated.emit(wafer)
        except ValueError:
            self.error_label.setText("Error: Invalid input")
        except Exception as e:
            self.error_label.setText(f"Error: {str(e)}")

    def toggle_overlay(self, state):
        self.visualization_option_changed.emit(bool(state))
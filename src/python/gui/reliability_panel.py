from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton, QLabel, QCheckBox, QComboBox
from PySide6.QtCore import Signal
from ..simulator import Simulator

class ReliabilityPanel(QWidget):
    reliability_updated = Signal(object)
    visualization_option_changed = Signal(str)  # Changed to string for metric selection

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.current_input = QLineEdit("0.001")  # A
        self.voltage_input = QLineEdit("5.0")    # V
        self.metric_selector = QComboBox()
        self.metric_selector.addItems(["None", "Electromigration", "Thermal Stress", "Dielectric Breakdown"])
        self.error_label = QLabel("")
        self.result_label = QLabel("")
        test_button = QPushButton("Run Reliability Test")

        layout.addWidget(QLabel("Current (A):"))
        layout.addWidget(self.current_input)
        layout.addWidget(QLabel("Voltage (V):"))
        layout.addWidget(self.voltage_input)
        layout.addWidget(QLabel("Visualize Metric:"))
        layout.addWidget(self.metric_selector)
        layout.addWidget(self.error_label)
        layout.addWidget(test_button)
        layout.addWidget(QLabel("Reliability Metrics:"))
        layout.addWidget(self.result_label)

        test_button.clicked.connect(self.run_test)
        self.metric_selector.currentTextChanged.connect(self.update_visualization)

    def run_test(self):
        try:
            current = float(self.current_input.text())
            voltage = float(self.voltage_input.text())
            if current < 0:
                self.error_label.setText("Error: Current must be non-negative")
                return
            if voltage < 0:
                self.error_label.setText("Error: Voltage must be non-negative")
                return
            self.error_label.setText("")
            self.simulator.perform_reliability_test(current, voltage)
            wafer = self.simulator.get_wafer()
            mttf = wafer.get_electromigration_mttf()
            stress = wafer.get_thermal_stress()
            field = wafer.get_dielectric_field()
            mttf_min = f"{mttf.min():.2e} s" if mttf.any() > 0 else "N/A"
            stress_max = f"{stress.max():.2f} MPa"
            field_max = f"{field.max():.2e} V/cm" if field.any() > 0 else "N/A"
            self.result_label.setText(
                f"Min MTTF: {mttf_min}\nMax Stress: {stress_max}\nMax Field: {field_max}"
            )
            self.reliability_updated.emit(wafer)
        except ValueError:
            self.error_label.setText("Error: Invalid input")
        except Exception as e:
            self.error_label.setText(f"Error: {str(e)}")

    def update_visualization(self, metric: str):
        self.visualization_option_changed.emit(metric)
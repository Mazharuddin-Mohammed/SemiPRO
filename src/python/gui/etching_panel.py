from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton, QComboBox, QLabel
from PySide6.QtCore import Signal
from ..simulator import Simulator

class EtchingPanel(QWidget):
    etching_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.depth_input = QLineEdit("0.05") # um
        self.type_combo = QComboBox()
        self.type_combo.addItems(["Isotropic", "Anisotropic"])
        self.error_label = QLabel("")
        etch_button = QPushButton("Run Etching")

        layout.addWidget(QLabel("Etch Depth (um):"))
        layout.addWidget(self.depth_input)
        layout.addWidget(QLabel("Etching Type:"))
        layout.addWidget(self.type_combo)
        layout.addWidget(self.error_label)
        layout.addWidget(etch_button)

        etch_button.clicked.connect(self.run_etching)

    def run_etching(self):
        try:
            depth = float(self.depth_input.text())
            etch_type = self.type_combo.currentText().lower()
            if depth <= 0:
                self.error_label.setText("Error: Depth must be positive")
                return
            self.error_label.setText("")
            self.simulator.run_etching(depth, etch_type)
            self.etching_updated.emit(self.simulator.get_wafer())
        except ValueError:
            self.error_label.setText("Error: Invalid depth")
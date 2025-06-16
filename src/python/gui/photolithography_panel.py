import json
from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton, QLabel
from PySide6.QtCore import Signal
from ..simulator import Simulator

class PhotolithographyPanel(QWidget):
    lithography_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.wavelength_input = QLineEdit("13.5") # nm (EUV)
        self.na_input = QLineEdit("0.33")         # Numerical aperture
        self.mask_input = QLineEdit("[[1,0,1],[0,1,0],[1,0,1]]") # JSON-like mask
        self.error_label = QLabel("")             # Display parsing errors
        expose_button = QPushButton("Run Exposure")
        multipattern_button = QPushButton("Run Multi-Patterning")

        layout.addWidget(QLabel("Wavelength (nm):"))
        layout.addWidget(self.wavelength_input)
        layout.addWidget(QLabel("Numerical Aperture:"))
        layout.addWidget(self.na_input)
        layout.addWidget(QLabel("Mask (JSON format):"))
        layout.addWidget(self.mask_input)
        layout.addWidget(self.error_label)
        layout.addWidget(expose_button)
        layout.addWidget(multipattern_button)

        expose_button.clicked.connect(self.run_exposure)
        multipattern_button.clicked.connect(self.run_multi_patterning)

    def parse_mask(self, mask_text):
        try:
            mask = json.loads(mask_text)
            if not isinstance(mask, list) or not all(isinstance(row, list) for row in mask):
                raise ValueError("Mask must be a 2D list of integers")
            if not all(all(isinstance(x, int) and x in [0, 1] for x in row) for row in mask):
                raise ValueError("Mask elements must be 0 or 1")
            return mask
        except (json.JSONDecodeError, ValueError) as e:
            self.error_label.setText(f"Error: {str(e)}")
            return None

    def run_exposure(self):
        try:
            wavelength = float(self.wavelength_input.text())
            na = float(self.na_input.text())
            mask = self.parse_mask(self.mask_input.text())
            if mask is None:
                return
            self.error_label.setText("")
            self.simulator.run_exposure(wavelength, na, mask)
            self.lithography_updated.emit(self.simulator.get_wafer())
        except ValueError as e:
            self.error_label.setText(f"Error: Invalid wavelength or NA")

    def run_multi_patterning(self):
        try:
            wavelength = float(self.wavelength_input.text())
            na = float(self.na_input.text())
            mask = self.parse_mask(self.mask_input.text())
            if mask is None:
                return
            self.error_label.setText("")
            masks = [mask, mask] # Example: Two identical masks
            self.simulator.run_multi_patterning(wavelength, na, masks)
            self.lithography_updated.emit(self.simulator.get_wafer())
        except ValueError as e:
            self.error_label.setText(f"Error: Invalid wavelength or NA")
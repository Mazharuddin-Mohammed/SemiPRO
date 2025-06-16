from PySide6.QtWidgets import QWidget, QVBoxLayout, QLineEdit, QPushButton
from PySide6.QtCore import Signal
from ..simulator import Simulator

class GeometryPanel(QWidget):
    geometry_updated = Signal(object)

    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        layout = QVBoxLayout(self)

        self.x_dim_input = QLineEdit("100")
        self.y_dim_input = QLineEdit("100")
        self.thickness_input = QLineEdit("0.1")
        self.material_input = QLineEdit("silicon")
        init_button = QPushButton("Initialize Grid")
        apply_button = QPushButton("Apply Layer")

        layout.addWidget(self.x_dim_input)
        layout.addWidget(self.y_dim_input)
        layout.addWidget(self.thickness_input)
        layout.addWidget(self.material_input)
        layout.addWidget(init_button)
        layout.addWidget(apply_button)

        init_button.clicked.connect(self.initialize_grid)
        apply_button.clicked.connect(self.apply_layer)

    def initialize_grid(self):
        x_dim = int(self.x_dim_input.text())
        y_dim = int(self.y_dim_input.text())
        self.simulator.initialize_geometry(x_dim, y_dim)
        self.geometry_updated.emit(self.simulator.get_wafer())

    def apply_layer(self):
        thickness = float(self.thickness_input.text())
        material_id = self.material_input.text()
        self.simulator.apply_layer(thickness, material_id)
        self.geometry_updated.emit(self.simulator.get_wafer())
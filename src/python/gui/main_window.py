from PySide6.QtWidgets import QMainWindow, QVBoxLayout, QWidget
from PySide6.QtCore import Qt
from .geometry_panel import GeometryPanel
from .oxidation_panel import OxidationPanel
from .doping_panel import DopingPanel
from .photolithography_panel import PhotolithographyPanel
from .deposition_panel import DepositionPanel
from .etching_panel import EtchingPanel
from .metallization_panel import MetallizationPanel
from .packaging_panel import PackagingPanel
from .thermal_panel import ThermalPanel
from .reliability_panel import ReliabilityPanel
from .visualization_panel import VisualizationPanel

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Semiconductor Simulator")
        self.resize(800, 600)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)

        self.geometry_panel = GeometryPanel()
        self.oxidation_panel = OxidationPanel()
        self.doping_panel = DopingPanel()
        self.photolithography_panel = PhotolithographyPanel()
        self.deposition_panel = DepositionPanel()
        self.etching_panel = EtchingPanel()
        self.metallization_panel = MetallizationPanel()
        self.packaging_panel = PackagingPanel()
        self.thermal_panel = ThermalPanel()
        self.reliability_panel = ReliabilityPanel()
        self.visualization_panel = VisualizationPanel(400, 400)
        layout.addWidget(self.geometry_panel)
        layout.addWidget(self.oxidation_panel)
        layout.addWidget(self.doping_panel)
        layout.addWidget(self.photolithography_panel)
        layout.addWidget(self.deposition_panel)
        layout.addWidget(self.etching_panel)
        layout.addWidget(self.metallization_panel)
        layout.addWidget(self.packaging_panel)
        layout.addWidget(self.thermal_panel)
        layout.addWidget(self.reliability_panel)
        layout.addWidget(self.visualization_panel)

        self.geometry_panel.geometry_updated.connect(self.visualization_panel.update_visualization)
        self.oxidation_panel.oxidation_updated.connect(self.visualization_panel.update_visualization)
        self.doping_panel.doping_updated.connect(self.visualization_panel.update_visualization)
        self.photolithography_panel.lithography_updated.connect(self.visualization_panel.update_visualization)
        self.deposition_panel.deposition_updated.connect(self.visualization_panel.update_visualization)
        self.etching_panel.etching_updated.connect(self.visualization_panel.update_visualization)
        self.metallization_panel.metallization_updated.connect(self.visualization_panel.update_visualization)
        self.packaging_panel.packaging_updated.connect(self.visualization_panel.update_visualization)
        self.thermal_panel.thermal_updated.connect(self.visualization_panel.update_visualization)
        self.thermal_panel.visualization_option_changed.connect(self.visualization_panel.set_temperature_overlay)
        self.reliability_panel.reliability_updated.connect(self.visualization_panel.update_visualization)
        self.reliability_panel.visualization_option_changed.connect(self.visualization_panel.set_reliability_view)
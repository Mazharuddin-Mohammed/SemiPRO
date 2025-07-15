# Author: Dr. Mazharuddin Mohammed
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
from .multi_die_panel import MultiDiePanel
from .drc_panel import DRCPanel
from .advanced_visualization_panel import AdvancedVisualizationPanel
from .defect_inspection_panel import DefectInspectionPanel
from .interconnect_panel import InterconnectPanel
from .metrology_panel import MetrologyPanel

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Semiconductor Simulator")
        self.resize(800, 600)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)

        # Create tab widget for better organization
        from PySide6.QtWidgets import QTabWidget
        tab_widget = QTabWidget()

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
        self.multi_die_panel = MultiDiePanel()
        self.drc_panel = DRCPanel()
        self.advanced_viz_panel = AdvancedVisualizationPanel()
        self.defect_inspection_panel = DefectInspectionPanel()
        self.interconnect_panel = InterconnectPanel()
        self.metrology_panel = MetrologyPanel()

        # Add tabs
        tab_widget.addTab(self.geometry_panel, "Geometry")
        tab_widget.addTab(self.oxidation_panel, "Oxidation")
        tab_widget.addTab(self.doping_panel, "Doping")
        tab_widget.addTab(self.photolithography_panel, "Lithography")
        tab_widget.addTab(self.deposition_panel, "Deposition")
        tab_widget.addTab(self.etching_panel, "Etching")
        tab_widget.addTab(self.metallization_panel, "Metallization")
        tab_widget.addTab(self.packaging_panel, "Packaging")
        tab_widget.addTab(self.thermal_panel, "Thermal")
        tab_widget.addTab(self.reliability_panel, "Reliability")
        tab_widget.addTab(self.multi_die_panel, "Multi-Die")
        tab_widget.addTab(self.drc_panel, "DRC")
        tab_widget.addTab(self.defect_inspection_panel, "Defect Inspection")
        tab_widget.addTab(self.interconnect_panel, "Interconnect")
        tab_widget.addTab(self.metrology_panel, "Metrology")
        tab_widget.addTab(self.advanced_viz_panel, "Advanced Viz")
        tab_widget.addTab(self.visualization_panel, "Visualization")

        layout.addWidget(tab_widget)

        # Connect signals for visualization updates
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

        # Connect new panel signals
        self.multi_die_panel.multi_die_updated.connect(self.visualization_panel.update_visualization)
        self.multi_die_panel.multi_die_updated.connect(self.advanced_viz_panel.visualization_updated)
        self.drc_panel.drc_updated.connect(self.visualization_panel.update_visualization)
        self.advanced_viz_panel.visualization_updated.connect(self.visualization_panel.update_visualization)
# Author: Dr. Mazharuddin Mohammed
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, 
                               QPushButton, QLabel, QComboBox, QSpinBox, 
                               QDoubleSpinBox, QTableWidget, QTableWidgetItem,
                               QGroupBox, QFormLayout, QTextEdit, QTabWidget)
from PySide6.QtCore import Signal, Qt
from ..simulator import Simulator
from ..multi_die import DIE_TYPE_LOGIC, DIE_TYPE_MEMORY, DIE_TYPE_ANALOG, DIE_TYPE_RF, DIE_TYPE_SENSOR, DIE_TYPE_POWER
from ..multi_die import INTEGRATION_WIRE_BONDING, INTEGRATION_FLIP_CHIP, INTEGRATION_TSV, INTEGRATION_WLP, INTEGRATION_CHIPLET

class MultiDiePanel(QWidget):
    multi_die_updated = Signal(object)
    
    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        self.setup_ui()
        
    def setup_ui(self):
        layout = QVBoxLayout(self)
        
        # Create tab widget
        tab_widget = QTabWidget()
        
        # Die Management Tab
        die_tab = self.create_die_management_tab()
        tab_widget.addTab(die_tab, "Die Management")
        
        # Integration Tab
        integration_tab = self.create_integration_tab()
        tab_widget.addTab(integration_tab, "Integration")
        
        # Analysis Tab
        analysis_tab = self.create_analysis_tab()
        tab_widget.addTab(analysis_tab, "Analysis")
        
        layout.addWidget(tab_widget)
        
    def create_die_management_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Die Creation Group
        die_group = QGroupBox("Create Die")
        die_layout = QFormLayout(die_group)
        
        self.die_id_input = QLineEdit("die_1")
        self.die_type_combo = QComboBox()
        self.die_type_combo.addItems(["Logic", "Memory", "Analog", "RF", "Sensor", "Power"])
        self.die_width_input = QDoubleSpinBox()
        self.die_width_input.setRange(0.1, 100.0)
        self.die_width_input.setValue(10.0)
        self.die_width_input.setSuffix(" mm")
        self.die_height_input = QDoubleSpinBox()
        self.die_height_input.setRange(0.1, 100.0)
        self.die_height_input.setValue(10.0)
        self.die_height_input.setSuffix(" mm")
        self.die_thickness_input = QDoubleSpinBox()
        self.die_thickness_input.setRange(0.01, 10.0)
        self.die_thickness_input.setValue(0.775)
        self.die_thickness_input.setSuffix(" mm")
        
        die_layout.addRow("Die ID:", self.die_id_input)
        die_layout.addRow("Type:", self.die_type_combo)
        die_layout.addRow("Width:", self.die_width_input)
        die_layout.addRow("Height:", self.die_height_input)
        die_layout.addRow("Thickness:", self.die_thickness_input)
        
        add_die_button = QPushButton("Add Die")
        add_die_button.clicked.connect(self.add_die)
        die_layout.addRow(add_die_button)
        
        layout.addWidget(die_group)
        
        # Die Positioning Group
        position_group = QGroupBox("Position Die")
        position_layout = QFormLayout(position_group)
        
        self.position_die_id = QLineEdit("die_1")
        self.position_x = QDoubleSpinBox()
        self.position_x.setRange(-100.0, 100.0)
        self.position_x.setSuffix(" mm")
        self.position_y = QDoubleSpinBox()
        self.position_y.setRange(-100.0, 100.0)
        self.position_y.setSuffix(" mm")
        
        position_layout.addRow("Die ID:", self.position_die_id)
        position_layout.addRow("X Position:", self.position_x)
        position_layout.addRow("Y Position:", self.position_y)
        
        position_button = QPushButton("Position Die")
        position_button.clicked.connect(self.position_die)
        position_layout.addRow(position_button)
        
        layout.addWidget(position_group)
        
        # Die List
        self.die_table = QTableWidget()
        self.die_table.setColumnCount(6)
        self.die_table.setHorizontalHeaderLabels(["ID", "Type", "Width", "Height", "Thickness", "Position"])
        layout.addWidget(QLabel("Dies:"))
        layout.addWidget(self.die_table)
        
        return widget
        
    def create_integration_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Wire Bonding Group
        wire_group = QGroupBox("Wire Bonding")
        wire_layout = QFormLayout(wire_group)
        
        self.wire_die1 = QLineEdit("die_1")
        self.wire_die2 = QLineEdit("die_2")
        self.wire_bonds_count = QSpinBox()
        self.wire_bonds_count.setRange(1, 1000)
        self.wire_bonds_count.setValue(100)
        
        wire_layout.addRow("Die 1:", self.wire_die1)
        wire_layout.addRow("Die 2:", self.wire_die2)
        wire_layout.addRow("Bond Count:", self.wire_bonds_count)
        
        wire_button = QPushButton("Perform Wire Bonding")
        wire_button.clicked.connect(self.perform_wire_bonding)
        wire_layout.addRow(wire_button)
        
        layout.addWidget(wire_group)
        
        # Flip Chip Group
        flip_group = QGroupBox("Flip Chip Bonding")
        flip_layout = QFormLayout(flip_group)
        
        self.flip_die1 = QLineEdit("die_1")
        self.flip_die2 = QLineEdit("die_2")
        self.bump_pitch = QDoubleSpinBox()
        self.bump_pitch.setRange(0.01, 1.0)
        self.bump_pitch.setValue(0.1)
        self.bump_pitch.setSuffix(" mm")
        self.bump_diameter = QDoubleSpinBox()
        self.bump_diameter.setRange(0.001, 0.1)
        self.bump_diameter.setValue(0.05)
        self.bump_diameter.setSuffix(" mm")
        
        flip_layout.addRow("Die 1:", self.flip_die1)
        flip_layout.addRow("Die 2:", self.flip_die2)
        flip_layout.addRow("Bump Pitch:", self.bump_pitch)
        flip_layout.addRow("Bump Diameter:", self.bump_diameter)
        
        flip_button = QPushButton("Perform Flip Chip Bonding")
        flip_button.clicked.connect(self.perform_flip_chip_bonding)
        flip_layout.addRow(flip_button)
        
        layout.addWidget(flip_group)
        
        # TSV Group
        tsv_group = QGroupBox("TSV Integration")
        tsv_layout = QFormLayout(tsv_group)
        
        self.tsv_die_id = QLineEdit("die_1")
        self.tsv_count = QSpinBox()
        self.tsv_count.setRange(1, 10000)
        self.tsv_count.setValue(100)
        self.tsv_diameter = QDoubleSpinBox()
        self.tsv_diameter.setRange(0.001, 0.1)
        self.tsv_diameter.setValue(0.01)
        self.tsv_diameter.setSuffix(" mm")
        self.tsv_depth = QDoubleSpinBox()
        self.tsv_depth.setRange(0.01, 1.0)
        self.tsv_depth.setValue(0.1)
        self.tsv_depth.setSuffix(" mm")
        
        tsv_layout.addRow("Die ID:", self.tsv_die_id)
        tsv_layout.addRow("TSV Count:", self.tsv_count)
        tsv_layout.addRow("TSV Diameter:", self.tsv_diameter)
        tsv_layout.addRow("TSV Depth:", self.tsv_depth)
        
        tsv_button = QPushButton("Perform TSV Integration")
        tsv_button.clicked.connect(self.perform_tsv_integration)
        tsv_layout.addRow(tsv_button)
        
        layout.addWidget(tsv_group)
        
        return widget
        
    def create_analysis_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Analysis Buttons
        button_layout = QHBoxLayout()
        
        electrical_button = QPushButton("Analyze Electrical Performance")
        electrical_button.clicked.connect(self.analyze_electrical)
        button_layout.addWidget(electrical_button)
        
        thermal_button = QPushButton("Analyze Thermal Performance")
        thermal_button.clicked.connect(self.analyze_thermal)
        button_layout.addWidget(thermal_button)
        
        layout.addLayout(button_layout)
        
        # Results Display
        self.results_text = QTextEdit()
        self.results_text.setReadOnly(True)
        layout.addWidget(QLabel("Analysis Results:"))
        layout.addWidget(self.results_text)
        
        # System Metrics Table
        self.metrics_table = QTableWidget()
        self.metrics_table.setColumnCount(2)
        self.metrics_table.setHorizontalHeaderLabels(["Metric", "Value"])
        layout.addWidget(QLabel("System Metrics:"))
        layout.addWidget(self.metrics_table)
        
        return widget
        
    def add_die(self):
        die_id = self.die_id_input.text()
        die_type = self.die_type_combo.currentIndex()
        width = self.die_width_input.value()
        height = self.die_height_input.value()
        thickness = self.die_thickness_input.value()
        
        try:
            die = self.simulator.add_die(die_id, die_type, width, height, thickness)
            self.update_die_table()
            self.results_text.append(f"Added die: {die_id} ({self.die_type_combo.currentText()})")
            self.multi_die_updated.emit(self.simulator.get_wafer())
        except Exception as e:
            self.results_text.append(f"Error adding die: {str(e)}")
            
    def position_die(self):
        die_id = self.position_die_id.text()
        x = self.position_x.value()
        y = self.position_y.value()
        
        try:
            self.simulator.position_die(die_id, x, y)
            self.update_die_table()
            self.results_text.append(f"Positioned die {die_id} at ({x}, {y})")
            self.multi_die_updated.emit(self.simulator.get_wafer())
        except Exception as e:
            self.results_text.append(f"Error positioning die: {str(e)}")
            
    def perform_wire_bonding(self):
        die1 = self.wire_die1.text()
        die2 = self.wire_die2.text()
        bond_count = self.wire_bonds_count.value()
        
        # Generate simple bond pattern
        bonds = []
        for i in range(bond_count):
            x1, y1 = i % 10, i // 10
            x2, y2 = x1 + 1, y1 + 1
            bonds.append(((x1, y1), (x2, y2)))
        
        try:
            self.simulator.perform_wire_bonding(die1, die2, bonds)
            self.results_text.append(f"Wire bonding completed between {die1} and {die2} ({bond_count} bonds)")
            self.multi_die_updated.emit(self.simulator.get_wafer())
        except Exception as e:
            self.results_text.append(f"Error in wire bonding: {str(e)}")
            
    def perform_flip_chip_bonding(self):
        die1 = self.flip_die1.text()
        die2 = self.flip_die2.text()
        pitch = self.bump_pitch.value()
        diameter = self.bump_diameter.value()
        
        try:
            self.simulator.perform_flip_chip_bonding(die1, die2, pitch, diameter)
            self.results_text.append(f"Flip chip bonding completed between {die1} and {die2}")
            self.multi_die_updated.emit(self.simulator.get_wafer())
        except Exception as e:
            self.results_text.append(f"Error in flip chip bonding: {str(e)}")
            
    def perform_tsv_integration(self):
        die_id = self.tsv_die_id.text()
        count = self.tsv_count.value()
        diameter = self.tsv_diameter.value()
        depth = self.tsv_depth.value()
        
        # Generate TSV positions
        positions = []
        for i in range(count):
            x = (i % 10) * 0.1
            y = (i // 10) * 0.1
            positions.append((x, y))
        
        try:
            self.simulator.perform_tsv_integration(die_id, positions, diameter, depth)
            self.results_text.append(f"TSV integration completed for {die_id} ({count} TSVs)")
            self.multi_die_updated.emit(self.simulator.get_wafer())
        except Exception as e:
            self.results_text.append(f"Error in TSV integration: {str(e)}")
            
    def analyze_electrical(self):
        try:
            self.simulator.analyze_electrical_performance()
            metrics = self.simulator.get_system_metrics()
            self.update_metrics_table(metrics)
            self.results_text.append("Electrical performance analysis completed")
        except Exception as e:
            self.results_text.append(f"Error in electrical analysis: {str(e)}")
            
    def analyze_thermal(self):
        try:
            self.simulator.analyze_thermal_performance()
            metrics = self.simulator.get_system_metrics()
            self.update_metrics_table(metrics)
            self.results_text.append("Thermal performance analysis completed")
        except Exception as e:
            self.results_text.append(f"Error in thermal analysis: {str(e)}")
            
    def update_die_table(self):
        # This would be implemented to show current dies
        # For now, just a placeholder
        pass
        
    def update_metrics_table(self, metrics):
        self.metrics_table.setRowCount(len(metrics))
        for i, (key, value) in enumerate(metrics.items()):
            self.metrics_table.setItem(i, 0, QTableWidgetItem(key))
            self.metrics_table.setItem(i, 1, QTableWidgetItem(f"{value:.6f}"))

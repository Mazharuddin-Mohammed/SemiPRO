# Author: Dr. Mazharuddin Mohammed
"""
Interconnect Panel
==================

GUI panel for damascene interconnect simulation.
"""

import os
import sys
import numpy as np
from typing import Dict, List, Optional, Tuple, Any

try:
    from PySide6.QtWidgets import (
        QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
        QComboBox, QDoubleSpinBox, QSpinBox, QCheckBox, QGroupBox,
        QTabWidget, QFileDialog, QTableWidget, QTableWidgetItem,
        QHeaderView, QSplitter, QMessageBox, QProgressBar
    )
    from PySide6.QtCore import Qt, Signal, Slot
    from PySide6.QtGui import QColor, QPixmap, QImage
except ImportError:
    print("PySide6 not found. GUI functionality will not be available.")
    
try:
    from matplotlib.figure import Figure
    from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
    from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
    import matplotlib.pyplot as plt
    from matplotlib.colors import LinearSegmentedColormap
except ImportError:
    print("Matplotlib not found. Visualization will not be available.")

# Import SemiPRO modules
try:
    from ..cpp_bridge import create_bridge
    interconnect_bridge = create_bridge('interconnect')
    PyDamasceneModel = interconnect_bridge.PyDamasceneModel
    PyDamasceneParameters = interconnect_bridge.PyDamasceneParameters
    PyDamasceneResult = interconnect_bridge.PyDamasceneResult
    
    # Constants
    DAMASCENE_SINGLE = interconnect_bridge.DAMASCENE_SINGLE
    DAMASCENE_DUAL = interconnect_bridge.DAMASCENE_DUAL
    DAMASCENE_TRIPLE = interconnect_bridge.DAMASCENE_TRIPLE
    
    BARRIER_TANTALUM = interconnect_bridge.BARRIER_TANTALUM
    BARRIER_TITANIUM = interconnect_bridge.BARRIER_TITANIUM
    BARRIER_TITANIUM_NITRIDE = interconnect_bridge.BARRIER_TITANIUM_NITRIDE
    BARRIER_TANTALUM_NITRIDE = interconnect_bridge.BARRIER_TANTALUM_NITRIDE
    BARRIER_RUTHENIUM = interconnect_bridge.BARRIER_RUTHENIUM
    BARRIER_COBALT = interconnect_bridge.BARRIER_COBALT
    
    METAL_COPPER = interconnect_bridge.METAL_COPPER
    METAL_ALUMINUM = interconnect_bridge.METAL_ALUMINUM
    METAL_TUNGSTEN = interconnect_bridge.METAL_TUNGSTEN
    METAL_COBALT = interconnect_bridge.METAL_COBALT
    METAL_RUTHENIUM = interconnect_bridge.METAL_RUTHENIUM
    
except ImportError as e:
    print(f"Failed to import interconnect module: {e}")
    PyDamasceneModel = None
    PyDamasceneParameters = None
    PyDamasceneResult = None

class InterconnectPanel(QWidget):
    """Panel for damascene interconnect simulation"""
    
    # Signal to notify when simulation is complete
    simulation_complete = Signal(object)
    
    def __init__(self, parent=None):
        """Initialize the interconnect panel"""
        super().__init__(parent)
        
        # Check if module is available
        if PyDamasceneModel is None:
            self.setEnabled(False)
            layout = QVBoxLayout()
            layout.addWidget(QLabel("Interconnect module not available"))
            self.setLayout(layout)
            return
        
        # Create damascene model
        self.model = PyDamasceneModel()
        
        # Initialize UI
        self.init_ui()
        
        # Connect signals
        self.connect_signals()
        
    def init_ui(self):
        """Initialize the user interface"""
        main_layout = QVBoxLayout()
        
        # Create tabs
        tabs = QTabWidget()
        tabs.addTab(self.create_process_tab(), "Process")
        tabs.addTab(self.create_results_tab(), "Results")
        tabs.addTab(self.create_visualization_tab(), "Visualization")
        
        main_layout.addWidget(tabs)
        self.setLayout(main_layout)
        
    def create_process_tab(self):
        """Create the process configuration tab"""
        tab = QWidget()
        layout = QVBoxLayout()
        
        # Damascene type selection
        type_group = QGroupBox("Damascene Type")
        type_layout = QVBoxLayout()
        
        self.type_combo = QComboBox()
        self.type_combo.addItem("Single Damascene", DAMASCENE_SINGLE)
        self.type_combo.addItem("Dual Damascene", DAMASCENE_DUAL)
        self.type_combo.addItem("Triple Damascene", DAMASCENE_TRIPLE)
        
        type_layout.addWidget(self.type_combo)
        type_group.setLayout(type_layout)
        layout.addWidget(type_group)
        
        # Geometry parameters
        geometry_group = QGroupBox("Geometry Parameters")
        geometry_layout = QVBoxLayout()
        
        # Line width
        width_layout = QHBoxLayout()
        width_layout.addWidget(QLabel("Line Width (nm):"))
        self.line_width_spin = QDoubleSpinBox()
        self.line_width_spin.setRange(10.0, 1000.0)
        self.line_width_spin.setValue(100.0)
        self.line_width_spin.setDecimals(1)
        width_layout.addWidget(self.line_width_spin)
        geometry_layout.addLayout(width_layout)
        
        # Line spacing
        spacing_layout = QHBoxLayout()
        spacing_layout.addWidget(QLabel("Line Spacing (nm):"))
        self.line_spacing_spin = QDoubleSpinBox()
        self.line_spacing_spin.setRange(10.0, 1000.0)
        self.line_spacing_spin.setValue(100.0)
        self.line_spacing_spin.setDecimals(1)
        spacing_layout.addWidget(self.line_spacing_spin)
        geometry_layout.addLayout(spacing_layout)
        
        # Metal thickness
        metal_thickness_layout = QHBoxLayout()
        metal_thickness_layout.addWidget(QLabel("Metal Thickness (nm):"))
        self.metal_thickness_spin = QDoubleSpinBox()
        self.metal_thickness_spin.setRange(10.0, 1000.0)
        self.metal_thickness_spin.setValue(200.0)
        self.metal_thickness_spin.setDecimals(1)
        metal_thickness_layout.addWidget(self.metal_thickness_spin)
        geometry_layout.addLayout(metal_thickness_layout)
        
        # Barrier thickness
        barrier_thickness_layout = QHBoxLayout()
        barrier_thickness_layout.addWidget(QLabel("Barrier Thickness (nm):"))
        self.barrier_thickness_spin = QDoubleSpinBox()
        self.barrier_thickness_spin.setRange(1.0, 50.0)
        self.barrier_thickness_spin.setValue(5.0)
        self.barrier_thickness_spin.setDecimals(1)
        barrier_thickness_layout.addWidget(self.barrier_thickness_spin)
        geometry_layout.addLayout(barrier_thickness_layout)
        
        # Dielectric thickness
        dielectric_thickness_layout = QHBoxLayout()
        dielectric_thickness_layout.addWidget(QLabel("Dielectric Thickness (nm):"))
        self.dielectric_thickness_spin = QDoubleSpinBox()
        self.dielectric_thickness_spin.setRange(50.0, 1000.0)
        self.dielectric_thickness_spin.setValue(300.0)
        self.dielectric_thickness_spin.setDecimals(1)
        dielectric_thickness_layout.addWidget(self.dielectric_thickness_spin)
        geometry_layout.addLayout(dielectric_thickness_layout)
        
        geometry_group.setLayout(geometry_layout)
        layout.addWidget(geometry_group)
        
        # Material selection
        materials_group = QGroupBox("Materials")
        materials_layout = QVBoxLayout()
        
        # Barrier material
        barrier_layout = QHBoxLayout()
        barrier_layout.addWidget(QLabel("Barrier Material:"))
        self.barrier_material_combo = QComboBox()
        self.barrier_material_combo.addItem("Tantalum", BARRIER_TANTALUM)
        self.barrier_material_combo.addItem("Titanium", BARRIER_TITANIUM)
        self.barrier_material_combo.addItem("Titanium Nitride", BARRIER_TITANIUM_NITRIDE)
        self.barrier_material_combo.addItem("Tantalum Nitride", BARRIER_TANTALUM_NITRIDE)
        self.barrier_material_combo.addItem("Ruthenium", BARRIER_RUTHENIUM)
        self.barrier_material_combo.addItem("Cobalt", BARRIER_COBALT)
        barrier_layout.addWidget(self.barrier_material_combo)
        materials_layout.addLayout(barrier_layout)
        
        # Metal material
        metal_layout = QHBoxLayout()
        metal_layout.addWidget(QLabel("Metal Material:"))
        self.metal_material_combo = QComboBox()
        self.metal_material_combo.addItem("Copper", METAL_COPPER)
        self.metal_material_combo.addItem("Aluminum", METAL_ALUMINUM)
        self.metal_material_combo.addItem("Tungsten", METAL_TUNGSTEN)
        self.metal_material_combo.addItem("Cobalt", METAL_COBALT)
        self.metal_material_combo.addItem("Ruthenium", METAL_RUTHENIUM)
        metal_layout.addWidget(self.metal_material_combo)
        materials_layout.addLayout(metal_layout)
        
        materials_group.setLayout(materials_layout)
        layout.addWidget(materials_group)
        
        # CMP parameters
        cmp_group = QGroupBox("CMP Parameters")
        cmp_layout = QVBoxLayout()
        
        self.cmp_enabled_check = QCheckBox("Enable CMP")
        self.cmp_enabled_check.setChecked(True)
        cmp_layout.addWidget(self.cmp_enabled_check)
        
        # CMP pressure
        pressure_layout = QHBoxLayout()
        pressure_layout.addWidget(QLabel("CMP Pressure (psi):"))
        self.cmp_pressure_spin = QDoubleSpinBox()
        self.cmp_pressure_spin.setRange(1.0, 20.0)
        self.cmp_pressure_spin.setValue(5.0)
        self.cmp_pressure_spin.setDecimals(1)
        pressure_layout.addWidget(self.cmp_pressure_spin)
        cmp_layout.addLayout(pressure_layout)
        
        # CMP time
        time_layout = QHBoxLayout()
        time_layout.addWidget(QLabel("CMP Time (s):"))
        self.cmp_time_spin = QDoubleSpinBox()
        self.cmp_time_spin.setRange(10.0, 300.0)
        self.cmp_time_spin.setValue(60.0)
        self.cmp_time_spin.setDecimals(1)
        time_layout.addWidget(self.cmp_time_spin)
        cmp_layout.addLayout(time_layout)
        
        cmp_group.setLayout(cmp_layout)
        layout.addWidget(cmp_group)
        
        # Run button
        self.run_button = QPushButton("Run Damascene Simulation")
        layout.addWidget(self.run_button)
        
        # Progress bar
        self.progress_bar = QProgressBar()
        self.progress_bar.setVisible(False)
        layout.addWidget(self.progress_bar)
        
        tab.setLayout(layout)
        return tab
        
    def create_results_tab(self):
        """Create the results tab"""
        tab = QWidget()
        layout = QVBoxLayout()
        
        # Results table
        self.results_table = QTableWidget(0, 2)
        self.results_table.setHorizontalHeaderLabels(["Parameter", "Value"])
        self.results_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        layout.addWidget(self.results_table)
        
        # Export button
        self.export_button = QPushButton("Export Results")
        self.export_button.setEnabled(False)
        layout.addWidget(self.export_button)
        
        tab.setLayout(layout)
        return tab
        
    def create_visualization_tab(self):
        """Create the visualization tab"""
        tab = QWidget()
        layout = QVBoxLayout()
        
        # Matplotlib figure
        self.figure = Figure(figsize=(10, 8), dpi=100)
        self.canvas = FigureCanvas(self.figure)
        self.toolbar = NavigationToolbar(self.canvas, self)
        
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        
        # Visualization options
        options_group = QGroupBox("Visualization Options")
        options_layout = QHBoxLayout()
        
        options_layout.addWidget(QLabel("View:"))
        self.view_combo = QComboBox()
        self.view_combo.addItems(["Cross Section", "Top View", "3D View"])
        options_layout.addWidget(self.view_combo)
        
        self.show_dimensions_check = QCheckBox("Show Dimensions")
        self.show_dimensions_check.setChecked(True)
        options_layout.addWidget(self.show_dimensions_check)
        
        options_group.setLayout(options_layout)
        layout.addWidget(options_group)
        
        tab.setLayout(layout)
        return tab
        
    def connect_signals(self):
        """Connect UI signals to slots"""
        self.run_button.clicked.connect(self.run_simulation)
        self.export_button.clicked.connect(self.export_results)
        self.type_combo.currentIndexChanged.connect(self.update_type_parameters)
        self.view_combo.currentIndexChanged.connect(self.update_visualization)
        self.show_dimensions_check.stateChanged.connect(self.update_visualization)
        
    def run_simulation(self):
        """Run the damascene simulation"""
        # Show progress
        self.progress_bar.setVisible(True)
        self.progress_bar.setRange(0, 0)  # Indeterminate progress
        self.run_button.setEnabled(False)
        
        try:
            # Create parameters
            params = PyDamasceneParameters(
                self.type_combo.currentData(),
                self.line_width_spin.value(),
                self.line_spacing_spin.value(),
                self.metal_thickness_spin.value(),
                self.barrier_thickness_spin.value(),
                self.dielectric_thickness_spin.value(),
                self.barrier_material_combo.currentData(),
                self.metal_material_combo.currentData(),
                self.cmp_enabled_check.isChecked(),
                self.cmp_pressure_spin.value(),
                self.cmp_time_spin.value()
            )
            
            # TODO: Get wafer from main application
            # For now, we'll use a dummy wafer
            from ..cpp_bridge import create_bridge
            geometry_bridge = create_bridge('geometry')
            wafer = geometry_bridge.PyWafer(100.0, 50, 50)
            
            # Run simulation
            result = self.model.simulate_damascene(wafer, params)
            self.display_results(result)
            self.simulation_complete.emit(result)
            
        except Exception as e:
            QMessageBox.critical(self, "Simulation Error", f"Error during simulation: {str(e)}")
        finally:
            # Hide progress
            self.progress_bar.setVisible(False)
            self.run_button.setEnabled(True)
            
    def display_results(self, result):
        """Display simulation results"""
        # Update table
        self.results_table.setRowCount(0)
        
        results_data = [
            ("Final Thickness", f"{result.final_thickness:.3f} nm"),
            ("Resistance", f"{result.resistance:.6f} Ω"),
            ("Capacitance", f"{result.capacitance:.3e} F"),
            ("RC Delay", f"{result.rc_delay:.3e} s"),
            ("Barrier Coverage", f"{result.barrier_coverage:.1f} %"),
            ("Void Probability", f"{result.void_probability:.3f}"),
            ("Electromigration Lifetime", f"{result.electromigration_lifetime:.2e} hours"),
            ("Quality Summary", result.quality_summary)
        ]
        
        for i, (param, value) in enumerate(results_data):
            self.results_table.insertRow(i)
            self.results_table.setItem(i, 0, QTableWidgetItem(param))
            self.results_table.setItem(i, 1, QTableWidgetItem(value))
            
        # Enable export
        self.export_button.setEnabled(True)
        
        # Update visualization
        self.update_visualization()
        
    def update_visualization(self):
        """Update the visualization based on current options"""
        # Check if we have results
        if not hasattr(self, 'results_table') or self.results_table.rowCount() == 0:
            return
            
        # Clear figure
        self.figure.clear()
        
        view = self.view_combo.currentText()
        
        if view == "Cross Section":
            self.plot_cross_section()
        elif view == "Top View":
            self.plot_top_view()
        elif view == "3D View":
            self.plot_3d_view()
            
        # Refresh canvas
        self.canvas.draw()
        
    def plot_cross_section(self):
        """Plot cross-sectional view of the damascene structure"""
        ax = self.figure.add_subplot(111)
        
        # Get parameters
        line_width = self.line_width_spin.value()
        metal_thickness = self.metal_thickness_spin.value()
        barrier_thickness = self.barrier_thickness_spin.value()
        dielectric_thickness = self.dielectric_thickness_spin.value()
        
        # Draw dielectric
        dielectric_rect = plt.Rectangle((0, 0), line_width*2, dielectric_thickness, 
                                       facecolor='lightblue', edgecolor='black', 
                                       label='Dielectric')
        ax.add_patch(dielectric_rect)
        
        # Draw trench
        trench_x = line_width/2
        trench_width = line_width
        trench_rect = plt.Rectangle((trench_x, 0), trench_width, metal_thickness + barrier_thickness, 
                                   facecolor='white', edgecolor='black')
        ax.add_patch(trench_rect)
        
        # Draw barrier
        barrier_rect = plt.Rectangle((trench_x, 0), trench_width, barrier_thickness, 
                                    facecolor='gray', edgecolor='black', 
                                    label='Barrier')
        ax.add_patch(barrier_rect)
        
        # Draw metal
        metal_rect = plt.Rectangle((trench_x, barrier_thickness), trench_width, metal_thickness, 
                                  facecolor='orange', edgecolor='black', 
                                  label='Metal')
        ax.add_patch(metal_rect)
        
        # Add dimensions if enabled
        if self.show_dimensions_check.isChecked():
            # Line width dimension
            ax.annotate('', xy=(trench_x, -20), xytext=(trench_x + trench_width, -20),
                       arrowprops=dict(arrowstyle='<->', color='red'))
            ax.text(trench_x + trench_width/2, -30, f'{line_width:.0f} nm', 
                   ha='center', va='top', color='red')
            
            # Metal thickness dimension
            ax.annotate('', xy=(line_width*2 + 20, barrier_thickness), 
                       xytext=(line_width*2 + 20, barrier_thickness + metal_thickness),
                       arrowprops=dict(arrowstyle='<->', color='red'))
            ax.text(line_width*2 + 30, barrier_thickness + metal_thickness/2, 
                   f'{metal_thickness:.0f} nm', ha='left', va='center', color='red', rotation=90)
        
        # Set labels and title
        ax.set_xlabel('Position (nm)')
        ax.set_ylabel('Height (nm)')
        ax.set_title('Damascene Cross Section')
        ax.legend()
        ax.set_aspect('equal')
        ax.grid(True, alpha=0.3)
        
    def plot_top_view(self):
        """Plot top view of the damascene structure"""
        ax = self.figure.add_subplot(111)
        
        # Get parameters
        line_width = self.line_width_spin.value()
        line_spacing = self.line_spacing_spin.value()
        
        # Draw multiple lines
        num_lines = 5
        total_width = num_lines * line_width + (num_lines - 1) * line_spacing
        
        for i in range(num_lines):
            x = i * (line_width + line_spacing)
            line_rect = plt.Rectangle((x, 0), line_width, 1000, 
                                     facecolor='orange', edgecolor='black', 
                                     alpha=0.7)
            ax.add_patch(line_rect)
            
        # Add dimensions if enabled
        if self.show_dimensions_check.isChecked():
            # Line width
            ax.annotate('', xy=(0, -50), xytext=(line_width, -50),
                       arrowprops=dict(arrowstyle='<->', color='red'))
            ax.text(line_width/2, -70, f'{line_width:.0f} nm', 
                   ha='center', va='top', color='red')
            
            # Line spacing
            if num_lines > 1:
                ax.annotate('', xy=(line_width, -100), 
                           xytext=(line_width + line_spacing, -100),
                           arrowprops=dict(arrowstyle='<->', color='blue'))
                ax.text(line_width + line_spacing/2, -120, f'{line_spacing:.0f} nm', 
                       ha='center', va='top', color='blue')
        
        # Set labels and title
        ax.set_xlabel('X Position (nm)')
        ax.set_ylabel('Y Position (nm)')
        ax.set_title('Damascene Top View')
        ax.set_xlim(-100, total_width + 100)
        ax.set_ylim(-150, 1100)
        ax.set_aspect('equal')
        ax.grid(True, alpha=0.3)
        
    def plot_3d_view(self):
        """Plot 3D view of the damascene structure"""
        ax = self.figure.add_subplot(111, projection='3d')
        
        # Get parameters
        line_width = self.line_width_spin.value()
        metal_thickness = self.metal_thickness_spin.value()
        barrier_thickness = self.barrier_thickness_spin.value()
        
        # Create 3D structure (simplified)
        x = np.array([0, line_width, line_width, 0, 0])
        y = np.array([0, 0, 1000, 1000, 0])
        
        # Bottom (barrier)
        z_bottom = np.zeros_like(x)
        z_barrier = np.full_like(x, barrier_thickness)
        
        # Top (metal)
        z_top = np.full_like(x, barrier_thickness + metal_thickness)
        
        # Plot surfaces
        for i in range(len(x)-1):
            # Barrier sides
            ax.plot([x[i], x[i+1]], [y[i], y[i+1]], [z_bottom[i], z_bottom[i+1]], 'gray')
            ax.plot([x[i], x[i+1]], [y[i], y[i+1]], [z_barrier[i], z_barrier[i+1]], 'gray')
            ax.plot([x[i], x[i]], [y[i], y[i]], [z_bottom[i], z_barrier[i]], 'gray')
            
            # Metal sides
            ax.plot([x[i], x[i+1]], [y[i], y[i+1]], [z_barrier[i], z_barrier[i+1]], 'orange')
            ax.plot([x[i], x[i+1]], [y[i], y[i+1]], [z_top[i], z_top[i+1]], 'orange')
            ax.plot([x[i], x[i]], [y[i], y[i]], [z_barrier[i], z_top[i]], 'orange')
        
        # Set labels and title
        ax.set_xlabel('X Position (nm)')
        ax.set_ylabel('Y Position (nm)')
        ax.set_zlabel('Height (nm)')
        ax.set_title('Damascene 3D View')
        
    def export_results(self):
        """Export simulation results to file"""
        filename, _ = QFileDialog.getSaveFileName(self, "Save Results", "", "CSV Files (*.csv);;Text Files (*.txt);;All Files (*)")
        
        if filename:
            try:
                with open(filename, 'w') as f:
                    f.write("Parameter,Value\n")
                    
                    for i in range(self.results_table.rowCount()):
                        param = self.results_table.item(i, 0).text()
                        value = self.results_table.item(i, 1).text()
                        f.write(f'"{param}","{value}"\n')
                
                QMessageBox.information(self, "Export Complete", f"Results saved to {filename}")
            except Exception as e:
                QMessageBox.critical(self, "Export Error", f"Error saving results: {str(e)}")
                
    def update_type_parameters(self):
        """Update parameter defaults based on damascene type"""
        damascene_type = self.type_combo.currentData()
        
        # Set default parameters based on type
        if damascene_type == DAMASCENE_SINGLE:
            self.metal_thickness_spin.setValue(200.0)
            self.barrier_thickness_spin.setValue(5.0)
        elif damascene_type == DAMASCENE_DUAL:
            self.metal_thickness_spin.setValue(300.0)
            self.barrier_thickness_spin.setValue(8.0)
        elif damascene_type == DAMASCENE_TRIPLE:
            self.metal_thickness_spin.setValue(400.0)
            self.barrier_thickness_spin.setValue(10.0)

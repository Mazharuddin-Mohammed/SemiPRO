# Author: Dr. Mazharuddin Mohammed
"""
Metrology Panel
===============

GUI panel for metrology measurements and analysis.
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
    metrology_bridge = create_bridge('metrology')
    PyMetrologyModel = metrology_bridge.PyMetrologyModel
    PyMetrologyParameters = metrology_bridge.PyMetrologyParameters
    PyMetrologyResult = metrology_bridge.PyMetrologyResult
    
    # Constants
    TECHNIQUE_OPTICAL_MICROSCOPY = metrology_bridge.TECHNIQUE_OPTICAL_MICROSCOPY
    TECHNIQUE_PROFILOMETRY = metrology_bridge.TECHNIQUE_PROFILOMETRY
    TECHNIQUE_ELLIPSOMETRY = metrology_bridge.TECHNIQUE_ELLIPSOMETRY
    TECHNIQUE_XRD = metrology_bridge.TECHNIQUE_XRD
    TECHNIQUE_XRF = metrology_bridge.TECHNIQUE_XRF
    TECHNIQUE_SEM = metrology_bridge.TECHNIQUE_SEM
    TECHNIQUE_TEM = metrology_bridge.TECHNIQUE_TEM
    TECHNIQUE_AFM = metrology_bridge.TECHNIQUE_AFM
    TECHNIQUE_SIMS = metrology_bridge.TECHNIQUE_SIMS
    TECHNIQUE_FTIR = metrology_bridge.TECHNIQUE_FTIR
    
    MEASUREMENT_THICKNESS = metrology_bridge.MEASUREMENT_THICKNESS
    MEASUREMENT_COMPOSITION = metrology_bridge.MEASUREMENT_COMPOSITION
    MEASUREMENT_ROUGHNESS = metrology_bridge.MEASUREMENT_ROUGHNESS
    MEASUREMENT_STRESS = metrology_bridge.MEASUREMENT_STRESS
    MEASUREMENT_RESISTIVITY = metrology_bridge.MEASUREMENT_RESISTIVITY
    MEASUREMENT_DOPANT_PROFILE = metrology_bridge.MEASUREMENT_DOPANT_PROFILE
    MEASUREMENT_CRYSTAL_STRUCTURE = metrology_bridge.MEASUREMENT_CRYSTAL_STRUCTURE
    MEASUREMENT_OPTICAL_PROPERTIES = metrology_bridge.MEASUREMENT_OPTICAL_PROPERTIES
    
except ImportError as e:
    print(f"Failed to import metrology module: {e}")
    PyMetrologyModel = None
    PyMetrologyParameters = None
    PyMetrologyResult = None

class MetrologyPanel(QWidget):
    """Panel for metrology measurements and analysis"""
    
    # Signal to notify when measurement is complete
    measurement_complete = Signal(object)
    
    def __init__(self, parent=None):
        """Initialize the metrology panel"""
        super().__init__(parent)
        
        # Check if module is available
        if PyMetrologyModel is None:
            self.setEnabled(False)
            layout = QVBoxLayout()
            layout.addWidget(QLabel("Metrology module not available"))
            self.setLayout(layout)
            return
        
        # Create metrology model
        self.model = PyMetrologyModel()
        
        # Initialize UI
        self.init_ui()
        
        # Connect signals
        self.connect_signals()
        
    def init_ui(self):
        """Initialize the user interface"""
        main_layout = QVBoxLayout()
        
        # Create tabs
        tabs = QTabWidget()
        tabs.addTab(self.create_measurement_tab(), "Measurement")
        tabs.addTab(self.create_results_tab(), "Results")
        tabs.addTab(self.create_visualization_tab(), "Visualization")
        
        main_layout.addWidget(tabs)
        self.setLayout(main_layout)
        
    def create_measurement_tab(self):
        """Create the measurement configuration tab"""
        tab = QWidget()
        layout = QVBoxLayout()
        
        # Technique selection
        technique_group = QGroupBox("Measurement Technique")
        technique_layout = QVBoxLayout()
        
        self.technique_combo = QComboBox()
        self.technique_combo.addItem("Optical Microscopy", TECHNIQUE_OPTICAL_MICROSCOPY)
        self.technique_combo.addItem("Profilometry", TECHNIQUE_PROFILOMETRY)
        self.technique_combo.addItem("Ellipsometry", TECHNIQUE_ELLIPSOMETRY)
        self.technique_combo.addItem("X-Ray Diffraction", TECHNIQUE_XRD)
        self.technique_combo.addItem("X-Ray Fluorescence", TECHNIQUE_XRF)
        self.technique_combo.addItem("SEM", TECHNIQUE_SEM)
        self.technique_combo.addItem("TEM", TECHNIQUE_TEM)
        self.technique_combo.addItem("AFM", TECHNIQUE_AFM)
        self.technique_combo.addItem("SIMS", TECHNIQUE_SIMS)
        self.technique_combo.addItem("FTIR", TECHNIQUE_FTIR)
        
        technique_layout.addWidget(self.technique_combo)
        technique_group.setLayout(technique_layout)
        layout.addWidget(technique_group)
        
        # Measurement type selection
        type_group = QGroupBox("Measurement Type")
        type_layout = QVBoxLayout()
        
        self.measurement_type_combo = QComboBox()
        self.measurement_type_combo.addItem("Thickness", MEASUREMENT_THICKNESS)
        self.measurement_type_combo.addItem("Composition", MEASUREMENT_COMPOSITION)
        self.measurement_type_combo.addItem("Roughness", MEASUREMENT_ROUGHNESS)
        self.measurement_type_combo.addItem("Stress", MEASUREMENT_STRESS)
        self.measurement_type_combo.addItem("Resistivity", MEASUREMENT_RESISTIVITY)
        self.measurement_type_combo.addItem("Dopant Profile", MEASUREMENT_DOPANT_PROFILE)
        self.measurement_type_combo.addItem("Crystal Structure", MEASUREMENT_CRYSTAL_STRUCTURE)
        self.measurement_type_combo.addItem("Optical Properties", MEASUREMENT_OPTICAL_PROPERTIES)
        
        type_layout.addWidget(self.measurement_type_combo)
        type_group.setLayout(type_layout)
        layout.addWidget(type_group)
        
        # Measurement parameters
        params_group = QGroupBox("Measurement Parameters")
        params_layout = QVBoxLayout()
        
        # Resolution
        resolution_layout = QHBoxLayout()
        resolution_layout.addWidget(QLabel("Resolution:"))
        self.resolution_spin = QDoubleSpinBox()
        self.resolution_spin.setRange(0.0001, 1.0)
        self.resolution_spin.setValue(0.001)
        self.resolution_spin.setDecimals(4)
        resolution_layout.addWidget(self.resolution_spin)
        params_layout.addLayout(resolution_layout)
        
        # Accuracy
        accuracy_layout = QHBoxLayout()
        accuracy_layout.addWidget(QLabel("Accuracy:"))
        self.accuracy_spin = QDoubleSpinBox()
        self.accuracy_spin.setRange(0.5, 1.0)
        self.accuracy_spin.setValue(0.99)
        self.accuracy_spin.setDecimals(3)
        accuracy_layout.addWidget(self.accuracy_spin)
        params_layout.addLayout(accuracy_layout)
        
        # Precision
        precision_layout = QHBoxLayout()
        precision_layout.addWidget(QLabel("Precision:"))
        self.precision_spin = QDoubleSpinBox()
        self.precision_spin.setRange(0.0001, 1.0)
        self.precision_spin.setValue(0.001)
        self.precision_spin.setDecimals(4)
        precision_layout.addWidget(self.precision_spin)
        params_layout.addLayout(precision_layout)
        
        # Scan area
        area_layout = QHBoxLayout()
        area_layout.addWidget(QLabel("Scan Area (mm²):"))
        self.scan_area_spin = QDoubleSpinBox()
        self.scan_area_spin.setRange(0.01, 100.0)
        self.scan_area_spin.setValue(1.0)
        self.scan_area_spin.setDecimals(2)
        area_layout.addWidget(self.scan_area_spin)
        params_layout.addLayout(area_layout)
        
        # Scan time
        time_layout = QHBoxLayout()
        time_layout.addWidget(QLabel("Scan Time (s):"))
        self.scan_time_spin = QDoubleSpinBox()
        self.scan_time_spin.setRange(1.0, 3600.0)
        self.scan_time_spin.setValue(60.0)
        self.scan_time_spin.setDecimals(1)
        time_layout.addWidget(self.scan_time_spin)
        params_layout.addLayout(time_layout)
        
        # Options
        options_layout = QHBoxLayout()
        self.destructive_check = QCheckBox("Destructive Measurement")
        self.destructive_check.setChecked(False)
        options_layout.addWidget(self.destructive_check)
        
        self.noise_simulation_check = QCheckBox("Enable Noise Simulation")
        self.noise_simulation_check.setChecked(True)
        options_layout.addWidget(self.noise_simulation_check)
        params_layout.addLayout(options_layout)
        
        params_group.setLayout(params_layout)
        layout.addWidget(params_group)
        
        # Run button
        self.run_button = QPushButton("Run Measurement")
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
        
        # Uncertainty information
        uncertainty_group = QGroupBox("Measurement Uncertainty")
        uncertainty_layout = QVBoxLayout()
        
        uncertainty_info_layout = QHBoxLayout()
        uncertainty_info_layout.addWidget(QLabel("Uncertainty:"))
        self.uncertainty_label = QLabel("N/A")
        uncertainty_info_layout.addWidget(self.uncertainty_label)
        
        uncertainty_info_layout.addWidget(QLabel("Signal-to-Noise:"))
        self.snr_label = QLabel("N/A")
        uncertainty_info_layout.addWidget(self.snr_label)
        
        uncertainty_layout.addLayout(uncertainty_info_layout)
        
        range_layout = QHBoxLayout()
        range_layout.addWidget(QLabel("Range:"))
        self.range_label = QLabel("N/A")
        range_layout.addWidget(self.range_label)
        uncertainty_layout.addLayout(range_layout)
        
        uncertainty_group.setLayout(uncertainty_layout)
        layout.addWidget(uncertainty_group)
        
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
        self.figure = Figure(figsize=(8, 6), dpi=100)
        self.canvas = FigureCanvas(self.figure)
        self.toolbar = NavigationToolbar(self.canvas, self)
        
        layout.addWidget(self.toolbar)
        layout.addWidget(self.canvas)
        
        # Visualization options
        options_group = QGroupBox("Visualization Options")
        options_layout = QHBoxLayout()
        
        options_layout.addWidget(QLabel("Plot Type:"))
        self.plot_type_combo = QComboBox()
        self.plot_type_combo.addItems(["Line Plot", "Scatter Plot", "Contour Plot", "Histogram"])
        options_layout.addWidget(self.plot_type_combo)
        
        self.show_error_bars_check = QCheckBox("Show Error Bars")
        self.show_error_bars_check.setChecked(True)
        options_layout.addWidget(self.show_error_bars_check)
        
        options_group.setLayout(options_layout)
        layout.addWidget(options_group)
        
        tab.setLayout(layout)
        return tab
        
    def connect_signals(self):
        """Connect UI signals to slots"""
        self.run_button.clicked.connect(self.run_measurement)
        self.export_button.clicked.connect(self.export_results)
        self.technique_combo.currentIndexChanged.connect(self.update_technique_parameters)
        self.measurement_type_combo.currentIndexChanged.connect(self.update_measurement_type_parameters)
        self.plot_type_combo.currentIndexChanged.connect(self.update_visualization)
        self.show_error_bars_check.stateChanged.connect(self.update_visualization)
        
    def run_measurement(self):
        """Run the metrology measurement"""
        # Show progress
        self.progress_bar.setVisible(True)
        self.progress_bar.setRange(0, 0)  # Indeterminate progress
        self.run_button.setEnabled(False)
        
        try:
            # Create parameters
            params = PyMetrologyParameters(
                self.technique_combo.currentData(),
                self.measurement_type_combo.currentData(),
                self.resolution_spin.value(),
                self.accuracy_spin.value(),
                self.precision_spin.value(),
                self.scan_area_spin.value(),
                self.scan_time_spin.value(),
                self.destructive_check.isChecked()
            )
            
            # Configure model
            self.model.enable_noise_simulation(self.noise_simulation_check.isChecked())
            self.model.set_calibration_factor(1.0)  # Default calibration
            
            # TODO: Get wafer from main application
            # For now, we'll use a dummy wafer
            from ..cpp_bridge import create_bridge
            geometry_bridge = create_bridge('geometry')
            wafer = geometry_bridge.PyWafer(100.0, 50, 50)
            
            # Run measurement
            result = self.model.perform_measurement(wafer, params)
            self.display_results(result)
            self.measurement_complete.emit(result)
            
        except Exception as e:
            QMessageBox.critical(self, "Measurement Error", f"Error during measurement: {str(e)}")
        finally:
            # Hide progress
            self.progress_bar.setVisible(False)
            self.run_button.setEnabled(True)
            
    def display_results(self, result):
        """Display measurement results"""
        # Update table
        self.results_table.setRowCount(0)
        
        # Get measurement type name and units
        measurement_type = self.measurement_type_combo.currentData()
        measurement_type_name = self.model.get_measurement_type_name(measurement_type)
        units = self.model.get_units(measurement_type)
        
        # Add main result
        self.results_table.insertRow(0)
        self.results_table.setItem(0, 0, QTableWidgetItem(f"{measurement_type_name}"))
        self.results_table.setItem(0, 1, QTableWidgetItem(f"{result.measured_value:.6g} {units}"))
        
        # Add uncertainty information
        self.uncertainty_label.setText(f"{result.uncertainty:.6g} {units}")
        self.snr_label.setText(f"{result.signal_to_noise:.2f}")
        self.range_label.setText(f"{result.min_value:.6g} - {result.max_value:.6g} {units}")
        
        # Enable export
        self.export_button.setEnabled(True)
        
        # Store result for visualization
        self.current_result = result
        
        # Update visualization
        self.update_visualization()
        
    def update_visualization(self):
        """Update the visualization based on current options"""
        # Check if we have results
        if not hasattr(self, 'current_result'):
            return
            
        # Clear figure
        self.figure.clear()
        ax = self.figure.add_subplot(111)
        
        # Get profile data
        profile_data = self.current_result.get_profile_data()
        
        if not profile_data:
            ax.text(0.5, 0.5, "No profile data available", 
                   ha='center', va='center', transform=ax.transAxes)
            self.canvas.draw()
            return
            
        # Extract x and y values
        x_values = [point[0] for point in profile_data]
        y_values = [point[1] for point in profile_data]
        
        # Get measurement type and units
        measurement_type = self.measurement_type_combo.currentData()
        measurement_type_name = self.model.get_measurement_type_name(measurement_type)
        units = self.model.get_units(measurement_type)
        
        # Plot based on selected type
        plot_type = self.plot_type_combo.currentText()
        
        if plot_type == "Line Plot":
            ax.plot(x_values, y_values, 'b-')
            
            if self.show_error_bars_check.isChecked():
                # Add error bars using uncertainty
                uncertainty = self.current_result.uncertainty
                ax.fill_between(x_values, 
                               [y - uncertainty for y in y_values],
                               [y + uncertainty for y in y_values],
                               alpha=0.2, color='blue')
                
        elif plot_type == "Scatter Plot":
            if self.show_error_bars_check.isChecked():
                # Add error bars
                uncertainty = self.current_result.uncertainty
                ax.errorbar(x_values, y_values, yerr=uncertainty, fmt='o', capsize=3)
            else:
                ax.scatter(x_values, y_values)
                
        elif plot_type == "Contour Plot":
            # For contour plot, we need 2D data
            # If we only have 1D data, we'll create a dummy 2D representation
            if len(x_values) > 10:
                # Create a grid
                n = int(np.sqrt(len(x_values)))
                x_grid = np.linspace(min(x_values), max(x_values), n)
                y_grid = np.linspace(min(x_values), max(x_values), n)
                X, Y = np.meshgrid(x_grid, y_grid)
                
                # Create Z values (use measured values repeated)
                Z = np.zeros_like(X)
                for i in range(n):
                    for j in range(n):
                        idx = i * n + j
                        if idx < len(y_values):
                            Z[i, j] = y_values[idx]
                        else:
                            Z[i, j] = y_values[-1]
                
                # Plot contour
                contour = ax.contourf(X, Y, Z, 20, cmap='viridis')
                self.figure.colorbar(contour, ax=ax, label=f"{measurement_type_name} ({units})")
            else:
                ax.text(0.5, 0.5, "Not enough data points for contour plot", 
                       ha='center', va='center', transform=ax.transAxes)
                
        elif plot_type == "Histogram":
            ax.hist(y_values, bins=min(20, len(y_values)//2), alpha=0.7)
            
            # Add mean and uncertainty lines
            mean_value = self.current_result.measured_value
            uncertainty = self.current_result.uncertainty
            
            ax.axvline(mean_value, color='r', linestyle='--', label=f'Mean: {mean_value:.4g}')
            
            if self.show_error_bars_check.isChecked():
                ax.axvline(mean_value - uncertainty, color='g', linestyle=':', 
                          label=f'Mean ± Uncertainty')
                ax.axvline(mean_value + uncertainty, color='g', linestyle=':')
                
            ax.legend()
        
        # Set labels and title
        if plot_type != "Histogram":
            ax.set_xlabel('Position (μm)')
        else:
            ax.set_xlabel(f'{measurement_type_name} ({units})')
            
        if plot_type != "Contour Plot":
            ax.set_ylabel(f'{measurement_type_name} ({units})')
            
        technique_name = self.model.get_technique_name(self.technique_combo.currentData())
        ax.set_title(f'{technique_name} Measurement of {measurement_type_name}')
        
        # Add grid
        ax.grid(True, alpha=0.3)
        
        # Refresh canvas
        self.canvas.draw()
        
    def export_results(self):
        """Export measurement results to file"""
        filename, _ = QFileDialog.getSaveFileName(self, "Save Results", "", "CSV Files (*.csv);;Text Files (*.txt);;All Files (*)")
        
        if filename:
            try:
                with open(filename, 'w') as f:
                    # Write header
                    measurement_type = self.measurement_type_combo.currentData()
                    measurement_type_name = self.model.get_measurement_type_name(measurement_type)
                    technique_name = self.model.get_technique_name(self.technique_combo.currentData())
                    units = self.model.get_units(measurement_type)
                    
                    f.write(f"# {technique_name} Measurement of {measurement_type_name}\n")
                    f.write(f"# Units: {units}\n")
                    f.write(f"# Measured Value: {self.current_result.measured_value:.6g} ± {self.current_result.uncertainty:.6g}\n")
                    f.write(f"# Signal-to-Noise Ratio: {self.current_result.signal_to_noise:.2f}\n")
                    f.write(f"# Range: {self.current_result.min_value:.6g} - {self.current_result.max_value:.6g}\n")
                    f.write(f"# Summary: {self.current_result.summary}\n\n")
                    
                    # Write profile data
                    f.write("Position,Value\n")
                    for point in self.current_result.get_profile_data():
                        f.write(f"{point[0]:.6g},{point[1]:.6g}\n")
                
                QMessageBox.information(self, "Export Complete", f"Results saved to {filename}")
            except Exception as e:
                QMessageBox.critical(self, "Export Error", f"Error saving results: {str(e)}")
                
    def update_technique_parameters(self):
        """Update parameter defaults based on selected technique"""
        technique = self.technique_combo.currentData()
        
        # Set default parameters based on technique
        if technique == TECHNIQUE_OPTICAL_MICROSCOPY:
            self.resolution_spin.setValue(0.1)
            self.scan_time_spin.setValue(10.0)
        elif technique == TECHNIQUE_PROFILOMETRY:
            self.resolution_spin.setValue(0.01)
            self.scan_time_spin.setValue(30.0)
        elif technique == TECHNIQUE_ELLIPSOMETRY:
            self.resolution_spin.setValue(0.001)
            self.scan_time_spin.setValue(60.0)
        elif technique == TECHNIQUE_XRD:
            self.resolution_spin.setValue(0.01)
            self.scan_time_spin.setValue(300.0)
        elif technique == TECHNIQUE_XRF:
            self.resolution_spin.setValue(0.05)
            self.scan_time_spin.setValue(120.0)
        elif technique == TECHNIQUE_SEM:
            self.resolution_spin.setValue(0.001)
            self.scan_time_spin.setValue(60.0)
        elif technique == TECHNIQUE_TEM:
            self.resolution_spin.setValue(0.0001)
            self.scan_time_spin.setValue(300.0)
            self.destructive_check.setChecked(True)
        elif technique == TECHNIQUE_AFM:
            self.resolution_spin.setValue(0.0001)
            self.scan_time_spin.setValue(600.0)
        elif technique == TECHNIQUE_SIMS:
            self.resolution_spin.setValue(0.001)
            self.scan_time_spin.setValue(300.0)
            self.destructive_check.setChecked(True)
        elif technique == TECHNIQUE_FTIR:
            self.resolution_spin.setValue(0.01)
            self.scan_time_spin.setValue(60.0)
            
    def update_measurement_type_parameters(self):
        """Update parameter defaults based on selected measurement type"""
        measurement_type = self.measurement_type_combo.currentData()
        
        # Update compatible techniques
        self.technique_combo.setEnabled(False)
        
        if measurement_type == MEASUREMENT_THICKNESS:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_ELLIPSOMETRY))
        elif measurement_type == MEASUREMENT_COMPOSITION:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_XRF))
        elif measurement_type == MEASUREMENT_ROUGHNESS:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_AFM))
        elif measurement_type == MEASUREMENT_STRESS:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_XRD))
        elif measurement_type == MEASUREMENT_RESISTIVITY:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_PROFILOMETRY))
        elif measurement_type == MEASUREMENT_DOPANT_PROFILE:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_SIMS))
        elif measurement_type == MEASUREMENT_CRYSTAL_STRUCTURE:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_XRD))
        elif measurement_type == MEASUREMENT_OPTICAL_PROPERTIES:
            self.technique_combo.setCurrentIndex(self.technique_combo.findData(TECHNIQUE_ELLIPSOMETRY))
            
        self.technique_combo.setEnabled(True)

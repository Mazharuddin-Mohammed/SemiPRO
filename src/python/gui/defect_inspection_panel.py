# Author: Dr. Mazharuddin Mohammed
"""
Defect Inspection Panel
======================

GUI panel for defect inspection simulation and analysis.
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
        QHeaderView, QSplitter, QMessageBox
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
    defect_inspection_bridge = create_bridge('defect_inspection')
    PyDefectInspectionModel = defect_inspection_bridge.PyDefectInspectionModel
    PyDefect = defect_inspection_bridge.PyDefect
    PyInspectionResult = defect_inspection_bridge.PyInspectionResult
    
    # Constants
    INSPECTION_OPTICAL_BRIGHTFIELD = defect_inspection_bridge.INSPECTION_OPTICAL_BRIGHTFIELD
    INSPECTION_OPTICAL_DARKFIELD = defect_inspection_bridge.INSPECTION_OPTICAL_DARKFIELD
    INSPECTION_SEM = defect_inspection_bridge.INSPECTION_SEM
    INSPECTION_AFM = defect_inspection_bridge.INSPECTION_AFM
    INSPECTION_X_RAY = defect_inspection_bridge.INSPECTION_X_RAY
    
    DEFECT_PARTICLE = defect_inspection_bridge.DEFECT_PARTICLE
    DEFECT_SCRATCH = defect_inspection_bridge.DEFECT_SCRATCH
    DEFECT_VOID = defect_inspection_bridge.DEFECT_VOID
    DEFECT_CONTAMINATION = defect_inspection_bridge.DEFECT_CONTAMINATION
    DEFECT_PATTERN = defect_inspection_bridge.DEFECT_PATTERN
    DEFECT_DIMENSIONAL = defect_inspection_bridge.DEFECT_DIMENSIONAL
    DEFECT_ELECTRICAL = defect_inspection_bridge.DEFECT_ELECTRICAL
    
    SEVERITY_MINOR = defect_inspection_bridge.SEVERITY_MINOR
    SEVERITY_MAJOR = defect_inspection_bridge.SEVERITY_MAJOR
    SEVERITY_CRITICAL = defect_inspection_bridge.SEVERITY_CRITICAL
    
except ImportError as e:
    print(f"Failed to import defect_inspection module: {e}")
    PyDefectInspectionModel = None
    PyDefect = None
    PyInspectionResult = None

class DefectInspectionPanel(QWidget):
    """Panel for defect inspection simulation and analysis"""
    
    # Signal to notify when inspection is complete
    inspection_complete = Signal(object)
    
    def __init__(self, parent=None):
        """Initialize the defect inspection panel"""
        super().__init__(parent)
        
        # Check if module is available
        if PyDefectInspectionModel is None:
            self.setEnabled(False)
            layout = QVBoxLayout()
            layout.addWidget(QLabel("Defect Inspection module not available"))
            self.setLayout(layout)
            return
        
        # Create defect inspection model
        self.model = PyDefectInspectionModel()
        
        # Initialize UI
        self.init_ui()
        
        # Connect signals
        self.connect_signals()
        
    def init_ui(self):
        """Initialize the user interface"""
        main_layout = QVBoxLayout()
        
        # Create tabs
        tabs = QTabWidget()
        tabs.addTab(self.create_inspection_tab(), "Inspection")
        tabs.addTab(self.create_analysis_tab(), "Analysis")
        tabs.addTab(self.create_visualization_tab(), "Visualization")
        
        main_layout.addWidget(tabs)
        self.setLayout(main_layout)
        
    def create_inspection_tab(self):
        """Create the inspection configuration tab"""
        tab = QWidget()
        layout = QVBoxLayout()
        
        # Inspection method selection
        method_group = QGroupBox("Inspection Method")
        method_layout = QVBoxLayout()
        
        self.method_combo = QComboBox()
        self.method_combo.addItem("Optical Brightfield", INSPECTION_OPTICAL_BRIGHTFIELD)
        self.method_combo.addItem("Optical Darkfield", INSPECTION_OPTICAL_DARKFIELD)
        self.method_combo.addItem("SEM", INSPECTION_SEM)
        self.method_combo.addItem("AFM", INSPECTION_AFM)
        self.method_combo.addItem("X-Ray Topography", INSPECTION_X_RAY)
        
        method_layout.addWidget(self.method_combo)
        method_group.setLayout(method_layout)
        layout.addWidget(method_group)
        
        # Inspection parameters
        params_group = QGroupBox("Inspection Parameters")
        params_layout = QVBoxLayout()
        
        # Pixel size
        pixel_layout = QHBoxLayout()
        pixel_layout.addWidget(QLabel("Pixel Size (μm):"))
        self.pixel_size_spin = QDoubleSpinBox()
        self.pixel_size_spin.setRange(0.001, 10.0)
        self.pixel_size_spin.setValue(0.1)
        self.pixel_size_spin.setDecimals(3)
        pixel_layout.addWidget(self.pixel_size_spin)
        params_layout.addLayout(pixel_layout)
        
        # Scan speed
        speed_layout = QHBoxLayout()
        speed_layout.addWidget(QLabel("Scan Speed (mm/s):"))
        self.scan_speed_spin = QDoubleSpinBox()
        self.scan_speed_spin.setRange(0.1, 100.0)
        self.scan_speed_spin.setValue(10.0)
        self.scan_speed_spin.setDecimals(1)
        speed_layout.addWidget(self.scan_speed_spin)
        params_layout.addLayout(speed_layout)
        
        # Sensitivity
        sensitivity_layout = QHBoxLayout()
        sensitivity_layout.addWidget(QLabel("Sensitivity:"))
        self.sensitivity_spin = QDoubleSpinBox()
        self.sensitivity_spin.setRange(0.1, 1.0)
        self.sensitivity_spin.setValue(0.9)
        self.sensitivity_spin.setDecimals(2)
        sensitivity_layout.addWidget(self.sensitivity_spin)
        params_layout.addLayout(sensitivity_layout)
        
        # Options
        options_layout = QHBoxLayout()
        self.classification_check = QCheckBox("Enable Defect Classification")
        self.classification_check.setChecked(True)
        options_layout.addWidget(self.classification_check)
        params_layout.addLayout(options_layout)
        
        params_group.setLayout(params_layout)
        layout.addWidget(params_group)
        
        # Run button
        self.run_button = QPushButton("Run Inspection")
        layout.addWidget(self.run_button)
        
        tab.setLayout(layout)
        return tab
        
    def create_analysis_tab(self):
        """Create the analysis tab"""
        tab = QWidget()
        layout = QVBoxLayout()
        
        # Results table
        self.results_table = QTableWidget(0, 5)
        self.results_table.setHorizontalHeaderLabels(["Type", "Severity", "Size (μm)", "Position", "Confidence"])
        self.results_table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        layout.addWidget(self.results_table)
        
        # Summary
        summary_group = QGroupBox("Inspection Summary")
        summary_layout = QVBoxLayout()
        
        self.summary_label = QLabel("No inspection results available")
        summary_layout.addWidget(self.summary_label)
        
        # Statistics
        stats_layout = QHBoxLayout()
        stats_layout.addWidget(QLabel("Total Defects:"))
        self.total_defects_label = QLabel("0")
        stats_layout.addWidget(self.total_defects_label)
        
        stats_layout.addWidget(QLabel("Defect Density:"))
        self.defect_density_label = QLabel("0 cm⁻²")
        stats_layout.addWidget(self.defect_density_label)
        
        stats_layout.addWidget(QLabel("Coverage:"))
        self.coverage_label = QLabel("0 %")
        stats_layout.addWidget(self.coverage_label)
        
        summary_layout.addLayout(stats_layout)
        
        # Export button
        self.export_button = QPushButton("Export Report")
        self.export_button.setEnabled(False)
        summary_layout.addWidget(self.export_button)
        
        summary_group.setLayout(summary_layout)
        layout.addWidget(summary_group)
        
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
        
        options_layout.addWidget(QLabel("Color by:"))
        self.color_by_combo = QComboBox()
        self.color_by_combo.addItems(["Defect Type", "Severity", "Size"])
        options_layout.addWidget(self.color_by_combo)
        
        self.show_labels_check = QCheckBox("Show Labels")
        self.show_labels_check.setChecked(True)
        options_layout.addWidget(self.show_labels_check)
        
        options_group.setLayout(options_layout)
        layout.addWidget(options_group)
        
        tab.setLayout(layout)
        return tab
        
    def connect_signals(self):
        """Connect UI signals to slots"""
        self.run_button.clicked.connect(self.run_inspection)
        self.export_button.clicked.connect(self.export_report)
        self.method_combo.currentIndexChanged.connect(self.update_method_parameters)
        self.color_by_combo.currentIndexChanged.connect(self.update_visualization)
        self.show_labels_check.stateChanged.connect(self.update_visualization)
        
    def run_inspection(self):
        """Run the defect inspection simulation"""
        # Get parameters
        method = self.method_combo.currentData()
        pixel_size = self.pixel_size_spin.value()
        scan_speed = self.scan_speed_spin.value()
        sensitivity = self.sensitivity_spin.value()
        
        # Configure model
        self.model.set_inspection_parameters(method, pixel_size, scan_speed, sensitivity)
        self.model.enable_defect_classification(self.classification_check.isChecked())
        
        # TODO: Get wafer from main application
        # For now, we'll use a dummy wafer
        from ..cpp_bridge import create_bridge
        geometry_bridge = create_bridge('geometry')
        wafer = geometry_bridge.PyWafer(100.0, 50, 50)
        
        # Run inspection
        try:
            result = self.model.perform_inspection(wafer, method)
            self.display_results(result)
            self.inspection_complete.emit(result)
        except Exception as e:
            QMessageBox.critical(self, "Inspection Error", f"Error during inspection: {str(e)}")
            
    def display_results(self, result):
        """Display inspection results"""
        # Update table
        self.results_table.setRowCount(0)
        defects = result.get_defects()
        
        for i, defect in enumerate(defects):
            self.results_table.insertRow(i)
            
            # Type
            type_names = {
                DEFECT_PARTICLE: "Particle",
                DEFECT_SCRATCH: "Scratch",
                DEFECT_VOID: "Void",
                DEFECT_CONTAMINATION: "Contamination",
                DEFECT_PATTERN: "Pattern",
                DEFECT_DIMENSIONAL: "Dimensional",
                DEFECT_ELECTRICAL: "Electrical"
            }
            type_item = QTableWidgetItem(type_names.get(defect.type, "Unknown"))
            self.results_table.setItem(i, 0, type_item)
            
            # Severity
            severity_names = {
                SEVERITY_MINOR: "Minor",
                SEVERITY_MAJOR: "Major",
                SEVERITY_CRITICAL: "Critical"
            }
            severity_item = QTableWidgetItem(severity_names.get(defect.severity, "Unknown"))
            self.results_table.setItem(i, 1, severity_item)
            
            # Size
            size_item = QTableWidgetItem(f"{defect.size:.3f}")
            self.results_table.setItem(i, 2, size_item)
            
            # Position
            pos_item = QTableWidgetItem(f"({defect.x:.1f}, {defect.y:.1f}, {defect.z:.1f})")
            self.results_table.setItem(i, 3, pos_item)
            
            # Confidence
            conf_item = QTableWidgetItem(f"{defect.confidence:.2f}")
            self.results_table.setItem(i, 4, conf_item)
            
        # Update summary
        self.summary_label.setText(result.summary)
        self.total_defects_label.setText(str(result.defect_count))
        self.defect_density_label.setText(f"{result.defect_count / result.coverage_area:.2f} cm⁻²")
        self.coverage_label.setText(f"{result.coverage_area:.2f} cm²")
        
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
        ax = self.figure.add_subplot(111)
        
        # Get defects
        defects = []
        for i in range(self.results_table.rowCount()):
            x = float(self.results_table.item(i, 3).text().split(',')[0].strip('('))
            y = float(self.results_table.item(i, 3).text().split(',')[1])
            size = float(self.results_table.item(i, 2).text())
            defect_type = self.results_table.item(i, 0).text()
            severity = self.results_table.item(i, 1).text()
            
            defects.append({
                'x': x,
                'y': y,
                'size': size,
                'type': defect_type,
                'severity': severity
            })
            
        # Plot defects
        color_by = self.color_by_combo.currentText()
        
        if color_by == "Defect Type":
            type_colors = {
                "Particle": "red",
                "Scratch": "blue",
                "Void": "green",
                "Contamination": "purple",
                "Pattern": "orange",
                "Dimensional": "cyan",
                "Electrical": "magenta"
            }
            
            for defect in defects:
                ax.scatter(defect['x'], defect['y'], 
                          s=defect['size']*100,
                          color=type_colors.get(defect['type'], 'gray'),
                          alpha=0.7)
                
                if self.show_labels_check.isChecked():
                    ax.annotate(defect['type'], 
                               (defect['x'], defect['y']),
                               fontsize=8)
                    
        elif color_by == "Severity":
            severity_colors = {
                "Minor": "green",
                "Major": "orange",
                "Critical": "red"
            }
            
            for defect in defects:
                ax.scatter(defect['x'], defect['y'], 
                          s=defect['size']*100,
                          color=severity_colors.get(defect['severity'], 'gray'),
                          alpha=0.7)
                
                if self.show_labels_check.isChecked():
                    ax.annotate(defect['severity'], 
                               (defect['x'], defect['y']),
                               fontsize=8)
                    
        elif color_by == "Size":
            sizes = [defect['size'] for defect in defects]
            min_size = min(sizes)
            max_size = max(sizes)
            
            for defect in defects:
                # Normalize size for color
                norm_size = (defect['size'] - min_size) / (max_size - min_size) if max_size > min_size else 0.5
                
                ax.scatter(defect['x'], defect['y'], 
                          s=defect['size']*100,
                          color=plt.cm.viridis(norm_size),
                          alpha=0.7)
                
                if self.show_labels_check.isChecked():
                    ax.annotate(f"{defect['size']:.2f}", 
                               (defect['x'], defect['y']),
                               fontsize=8)
        
        # Set labels and title
        ax.set_xlabel('X Position (μm)')
        ax.set_ylabel('Y Position (μm)')
        ax.set_title('Defect Map')
        
        # Add colorbar if color by size
        if color_by == "Size":
            from matplotlib.cm import ScalarMappable
            from matplotlib.colors import Normalize
            
            sm = ScalarMappable(cmap=plt.cm.viridis, norm=Normalize(vmin=min_size, vmax=max_size))
            sm.set_array([])
            cbar = self.figure.colorbar(sm, ax=ax)
            cbar.set_label('Defect Size (μm)')
            
        # Add legend if color by type or severity
        if color_by == "Defect Type":
            from matplotlib.lines import Line2D
            legend_elements = [Line2D([0], [0], marker='o', color='w', 
                                     markerfacecolor=color, label=defect_type, markersize=8)
                              for defect_type, color in type_colors.items()]
            ax.legend(handles=legend_elements, loc='upper right')
            
        elif color_by == "Severity":
            from matplotlib.lines import Line2D
            legend_elements = [Line2D([0], [0], marker='o', color='w', 
                                     markerfacecolor=color, label=severity, markersize=8)
                              for severity, color in severity_colors.items()]
            ax.legend(handles=legend_elements, loc='upper right')
        
        # Refresh canvas
        self.canvas.draw()
        
    def export_report(self):
        """Export inspection report to file"""
        filename, _ = QFileDialog.getSaveFileName(self, "Save Report", "", "HTML Files (*.html);;Text Files (*.txt);;All Files (*)")
        
        if filename:
            try:
                # TODO: Implement proper report generation
                # For now, just save a simple text file
                with open(filename, 'w') as f:
                    f.write(f"Defect Inspection Report\n")
                    f.write(f"=======================\n\n")
                    f.write(f"Summary: {self.summary_label.text()}\n")
                    f.write(f"Total Defects: {self.total_defects_label.text()}\n")
                    f.write(f"Defect Density: {self.defect_density_label.text()}\n")
                    f.write(f"Coverage Area: {self.coverage_label.text()}\n\n")
                    
                    f.write(f"Defect Details:\n")
                    f.write(f"---------------\n")
                    
                    for i in range(self.results_table.rowCount()):
                        defect_type = self.results_table.item(i, 0).text()
                        severity = self.results_table.item(i, 1).text()
                        size = self.results_table.item(i, 2).text()
                        position = self.results_table.item(i, 3).text()
                        confidence = self.results_table.item(i, 4).text()
                        
                        f.write(f"Defect {i+1}:\n")
                        f.write(f"  Type: {defect_type}\n")
                        f.write(f"  Severity: {severity}\n")
                        f.write(f"  Size: {size} μm\n")
                        f.write(f"  Position: {position}\n")
                        f.write(f"  Confidence: {confidence}\n\n")
                
                QMessageBox.information(self, "Export Complete", f"Report saved to {filename}")
            except Exception as e:
                QMessageBox.critical(self, "Export Error", f"Error saving report: {str(e)}")
                
    def update_method_parameters(self):
        """Update parameter defaults based on selected method"""
        method = self.method_combo.currentData()
        
        # Set default parameters based on method
        if method == INSPECTION_OPTICAL_BRIGHTFIELD:
            self.pixel_size_spin.setValue(0.1)
            self.scan_speed_spin.setValue(10.0)
            self.sensitivity_spin.setValue(0.8)
        elif method == INSPECTION_OPTICAL_DARKFIELD:
            self.pixel_size_spin.setValue(0.1)
            self.scan_speed_spin.setValue(8.0)
            self.sensitivity_spin.setValue(0.85)
        elif method == INSPECTION_SEM:
            self.pixel_size_spin.setValue(0.01)
            self.scan_speed_spin.setValue(1.0)
            self.sensitivity_spin.setValue(0.95)
        elif method == INSPECTION_AFM:
            self.pixel_size_spin.setValue(0.001)
            self.scan_speed_spin.setValue(0.5)
            self.sensitivity_spin.setValue(0.98)
        elif method == INSPECTION_X_RAY:
            self.pixel_size_spin.setValue(0.05)
            self.scan_speed_spin.setValue(5.0)
            self.sensitivity_spin.setValue(0.75)

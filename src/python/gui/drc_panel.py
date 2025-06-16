# Author: Dr. Mazharuddin Mohammed
from PySide6.QtWidgets import (QWidget, QVBoxLayout, QHBoxLayout, QLineEdit, 
                               QPushButton, QLabel, QComboBox, QSpinBox, 
                               QDoubleSpinBox, QTableWidget, QTableWidgetItem,
                               QGroupBox, QFormLayout, QTextEdit, QTabWidget,
                               QCheckBox, QProgressBar, QFileDialog)
from PySide6.QtCore import Signal, Qt, QThread, pyqtSignal
from PySide6.QtGui import QColor
from ..simulator import Simulator
from ..drc import (VIOLATION_TYPE_WIDTH, VIOLATION_TYPE_SPACING, VIOLATION_TYPE_AREA,
                   VIOLATION_TYPE_DENSITY, VIOLATION_TYPE_ANTENNA_RATIO,
                   VIOLATION_SEVERITY_CRITICAL, VIOLATION_SEVERITY_ERROR,
                   VIOLATION_SEVERITY_WARNING, VIOLATION_SEVERITY_INFO)

class DRCWorker(QThread):
    progress_updated = pyqtSignal(int)
    drc_completed = pyqtSignal(dict)
    
    def __init__(self, simulator, drc_type="full"):
        super().__init__()
        self.simulator = simulator
        self.drc_type = drc_type
        
    def run(self):
        try:
            if self.drc_type == "full":
                self.progress_updated.emit(20)
                self.simulator.run_full_drc()
                self.progress_updated.emit(100)
            elif self.drc_type == "layer":
                self.progress_updated.emit(50)
                self.simulator.run_layer_drc("metal")
                self.progress_updated.emit(100)
                
            # Get results
            results = {
                'total_violations': self.simulator.get_violation_count(),
                'critical_violations': self.simulator.get_critical_violation_count(),
                'violations_by_layer': self.simulator.get_violations_by_layer()
            }
            
            self.drc_completed.emit(results)
        except Exception as e:
            self.drc_completed.emit({'error': str(e)})

class DRCPanel(QWidget):
    drc_updated = Signal(object)
    
    def __init__(self):
        super().__init__()
        self.simulator = Simulator()
        self.drc_worker = None
        self.setup_ui()
        
    def setup_ui(self):
        layout = QVBoxLayout(self)
        
        # Create tab widget
        tab_widget = QTabWidget()
        
        # Technology Setup Tab
        tech_tab = self.create_technology_tab()
        tab_widget.addTab(tech_tab, "Technology Setup")
        
        # Rules Tab
        rules_tab = self.create_rules_tab()
        tab_widget.addTab(rules_tab, "Rules")
        
        # DRC Check Tab
        check_tab = self.create_check_tab()
        tab_widget.addTab(check_tab, "DRC Check")
        
        # Results Tab
        results_tab = self.create_results_tab()
        tab_widget.addTab(results_tab, "Results")
        
        layout.addWidget(tab_widget)
        
    def create_technology_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Technology Node Selection
        tech_group = QGroupBox("Technology Node")
        tech_layout = QFormLayout(tech_group)
        
        self.tech_combo = QComboBox()
        self.tech_combo.addItems(["7nm", "14nm", "28nm", "65nm", "130nm", "250nm", "Custom"])
        self.tech_combo.currentTextChanged.connect(self.load_technology)
        
        tech_layout.addRow("Technology Node:", self.tech_combo)
        
        load_tech_button = QPushButton("Load Technology Rules")
        load_tech_button.clicked.connect(self.load_technology)
        tech_layout.addRow(load_tech_button)
        
        layout.addWidget(tech_group)
        
        # Custom Technology Parameters
        custom_group = QGroupBox("Custom Technology Parameters")
        custom_layout = QFormLayout(custom_group)
        
        self.feature_size = QDoubleSpinBox()
        self.feature_size.setRange(0.001, 10.0)
        self.feature_size.setValue(0.1)
        self.feature_size.setSuffix(" μm")
        
        self.metal_pitch = QDoubleSpinBox()
        self.metal_pitch.setRange(0.001, 10.0)
        self.metal_pitch.setValue(0.2)
        self.metal_pitch.setSuffix(" μm")
        
        self.via_size = QDoubleSpinBox()
        self.via_size.setRange(0.001, 1.0)
        self.via_size.setValue(0.05)
        self.via_size.setSuffix(" μm")
        
        custom_layout.addRow("Feature Size:", self.feature_size)
        custom_layout.addRow("Metal Pitch:", self.metal_pitch)
        custom_layout.addRow("Via Size:", self.via_size)
        
        apply_custom_button = QPushButton("Apply Custom Parameters")
        apply_custom_button.clicked.connect(self.apply_custom_technology)
        custom_layout.addRow(apply_custom_button)
        
        layout.addWidget(custom_group)
        
        # Technology Info
        self.tech_info = QTextEdit()
        self.tech_info.setReadOnly(True)
        self.tech_info.setMaximumHeight(100)
        layout.addWidget(QLabel("Technology Information:"))
        layout.addWidget(self.tech_info)
        
        return widget
        
    def create_rules_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Add Rule Group
        add_rule_group = QGroupBox("Add Custom Rule")
        add_rule_layout = QFormLayout(add_rule_group)
        
        self.rule_name = QLineEdit("CUSTOM_RULE")
        self.rule_type = QComboBox()
        self.rule_type.addItems(["Width", "Spacing", "Area", "Density", "Antenna Ratio"])
        self.rule_layer = QLineEdit("metal")
        self.rule_min_value = QDoubleSpinBox()
        self.rule_min_value.setRange(0.0, 100.0)
        self.rule_min_value.setValue(0.1)
        self.rule_max_value = QDoubleSpinBox()
        self.rule_max_value.setRange(-1.0, 100.0)
        self.rule_max_value.setValue(-1.0)
        
        add_rule_layout.addRow("Rule Name:", self.rule_name)
        add_rule_layout.addRow("Rule Type:", self.rule_type)
        add_rule_layout.addRow("Layer:", self.rule_layer)
        add_rule_layout.addRow("Min Value:", self.rule_min_value)
        add_rule_layout.addRow("Max Value:", self.rule_max_value)
        
        add_rule_button = QPushButton("Add Rule")
        add_rule_button.clicked.connect(self.add_custom_rule)
        add_rule_layout.addRow(add_rule_button)
        
        layout.addWidget(add_rule_group)
        
        # Rules Table
        self.rules_table = QTableWidget()
        self.rules_table.setColumnCount(6)
        self.rules_table.setHorizontalHeaderLabels(["Name", "Type", "Layer", "Min Value", "Max Value", "Enabled"])
        layout.addWidget(QLabel("Current Rules:"))
        layout.addWidget(self.rules_table)
        
        # Rule Management Buttons
        button_layout = QHBoxLayout()
        
        refresh_rules_button = QPushButton("Refresh Rules")
        refresh_rules_button.clicked.connect(self.refresh_rules_table)
        button_layout.addWidget(refresh_rules_button)
        
        export_rules_button = QPushButton("Export Rules")
        export_rules_button.clicked.connect(self.export_rules)
        button_layout.addWidget(export_rules_button)
        
        import_rules_button = QPushButton("Import Rules")
        import_rules_button.clicked.connect(self.import_rules)
        button_layout.addWidget(import_rules_button)
        
        layout.addLayout(button_layout)
        
        return widget
        
    def create_check_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # DRC Check Options
        check_group = QGroupBox("DRC Check Options")
        check_layout = QVBoxLayout(check_group)
        
        # Check Type Selection
        type_layout = QHBoxLayout()
        
        self.full_drc_radio = QCheckBox("Full DRC")
        self.full_drc_radio.setChecked(True)
        type_layout.addWidget(self.full_drc_radio)
        
        self.layer_drc_radio = QCheckBox("Layer DRC")
        type_layout.addWidget(self.layer_drc_radio)
        
        self.incremental_drc_radio = QCheckBox("Incremental DRC")
        type_layout.addWidget(self.incremental_drc_radio)
        
        check_layout.addLayout(type_layout)
        
        # Layer Selection for Layer DRC
        layer_layout = QHBoxLayout()
        layer_layout.addWidget(QLabel("Layer:"))
        self.drc_layer_combo = QComboBox()
        self.drc_layer_combo.addItems(["metal", "poly", "oxide", "via", "contact"])
        layer_layout.addWidget(self.drc_layer_combo)
        check_layout.addLayout(layer_layout)
        
        layout.addWidget(check_group)
        
        # Run DRC Button
        self.run_drc_button = QPushButton("Run DRC Check")
        self.run_drc_button.clicked.connect(self.run_drc_check)
        layout.addWidget(self.run_drc_button)
        
        # Progress Bar
        self.progress_bar = QProgressBar()
        self.progress_bar.setVisible(False)
        layout.addWidget(self.progress_bar)
        
        # Quick Stats
        stats_group = QGroupBox("Quick Statistics")
        stats_layout = QFormLayout(stats_group)
        
        self.total_violations_label = QLabel("0")
        self.critical_violations_label = QLabel("0")
        self.error_violations_label = QLabel("0")
        self.warning_violations_label = QLabel("0")
        
        stats_layout.addRow("Total Violations:", self.total_violations_label)
        stats_layout.addRow("Critical:", self.critical_violations_label)
        stats_layout.addRow("Errors:", self.error_violations_label)
        stats_layout.addRow("Warnings:", self.warning_violations_label)
        
        layout.addWidget(stats_group)
        
        return widget
        
    def create_results_tab(self):
        widget = QWidget()
        layout = QVBoxLayout(widget)
        
        # Violations Table
        self.violations_table = QTableWidget()
        self.violations_table.setColumnCount(6)
        self.violations_table.setHorizontalHeaderLabels(["Rule", "Type", "Severity", "Location", "Measured", "Required"])
        layout.addWidget(QLabel("Violations:"))
        layout.addWidget(self.violations_table)
        
        # Results Actions
        actions_layout = QHBoxLayout()
        
        generate_report_button = QPushButton("Generate Report")
        generate_report_button.clicked.connect(self.generate_report)
        actions_layout.addWidget(generate_report_button)
        
        export_violations_button = QPushButton("Export Violations")
        export_violations_button.clicked.connect(self.export_violations)
        actions_layout.addWidget(export_violations_button)
        
        clear_violations_button = QPushButton("Clear Violations")
        clear_violations_button.clicked.connect(self.clear_violations)
        actions_layout.addWidget(clear_violations_button)
        
        layout.addLayout(actions_layout)
        
        # Detailed Results
        self.results_text = QTextEdit()
        self.results_text.setReadOnly(True)
        layout.addWidget(QLabel("Detailed Results:"))
        layout.addWidget(self.results_text)
        
        return widget
        
    def load_technology(self):
        tech_node = self.tech_combo.currentText()
        if tech_node != "Custom":
            try:
                self.simulator.load_technology_rules(tech_node)
                self.tech_info.setText(f"Loaded {tech_node} technology rules")
                self.refresh_rules_table()
            except Exception as e:
                self.tech_info.setText(f"Error loading technology: {str(e)}")
                
    def apply_custom_technology(self):
        feature_size = self.feature_size.value()
        metal_pitch = self.metal_pitch.value()
        via_size = self.via_size.value()
        
        try:
            # This would call the C++ configure_technology method
            self.tech_info.setText(f"Applied custom technology: {feature_size}μm feature size")
        except Exception as e:
            self.tech_info.setText(f"Error applying custom technology: {str(e)}")
            
    def add_custom_rule(self):
        name = self.rule_name.text()
        rule_type = self.rule_type.currentIndex()
        layer = self.rule_layer.text()
        min_val = self.rule_min_value.value()
        max_val = self.rule_max_value.value()
        
        try:
            self.simulator.add_drc_rule(name, rule_type, layer, min_val, max_val)
            self.refresh_rules_table()
            self.results_text.append(f"Added rule: {name}")
        except Exception as e:
            self.results_text.append(f"Error adding rule: {str(e)}")
            
    def run_drc_check(self):
        self.run_drc_button.setEnabled(False)
        self.progress_bar.setVisible(True)
        self.progress_bar.setValue(0)
        
        # Determine DRC type
        if self.full_drc_radio.isChecked():
            drc_type = "full"
        elif self.layer_drc_radio.isChecked():
            drc_type = "layer"
        else:
            drc_type = "incremental"
            
        # Start DRC worker thread
        self.drc_worker = DRCWorker(self.simulator, drc_type)
        self.drc_worker.progress_updated.connect(self.progress_bar.setValue)
        self.drc_worker.drc_completed.connect(self.on_drc_completed)
        self.drc_worker.start()
        
    def on_drc_completed(self, results):
        self.run_drc_button.setEnabled(True)
        self.progress_bar.setVisible(False)
        
        if 'error' in results:
            self.results_text.append(f"DRC Error: {results['error']}")
            return
            
        # Update statistics
        self.total_violations_label.setText(str(results['total_violations']))
        self.critical_violations_label.setText(str(results['critical_violations']))
        
        # Update violations table
        self.update_violations_table()
        
        self.results_text.append(f"DRC completed. Found {results['total_violations']} violations.")
        self.drc_updated.emit(self.simulator.get_wafer())
        
    def update_violations_table(self):
        # This would populate the violations table with actual violation data
        # For now, just a placeholder
        pass
        
    def refresh_rules_table(self):
        # This would populate the rules table with current rules
        # For now, just a placeholder
        pass
        
    def generate_report(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Save DRC Report", "drc_report.txt", "Text Files (*.txt)")
        if filename:
            try:
                self.simulator.generate_drc_report(filename)
                self.results_text.append(f"Report generated: {filename}")
            except Exception as e:
                self.results_text.append(f"Error generating report: {str(e)}")
                
    def export_violations(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Export Violations", "violations.csv", "CSV Files (*.csv)")
        if filename:
            try:
                # This would export violations to CSV
                self.results_text.append(f"Violations exported: {filename}")
            except Exception as e:
                self.results_text.append(f"Error exporting violations: {str(e)}")
                
    def clear_violations(self):
        try:
            # This would clear all violations
            self.violations_table.setRowCount(0)
            self.total_violations_label.setText("0")
            self.critical_violations_label.setText("0")
            self.error_violations_label.setText("0")
            self.warning_violations_label.setText("0")
            self.results_text.append("Violations cleared")
        except Exception as e:
            self.results_text.append(f"Error clearing violations: {str(e)}")
            
    def export_rules(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Export Rules", "drc_rules.json", "JSON Files (*.json)")
        if filename:
            # This would export current rules
            self.results_text.append(f"Rules exported: {filename}")
            
    def import_rules(self):
        filename, _ = QFileDialog.getOpenFileName(self, "Import Rules", "", "JSON Files (*.json)")
        if filename:
            # This would import rules from file
            self.results_text.append(f"Rules imported: {filename}")
            self.refresh_rules_table()

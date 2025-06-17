# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO GUI Frontend with Vulkan Rendering
Simple GUI interface for the semiconductor process simulator
"""

import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import threading
import time
import numpy as np
from .simulator import Simulator

class SemiPROGUI:
    """Main GUI application for SemiPRO"""
    
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("SemiPRO - Semiconductor Process Simulator")
        self.root.geometry("1200x800")
        
        # Initialize simulator
        self.simulator = None
        self.simulation_running = False
        
        # Setup GUI
        self.setup_gui()
        
    def setup_gui(self):
        """Setup the main GUI layout"""
        # Create main frame
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)
        
        # Title
        title_label = ttk.Label(main_frame, text="SemiPRO - Semiconductor Process Simulator", 
                               font=("Arial", 16, "bold"))
        title_label.pack(pady=(0, 20))
        
        # Create notebook for tabs
        self.notebook = ttk.Notebook(main_frame)
        self.notebook.pack(fill=tk.BOTH, expand=True)
        
        # Setup tabs
        self.setup_simulation_tab()
        self.setup_workflow_tab()
        self.setup_visualization_tab()
        self.setup_results_tab()
        
        # Status bar
        self.status_var = tk.StringVar()
        self.status_var.set("Ready")
        status_bar = ttk.Label(main_frame, textvariable=self.status_var, relief=tk.SUNKEN)
        status_bar.pack(side=tk.BOTTOM, fill=tk.X, pady=(10, 0))
        
    def setup_simulation_tab(self):
        """Setup the simulation control tab"""
        sim_frame = ttk.Frame(self.notebook)
        self.notebook.add(sim_frame, text="Simulation")
        
        # Wafer initialization
        wafer_frame = ttk.LabelFrame(sim_frame, text="Wafer Initialization")
        wafer_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Label(wafer_frame, text="Grid Size:").grid(row=0, column=0, sticky=tk.W, padx=5, pady=5)
        self.grid_x_var = tk.StringVar(value="100")
        self.grid_y_var = tk.StringVar(value="100")
        ttk.Entry(wafer_frame, textvariable=self.grid_x_var, width=10).grid(row=0, column=1, padx=5)
        ttk.Label(wafer_frame, text="x").grid(row=0, column=2, padx=2)
        ttk.Entry(wafer_frame, textvariable=self.grid_y_var, width=10).grid(row=0, column=3, padx=5)
        
        ttk.Button(wafer_frame, text="Initialize Wafer", 
                  command=self.initialize_wafer).grid(row=0, column=4, padx=10)
        
        # Process controls
        process_frame = ttk.LabelFrame(sim_frame, text="Process Simulation")
        process_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        # Oxidation
        ox_frame = ttk.Frame(process_frame)
        ox_frame.pack(fill=tk.X, pady=5)
        ttk.Label(ox_frame, text="Oxidation:").pack(side=tk.LEFT)
        self.ox_temp_var = tk.StringVar(value="1000")
        self.ox_time_var = tk.StringVar(value="2.0")
        ttk.Entry(ox_frame, textvariable=self.ox_temp_var, width=8).pack(side=tk.LEFT, padx=5)
        ttk.Label(ox_frame, text="°C").pack(side=tk.LEFT)
        ttk.Entry(ox_frame, textvariable=self.ox_time_var, width=8).pack(side=tk.LEFT, padx=5)
        ttk.Label(ox_frame, text="hours").pack(side=tk.LEFT)
        ttk.Button(ox_frame, text="Run Oxidation", 
                  command=self.run_oxidation).pack(side=tk.LEFT, padx=10)
        
        # Doping
        dop_frame = ttk.Frame(process_frame)
        dop_frame.pack(fill=tk.X, pady=5)
        ttk.Label(dop_frame, text="Doping:").pack(side=tk.LEFT)
        self.dop_type_var = tk.StringVar(value="boron")
        self.dop_conc_var = tk.StringVar(value="1e16")
        self.dop_energy_var = tk.StringVar(value="50")
        ttk.Combobox(dop_frame, textvariable=self.dop_type_var, 
                    values=["boron", "phosphorus", "arsenic"], width=10).pack(side=tk.LEFT, padx=5)
        ttk.Entry(dop_frame, textvariable=self.dop_conc_var, width=10).pack(side=tk.LEFT, padx=5)
        ttk.Label(dop_frame, text="cm⁻³").pack(side=tk.LEFT)
        ttk.Entry(dop_frame, textvariable=self.dop_energy_var, width=8).pack(side=tk.LEFT, padx=5)
        ttk.Label(dop_frame, text="keV").pack(side=tk.LEFT)
        ttk.Button(dop_frame, text="Run Doping", 
                  command=self.run_doping).pack(side=tk.LEFT, padx=10)
        
        # Deposition
        dep_frame = ttk.Frame(process_frame)
        dep_frame.pack(fill=tk.X, pady=5)
        ttk.Label(dep_frame, text="Deposition:").pack(side=tk.LEFT)
        self.dep_material_var = tk.StringVar(value="aluminum")
        self.dep_thickness_var = tk.StringVar(value="0.5")
        self.dep_temp_var = tk.StringVar(value="300")
        ttk.Combobox(dep_frame, textvariable=self.dep_material_var,
                    values=["aluminum", "polysilicon", "silicon dioxide", "silicon nitride"], 
                    width=12).pack(side=tk.LEFT, padx=5)
        ttk.Entry(dep_frame, textvariable=self.dep_thickness_var, width=8).pack(side=tk.LEFT, padx=5)
        ttk.Label(dep_frame, text="μm").pack(side=tk.LEFT)
        ttk.Entry(dep_frame, textvariable=self.dep_temp_var, width=8).pack(side=tk.LEFT, padx=5)
        ttk.Label(dep_frame, text="°C").pack(side=tk.LEFT)
        ttk.Button(dep_frame, text="Run Deposition", 
                  command=self.run_deposition).pack(side=tk.LEFT, padx=10)
        
        # Progress bar
        self.progress_var = tk.DoubleVar()
        self.progress_bar = ttk.Progressbar(sim_frame, variable=self.progress_var, maximum=100)
        self.progress_bar.pack(fill=tk.X, padx=10, pady=10)
        
    def setup_workflow_tab(self):
        """Setup the workflow management tab"""
        workflow_frame = ttk.Frame(self.notebook)
        self.notebook.add(workflow_frame, text="Workflow")
        
        # Predefined workflows
        preset_frame = ttk.LabelFrame(workflow_frame, text="Predefined Workflows")
        preset_frame.pack(fill=tk.X, padx=10, pady=5)
        
        workflows = [
            ("MOSFET Fabrication", self.run_mosfet_workflow),
            ("CMOS Process", self.run_cmos_workflow),
            ("Memory Cell", self.run_memory_workflow),
        ]
        
        for i, (name, command) in enumerate(workflows):
            ttk.Button(preset_frame, text=name, command=command).grid(row=0, column=i, padx=10, pady=10)
        
        # Workflow log
        log_frame = ttk.LabelFrame(workflow_frame, text="Workflow Log")
        log_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        self.workflow_log = tk.Text(log_frame, height=20, state=tk.DISABLED)
        scrollbar = ttk.Scrollbar(log_frame, orient=tk.VERTICAL, command=self.workflow_log.yview)
        self.workflow_log.configure(yscrollcommand=scrollbar.set)
        
        self.workflow_log.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
    def setup_visualization_tab(self):
        """Setup the visualization tab"""
        viz_frame = ttk.Frame(self.notebook)
        self.notebook.add(viz_frame, text="Visualization")
        
        # Rendering controls
        render_frame = ttk.LabelFrame(viz_frame, text="Vulkan Rendering")
        render_frame.pack(fill=tk.X, padx=10, pady=5)
        
        ttk.Button(render_frame, text="Initialize Renderer", 
                  command=self.initialize_renderer).pack(side=tk.LEFT, padx=10, pady=10)
        ttk.Button(render_frame, text="Render Wafer", 
                  command=self.render_wafer).pack(side=tk.LEFT, padx=10, pady=10)
        
        # Visualization options
        viz_options_frame = ttk.LabelFrame(viz_frame, text="Visualization Options")
        viz_options_frame.pack(fill=tk.X, padx=10, pady=5)
        
        self.viz_type_var = tk.StringVar(value="concentration")
        viz_types = ["concentration", "temperature", "stress", "dopant_profile"]
        
        for i, viz_type in enumerate(viz_types):
            ttk.Radiobutton(viz_options_frame, text=viz_type.replace("_", " ").title(),
                           variable=self.viz_type_var, value=viz_type).grid(row=0, column=i, padx=10, pady=5)
        
    def setup_results_tab(self):
        """Setup the results display tab"""
        results_frame = ttk.Frame(self.notebook)
        self.notebook.add(results_frame, text="Results")
        
        # Results display
        self.results_text = tk.Text(results_frame, height=30, state=tk.DISABLED)
        results_scrollbar = ttk.Scrollbar(results_frame, orient=tk.VERTICAL, command=self.results_text.yview)
        self.results_text.configure(yscrollcommand=results_scrollbar.set)
        
        self.results_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True, padx=10, pady=10)
        results_scrollbar.pack(side=tk.RIGHT, fill=tk.Y, pady=10)
        
        # Export buttons
        export_frame = ttk.Frame(results_frame)
        export_frame.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=5)
        
        ttk.Button(export_frame, text="Export Results", 
                  command=self.export_results).pack(side=tk.LEFT, padx=5)
        ttk.Button(export_frame, text="Clear Results", 
                  command=self.clear_results).pack(side=tk.LEFT, padx=5)
        
    def initialize_wafer(self):
        """Initialize the wafer and simulator"""
        try:
            self.simulator = Simulator()
            x_dim = int(self.grid_x_var.get())
            y_dim = int(self.grid_y_var.get())
            
            self.simulator.initialize_geometry(x_dim, y_dim)
            self.status_var.set(f"Wafer initialized: {x_dim}x{y_dim} grid")
            self.log_message(f"Wafer initialized with {x_dim}x{y_dim} grid")
            
        except Exception as e:
            messagebox.showerror("Error", f"Failed to initialize wafer: {e}")
            
    def run_oxidation(self):
        """Run oxidation simulation"""
        if not self.simulator:
            messagebox.showwarning("Warning", "Please initialize wafer first")
            return
            
        try:
            temp = float(self.ox_temp_var.get())
            time_val = float(self.ox_time_var.get())
            
            self.simulator.simulate_oxidation(temp, time_val, "dry")
            self.status_var.set(f"Oxidation completed: {temp}°C, {time_val}h")
            self.log_message(f"Oxidation: {temp}°C, {time_val}h")
            self.update_results()
            
        except Exception as e:
            messagebox.showerror("Error", f"Oxidation failed: {e}")
            
    def run_doping(self):
        """Run doping simulation"""
        if not self.simulator:
            messagebox.showwarning("Warning", "Please initialize wafer first")
            return
            
        try:
            dopant = self.dop_type_var.get()
            conc = float(self.dop_conc_var.get())
            energy = float(self.dop_energy_var.get())
            
            self.simulator.simulate_doping(dopant, conc, energy, 1000.0)
            self.status_var.set(f"Doping completed: {dopant}, {conc:.2e} cm⁻³")
            self.log_message(f"Doping: {dopant}, {conc:.2e} cm⁻³, {energy} keV")
            self.update_results()
            
        except Exception as e:
            messagebox.showerror("Error", f"Doping failed: {e}")
            
    def run_deposition(self):
        """Run deposition simulation"""
        if not self.simulator:
            messagebox.showwarning("Warning", "Please initialize wafer first")
            return
            
        try:
            material = self.dep_material_var.get()
            thickness = float(self.dep_thickness_var.get())
            temp = float(self.dep_temp_var.get())
            
            self.simulator.simulate_deposition(material, thickness, temp)
            self.status_var.set(f"Deposition completed: {material}, {thickness}μm")
            self.log_message(f"Deposition: {material}, {thickness}μm, {temp}°C")
            self.update_results()
            
        except Exception as e:
            messagebox.showerror("Error", f"Deposition failed: {e}")
            
    def run_mosfet_workflow(self):
        """Run complete MOSFET workflow"""
        if not self.simulator:
            messagebox.showwarning("Warning", "Please initialize wafer first")
            return
            
        def workflow():
            try:
                self.simulation_running = True
                self.log_message("Starting MOSFET fabrication workflow...")
                
                steps = [
                    ("Gate oxide growth", lambda: self.simulator.simulate_oxidation(1000.0, 2.0, "dry")),
                    ("Polysilicon deposition", lambda: self.simulator.simulate_deposition("polysilicon", 0.3, 600.0)),
                    ("Gate patterning", lambda: self.simulator.simulate_lithography(193.0, 25.0)),
                    ("Source/Drain implantation", lambda: self.simulator.simulate_doping("phosphorus", 1e20, 80.0, 1000.0)),
                    ("Metal deposition", lambda: self.simulator.simulate_deposition("aluminum", 0.8, 400.0)),
                ]
                
                for i, (step_name, step_func) in enumerate(steps):
                    self.log_message(f"Step {i+1}: {step_name}")
                    step_func()
                    self.progress_var.set((i+1) / len(steps) * 100)
                    time.sleep(0.5)  # Simulate processing time
                
                self.log_message("MOSFET workflow completed successfully!")
                self.status_var.set("MOSFET workflow completed")
                self.update_results()
                
            except Exception as e:
                self.log_message(f"Workflow failed: {e}")
                messagebox.showerror("Error", f"Workflow failed: {e}")
            finally:
                self.simulation_running = False
                self.progress_var.set(0)
        
        threading.Thread(target=workflow, daemon=True).start()
        
    def run_cmos_workflow(self):
        """Run CMOS process workflow"""
        messagebox.showinfo("Info", "CMOS workflow not implemented yet")
        
    def run_memory_workflow(self):
        """Run memory cell workflow"""
        messagebox.showinfo("Info", "Memory workflow not implemented yet")
        
    def initialize_renderer(self):
        """Initialize Vulkan renderer"""
        if not self.simulator:
            messagebox.showwarning("Warning", "Please initialize wafer first")
            return
            
        try:
            self.simulator.initialize_renderer(800, 600)
            self.status_var.set("Vulkan renderer initialized")
            self.log_message("Vulkan renderer initialized")
            
        except Exception as e:
            messagebox.showerror("Error", f"Renderer initialization failed: {e}")
            
    def render_wafer(self):
        """Render wafer using Vulkan"""
        if not self.simulator:
            messagebox.showwarning("Warning", "Please initialize wafer first")
            return
            
        try:
            viz_type = self.viz_type_var.get()
            self.simulator.render(self.simulator.get_wafer())
            self.status_var.set(f"Rendered wafer: {viz_type}")
            self.log_message(f"Rendered wafer with {viz_type} visualization")
            
        except Exception as e:
            messagebox.showerror("Error", f"Rendering failed: {e}")
            
    def log_message(self, message):
        """Add message to workflow log"""
        self.workflow_log.config(state=tk.NORMAL)
        timestamp = time.strftime("%H:%M:%S")
        self.workflow_log.insert(tk.END, f"[{timestamp}] {message}\n")
        self.workflow_log.see(tk.END)
        self.workflow_log.config(state=tk.DISABLED)
        
    def update_results(self):
        """Update results display"""
        if not self.simulator or not hasattr(self.simulator, 'simulation_results'):
            return
            
        self.results_text.config(state=tk.NORMAL)
        self.results_text.delete(1.0, tk.END)
        
        self.results_text.insert(tk.END, "SemiPRO Simulation Results\n")
        self.results_text.insert(tk.END, "=" * 40 + "\n\n")
        
        for process, result in self.simulator.simulation_results.items():
            self.results_text.insert(tk.END, f"{process.upper()}:\n")
            for key, value in result.items():
                self.results_text.insert(tk.END, f"  {key}: {value}\n")
            self.results_text.insert(tk.END, "\n")
            
        self.results_text.config(state=tk.DISABLED)
        
    def export_results(self):
        """Export results to file"""
        if not self.simulator:
            messagebox.showwarning("Warning", "No simulation results to export")
            return
            
        filename = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")]
        )
        
        if filename:
            try:
                with open(filename, 'w') as f:
                    f.write(self.results_text.get(1.0, tk.END))
                messagebox.showinfo("Success", f"Results exported to {filename}")
            except Exception as e:
                messagebox.showerror("Error", f"Export failed: {e}")
                
    def clear_results(self):
        """Clear results display"""
        self.results_text.config(state=tk.NORMAL)
        self.results_text.delete(1.0, tk.END)
        self.results_text.config(state=tk.DISABLED)
        
    def run(self):
        """Start the GUI application"""
        self.root.mainloop()

def main():
    """Main entry point for GUI application"""
    app = SemiPROGUI()
    app.run()

if __name__ == "__main__":
    main()

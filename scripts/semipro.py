#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO - Semiconductor Process Simulator
Main entry point for the standalone application

Usage:
    python semipro.py                    # Launch GUI
    python semipro.py --cli              # Command-line interface
    python semipro.py --test             # Run tests
    python semipro.py --workflow <file>  # Run workflow from file
"""

import sys
import os
import argparse
import asyncio

# Add src to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'src'))

def launch_gui():
    """Launch the GUI application"""
    try:
        from src.python.gui import SemiPROGUI
        print("🚀 Launching SemiPRO GUI...")
        app = SemiPROGUI()
        app.run()
    except ImportError as e:
        print(f"❌ GUI not available: {e}")
        print("💡 Try installing tkinter: sudo apt install python3-tk")
        return False
    except Exception as e:
        print(f"❌ GUI launch failed: {e}")
        return False
    return True

def launch_cli():
    """Launch command-line interface"""
    try:
        from src.python.simulator import Simulator
        
        print("🔬 SemiPRO Command-Line Interface")
        print("Author: Dr. Mazharuddin Mohammed")
        print("=" * 50)
        
        # Create simulator
        sim = Simulator()
        print("✅ Simulator initialized")
        
        # Initialize wafer
        sim.initialize_geometry(100, 100)
        print("✅ Wafer initialized (100x100 grid)")
        
        # Interactive command loop
        print("\nAvailable commands:")
        print("  oxidation <temp> <time>     - Run oxidation")
        print("  doping <type> <conc> <energy> - Run doping")
        print("  deposition <material> <thickness> - Run deposition")
        print("  etching <depth> <type>      - Run etching")
        print("  mosfet                      - Run MOSFET workflow")
        print("  render                      - Render wafer")
        print("  results                     - Show results")
        print("  quit                        - Exit")
        print()
        
        while True:
            try:
                command = input("SemiPRO> ").strip().split()
                if not command:
                    continue
                    
                cmd = command[0].lower()
                
                if cmd == "quit" or cmd == "exit":
                    break
                elif cmd == "oxidation" and len(command) >= 3:
                    temp = float(command[1])
                    time_val = float(command[2])
                    sim.simulate_oxidation(temp, time_val)
                    print(f"✅ Oxidation completed: {temp}°C, {time_val}h")
                elif cmd == "doping" and len(command) >= 4:
                    dopant = command[1]
                    conc = float(command[2])
                    energy = float(command[3])
                    sim.simulate_doping(dopant, conc, energy, 1000.0)
                    print(f"✅ Doping completed: {dopant}, {conc:.2e} cm⁻³")
                elif cmd == "deposition" and len(command) >= 3:
                    material = command[1]
                    thickness = float(command[2])
                    sim.simulate_deposition(material, thickness, 300.0)
                    print(f"✅ Deposition completed: {material}, {thickness}μm")
                elif cmd == "etching" and len(command) >= 3:
                    depth = float(command[1])
                    etch_type = command[2]
                    sim.simulate_etching(depth, etch_type)
                    print(f"✅ Etching completed: {depth}μm, {etch_type}")
                elif cmd == "mosfet":
                    print("🔬 Running MOSFET workflow...")
                    run_mosfet_workflow(sim)
                elif cmd == "render":
                    sim.initialize_renderer(800, 600)
                    sim.render(sim.get_wafer())
                    print("✅ Rendering completed")
                elif cmd == "results":
                    show_results(sim)
                else:
                    print("❌ Unknown command or insufficient arguments")
                    
            except KeyboardInterrupt:
                print("\n👋 Goodbye!")
                break
            except Exception as e:
                print(f"❌ Error: {e}")
                
    except Exception as e:
        print(f"❌ CLI launch failed: {e}")
        return False
    return True

def run_mosfet_workflow(sim):
    """Run MOSFET fabrication workflow"""
    steps = [
        ("Gate oxide growth", lambda: sim.simulate_oxidation(1000.0, 2.0, "dry")),
        ("Polysilicon deposition", lambda: sim.simulate_deposition("polysilicon", 0.3, 600.0)),
        ("Gate patterning", lambda: sim.simulate_lithography(193.0, 25.0)),
        ("Source/Drain implantation", lambda: sim.simulate_doping("phosphorus", 1e20, 80.0, 1000.0)),
        ("Metal deposition", lambda: sim.simulate_deposition("aluminum", 0.8, 400.0)),
    ]
    
    for i, (step_name, step_func) in enumerate(steps):
        print(f"   {i+1}. {step_name}")
        step_func()
    
    print("🎉 MOSFET workflow completed!")

def show_results(sim):
    """Show simulation results"""
    if hasattr(sim, 'simulation_results') and sim.simulation_results:
        print("\n📊 Simulation Results:")
        print("-" * 30)
        for process, result in sim.simulation_results.items():
            print(f"{process.upper()}:")
            for key, value in result.items():
                print(f"  {key}: {value}")
            print()
    else:
        print("📊 No simulation results available")

def run_tests():
    """Run comprehensive tests"""
    try:
        import subprocess
        result = subprocess.run([sys.executable, "test_standalone_app.py"], 
                              capture_output=True, text=True)
        print(result.stdout)
        if result.stderr:
            print("Errors:", result.stderr)
        return result.returncode == 0
    except Exception as e:
        print(f"❌ Test execution failed: {e}")
        return False

async def run_workflow_file(workflow_file):
    """Run workflow from file"""
    try:
        from src.python.simulator import Simulator
        
        sim = Simulator()
        sim.initialize_geometry(100, 100)
        
        # Load and execute workflow
        sim.load_simulation_flow(workflow_file)
        result = await sim.execute_simulation_flow()
        
        if result:
            print(f"✅ Workflow {workflow_file} completed successfully")
        else:
            print(f"❌ Workflow {workflow_file} failed")
            
        return result
        
    except Exception as e:
        print(f"❌ Workflow execution failed: {e}")
        return False

def check_system():
    """Check system requirements"""
    print("🔍 Checking system requirements...")
    
    # Check C++ backend
    cpp_executables = [
        "./build/simulator",
        "./build/example_geometry",
        "./build/example_oxidation"
    ]
    
    cpp_available = all(os.path.exists(exe) for exe in cpp_executables)
    print(f"C++ Backend: {'✅' if cpp_available else '❌'}")
    
    # Check Python dependencies
    try:
        import numpy
        print(f"NumPy: ✅ {numpy.__version__}")
    except ImportError:
        print("NumPy: ❌ Not available")
    
    try:
        import matplotlib
        print(f"Matplotlib: ✅ {matplotlib.__version__}")
    except ImportError:
        print("Matplotlib: ❌ Not available")
    
    try:
        import tkinter
        print("Tkinter: ✅ Available")
    except ImportError:
        print("Tkinter: ❌ Not available")
    
    # Check Vulkan
    vulkan_available = os.system("vulkaninfo --summary > /dev/null 2>&1") == 0
    print(f"Vulkan: {'✅' if vulkan_available else '❌'}")
    
    print()
    return cpp_available

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="SemiPRO - Semiconductor Process Simulator",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python semipro.py                    # Launch GUI
  python semipro.py --cli              # Command-line interface
  python semipro.py --test             # Run tests
  python semipro.py --check            # Check system requirements
  python semipro.py --workflow flow.yaml  # Run workflow from file
        """
    )
    
    parser.add_argument("--cli", action="store_true", help="Launch command-line interface")
    parser.add_argument("--test", action="store_true", help="Run comprehensive tests")
    parser.add_argument("--check", action="store_true", help="Check system requirements")
    parser.add_argument("--workflow", type=str, help="Run workflow from file")
    parser.add_argument("--version", action="version", version="SemiPRO 1.0.0")
    
    args = parser.parse_args()
    
    # Print header
    print("🔬 SemiPRO - Semiconductor Process Simulator")
    print("Author: Dr. Mazharuddin Mohammed")
    print("Version: 1.0.0")
    print("=" * 60)
    
    # Handle arguments
    if args.check:
        return 0 if check_system() else 1
    elif args.test:
        return 0 if run_tests() else 1
    elif args.workflow:
        result = asyncio.run(run_workflow_file(args.workflow))
        return 0 if result else 1
    elif args.cli:
        return 0 if launch_cli() else 1
    else:
        # Default: launch GUI
        return 0 if launch_gui() else 1

if __name__ == "__main__":
    exit(main())

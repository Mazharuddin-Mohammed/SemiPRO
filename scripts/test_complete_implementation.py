# Author: Dr. Mazharuddin Mohammed
"""
Comprehensive test script for the complete SemiPRO implementation
Tests all modules including new multi-die, DRC, and advanced visualization features
"""

import sys
import os
sys.path.append('src/python')

def test_basic_simulation():
    """Test basic simulation functionality"""
    print("Testing basic simulation...")
    
    try:
        from simulator import Simulator
        
        # Create simulator
        sim = Simulator()
        
        # Test basic wafer operations
        sim.initialize_geometry(100, 100)
        sim.apply_layer(0.1, "oxide")
        
        # Test oxidation
        sim.simulate_oxidation(1000.0, 2.0, "dry")
        
        # Test doping
        sim.simulate_doping("boron", 1e16, 50.0, 1000.0)
        
        # Test photolithography
        sim.apply_photoresist(0.5)
        sim.expose_photoresist(365.0, 100.0)
        sim.develop_photoresist()
        
        # Test deposition
        sim.simulate_deposition("aluminum", 0.5, 300.0)
        
        # Test etching
        sim.simulate_etching(0.2, "isotropic")
        
        # Test metallization
        sim.add_metal_layer(0.3, "copper")
        
        # Test packaging
        sim.add_packaging(100.0, "ceramic", [])
        
        # Test thermal simulation
        sim.simulate_thermal(300.0, 1.0)
        
        print("✓ Basic simulation tests passed")
        return True
        
    except Exception as e:
        print(f"✗ Basic simulation test failed: {e}")
        return False

def test_multi_die_functionality():
    """Test multi-die integration functionality"""
    print("Testing multi-die functionality...")
    
    try:
        from simulator import Simulator
        from multi_die import DIE_TYPE_LOGIC, DIE_TYPE_MEMORY
        
        sim = Simulator()
        sim.initialize_geometry(100, 100)
        
        # Add dies
        die1 = sim.add_die("logic_die", DIE_TYPE_LOGIC, 10.0, 10.0, 0.775)
        die2 = sim.add_die("memory_die", DIE_TYPE_MEMORY, 8.0, 12.0, 0.775)
        
        # Position dies
        sim.position_die("logic_die", 0.0, 0.0)
        sim.position_die("memory_die", 15.0, 0.0)
        
        # Test wire bonding
        bonds = [((0, 0), (1, 1)), ((2, 2), (3, 3))]
        sim.perform_wire_bonding("logic_die", "memory_die", bonds)
        
        # Test flip chip bonding
        sim.perform_flip_chip_bonding("logic_die", "memory_die", 0.1, 0.05)
        
        # Test TSV integration
        tsv_positions = [(1.0, 1.0), (2.0, 2.0), (3.0, 3.0)]
        sim.perform_tsv_integration("logic_die", tsv_positions, 0.01, 0.1)
        
        # Test analysis
        sim.analyze_electrical_performance()
        sim.analyze_thermal_performance()
        
        # Get system metrics
        metrics = sim.get_system_metrics()
        print(f"System metrics: {metrics}")
        
        print("✓ Multi-die functionality tests passed")
        return True
        
    except Exception as e:
        print(f"✗ Multi-die functionality test failed: {e}")
        return False

def test_drc_functionality():
    """Test Design Rule Check functionality"""
    print("Testing DRC functionality...")
    
    try:
        from simulator import Simulator
        from drc import VIOLATION_TYPE_WIDTH, VIOLATION_TYPE_SPACING
        
        sim = Simulator()
        sim.initialize_geometry(100, 100)
        
        # Load technology rules
        sim.load_technology_rules("28nm")
        
        # Add custom rules
        sim.add_drc_rule("CUSTOM_WIDTH", VIOLATION_TYPE_WIDTH, "metal", 0.1)
        sim.add_drc_rule("CUSTOM_SPACING", VIOLATION_TYPE_SPACING, "metal", 0.1)
        
        # Apply some layers for DRC checking
        sim.apply_layer(0.1, "metal")
        
        # Run DRC checks
        sim.run_full_drc()
        
        # Get violation counts
        total_violations = sim.get_violation_count()
        critical_violations = sim.get_critical_violation_count()
        violations_by_layer = sim.get_violations_by_layer()
        
        print(f"Total violations: {total_violations}")
        print(f"Critical violations: {critical_violations}")
        print(f"Violations by layer: {violations_by_layer}")
        
        # Generate DRC report
        sim.generate_drc_report("test_drc_report.txt")
        
        print("✓ DRC functionality tests passed")
        return True
        
    except Exception as e:
        print(f"✗ DRC functionality test failed: {e}")
        return False

def test_advanced_visualization():
    """Test advanced visualization functionality"""
    print("Testing advanced visualization...")
    
    try:
        from simulator import Simulator
        from advanced_visualization import (RENDERING_MODE_PBR, LAYER_METAL, 
                                          LAYER_TEMPERATURE, LAYER_DOPANT)
        
        sim = Simulator()
        sim.initialize_geometry(100, 100)
        sim.apply_layer(0.1, "metal")
        
        # Set rendering mode
        sim.set_rendering_mode(RENDERING_MODE_PBR)
        
        # Configure layers
        sim.enable_layer(LAYER_METAL, True)
        sim.enable_layer(LAYER_TEMPERATURE, True)
        
        # Set camera
        sim.set_camera_position(5.0, 5.0, 5.0)
        sim.set_camera_target(0.0, 0.0, 0.0)
        sim.orbit_camera(45.0, 30.0, 10.0)
        
        # Add lighting
        sim.add_light([5.0, 5.0, 5.0], [1.0, 1.0, 1.0], 1.0)
        
        # Enable effects
        sim.enable_bloom(True, 0.8, 0.5)
        sim.set_rendering_quality(0.8)
        
        # Enable volumetric rendering
        sim.enable_volumetric_rendering(True)
        sim.render_temperature_field()
        sim.render_dopant_distribution()
        
        # Render scene
        sim.render_advanced()
        
        # Export results
        sim.export_image("test_render.png", 1920, 1080)
        sim.export_stl("test_wafer.stl")
        
        # Get performance metrics
        frame_rate = sim.get_frame_rate()
        render_time = sim.get_render_time()
        
        print(f"Frame rate: {frame_rate} FPS")
        print(f"Render time: {render_time} ms")
        
        print("✓ Advanced visualization tests passed")
        return True
        
    except Exception as e:
        print(f"✗ Advanced visualization test failed: {e}")
        return False

def test_enhanced_bindings():
    """Test enhanced Python bindings"""
    print("Testing enhanced Python bindings...")
    
    try:
        from enhanced_bindings import ProcessSimulator, EnhancedWafer
        
        # Create enhanced simulator
        sim = ProcessSimulator()
        
        # Create wafer
        wafer = sim.create_wafer("test_wafer", 200.0, 525.0)
        
        # Simulate process flow
        sim.simulate_oxidation("test_wafer", 1000.0, 2.0, "dry")
        sim.simulate_deposition("test_wafer", "Si3N4", 0.1, "LPCVD")
        sim.simulate_deposition("test_wafer", "Al", 0.5, "Sputtering")
        sim.simulate_etch("test_wafer", 0.2, "anisotropic")
        
        # Generate reports
        process_flow = sim.generate_process_flow("test_wafer")
        layer_stack = wafer.get_layer_stack()
        summary = sim.create_summary_report()
        
        print(f"Process steps: {len(process_flow)}")
        print(f"Layer count: {len(layer_stack)}")
        print(f"Total thickness: {wafer.get_total_thickness():.3f} μm")
        
        # Export results
        sim.export_results("test_results.json", "json")
        
        print("✓ Enhanced bindings tests passed")
        return True
        
    except Exception as e:
        print(f"✗ Enhanced bindings test failed: {e}")
        return False

def test_gui_components():
    """Test GUI components (basic instantiation)"""
    print("Testing GUI components...")
    
    try:
        # Test if GUI components can be imported and instantiated
        from PySide6.QtWidgets import QApplication
        from gui.main_window import MainWindow
        from gui.multi_die_panel import MultiDiePanel
        from gui.drc_panel import DRCPanel
        from gui.advanced_visualization_panel import AdvancedVisualizationPanel
        
        # Create QApplication (required for Qt widgets)
        app = QApplication.instance()
        if app is None:
            app = QApplication([])
        
        # Test panel instantiation
        multi_die_panel = MultiDiePanel()
        drc_panel = DRCPanel()
        viz_panel = AdvancedVisualizationPanel()
        
        # Test main window
        main_window = MainWindow()
        
        print("✓ GUI components tests passed")
        return True
        
    except Exception as e:
        print(f"✗ GUI components test failed: {e}")
        return False

def run_all_tests():
    """Run all tests and report results"""
    print("=" * 60)
    print("SemiPRO Complete Implementation Test Suite")
    print("Author: Dr. Mazharuddin Mohammed")
    print("=" * 60)
    
    tests = [
        ("Basic Simulation", test_basic_simulation),
        ("Multi-Die Functionality", test_multi_die_functionality),
        ("DRC Functionality", test_drc_functionality),
        ("Advanced Visualization", test_advanced_visualization),
        ("Enhanced Bindings", test_enhanced_bindings),
        ("GUI Components", test_gui_components),
    ]
    
    results = []
    
    for test_name, test_func in tests:
        print(f"\n{test_name}:")
        print("-" * 40)
        try:
            result = test_func()
            results.append((test_name, result))
        except Exception as e:
            print(f"✗ {test_name} failed with exception: {e}")
            results.append((test_name, False))
    
    # Summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    
    passed = 0
    total = len(results)
    
    for test_name, result in results:
        status = "PASS" if result else "FAIL"
        print(f"{test_name:<30} {status}")
        if result:
            passed += 1
    
    print("-" * 60)
    print(f"Total: {passed}/{total} tests passed ({passed/total*100:.1f}%)")
    
    if passed == total:
        print("🎉 All tests passed! Implementation is complete.")
    else:
        print("⚠️  Some tests failed. Check the implementation.")
    
    return passed == total

if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)

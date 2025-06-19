#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
# Debug simulator initialization

import sys
import os

# Add src to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'src'))

def debug_simulator():
    """Debug simulator step by step"""
    
    try:
        print("🔍 Debug Step 1: Import simulator module")
        from python.simulator import Simulator
        print("✅ Simulator module imported successfully")
        
        print("🔍 Debug Step 2: Create simulator instance")
        sim = Simulator()
        print("✅ Simulator instance created successfully")
        
        print("🔍 Debug Step 3: Test basic method")
        print(f"   Simulator type: {type(sim)}")
        print(f"   Available methods: {[m for m in dir(sim) if not m.startswith('_')][:10]}...")
        
        print("🔍 Debug Step 4: Initialize geometry")
        result = sim.initialize_geometry(50, 50)
        print(f"✅ Geometry initialized: {result}")
        
        print("🔍 Debug Step 5: Test oxidation")
        result = sim.simulate_oxidation(1000.0, 1.0, "dry")
        print(f"✅ Oxidation simulation: {result}")
        
        print("🔍 Debug Step 6: Test doping")
        result = sim.simulate_doping("boron", 1e16, 50.0, 1000.0)
        print(f"✅ Doping simulation: {result}")
        
        print("🔍 Debug Step 7: Test deposition")
        result = sim.simulate_deposition("aluminum", 0.5, 300.0)
        print(f"✅ Deposition simulation: {result}")
        
        print("🎉 All basic operations working!")
        return True
        
    except Exception as e:
        print(f"❌ Error at debug step: {e}")
        import traceback
        traceback.print_exc()
        return False

if __name__ == "__main__":
    success = debug_simulator()
    sys.exit(0 if success else 1)

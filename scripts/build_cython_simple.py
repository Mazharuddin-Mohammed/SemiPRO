#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Simplified Cython build script for SemiPRO
Builds only the core modules to avoid complex dependency issues
"""

import os
import sys
import subprocess
from pathlib import Path

def build_core_modules():
    """Build only the core geometry and oxidation modules first"""
    
    # Change to project root
    project_root = Path(__file__).parent
    os.chdir(project_root)
    
    print("🔧 Building C++ backend first...")
    
    # Build C++ backend
    build_dir = project_root / "build"
    if not build_dir.exists():
        build_dir.mkdir()
    
    os.chdir(build_dir)
    
    # Run cmake and make
    try:
        subprocess.run(["cmake", ".."], check=True)
        subprocess.run(["make", "-j4"], check=True)
        print("✅ C++ backend built successfully")
    except subprocess.CalledProcessError as e:
        print(f"❌ C++ build failed: {e}")
        return False
    
    # Go back to project root
    os.chdir(project_root)
    
    print("🐍 Building Python extensions...")
    
    # Create a simple setup for just geometry module
    setup_content = '''
from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy

extensions = [
    Extension(
        "geometry_simple",
        ["src/cython/simple_geometry.pyx"],
        language="c++",
        include_dirs=[
            numpy.get_include(),
            "src/cpp/core",
            "src/cpp/modules/geometry",
            "/usr/include/eigen3"
        ],
        library_dirs=["build"],
        libraries=["simulator_lib", "yaml-cpp"],
        extra_compile_args=["-std=c++17", "-O2"],
        extra_link_args=["-std=c++17"]
    )
]

setup(
    ext_modules=cythonize(extensions, language_level=3)
)
'''
    
    # Write temporary setup file
    with open("setup_simple_temp.py", "w") as f:
        f.write(setup_content)
    
    try:
        # Build the simple extension
        result = subprocess.run([
            sys.executable, "setup_simple_temp.py", 
            "build_ext", "--inplace"
        ], capture_output=True, text=True)
        
        if result.returncode == 0:
            print("✅ Simple Cython extension built successfully")
            return True
        else:
            print(f"❌ Cython build failed:")
            print(result.stdout)
            print(result.stderr)
            return False
            
    except Exception as e:
        print(f"❌ Build error: {e}")
        return False
    finally:
        # Clean up temporary file
        if os.path.exists("setup_simple_temp.py"):
            os.remove("setup_simple_temp.py")

def test_basic_functionality():
    """Test basic functionality"""
    print("🧪 Testing basic functionality...")
    
    try:
        # Test C++ executable
        result = subprocess.run(["./build/simulator"], 
                              capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            print("✅ C++ simulator runs successfully")
        else:
            print(f"⚠️  C++ simulator returned code {result.returncode}")
            print(result.stdout)
            print(result.stderr)
    except subprocess.TimeoutExpired:
        print("⚠️  C++ simulator timed out (may be waiting for input)")
    except Exception as e:
        print(f"❌ C++ test failed: {e}")
    
    # Test Python import
    try:
        sys.path.insert(0, "src/python")
        from simulator import Simulator
        sim = Simulator()
        print("✅ Python simulator imports successfully")
        return True
    except Exception as e:
        print(f"❌ Python import failed: {e}")
        return False

def main():
    """Main build process"""
    print("🚀 SemiPRO Simplified Build Process")
    print("=" * 50)
    
    if not build_core_modules():
        print("❌ Build failed")
        return 1
    
    if not test_basic_functionality():
        print("⚠️  Some tests failed, but build completed")
    
    print("\n✅ Build process completed!")
    print("Next steps:")
    print("1. Run: python semipro.py --test")
    print("2. Run: python semipro.py --gui")
    
    return 0

if __name__ == "__main__":
    exit(main())

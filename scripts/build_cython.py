#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Build script for SemiPRO Cython extensions
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

def check_dependencies():
    """Check if required dependencies are available"""
    try:
        import numpy
        print(f"✅ NumPy {numpy.__version__} found")
    except ImportError:
        print("❌ NumPy not found")
        return False
    
    try:
        import Cython
        print(f"✅ Cython {Cython.__version__} found")
    except ImportError:
        print("❌ Cython not found")
        return False
    
    return True

def build_cpp_library():
    """Build the C++ library first"""
    print("\n🔨 Building C++ library...")
    
    if not os.path.exists("build"):
        print("❌ C++ build directory not found. Please run 'make' first.")
        return False
    
    # Check if library exists
    lib_path = "build/libsimulator_lib.a"
    if not os.path.exists(lib_path):
        print("❌ C++ library not found. Building...")
        result = subprocess.run(["make", "-C", "build"], capture_output=True, text=True)
        if result.returncode != 0:
            print(f"❌ C++ build failed: {result.stderr}")
            return False
    
    print("✅ C++ library ready")
    return True

def build_single_extension(module_name):
    """Build a single Cython extension"""
    print(f"\n🔨 Building {module_name} extension...")
    
    pyx_file = f"src/cython/{module_name}.pyx"
    if not os.path.exists(pyx_file):
        print(f"❌ {pyx_file} not found")
        return False
    
    # Create a simple setup for this module
    setup_content = f'''
from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy
import os

# Get absolute paths
build_dir = os.path.abspath("build")
cpp_dir = os.path.abspath("src/cpp")

extension = Extension(
    "{module_name}",
    ["{pyx_file}", "src/cpp/core/wafer.cpp", "src/cpp/core/utils.cpp"],
    language="c++",
    include_dirs=[
        numpy.get_include(),
        "src/cpp/core",
        "src/cpp/modules/geometry",
        "/usr/include/eigen3",
    ],
    library_dirs=[build_dir],
    libraries=["yaml-cpp"],
    extra_compile_args=["-std=c++17", "-O3"],
    extra_link_args=["-std=c++17"],
)

setup(
    ext_modules=cythonize([extension], compiler_directives={{"language_level": "3"}}),
    zip_safe=False,
)
'''
    
    # Write temporary setup file
    setup_file = f"setup_{module_name}.py"
    with open(setup_file, 'w') as f:
        f.write(setup_content)
    
    try:
        # Build the extension
        result = subprocess.run([
            sys.executable, setup_file, "build_ext", "--inplace"
        ], capture_output=True, text=True, cwd=".")
        
        if result.returncode == 0:
            print(f"✅ {module_name} extension built successfully")
            return True
        else:
            print(f"❌ {module_name} extension build failed:")
            print(result.stdout)
            print(result.stderr)
            return False
    finally:
        # Clean up
        if os.path.exists(setup_file):
            os.remove(setup_file)
    
def test_extension(module_name):
    """Test if the extension can be imported"""
    try:
        # Add current directory to path
        sys.path.insert(0, '.')
        
        module = __import__(module_name)
        print(f"✅ {module_name} extension imports successfully")
        
        # Try to create objects if possible
        if hasattr(module, 'PyWafer'):
            wafer = module.PyWafer(100.0, 0.5, "silicon")
            print(f"✅ {module_name}.PyWafer created successfully")
        
        return True
    except Exception as e:
        print(f"❌ {module_name} extension test failed: {e}")
        return False

def main():
    """Main build process"""
    print("🔬 SemiPRO Cython Extension Builder")
    print("Author: Dr. Mazharuddin Mohammed")
    print("=" * 50)
    
    # Check dependencies
    if not check_dependencies():
        print("❌ Missing dependencies. Please install numpy and cython.")
        return 1
    
    # Build C++ library
    if not build_cpp_library():
        print("❌ C++ library build failed.")
        return 1
    
    # List of extensions to build
    extensions = [
        "geometry",
        # "oxidation",
        # "doping", 
        # "deposition",
        # "etching",
        # "renderer"
    ]
    
    built_extensions = []
    failed_extensions = []
    
    # Build each extension
    for ext in extensions:
        if build_single_extension(ext):
            if test_extension(ext):
                built_extensions.append(ext)
            else:
                failed_extensions.append(ext)
        else:
            failed_extensions.append(ext)
    
    # Summary
    print("\n" + "=" * 50)
    print("BUILD SUMMARY")
    print("=" * 50)
    
    if built_extensions:
        print("✅ Successfully built extensions:")
        for ext in built_extensions:
            print(f"   - {ext}")
    
    if failed_extensions:
        print("❌ Failed to build extensions:")
        for ext in failed_extensions:
            print(f"   - {ext}")
    
    success_rate = len(built_extensions) / len(extensions) * 100
    print(f"\nSuccess rate: {len(built_extensions)}/{len(extensions)} ({success_rate:.1f}%)")
    
    if len(built_extensions) > 0:
        print("\n🎉 At least some extensions built successfully!")
        print("You can now use the Python frontend with C++ backend integration.")
        return 0
    else:
        print("\n❌ No extensions built successfully.")
        return 1

if __name__ == "__main__":
    exit(main())

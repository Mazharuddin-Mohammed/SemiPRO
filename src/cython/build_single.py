# Author: Dr. Mazharuddin Mohammed
# Simple build script for individual Cython modules

import sys
import os
from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy

def build_module(module_name):
    """Build a single Cython module"""
    
    # Get the absolute path to the build directory
    build_dir = os.path.abspath("../../build")
    cpp_dir = os.path.abspath("../cpp")
    
    # Common settings
    common_include_dirs = [
        numpy.get_include(),
        os.path.join(cpp_dir, "core"),
        "/usr/include/eigen3",
    ]
    
    common_library_dirs = [build_dir]
    common_libraries = ["simulator_lib", "yaml-cpp"]
    common_extra_compile_args = ["-std=c++17", "-O3"]
    common_extra_link_args = ["-std=c++17"]
    
    # Module-specific settings
    module_configs = {
        "geometry": {
            "include_dirs": [os.path.join(cpp_dir, "modules/geometry")],
            "libraries": common_libraries,
        },
        "oxidation": {
            "include_dirs": [os.path.join(cpp_dir, "modules/oxidation")],
            "libraries": common_libraries,
        },
        "doping": {
            "include_dirs": [os.path.join(cpp_dir, "modules/doping")],
            "libraries": common_libraries,
        },
        "advanced_visualization": {
            "include_dirs": [os.path.join(cpp_dir, "modules/advanced_visualization")],
            "libraries": common_libraries + ["vulkan", "glfw"],
        },
    }
    
    if module_name not in module_configs:
        print(f"Unknown module: {module_name}")
        return False
    
    config = module_configs[module_name]
    
    extension = Extension(
        module_name,
        [f"{module_name}.pyx"],
        language="c++",
        include_dirs=common_include_dirs + config["include_dirs"],
        library_dirs=common_library_dirs,
        libraries=config["libraries"],
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    )
    
    try:
        setup(
            ext_modules=cythonize([extension]),
            zip_safe=False,
        )
        print(f"Successfully built {module_name}")
        return True
    except Exception as e:
        print(f"Failed to build {module_name}: {e}")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python build_single.py <module_name>")
        sys.exit(1)
    
    module_name = sys.argv[1]
    success = build_module(module_name)
    sys.exit(0 if success else 1)

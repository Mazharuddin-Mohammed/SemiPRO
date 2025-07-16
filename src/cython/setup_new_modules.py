#!/usr/bin/env python3
"""
Setup script for building only the new Cython modules
"""

from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy
import os

# Get the absolute path to the build directory
build_dir = os.path.abspath("../../build")
cpp_dir = os.path.abspath("../cpp")

# Common settings for all extensions
common_include_dirs = [
    numpy.get_include(),
    os.path.join(cpp_dir, "core"),
    "/usr/include/eigen3",
]

common_library_dirs = [build_dir]
common_libraries = ["simulator_lib", "yaml-cpp"]
common_extra_compile_args = ["-std=c++17", "-O3"]
common_extra_link_args = ["-std=c++17"]

# Build only our new modules that don't depend on geometry
extensions = [
    Extension(
        "defect_inspection",
        ["defect_inspection.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/defect_inspection")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "interconnect",
        ["interconnect.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/interconnect")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "metrology",
        ["metrology.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/metrology")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
]

if __name__ == "__main__":
    setup(
        name="semipro_new_modules",
        ext_modules=cythonize(extensions),
    )

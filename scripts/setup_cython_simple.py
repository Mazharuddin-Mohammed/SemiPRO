#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Simple Cython setup for SemiPRO geometry module
"""

from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy
import os

# Get absolute paths
project_root = os.path.abspath(".")
cpp_core = os.path.join(project_root, "src", "cpp", "core")
cpp_geometry = os.path.join(project_root, "src", "cpp", "modules", "geometry")

# Define the extension
extensions = [
    Extension(
        "geometry",
        [
            "src/cython/geometry.pyx",
            "src/cpp/core/wafer.cpp",
            "src/cpp/core/utils.cpp",
        ],
        language="c++",
        include_dirs=[
            numpy.get_include(),
            cpp_core,
            cpp_geometry,
            "/usr/include/eigen3",
        ],
        extra_compile_args=["-std=c++17", "-O3", "-DWITH_CYTHON"],
        extra_link_args=["-std=c++17"],
    )
]

setup(
    name="SemiPRO-Cython",
    ext_modules=cythonize(extensions, compiler_directives={"language_level": "3"}),
    zip_safe=False,
)

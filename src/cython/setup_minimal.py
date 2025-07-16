#!/usr/bin/env python3
"""
Minimal setup script for testing Cython integration
"""

from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy

# Simple extension without C++ dependencies
extensions = [
    Extension(
        "minimal_test",
        ["minimal_test.pyx"],
        language="c++",
        include_dirs=[numpy.get_include()],
        extra_compile_args=["-std=c++17"],
        extra_link_args=["-std=c++17"],
    ),
]

if __name__ == "__main__":
    setup(
        name="semipro_minimal_test",
        ext_modules=cythonize(extensions),
    )

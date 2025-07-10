#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Simple working Cython setup for SemiPRO
"""

from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy

# Define the extension
extensions = [
    Extension(
        "simple_geometry",
        ["src/cython/simple_geometry.pyx"],
        language="c++",
        include_dirs=[numpy.get_include()],
        extra_compile_args=["-std=c++17", "-O3"],
        extra_link_args=["-std=c++17"],
    )
]

setup(
    name="SemiPRO-Simple",
    ext_modules=cythonize(extensions, compiler_directives={"language_level": "3"}),
    zip_safe=False,
)

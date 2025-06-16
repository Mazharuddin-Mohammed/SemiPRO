from setuptools import setup, setup
from Cython.Build import cythonize
import numpy

extensions = [
    Extension(
        "geometry",
        ["geometry.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/geometry"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "oxidation",
        ["oxidation.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/oxidation"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "doping",
        ["doping.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/doping"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "photolithography",
        ["photolithography.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/photolithography"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "deposition",
        ["deposition.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/deposition"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "etching",
        ["etching.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/etching"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "metallization",
        ["metallization.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/metallization"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "packaging",
        ["packaging.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/packaging"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "thermal",
        ["thermal.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/thermal"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "reliability",
        ["reliability.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/modules/reliability"],
        extra_compile_args=["-std=c++17"],
    ),
    Extension(
        "renderer",
        ["renderer.pyx"],
        language="c++",
        include_dirs=[numpy.get_include(), "../cpp/core", "../cpp/renderer"],
        extra_compile_args=["-std=c++17"],
        libraries=["vulkan", "glfw"],
    ),
]

setup(
    ext_modules=cythonize(extensions),
)
# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO: Advanced Semiconductor Process Simulator
Setup script for building and installing the complete package
"""

from setuptools import setup, find_packages, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
from pybind11 import get_cmake_dir
import pybind11
import numpy
import os
import sys
from pathlib import Path

# Version information
__version__ = "1.0.0"

# Project root directory
PROJECT_ROOT = Path(__file__).parent

# Source directories
CPP_SRC_DIR = PROJECT_ROOT / "src" / "cpp"
PYTHON_SRC_DIR = PROJECT_ROOT / "src" / "python"
CYTHON_SRC_DIR = PROJECT_ROOT / "src" / "cython"

# Include directories
INCLUDE_DIRS = [
    str(CPP_SRC_DIR / "core"),
    str(CPP_SRC_DIR / "modules"),
    str(CPP_SRC_DIR / "renderer"),
    str(CPP_SRC_DIR / "modules" / "geometry"),
    str(CPP_SRC_DIR / "modules" / "oxidation"),
    str(CPP_SRC_DIR / "modules" / "doping"),
    str(CPP_SRC_DIR / "modules" / "lithography"),
    str(CPP_SRC_DIR / "modules" / "deposition"),
    str(CPP_SRC_DIR / "modules" / "etching"),
    str(CPP_SRC_DIR / "modules" / "metallization"),
    str(CPP_SRC_DIR / "modules" / "packaging"),
    str(CPP_SRC_DIR / "modules" / "thermal"),
    str(CPP_SRC_DIR / "modules" / "reliability"),
    str(CPP_SRC_DIR / "modules" / "multi_die"),
    str(CPP_SRC_DIR / "modules" / "design_rule_check"),
    str(CPP_SRC_DIR / "modules" / "advanced_visualization"),
    str(CPP_SRC_DIR / "modules" / "metrology"),
    str(CPP_SRC_DIR / "modules" / "interconnect"),
    str(CPP_SRC_DIR / "modules" / "defect_inspection"),
    numpy.get_include(),
    pybind11.get_include()
]

# Library directories
LIBRARY_DIRS = [
    "/usr/lib",
    "/usr/local/lib",
    "/opt/local/lib"
]

# Libraries to link
LIBRARIES = [
    "yaml-cpp",
    "vulkan",
    "glfw",
    "pthread"
]

# Compiler flags
COMPILE_ARGS = [
    "-std=c++17",
    "-O3",
    "-Wall",
    "-Wextra",
    "-fPIC",
    "-DWITH_PYTHON_BINDINGS",
    "-DEIGEN_USE_MKL_ALL" if "MKL_ROOT" in os.environ else "",
]

# Remove empty flags
COMPILE_ARGS = [arg for arg in COMPILE_ARGS if arg]

# Linker flags
LINK_ARGS = [
    "-Wl,-rpath,$ORIGIN",
    "-Wl,--enable-new-dtags"
]

# Define C++ source files for each module
def get_cpp_sources(module_name):
    """Get C++ source files for a specific module"""
    module_dir = CPP_SRC_DIR / "modules" / module_name
    sources = []
    
    if module_dir.exists():
        for cpp_file in module_dir.glob("*.cpp"):
            sources.append(str(cpp_file))
    
    return sources

# Core C++ sources
CORE_SOURCES = [
    str(CPP_SRC_DIR / "core" / "wafer.cpp"),
    str(CPP_SRC_DIR / "core" / "utils.cpp"),
    str(CPP_SRC_DIR / "core" / "simulation_orchestrator.cpp"),
    str(CPP_SRC_DIR / "core" / "input_parser.cpp"),
    str(CPP_SRC_DIR / "core" / "output_generator.cpp"),
]

# Renderer sources
RENDERER_SOURCES = [
    str(CPP_SRC_DIR / "renderer" / "vulkan_renderer.cpp"),
]

# Module sources
MODULE_SOURCES = {
    "geometry": get_cpp_sources("geometry"),
    "oxidation": get_cpp_sources("oxidation"),
    "doping": get_cpp_sources("doping"),
    "lithography": get_cpp_sources("lithography"),
    "deposition": get_cpp_sources("deposition"),
    "etching": get_cpp_sources("etching"),
    "metallization": get_cpp_sources("metallization"),
    "packaging": get_cpp_sources("packaging"),
    "thermal": get_cpp_sources("thermal"),
    "reliability": get_cpp_sources("reliability"),
    "multi_die": get_cpp_sources("multi_die"),
    "design_rule_check": get_cpp_sources("design_rule_check"),
    "advanced_visualization": get_cpp_sources("advanced_visualization"),
    "metrology": get_cpp_sources("metrology"),
    "interconnect": get_cpp_sources("interconnect"),
    "defect_inspection": get_cpp_sources("defect_inspection"),
}

# Check if we're building for documentation
BUILDING_DOCS = os.environ.get('READTHEDOCS') == 'True' or 'sphinx' in sys.argv

# Create extensions
extensions = []

if not BUILDING_DOCS:
    # Core extension
    core_sources = CORE_SOURCES + RENDERER_SOURCES
    for module_name, sources in MODULE_SOURCES.items():
        core_sources.extend(sources)

    # Main SemiPRO extension
    semipro_ext = Pybind11Extension(
        "semipro_core",
        sources=core_sources,
        include_dirs=INCLUDE_DIRS,
        library_dirs=LIBRARY_DIRS,
        libraries=LIBRARIES,
        language="c++",
        cxx_std=17,
    )

    extensions.append(semipro_ext)

    # Individual module extensions for Cython
    cython_extensions = []

    try:
        from Cython.Build import cythonize
        from Cython.Distutils import build_ext as cython_build_ext

        # Cython extensions
        cython_files = list(CYTHON_SRC_DIR.glob("*.pyx"))

        for pyx_file in cython_files:
            module_name = pyx_file.stem

            ext = Extension(
                f"semipro.{module_name}",
                sources=[str(pyx_file)],
                include_dirs=INCLUDE_DIRS,
                library_dirs=LIBRARY_DIRS,
                libraries=LIBRARIES,
                extra_compile_args=COMPILE_ARGS,
                extra_link_args=LINK_ARGS,
                language="c++"
            )
            cython_extensions.append(ext)

        # Cythonize extensions
        if cython_extensions:
            cython_extensions = cythonize(
                cython_extensions,
                compiler_directives={
                    "language_level": 3,
                    "embedsignature": True,
                    "boundscheck": False,
                    "wraparound": False,
                    "cdivision": True,
                }
            )
            extensions.extend(cython_extensions)

    except ImportError:
        print("Warning: Cython not available. Skipping Cython extensions.")
else:
    print("Building for documentation - skipping C++ extensions")

# Read long description from README
long_description = ""
readme_file = PROJECT_ROOT / "README.md"
if readme_file.exists():
    with open(readme_file, "r", encoding="utf-8") as f:
        long_description = f.read()

# Read requirements
requirements = []
requirements_file = PROJECT_ROOT / "requirements.txt"
if requirements_file.exists():
    with open(requirements_file, "r") as f:
        requirements = [line.strip() for line in f if line.strip() and not line.startswith("#")]

# Development requirements
dev_requirements = [
    "pytest>=6.0",
    "pytest-cov>=2.0",
    "black>=21.0",
    "flake8>=3.8",
    "mypy>=0.800",
    "sphinx>=4.0",
    "sphinx-rtd-theme>=1.0",
    "cython>=0.29",
    "pybind11>=2.6",
]

# Documentation requirements
docs_requirements = [
    "sphinx>=4.0",
    "sphinx-rtd-theme>=1.0",
    "sphinx-copybutton>=0.4",
    "myst-parser>=0.17",
    "sphinx-autodoc-typehints>=1.12",
]

# Setup configuration
setup(
    name="semipro",
    version=__version__,
    author="Dr. Mazharuddin Mohammed",
    author_email="mazhar@example.com",
    description="Advanced Semiconductor Process Simulator",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/your-repo/SemiPRO",
    project_urls={
        "Documentation": "https://semipro.readthedocs.io",
        "Source": "https://github.com/your-repo/SemiPRO",
        "Tracker": "https://github.com/your-repo/SemiPRO/issues",
    },
    packages=find_packages(where="src/python"),
    package_dir={"": "src/python"},
    ext_modules=extensions,
    cmdclass={"build_ext": build_ext},
    python_requires=">=3.8",
    install_requires=requirements,
    extras_require={
        "dev": dev_requirements,
        "docs": docs_requirements,
        "all": dev_requirements + docs_requirements,
    },
    classifiers=[
        "Development Status :: 4 - Beta",
        "Intended Audience :: Science/Research",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: POSIX :: Linux",
        "Operating System :: MacOS :: MacOS X",
        "Operating System :: Microsoft :: Windows",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: C++",
        "Topic :: Scientific/Engineering",
        "Topic :: Scientific/Engineering :: Physics",
        "Topic :: Scientific/Engineering :: Electronic Design Automation (EDA)",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
    keywords="semiconductor simulation fabrication process modeling physics",
    include_package_data=True,
    package_data={
        "semipro": [
            "data/*.yaml",
            "data/*.json",
            "shaders/*.vert",
            "shaders/*.frag",
            "shaders/*.comp",
            "templates/*.html",
            "templates/*.tex",
        ]
    },
    entry_points={
        "console_scripts": [
            "semipro=semipro.cli:main",
            "semipro-gui=semipro.gui.main_window:main",
            "semipro-orchestrator=semipro.orchestrator:main",
        ],
    },
    zip_safe=False,
    platforms=["Linux", "macOS", "Windows"],
    license="MIT",
)

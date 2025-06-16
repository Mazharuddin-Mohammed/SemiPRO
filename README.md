# Semiconductor Simulator

A modular simulator for semiconductor fabrication processes, including geometry, oxidation, doping, photolithography, deposition, etching, metallization, packaging, and thermal simulation.

## Features
- **Geometry**: Initialize wafer with grid-based properties.
- **Oxidation**: Simulate thermal oxidation (SiO2 growth).
- **Doping**: Model ion implantation and diffusion.
- **Photolithography**: Simulate exposure and multi-patterning.
- **Deposition**: Uniform and conformal film deposition.
- **Etching**: Anisotropic and isotropic etching.
- **Metallization**: Deposit metal layers (e.g., Cu, Al).
- **Packaging**: Die/wire bonding and electrical testing (resistance, capacitance).
- **Thermal Simulation**: Model heat distribution from Joule heating in interconnects.
- **Reliability Testing**: Simulate electromigration, thermal stress, and dielectric breakdown.
- **Visualization**: Vulkan-based rendering of wafer layers, electrical properties, and temperature heatmap.

## Prerequisites
- C++17 compiler (e.g., g++)
- CMake 3.10+
- Eigen3
- Vulkan SDK
- GLFW3
- Python 3.8+
- Python packages: numpy, PySide6, Cython, pytest, sphinx

## Installation
```bash
# Install system dependencies (Ubuntu)
sudo apt-get update
sudo apt-get install -y g++ cmake libeigen3-dev libvulkan-dev libglfw3-dev python3-dev python3-pip

# Install Python dependencies
pip3 install -r requirements.txt
pip3 install -r docs/requirements.txt
pip3 install -r tests/requirements.txt

# Build C++ backend
mkdir build && cd build
cmake ..
make

# Build Cython bindings
cd src/cython
python3 setup.py build_ext --inplace
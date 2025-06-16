Installation Guide
==================

**Author: Dr. Mazharuddin Mohammed**

This guide provides detailed instructions for installing SemiPRO on various platforms.

System Requirements
-------------------

Hardware Requirements
~~~~~~~~~~~~~~~~~~~~~

**Minimum Requirements:**
- CPU: Intel Core i5 or AMD Ryzen 5 (4 cores)
- RAM: 8 GB
- Storage: 5 GB free space
- GPU: OpenGL 3.3 compatible (for basic visualization)

**Recommended Requirements:**
- CPU: Intel Core i7 or AMD Ryzen 7 (8+ cores)
- RAM: 16 GB or more
- Storage: 20 GB free space (SSD recommended)
- GPU: Vulkan-compatible GPU with 4+ GB VRAM

**For Advanced Features:**
- GPU: NVIDIA RTX series or AMD RDNA2 (for ray tracing)
- RAM: 32 GB (for large simulations)
- Storage: NVMe SSD (for fast I/O)

Software Requirements
~~~~~~~~~~~~~~~~~~~~

**Operating Systems:**
- Ubuntu 20.04 LTS or later
- Debian 11 or later
- CentOS 8 or later
- Windows 10/11 (with WSL2)
- macOS 11.0 or later

**Compilers:**
- GCC 9.0+ or Clang 10.0+
- CMake 3.16+
- Python 3.8+

**Graphics:**
- Vulkan SDK 1.2+
- OpenGL 4.5+ drivers

Dependencies
------------

Core Dependencies
~~~~~~~~~~~~~~~~

**C++ Libraries:**
- Eigen3 (3.3+) - Linear algebra
- yaml-cpp (0.6+) - Configuration files
- Vulkan SDK - Graphics rendering
- GLFW3 - Window management

**Python Dependencies:**
- NumPy (1.19+) - Numerical computing
- Cython (0.29+) - Python-C++ bindings
- PySide6 (6.0+) - GUI framework
- Matplotlib (3.3+) - Plotting
- Pandas (1.3+) - Data analysis

Installation Methods
--------------------

Method 1: Automated Installation (Recommended)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Ubuntu/Debian:**

.. code-block:: bash

   # Update system
   sudo apt update && sudo apt upgrade -y

   # Install system dependencies
   sudo apt install -y cmake g++ python3-dev python3-pip git
   sudo apt install -y libeigen3-dev libvulkan-dev libglfw3-dev
   sudo apt install -y libyaml-cpp-dev vulkan-tools glslc

   # Install Python dependencies
   pip3 install --user numpy cython pyside6 matplotlib pandas pyyaml

   # Clone and build SemiPRO
   git clone https://github.com/your-repo/SemiPRO.git
   cd SemiPRO
   python3 scripts/build_complete.py

**CentOS/RHEL:**

.. code-block:: bash

   # Enable EPEL repository
   sudo dnf install -y epel-release

   # Install system dependencies
   sudo dnf groupinstall -y "Development Tools"
   sudo dnf install -y cmake python3-devel python3-pip git
   sudo dnf install -y eigen3-devel vulkan-devel glfw-devel
   sudo dnf install -y yaml-cpp-devel vulkan-tools glslc

   # Install Python dependencies
   pip3 install --user numpy cython pyside6 matplotlib pandas pyyaml

   # Clone and build SemiPRO
   git clone https://github.com/your-repo/SemiPRO.git
   cd SemiPRO
   python3 scripts/build_complete.py

**Windows (WSL2):**

.. code-block:: bash

   # Install WSL2 with Ubuntu
   wsl --install -d Ubuntu

   # Follow Ubuntu instructions above
   # Note: GUI requires X11 forwarding or VcXsrv

**macOS:**

.. code-block:: bash

   # Install Homebrew
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

   # Install dependencies
   brew install cmake eigen yaml-cpp glfw3 vulkan-headers
   brew install python@3.9

   # Install Python dependencies
   pip3 install numpy cython pyside6 matplotlib pandas pyyaml

   # Clone and build SemiPRO
   git clone https://github.com/your-repo/SemiPRO.git
   cd SemiPRO
   python3 scripts/build_complete.py

Method 2: Manual Installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Step 1: Install System Dependencies**

Follow the package installation commands for your platform above.

**Step 2: Clone Repository**

.. code-block:: bash

   git clone https://github.com/your-repo/SemiPRO.git
   cd SemiPRO

**Step 3: Build C++ Core**

.. code-block:: bash

   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)
   cd ..

**Step 4: Compile Shaders**

.. code-block:: bash

   ./compile_shaders.sh

**Step 5: Build Cython Extensions**

.. code-block:: bash

   cd src/cython
   python3 setup.py build_ext --inplace
   cd ../..

**Step 6: Install Python Package**

.. code-block:: bash

   pip3 install -e .

Method 3: Docker Installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Using Docker:**

.. code-block:: bash

   # Pull the official image
   docker pull semipro/semipro:latest

   # Run with GUI support (Linux)
   docker run -it --rm \
     -e DISPLAY=$DISPLAY \
     -v /tmp/.X11-unix:/tmp/.X11-unix \
     -v $(pwd):/workspace \
     semipro/semipro:latest

   # Run without GUI (headless)
   docker run -it --rm \
     -v $(pwd):/workspace \
     semipro/semipro:latest

**Building from Dockerfile:**

.. code-block:: bash

   git clone https://github.com/your-repo/SemiPRO.git
   cd SemiPRO
   docker build -t semipro:local .

Verification
------------

Test Installation
~~~~~~~~~~~~~~~~

**Basic Test:**

.. code-block:: bash

   # Run the test suite
   python3 scripts/test_complete_implementation.py

**Quick Verification:**

.. code-block:: python

   from src.python.simulator import Simulator

   # Create simulator
   sim = Simulator()
   sim.initialize_geometry(50, 50)
   sim.simulate_oxidation(1000.0, 1.0, "dry")

   print("✓ SemiPRO installation verified!")

**GUI Test:**

.. code-block:: bash

   # Launch the GUI
   python3 -m src.python.gui.main_window

Performance Benchmarks
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Run performance benchmarks
   python3 tests/benchmark_suite.py

Expected output:
- Geometry initialization: < 100 ms
- Oxidation simulation: < 500 ms
- Visualization rendering: > 30 FPS

Troubleshooting
---------------

Common Issues
~~~~~~~~~~~~

**Issue: CMake cannot find Eigen3**

.. code-block:: bash

   # Solution: Install Eigen3 development package
   sudo apt install libeigen3-dev

   # Or specify path manually
   cmake .. -DEigen3_DIR=/usr/share/eigen3/cmake

**Issue: Vulkan not found**

.. code-block:: bash

   # Solution: Install Vulkan SDK
   sudo apt install vulkan-sdk vulkan-tools

   # Verify installation
   vulkan-info

**Issue: Python import errors**

.. code-block:: bash

   # Solution: Ensure Python path is set
   export PYTHONPATH=$PYTHONPATH:$(pwd)/src/python

   # Rebuild Cython extensions
   cd src/cython
   python3 setup.py build_ext --inplace --force

**Issue: GUI not displaying**

.. code-block:: bash

   # Solution: Install Qt dependencies
   sudo apt install python3-pyside6

   # For WSL2, install X11 server
   # Download and run VcXsrv on Windows

**Issue: Compilation errors**

.. code-block:: bash

   # Solution: Update compiler
   sudo apt install gcc-9 g++-9

   # Set as default
   sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
   sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90

**Issue: Out of memory during compilation**

.. code-block:: bash

   # Solution: Reduce parallel jobs
   make -j2  # Instead of make -j$(nproc)

   # Or increase swap space
   sudo fallocate -l 4G /swapfile
   sudo chmod 600 /swapfile
   sudo mkswap /swapfile
   sudo swapon /swapfile

Performance Optimization
~~~~~~~~~~~~~~~~~~~~~~~~

**For better performance:**

.. code-block:: bash

   # Enable optimizations
   cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_OPTIMIZATIONS=ON

   # Use Clang for better optimization
   cmake .. -DCMAKE_CXX_COMPILER=clang++

   # Enable parallel processing
   export OMP_NUM_THREADS=$(nproc)

**GPU Acceleration:**

.. code-block:: bash

   # Ensure GPU drivers are installed
   nvidia-smi  # For NVIDIA
   rocm-smi    # For AMD

   # Enable GPU acceleration in SemiPRO
   export SEMIPRO_USE_GPU=1

Development Setup
-----------------

For Developers
~~~~~~~~~~~~~

**Additional Dependencies:**

.. code-block:: bash

   # Development tools
   sudo apt install gdb valgrind doxygen graphviz

   # Python development
   pip3 install pytest sphinx sphinx-rtd-theme black flake8

   # Pre-commit hooks
   pip3 install pre-commit
   pre-commit install

**IDE Setup:**

**Visual Studio Code:**

.. code-block:: json

   {
     "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools",
     "python.defaultInterpreterPath": "/usr/bin/python3",
     "python.linting.enabled": true,
     "python.linting.flake8Enabled": true
   }

**CLion:**

1. Open project directory
2. Set CMake profile to Release
3. Configure Python interpreter
4. Enable code formatting

Next Steps
----------

After successful installation:

1. **Read the Quick Start Guide**: :doc:`../user_guide/quickstart`
2. **Try the Tutorials**: :doc:`../tutorials/basic_simulation`
3. **Explore Examples**: Check the ``examples/`` directory
4. **Join the Community**: Visit our GitHub discussions

For support, please visit our `GitHub Issues <https://github.com/your-repo/SemiPRO/issues>`_ page.

Quick Start Guide
=================

**Author: Dr. Mazharuddin Mohammed**

Welcome to SemiPRO! This guide will get you up and running with semiconductor process simulation in just a few minutes.

Installation
------------

**Quick Install (Ubuntu/Debian):**

.. code-block:: bash

   # Install dependencies
   sudo apt update
   sudo apt install cmake g++ python3-dev python3-pip
   sudo apt install libeigen3-dev libvulkan-dev libglfw3-dev libyaml-cpp-dev
   pip install numpy cython pyside6 matplotlib pandas pyyaml

   # Clone and build
   git clone https://github.com/your-repo/SemiPRO.git
   cd SemiPRO
   python scripts/build_complete.py

For detailed installation instructions, see :doc:`../installation/index`.

Your First Simulation
---------------------

Let's create a simple oxidation simulation:

.. code-block:: python

   from src.python.simulator import Simulator

   # Create simulator
   sim = Simulator()
   print("SemiPRO initialized!")

   # Initialize wafer geometry
   sim.initialize_geometry(100, 100)  # 100x100 grid
   print("Wafer geometry created")

   # Simulate thermal oxidation
   sim.simulate_oxidation(
       temperature=1000.0,  # 1000°C
       time=2.0,           # 2 hours
       atmosphere="dry"    # Dry oxidation
   )
   print("Oxidation simulation completed")

   # Get results
   thickness = sim.get_oxide_thickness()
   print(f"Oxide thickness: {thickness:.2f} nm")

**Expected Output:**

.. code-block:: text

   SemiPRO initialized!
   Wafer geometry created
   Oxidation simulation completed
   Oxide thickness: 45.23 nm

GUI Quick Start
--------------

Launch the graphical interface:

.. code-block:: bash

   python -m src.python.gui.main_window

**GUI Features:**

1. **Geometry Panel**: Initialize wafer and set parameters
2. **Process Panels**: Oxidation, doping, lithography, etc.
3. **Multi-Die Panel**: Advanced system integration
4. **DRC Panel**: Design rule checking
5. **Visualization Panel**: 3D rendering and analysis

Basic Process Flow
-----------------

Here's a complete NMOS transistor simulation:

.. code-block:: python

   from src.python.simulator import Simulator

   # Initialize
   sim = Simulator()
   sim.initialize_geometry(200, 200)

   # Gate oxidation
   sim.simulate_oxidation(1000.0, 0.5, "dry")  # Thin gate oxide

   # Gate definition (simplified)
   sim.apply_photoresist(0.5)
   sim.expose_photoresist(193.0, 25.0)  # ArF lithography
   sim.develop_photoresist()
   sim.simulate_etching(0.02, "anisotropic")  # Etch oxide
   sim.remove_photoresist()

   # Source/drain implantation
   sim.simulate_doping("phosphorus", 1e20, 30.0, 1000.0)

   # Activation anneal
   sim.simulate_thermal_annealing(1000.0, 0.05, "N2")

   # Metallization
   sim.simulate_deposition("aluminum", 0.5, 300.0)

   print("NMOS transistor simulation completed!")

Multi-Die Integration
--------------------

Create a heterogeneous system:

.. code-block:: python

   from src.cython.multi_die import PyMultiDieModel, DIE_TYPE_LOGIC, DIE_TYPE_MEMORY

   # Create multi-die system
   system = PyMultiDieModel()

   # Add CPU die
   cpu = system.add_die("cpu", DIE_TYPE_LOGIC, 10.0, 10.0, 0.775)
   
   # Add memory die
   ram = system.add_die("ram", DIE_TYPE_MEMORY, 8.0, 12.0, 0.775)

   # Position and integrate
   system.position_die("cpu", 0.0, 0.0)
   system.position_die("ram", 12.0, 0.0)
   system.perform_flip_chip_bonding("cpu", "ram", 0.1, 0.05)

   # Analyze performance
   system.analyze_electrical_performance()
   metrics = system.get_system_metrics()
   print(f"System performance: {metrics}")

Design Rule Check
----------------

Verify your design meets manufacturing requirements:

.. code-block:: python

   from src.cython.drc import PyDRCModel, VIOLATION_TYPE_WIDTH, TECH_NODE_28NM

   # Create DRC engine
   drc = PyDRCModel()
   drc.load_technology_rules("28nm")
   drc.set_technology_node(TECH_NODE_28NM)

   # Add custom rules
   drc.add_rule("MIN_GATE_WIDTH", VIOLATION_TYPE_WIDTH, "polysilicon", 0.028)

   # Run DRC
   drc.run_full_drc()

   # Check results
   violations = drc.get_violation_count()
   print(f"DRC violations found: {violations}")

Advanced Visualization
---------------------

Create stunning 3D visualizations:

.. code-block:: python

   from src.cython.advanced_visualization import PyAdvancedVisualizationModel
   from src.cython.advanced_visualization import RENDERING_MODE_PBR

   # Create visualization
   viz = PyAdvancedVisualizationModel()
   viz.set_rendering_mode(RENDERING_MODE_PBR)
   viz.set_camera_position(10.0, 10.0, 5.0)

   # Add lighting
   viz.add_light([15.0, 15.0, 10.0], [1.0, 1.0, 1.0], 2.0)

   # Enable effects
   viz.enable_bloom(True, 0.8, 0.3)
   viz.set_rendering_quality(0.9)

   # Render and export
   viz.render_advanced()
   viz.export_image("wafer_render.png", 1920, 1080)

Common Parameters
----------------

**Temperature Units:** Celsius (°C)
**Time Units:** Hours (h)
**Length Units:** Micrometers (μm)
**Concentration Units:** cm⁻³
**Energy Units:** keV (for implantation)

**Typical Values:**

- **Oxidation Temperature:** 900-1200°C
- **Oxidation Time:** 0.1-10 hours
- **Implant Energy:** 10-200 keV
- **Implant Dose:** 1e12-1e16 cm⁻²
- **Anneal Temperature:** 800-1100°C
- **Metal Thickness:** 0.1-2.0 μm

Tips for Success
---------------

1. **Start Simple:** Begin with basic processes before advanced features
2. **Check Parameters:** Verify physical parameters are reasonable
3. **Use GUI:** The graphical interface is great for learning
4. **Read Logs:** Check console output for warnings and errors
5. **Save Work:** Export results and configurations regularly

Performance Tips
---------------

**For Faster Simulations:**

.. code-block:: python

   # Reduce grid resolution for testing
   sim.initialize_geometry(50, 50)  # Instead of 200x200

   # Enable parallel processing
   sim.enable_parallel_processing(True)
   sim.set_thread_count(8)

   # Use GPU acceleration
   sim.enable_gpu_acceleration(True)

**For Better Accuracy:**

.. code-block:: python

   # Increase grid resolution
   sim.initialize_geometry(500, 500)

   # Use adaptive meshing
   sim.enable_adaptive_meshing(True)

   # Increase solver precision
   sim.set_solver_tolerance(1e-8)

Troubleshooting
--------------

**Common Issues:**

1. **Import Error:** Check Python path and Cython compilation
2. **Slow Performance:** Reduce grid size or enable parallelization
3. **Memory Error:** Use smaller simulation domains
4. **Visualization Issues:** Check GPU drivers and Vulkan support

**Getting Help:**

- Check the :doc:`../installation/index` guide
- Read the :doc:`../tutorials/basic_simulation` tutorial
- Visit our `GitHub Issues <https://github.com/your-repo/SemiPRO/issues>`_
- Join our community discussions

Next Steps
----------

Now that you've completed the quick start:

1. **Try the Full Tutorial:** :doc:`../tutorials/basic_simulation`
2. **Explore Examples:** Check the ``examples/`` directory
3. **Read the API Reference:** :doc:`../api/python_api`
4. **Learn the Theory:** :doc:`../theory/semiconductor_physics`

**Advanced Topics:**

- :doc:`../tutorials/advanced_features`
- :doc:`../user_guide/multi_die_integration`
- :doc:`../user_guide/design_rule_check`
- :doc:`../user_guide/advanced_visualization`

Welcome to the SemiPRO community! 🚀

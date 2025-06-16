Python API Reference
====================

**Author: Dr. Mazharuddin Mohammed**

This section provides comprehensive documentation for the SemiPRO Python API.

Core Simulator
--------------

.. automodule:: src.python.simulator
   :members:
   :undoc-members:
   :show-inheritance:

Simulator Class
~~~~~~~~~~~~~~~

.. autoclass:: src.python.simulator.Simulator
   :members:
   :undoc-members:
   :show-inheritance:

   The main simulator class that provides access to all SemiPRO functionality.

   **Example Usage:**

   .. code-block:: python

      from src.python.simulator import Simulator

      # Create simulator instance
      sim = Simulator()

      # Initialize wafer geometry
      sim.initialize_geometry(100, 100)

      # Run basic process flow
      sim.simulate_oxidation(1000.0, 2.0, "dry")
      sim.simulate_doping("boron", 1e16, 50.0, 1000.0)

   **Key Methods:**

   .. automethod:: initialize_geometry
   .. automethod:: simulate_oxidation
   .. automethod:: simulate_doping
   .. automethod:: apply_photoresist
   .. automethod:: expose_photoresist
   .. automethod:: develop_photoresist
   .. automethod:: simulate_deposition
   .. automethod:: simulate_etching

Multi-Die Integration
--------------------

.. automodule:: src.cython.multi_die
   :members:
   :undoc-members:
   :show-inheritance:

PyMultiDieModel Class
~~~~~~~~~~~~~~~~~~~~

.. autoclass:: src.cython.multi_die.PyMultiDieModel
   :members:
   :undoc-members:
   :show-inheritance:

   Advanced multi-die integration and system-level modeling.

   **Die Types:**

   - ``DIE_TYPE_LOGIC``: Logic processing dies (CPUs, GPUs)
   - ``DIE_TYPE_MEMORY``: Memory dies (DRAM, SRAM, Flash)
   - ``DIE_TYPE_ANALOG``: Analog/mixed-signal dies
   - ``DIE_TYPE_RF``: Radio frequency dies
   - ``DIE_TYPE_SENSOR``: Sensor dies (MEMS, image sensors)
   - ``DIE_TYPE_POWER``: Power management dies

   **Integration Methods:**

   - ``INTEGRATION_WIRE_BONDING``: Traditional wire bonding
   - ``INTEGRATION_FLIP_CHIP``: Flip-chip bonding
   - ``INTEGRATION_TSV``: Through-silicon vias
   - ``INTEGRATION_WAFER_LEVEL``: Wafer-level packaging
   - ``INTEGRATION_CHIPLET``: Advanced chiplet integration

   **Example Usage:**

   .. code-block:: python

      from src.cython.multi_die import PyMultiDieModel, DIE_TYPE_LOGIC, DIE_TYPE_MEMORY

      # Create multi-die model
      model = PyMultiDieModel()

      # Add dies
      cpu_die = model.add_die("cpu", DIE_TYPE_LOGIC, 10.0, 10.0, 0.775)
      ram_die = model.add_die("ram", DIE_TYPE_MEMORY, 8.0, 12.0, 0.775)

      # Position dies
      model.position_die("cpu", 0.0, 0.0)
      model.position_die("ram", 15.0, 0.0)

      # Perform integration
      model.perform_flip_chip_bonding("cpu", "ram", 0.1, 0.05)

      # Analyze performance
      model.analyze_electrical_performance()
      metrics = model.get_system_metrics()

PyDie Class
~~~~~~~~~~

.. autoclass:: src.cython.multi_die.PyDie
   :members:
   :undoc-members:
   :show-inheritance:

   Individual die representation with properties and capabilities.

PyInterconnect Class
~~~~~~~~~~~~~~~~~~~

.. autoclass:: src.cython.multi_die.PyInterconnect
   :members:
   :undoc-members:
   :show-inheritance:

   Interconnect modeling for multi-die systems.

Design Rule Check
-----------------

.. automodule:: src.cython.drc
   :members:
   :undoc-members:
   :show-inheritance:

PyDRCModel Class
~~~~~~~~~~~~~~~

.. autoclass:: src.cython.drc.PyDRCModel
   :members:
   :undoc-members:
   :show-inheritance:

   Professional design rule checking engine.

   **Violation Types:**

   - ``VIOLATION_TYPE_WIDTH``: Minimum width violations
   - ``VIOLATION_TYPE_SPACING``: Minimum spacing violations
   - ``VIOLATION_TYPE_AREA``: Minimum area violations
   - ``VIOLATION_TYPE_DENSITY``: Density violations
   - ``VIOLATION_TYPE_ANTENNA_RATIO``: Antenna ratio violations
   - ``VIOLATION_TYPE_VIA_ENCLOSURE``: Via enclosure violations
   - ``VIOLATION_TYPE_OVERLAP``: Overlap violations
   - ``VIOLATION_TYPE_EXTENSION``: Extension violations
   - ``VIOLATION_TYPE_NOTCH``: Notch violations
   - ``VIOLATION_TYPE_CORNER``: Corner violations

   **Technology Nodes:**

   - ``TECH_NODE_7NM``: 7nm technology
   - ``TECH_NODE_14NM``: 14nm technology
   - ``TECH_NODE_28NM``: 28nm technology
   - ``TECH_NODE_65NM``: 65nm technology
   - ``TECH_NODE_130NM``: 130nm technology
   - ``TECH_NODE_250NM``: 250nm technology

   **Example Usage:**

   .. code-block:: python

      from src.cython.drc import PyDRCModel, VIOLATION_TYPE_WIDTH, TECH_NODE_28NM

      # Create DRC model
      drc = PyDRCModel()

      # Load technology
      drc.load_technology_rules("28nm")
      drc.set_technology_node(TECH_NODE_28NM)

      # Add custom rules
      drc.add_rule("MIN_METAL_WIDTH", VIOLATION_TYPE_WIDTH, "metal", 0.1)

      # Run DRC
      drc.run_full_drc()

      # Check results
      violations = drc.get_violation_count()
      critical = drc.get_critical_violation_count()

PyDRCRule Class
~~~~~~~~~~~~~~

.. autoclass:: src.cython.drc.PyDRCRule
   :members:
   :undoc-members:
   :show-inheritance:

   Individual design rule representation.

PyDRCViolation Class
~~~~~~~~~~~~~~~~~~~

.. autoclass:: src.cython.drc.PyDRCViolation
   :members:
   :undoc-members:
   :show-inheritance:

   Design rule violation with location and severity information.

Advanced Visualization
---------------------

.. automodule:: src.cython.advanced_visualization
   :members:
   :undoc-members:
   :show-inheritance:

PyAdvancedVisualizationModel Class
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. autoclass:: src.cython.advanced_visualization.PyAdvancedVisualizationModel
   :members:
   :undoc-members:
   :show-inheritance:

   Advanced 3D visualization with Vulkan backend.

   **Rendering Modes:**

   - ``RENDERING_MODE_WIREFRAME``: Wireframe rendering
   - ``RENDERING_MODE_SOLID``: Solid color rendering
   - ``RENDERING_MODE_TRANSPARENT``: Transparent rendering
   - ``RENDERING_MODE_PBR``: Physically-based rendering
   - ``RENDERING_MODE_RAY_TRACED``: Ray traced rendering
   - ``RENDERING_MODE_VOLUMETRIC``: Volumetric rendering

   **Layer Types:**

   - ``LAYER_GEOMETRY``: Geometric layers
   - ``LAYER_DOPING``: Doping concentration
   - ``LAYER_TEMPERATURE``: Temperature distribution
   - ``LAYER_STRESS``: Mechanical stress
   - ``LAYER_ELECTRIC_FIELD``: Electric field
   - ``LAYER_CURRENT_DENSITY``: Current density

   **Example Usage:**

   .. code-block:: python

      from src.cython.advanced_visualization import PyAdvancedVisualizationModel
      from src.cython.advanced_visualization import RENDERING_MODE_PBR, LAYER_TEMPERATURE

      # Create visualization model
      viz = PyAdvancedVisualizationModel()

      # Set up rendering
      viz.set_rendering_mode(RENDERING_MODE_PBR)
      viz.set_camera_position(5.0, 5.0, 5.0)
      viz.add_light([10.0, 10.0, 10.0], [1.0, 1.0, 1.0], 1.0)

      # Enable effects
      viz.enable_bloom(True, 0.8, 0.5)
      viz.set_rendering_quality(0.9)

      # Volumetric rendering
      viz.enable_volumetric_rendering(True)
      viz.render_layer(LAYER_TEMPERATURE)

      # Export
      viz.export_image("render.png", 1920, 1080)

PyRenderingParameters Class
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. autoclass:: src.cython.advanced_visualization.PyRenderingParameters
   :members:
   :undoc-members:
   :show-inheritance:

   Rendering configuration parameters.

PyCameraParameters Class
~~~~~~~~~~~~~~~~~~~~~~~

.. autoclass:: src.cython.advanced_visualization.PyCameraParameters
   :members:
   :undoc-members:
   :show-inheritance:

   Camera configuration and control.

Enhanced Bindings
-----------------

.. automodule:: src.python.enhanced_bindings
   :members:
   :undoc-members:
   :show-inheritance:

ProcessSimulator Class
~~~~~~~~~~~~~~~~~~~~~

.. autoclass:: src.python.enhanced_bindings.ProcessSimulator
   :members:
   :undoc-members:
   :show-inheritance:

   Enhanced process simulation interface with additional functionality.

Wafer Class
~~~~~~~~~~

.. autoclass:: src.python.enhanced_bindings.Wafer
   :members:
   :undoc-members:
   :show-inheritance:

   Enhanced wafer representation with visualization capabilities.

GUI Components
--------------

Main Window
~~~~~~~~~~

.. automodule:: src.python.gui.main_window
   :members:
   :undoc-members:
   :show-inheritance:

Geometry Panel
~~~~~~~~~~~~~

.. automodule:: src.python.gui.geometry_panel
   :members:
   :undoc-members:
   :show-inheritance:

Multi-Die Panel
~~~~~~~~~~~~~~

.. automodule:: src.python.gui.multi_die_panel
   :members:
   :undoc-members:
   :show-inheritance:

DRC Panel
~~~~~~~~

.. automodule:: src.python.gui.drc_panel
   :members:
   :undoc-members:
   :show-inheritance:

Advanced Visualization Panel
~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. automodule:: src.python.gui.advanced_visualization_panel
   :members:
   :undoc-members:
   :show-inheritance:

Constants and Enumerations
--------------------------

Die Types
~~~~~~~~

.. code-block:: python

   DIE_TYPE_LOGIC = 0      # Logic processing dies
   DIE_TYPE_MEMORY = 1     # Memory dies
   DIE_TYPE_ANALOG = 2     # Analog/mixed-signal dies
   DIE_TYPE_RF = 3         # Radio frequency dies
   DIE_TYPE_SENSOR = 4     # Sensor dies
   DIE_TYPE_POWER = 5      # Power management dies

Integration Methods
~~~~~~~~~~~~~~~~~~

.. code-block:: python

   INTEGRATION_WIRE_BONDING = 0    # Wire bonding
   INTEGRATION_FLIP_CHIP = 1       # Flip-chip bonding
   INTEGRATION_TSV = 2             # Through-silicon vias
   INTEGRATION_WAFER_LEVEL = 3     # Wafer-level packaging
   INTEGRATION_CHIPLET = 4         # Chiplet integration

Violation Types
~~~~~~~~~~~~~~

.. code-block:: python

   VIOLATION_TYPE_WIDTH = 0           # Width violations
   VIOLATION_TYPE_SPACING = 1         # Spacing violations
   VIOLATION_TYPE_AREA = 2            # Area violations
   VIOLATION_TYPE_DENSITY = 3         # Density violations
   VIOLATION_TYPE_ANTENNA_RATIO = 4   # Antenna ratio violations

Technology Nodes
~~~~~~~~~~~~~~~

.. code-block:: python

   TECH_NODE_7NM = 0      # 7nm technology
   TECH_NODE_14NM = 1     # 14nm technology
   TECH_NODE_28NM = 2     # 28nm technology
   TECH_NODE_65NM = 3     # 65nm technology
   TECH_NODE_130NM = 4    # 130nm technology
   TECH_NODE_250NM = 5    # 250nm technology

Rendering Modes
~~~~~~~~~~~~~~

.. code-block:: python

   RENDERING_MODE_WIREFRAME = 0     # Wireframe rendering
   RENDERING_MODE_SOLID = 1         # Solid rendering
   RENDERING_MODE_TRANSPARENT = 2   # Transparent rendering
   RENDERING_MODE_PBR = 3           # Physically-based rendering
   RENDERING_MODE_RAY_TRACED = 4    # Ray traced rendering
   RENDERING_MODE_VOLUMETRIC = 5    # Volumetric rendering

Error Handling
--------------

SemiPRO uses standard Python exceptions for error handling:

.. code-block:: python

   try:
       sim = Simulator()
       sim.initialize_geometry(100, 100)
       sim.simulate_oxidation(1000.0, 2.0, "dry")
   except ValueError as e:
       print(f"Invalid parameter: {e}")
   except RuntimeError as e:
       print(f"Simulation error: {e}")
   except Exception as e:
       print(f"Unexpected error: {e}")

Common exceptions:
- ``ValueError``: Invalid parameters or configuration
- ``RuntimeError``: Simulation or computation errors
- ``FileNotFoundError``: Missing configuration or data files
- ``MemoryError``: Insufficient memory for large simulations

SemiPRO REST API Guide
======================

Author: Dr. Mazharuddin Mohammed

The SemiPRO REST API provides a comprehensive interface for managing semiconductor process simulations. This guide covers all available endpoints, request/response formats, and usage examples.

Base URL
--------

The default API base URL is::

    http://localhost:5000

All endpoints are relative to this base URL.

Authentication
--------------

Currently, the API supports optional API key authentication. Include the API key in the Authorization header::

    Authorization: Bearer YOUR_API_KEY

Health Check
------------

Check API Health
~~~~~~~~~~~~~~~~

**GET** ``/health``

Returns the current health status of the API server.

**Response:**

.. code-block:: json

    {
        "status": "healthy",
        "timestamp": "2024-06-16T18:00:00.000Z",
        "version": "1.0.0"
    }

Simulator Management
--------------------

Create Simulator
~~~~~~~~~~~~~~~~

**POST** ``/simulators``

Creates a new simulator instance.

**Request Body:**

.. code-block:: json

    {
        "config_file": "path/to/config.yaml",
        "input_directory": "input",
        "output_directory": "output"
    }

**Response:**

.. code-block:: json

    {
        "simulator_id": "uuid-string",
        "status": "created",
        "config": {
            "input_directory": "input",
            "output_directory": "output"
        }
    }

Get Simulator Info
~~~~~~~~~~~~~~~~~~

**GET** ``/simulators/{simulator_id}``

Retrieves information about a specific simulator.

**Response:**

.. code-block:: json

    {
        "simulator_id": "uuid-string",
        "input_directory": "input",
        "output_directory": "output",
        "status": "active"
    }

Delete Simulator
~~~~~~~~~~~~~~~~

**DELETE** ``/simulators/{simulator_id}``

Deletes a simulator instance and cancels any running simulations.

**Response:**

.. code-block:: json

    {
        "status": "deleted"
    }

Wafer Management
----------------

Create Wafer
~~~~~~~~~~~~

**POST** ``/simulators/{simulator_id}/wafers``

Creates a new wafer for simulation.

**Request Body:**

.. code-block:: json

    {
        "diameter": 300.0,
        "thickness": 775.0,
        "material": "silicon",
        "orientation": "(100)",
        "resistivity": 10.0,
        "doping_type": "p-type",
        "width": 100,
        "height": 100
    }

**Response:**

.. code-block:: json

    {
        "wafer": {
            "diameter": 300.0,
            "thickness": 775.0,
            "material": "silicon",
            "geometry": {
                "width": 100,
                "height": 100
            }
        },
        "status": "created"
    }

Process Step Management
-----------------------

Add Process Step
~~~~~~~~~~~~~~~~

**POST** ``/simulators/{simulator_id}/steps``

Adds a process step to the simulation flow.

**Request Body:**

.. code-block:: json

    {
        "type": "oxidation",
        "name": "gate_oxidation",
        "parameters": {
            "temperature": 1000.0,
            "time": 0.5,
            "atmosphere": "dry"
        },
        "input_files": [],
        "output_files": [],
        "dependencies": [],
        "estimated_duration": 30.0,
        "priority": 1,
        "parallel_compatible": false
    }

**Process Types:**

- ``oxidation``: Thermal oxidation
- ``doping``: Ion implantation and diffusion
- ``lithography``: Photolithography
- ``deposition``: Material deposition
- ``etching``: Material removal
- ``metallization``: Metal layer formation
- ``annealing``: Thermal treatment
- ``cmp``: Chemical mechanical polishing
- ``inspection``: Metrology and inspection
- ``custom``: User-defined process

**Response:**

.. code-block:: json

    {
        "step": {
            "type": "oxidation",
            "name": "gate_oxidation",
            "parameters": {
                "temperature": 1000.0,
                "time": 0.5,
                "atmosphere": "dry"
            }
        },
        "status": "added"
    }

Simulation Execution
--------------------

Start Simulation
~~~~~~~~~~~~~~~~

**POST** ``/simulators/{simulator_id}/simulate``

Starts simulation execution.

**Request Body:**

.. code-block:: json

    {
        "wafer_name": "test_wafer"
    }

**Response:**

.. code-block:: json

    {
        "task_id": "uuid-string",
        "status": "started",
        "wafer_name": "test_wafer"
    }

Get Simulation Status
~~~~~~~~~~~~~~~~~~~~~

**GET** ``/simulations/{task_id}/status``

Retrieves the current status and progress of a simulation.

**Response:**

.. code-block:: json

    {
        "task_id": "uuid-string",
        "status": "running",
        "progress": {
            "state": "running",
            "current_step": 2,
            "total_steps": 5,
            "progress_percentage": 40.0,
            "current_operation": "Oxidation simulation",
            "completed_steps": ["step1", "step2"],
            "errors": []
        },
        "elapsed_time": 120.5
    }

**Status Values:**

- ``running``: Simulation in progress
- ``completed``: Simulation finished successfully
- ``failed``: Simulation failed with error
- ``cancelled``: Simulation was cancelled

Cancel Simulation
~~~~~~~~~~~~~~~~~

**POST** ``/simulations/{task_id}/cancel``

Cancels a running simulation.

**Response:**

.. code-block:: json

    {
        "task_id": "uuid-string",
        "status": "cancelled"
    }

Results and Data Access
-----------------------

Get Results
~~~~~~~~~~~

**GET** ``/simulators/{simulator_id}/results``

Retrieves simulation results.

**Response:**

.. code-block:: json

    {
        "results": {
            "simulation_id": "uuid-string",
            "wafer_id": "wafer-id",
            "flow_name": "process_flow",
            "start_time": "2024-06-16T18:00:00.000Z",
            "end_time": "2024-06-16T18:05:00.000Z",
            "duration": 300.0,
            "status": "completed",
            "steps_completed": ["step1", "step2", "step3"],
            "steps_failed": [],
            "metrics": {
                "total_execution_time": 300.0,
                "successful_steps": 3,
                "failed_steps": 0
            },
            "outputs": {
                "temperature_profile": {
                    "type": "numpy_array",
                    "shape": [100, 100],
                    "data": "base64-encoded-data"
                }
            }
        },
        "timestamp": "2024-06-16T18:05:00.000Z"
    }

Export Data
~~~~~~~~~~~

**POST** ``/simulators/{simulator_id}/export``

Exports simulation data in various formats.

**Request Body:**

.. code-block:: json

    {
        "format": "json",
        "data_type": "results"
    }

**Supported Formats:**

- ``json``: JSON format
- ``csv``: Comma-separated values
- ``yaml``: YAML format
- ``hdf5``: HDF5 binary format
- ``vtk``: VTK format for visualization

**Response:**

Returns the exported file as a binary download.

Configuration Management
------------------------

Get Configuration
~~~~~~~~~~~~~~~~~

**GET** ``/simulators/{simulator_id}/config``

Retrieves the current simulator configuration.

**Response:**

.. code-block:: json

    {
        "config": {
            "input_directory": "input",
            "output_directory": "output",
            "solver": {
                "tolerance": 1e-8,
                "max_iterations": 1000
            }
        }
    }

Update Configuration
~~~~~~~~~~~~~~~~~~~~

**PUT** ``/simulators/{simulator_id}/config``

Updates simulator configuration.

**Request Body:**

.. code-block:: json

    {
        "input_directory": "new_input",
        "output_directory": "new_output",
        "solver": {
            "tolerance": 1e-9
        }
    }

**Response:**

.. code-block:: json

    {
        "status": "updated",
        "config": {
            "input_directory": "new_input",
            "output_directory": "new_output"
        }
    }

Error Handling
--------------

The API uses standard HTTP status codes:

- ``200 OK``: Request successful
- ``201 Created``: Resource created successfully
- ``202 Accepted``: Request accepted (async operation)
- ``400 Bad Request``: Invalid request data
- ``404 Not Found``: Resource not found
- ``429 Too Many Requests``: Rate limit exceeded
- ``500 Internal Server Error``: Server error

Error responses include details:

.. code-block:: json

    {
        "error": "Invalid wafer configuration: diameter must be positive",
        "code": "VALIDATION_ERROR",
        "details": {
            "field": "diameter",
            "value": -100,
            "constraint": "positive"
        }
    }

Rate Limiting
-------------

The API implements rate limiting:

- 200 requests per day per IP
- 50 requests per hour per IP
- 10 simulator creations per minute per IP
- 5 simulation starts per minute per IP

Rate limit headers are included in responses:

.. code-block:: http

    X-RateLimit-Limit: 50
    X-RateLimit-Remaining: 45
    X-RateLimit-Reset: 1624723200

Python Client Usage
-------------------

The SemiPRO Python client provides a high-level interface:

.. code-block:: python

    from src.python.api.client import SemiPROClient

    # Create client
    client = SemiPROClient(base_url="http://localhost:5000")

    # Create simulator
    simulator_id = client.create_simulator()

    # Create wafer
    wafer_config = {
        'diameter': 300.0,
        'thickness': 775.0,
        'material': 'silicon',
        'width': 100,
        'height': 100
    }
    client.create_wafer(simulator_id, wafer_config)

    # Add process steps
    client.add_oxidation_step(simulator_id, 'gate_oxide', 1000.0, 0.5)
    client.add_doping_step(simulator_id, 'source_drain', 'phosphorus', 1e20, 80.0, 1000.0)

    # Run simulation
    task_id = client.start_simulation(simulator_id)
    results = client.wait_for_simulation(task_id)

    # Get results
    final_results = client.get_results(simulator_id)

Complete MOSFET Example
-----------------------

.. code-block:: python

    # Run complete MOSFET fabrication
    results = client.run_mosfet_simulation(
        gate_length=0.25,
        gate_oxide_thickness=0.005,
        source_drain_depth=0.2
    )

WebSocket API
-------------

For real-time updates, the API also provides WebSocket endpoints:

- ``ws://localhost:5000/ws/simulation/{task_id}`` - Real-time simulation progress
- ``ws://localhost:5000/ws/status`` - General status updates

.. code-block:: javascript

    const ws = new WebSocket('ws://localhost:5000/ws/simulation/task-id');
    ws.onmessage = function(event) {
        const progress = JSON.parse(event.data);
        console.log(`Progress: ${progress.percentage}%`);
    };

This comprehensive API enables full control over semiconductor process simulations with support for complex workflows, real-time monitoring, and flexible data export options.

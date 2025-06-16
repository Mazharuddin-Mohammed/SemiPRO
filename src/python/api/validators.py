# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO API Validators
=====================

Comprehensive validation for API inputs and data structures.
Ensures data integrity and parameter validity for simulation operations.
"""

import re
from typing import Dict, List, Any, Optional, Union
from jsonschema import validate, ValidationError as JsonSchemaError

class ValidationError(Exception):
    """Custom validation error"""
    pass

# JSON Schema definitions
WAFER_SCHEMA = {
    "type": "object",
    "properties": {
        "diameter": {
            "type": "number",
            "minimum": 25.0,
            "maximum": 450.0,
            "description": "Wafer diameter in mm"
        },
        "thickness": {
            "type": "number",
            "minimum": 100.0,
            "maximum": 1500.0,
            "description": "Wafer thickness in micrometers"
        },
        "material": {
            "type": "string",
            "enum": ["silicon", "germanium", "gaas", "inp", "gan", "sic"],
            "description": "Wafer material"
        },
        "orientation": {
            "type": "string",
            "pattern": "^\\([0-9]{3}\\)$",
            "description": "Crystal orientation (e.g., (100), (111))"
        },
        "resistivity": {
            "type": "number",
            "minimum": 0.001,
            "maximum": 10000.0,
            "description": "Resistivity in ohm-cm"
        },
        "doping_type": {
            "type": "string",
            "enum": ["p-type", "n-type", "intrinsic"],
            "description": "Doping type"
        },
        "width": {
            "type": "integer",
            "minimum": 10,
            "maximum": 2000,
            "description": "Grid width in points"
        },
        "height": {
            "type": "integer",
            "minimum": 10,
            "maximum": 2000,
            "description": "Grid height in points"
        }
    },
    "required": ["diameter", "thickness", "material", "width", "height"],
    "additionalProperties": True
}

PROCESS_STEP_SCHEMA = {
    "type": "object",
    "properties": {
        "type": {
            "type": "string",
            "enum": ["oxidation", "doping", "lithography", "deposition", 
                    "etching", "metallization", "annealing", "cmp", "inspection", "custom"],
            "description": "Process step type"
        },
        "name": {
            "type": "string",
            "minLength": 1,
            "maxLength": 100,
            "pattern": "^[a-zA-Z0-9_-]+$",
            "description": "Process step name"
        },
        "parameters": {
            "type": "object",
            "description": "Process parameters"
        },
        "input_files": {
            "type": "array",
            "items": {"type": "string"},
            "description": "Input file paths"
        },
        "output_files": {
            "type": "array",
            "items": {"type": "string"},
            "description": "Output file paths"
        },
        "dependencies": {
            "type": "array",
            "items": {"type": "string"},
            "description": "Step dependencies"
        },
        "estimated_duration": {
            "type": "number",
            "minimum": 0,
            "description": "Estimated duration in minutes"
        },
        "priority": {
            "type": "integer",
            "minimum": 0,
            "maximum": 10,
            "description": "Step priority (0-10)"
        },
        "parallel_compatible": {
            "type": "boolean",
            "description": "Can run in parallel with other steps"
        }
    },
    "required": ["type", "name"],
    "additionalProperties": True
}

SIMULATION_FLOW_SCHEMA = {
    "type": "object",
    "properties": {
        "name": {
            "type": "string",
            "minLength": 1,
            "maxLength": 100,
            "description": "Flow name"
        },
        "description": {
            "type": "string",
            "maxLength": 1000,
            "description": "Flow description"
        },
        "version": {
            "type": "string",
            "pattern": "^\\d+\\.\\d+\\.\\d+$",
            "description": "Version string (semantic versioning)"
        },
        "execution_mode": {
            "type": "string",
            "enum": ["sequential", "parallel", "pipeline", "batch"],
            "description": "Execution mode"
        },
        "global_parameters": {
            "type": "object",
            "description": "Global parameters for all steps"
        },
        "steps": {
            "type": "array",
            "items": PROCESS_STEP_SCHEMA,
            "minItems": 1,
            "description": "Process steps"
        }
    },
    "required": ["name", "steps"],
    "additionalProperties": True
}

# Parameter validation schemas for different process types
OXIDATION_PARAMETERS_SCHEMA = {
    "type": "object",
    "properties": {
        "temperature": {
            "type": "number",
            "minimum": 800.0,
            "maximum": 1200.0,
            "description": "Temperature in Celsius"
        },
        "time": {
            "type": "number",
            "minimum": 0.01,
            "maximum": 100.0,
            "description": "Time in hours"
        },
        "atmosphere": {
            "type": "string",
            "enum": ["dry", "wet", "steam"],
            "description": "Oxidation atmosphere"
        },
        "pressure": {
            "type": "number",
            "minimum": 0.1,
            "maximum": 10.0,
            "description": "Pressure in atm"
        }
    },
    "required": ["temperature", "time", "atmosphere"],
    "additionalProperties": False
}

DOPING_PARAMETERS_SCHEMA = {
    "type": "object",
    "properties": {
        "dopant": {
            "type": "string",
            "enum": ["boron", "phosphorus", "arsenic", "antimony", "aluminum", "gallium", "indium"],
            "description": "Dopant species"
        },
        "concentration": {
            "type": "number",
            "minimum": 1e12,
            "maximum": 1e22,
            "description": "Concentration in cm^-3"
        },
        "energy": {
            "type": "number",
            "minimum": 1.0,
            "maximum": 1000.0,
            "description": "Implant energy in keV"
        },
        "dose": {
            "type": "number",
            "minimum": 1e10,
            "maximum": 1e18,
            "description": "Implant dose in cm^-2"
        },
        "temperature": {
            "type": "number",
            "minimum": 20.0,
            "maximum": 1200.0,
            "description": "Temperature in Celsius"
        },
        "angle": {
            "type": "number",
            "minimum": 0.0,
            "maximum": 45.0,
            "description": "Implant angle in degrees"
        }
    },
    "required": ["dopant", "concentration", "energy"],
    "additionalProperties": False
}

LITHOGRAPHY_PARAMETERS_SCHEMA = {
    "type": "object",
    "properties": {
        "wavelength": {
            "type": "number",
            "minimum": 13.5,
            "maximum": 436.0,
            "description": "Exposure wavelength in nm"
        },
        "dose": {
            "type": "number",
            "minimum": 1.0,
            "maximum": 1000.0,
            "description": "Exposure dose in mJ/cm^2"
        },
        "numerical_aperture": {
            "type": "number",
            "minimum": 0.1,
            "maximum": 1.5,
            "description": "Numerical aperture"
        },
        "resist_thickness": {
            "type": "number",
            "minimum": 0.01,
            "maximum": 10.0,
            "description": "Resist thickness in micrometers"
        },
        "focus_offset": {
            "type": "number",
            "minimum": -1.0,
            "maximum": 1.0,
            "description": "Focus offset in micrometers"
        }
    },
    "required": ["wavelength", "dose"],
    "additionalProperties": False
}

DEPOSITION_PARAMETERS_SCHEMA = {
    "type": "object",
    "properties": {
        "material": {
            "type": "string",
            "enum": ["silicon", "sio2", "si3n4", "polysilicon", "aluminum", "copper", 
                    "tungsten", "titanium", "tantalum", "hafnium_oxide"],
            "description": "Deposition material"
        },
        "thickness": {
            "type": "number",
            "minimum": 0.001,
            "maximum": 100.0,
            "description": "Thickness in micrometers"
        },
        "temperature": {
            "type": "number",
            "minimum": 20.0,
            "maximum": 1200.0,
            "description": "Deposition temperature in Celsius"
        },
        "pressure": {
            "type": "number",
            "minimum": 1e-6,
            "maximum": 1000.0,
            "description": "Pressure in Torr"
        },
        "method": {
            "type": "string",
            "enum": ["cvd", "pvd", "ald", "evaporation", "sputtering", "epitaxy"],
            "description": "Deposition method"
        },
        "rate": {
            "type": "number",
            "minimum": 0.001,
            "maximum": 100.0,
            "description": "Deposition rate in nm/min"
        }
    },
    "required": ["material", "thickness", "temperature"],
    "additionalProperties": False
}

ETCHING_PARAMETERS_SCHEMA = {
    "type": "object",
    "properties": {
        "depth": {
            "type": "number",
            "minimum": 0.001,
            "maximum": 100.0,
            "description": "Etch depth in micrometers"
        },
        "type": {
            "type": "string",
            "enum": ["isotropic", "anisotropic", "selective"],
            "description": "Etch type"
        },
        "chemistry": {
            "type": "string",
            "enum": ["cf4", "sf6", "chf3", "hbr", "cl2", "bcl3", "hf", "koh", "tmah"],
            "description": "Etch chemistry"
        },
        "selectivity": {
            "type": "number",
            "minimum": 1.0,
            "maximum": 1000.0,
            "description": "Etch selectivity"
        },
        "rate": {
            "type": "number",
            "minimum": 0.1,
            "maximum": 1000.0,
            "description": "Etch rate in nm/min"
        },
        "temperature": {
            "type": "number",
            "minimum": -100.0,
            "maximum": 500.0,
            "description": "Etch temperature in Celsius"
        }
    },
    "required": ["depth", "type"],
    "additionalProperties": False
}

# Parameter schemas mapping
PARAMETER_SCHEMAS = {
    "oxidation": OXIDATION_PARAMETERS_SCHEMA,
    "doping": DOPING_PARAMETERS_SCHEMA,
    "lithography": LITHOGRAPHY_PARAMETERS_SCHEMA,
    "deposition": DEPOSITION_PARAMETERS_SCHEMA,
    "etching": ETCHING_PARAMETERS_SCHEMA
}

def validate_wafer_config(config: Dict[str, Any]) -> None:
    """Validate wafer configuration"""
    try:
        validate(instance=config, schema=WAFER_SCHEMA)
    except JsonSchemaError as e:
        raise ValidationError(f"Invalid wafer configuration: {e.message}")
    
    # Additional custom validations
    if config.get('material') == 'silicon':
        if config.get('orientation') and config['orientation'] not in ['(100)', '(111)', '(110)']:
            raise ValidationError("Silicon wafers must have (100), (111), or (110) orientation")
    
    # Check grid size constraints
    width = config.get('width', 0)
    height = config.get('height', 0)
    if width * height > 1000000:  # 1M points max
        raise ValidationError("Grid size too large (max 1M points)")

def validate_process_parameters(step_config: Dict[str, Any]) -> None:
    """Validate process step configuration"""
    try:
        validate(instance=step_config, schema=PROCESS_STEP_SCHEMA)
    except JsonSchemaError as e:
        raise ValidationError(f"Invalid process step configuration: {e.message}")
    
    # Validate step-specific parameters
    step_type = step_config.get('type')
    parameters = step_config.get('parameters', {})
    
    if step_type in PARAMETER_SCHEMAS:
        try:
            validate(instance=parameters, schema=PARAMETER_SCHEMAS[step_type])
        except JsonSchemaError as e:
            raise ValidationError(f"Invalid {step_type} parameters: {e.message}")
    
    # Additional custom validations
    if step_type == 'oxidation':
        temp = parameters.get('temperature', 0)
        time = parameters.get('time', 0)
        if temp > 1100 and time > 10:
            raise ValidationError("High temperature oxidation time should be limited")
    
    elif step_type == 'doping':
        concentration = parameters.get('concentration', 0)
        energy = parameters.get('energy', 0)
        if concentration > 1e21 and energy < 10:
            raise ValidationError("High concentration doping requires higher energy")
    
    elif step_type == 'lithography':
        wavelength = parameters.get('wavelength', 0)
        dose = parameters.get('dose', 0)
        if wavelength < 200 and dose < 10:
            raise ValidationError("Short wavelength lithography requires higher dose")

def validate_simulation_flow(flow_config: Dict[str, Any]) -> None:
    """Validate simulation flow configuration"""
    try:
        validate(instance=flow_config, schema=SIMULATION_FLOW_SCHEMA)
    except JsonSchemaError as e:
        raise ValidationError(f"Invalid simulation flow: {e.message}")
    
    # Validate each step
    steps = flow_config.get('steps', [])
    step_names = set()
    
    for i, step in enumerate(steps):
        # Check for duplicate step names
        step_name = step.get('name')
        if step_name in step_names:
            raise ValidationError(f"Duplicate step name: {step_name}")
        step_names.add(step_name)
        
        # Validate step parameters
        validate_process_parameters(step)
        
        # Check dependencies
        dependencies = step.get('dependencies', [])
        for dep in dependencies:
            if dep not in step_names and dep != step_name:
                # Check if dependency exists in previous steps
                prev_step_names = {s.get('name') for s in steps[:i]}
                if dep not in prev_step_names:
                    raise ValidationError(f"Step '{step_name}' depends on undefined step '{dep}'")

def validate_file_path(file_path: str) -> None:
    """Validate file path format"""
    if not file_path:
        raise ValidationError("File path cannot be empty")
    
    # Check for invalid characters
    invalid_chars = ['<', '>', ':', '"', '|', '?', '*']
    if any(char in file_path for char in invalid_chars):
        raise ValidationError(f"File path contains invalid characters: {file_path}")
    
    # Check path length
    if len(file_path) > 260:  # Windows MAX_PATH limitation
        raise ValidationError("File path too long (max 260 characters)")

def validate_material_name(material: str) -> None:
    """Validate material name"""
    valid_materials = [
        'silicon', 'germanium', 'gaas', 'inp', 'gan', 'sic',
        'sio2', 'si3n4', 'polysilicon', 'aluminum', 'copper',
        'tungsten', 'titanium', 'tantalum', 'hafnium_oxide',
        'photoresist', 'oxide', 'nitride', 'metal'
    ]
    
    if material.lower() not in valid_materials:
        raise ValidationError(f"Unknown material: {material}")

def validate_temperature_range(temperature: float, process_type: str) -> None:
    """Validate temperature for specific process"""
    temp_ranges = {
        'oxidation': (800, 1200),
        'doping': (20, 1200),
        'deposition': (20, 1200),
        'annealing': (400, 1200),
        'lithography': (15, 150),
        'etching': (-100, 500)
    }
    
    if process_type in temp_ranges:
        min_temp, max_temp = temp_ranges[process_type]
        if not (min_temp <= temperature <= max_temp):
            raise ValidationError(
                f"Temperature {temperature}°C out of range for {process_type} "
                f"(valid range: {min_temp}-{max_temp}°C)"
            )

def validate_concentration(concentration: float) -> None:
    """Validate doping concentration"""
    if not (1e10 <= concentration <= 1e22):
        raise ValidationError(
            f"Concentration {concentration:.2e} cm⁻³ out of valid range (1e10-1e22 cm⁻³)"
        )

def validate_dimension(value: float, dimension_type: str) -> None:
    """Validate physical dimensions"""
    dimension_ranges = {
        'thickness': (0.001, 100.0),  # micrometers
        'depth': (0.001, 100.0),      # micrometers
        'width': (0.01, 1000.0),      # micrometers
        'height': (0.01, 1000.0),     # micrometers
        'diameter': (25.0, 450.0)     # millimeters
    }
    
    if dimension_type in dimension_ranges:
        min_val, max_val = dimension_ranges[dimension_type]
        if not (min_val <= value <= max_val):
            raise ValidationError(
                f"{dimension_type.capitalize()} {value} out of valid range "
                f"({min_val}-{max_val})"
            )

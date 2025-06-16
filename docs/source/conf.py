# Configuration file for the Sphinx documentation builder.
# Author: Dr. Mazharuddin Mohammed

import os
import sys

# Add the project root to the Python path
sys.path.insert(0, os.path.abspath('../../src/python'))

# Set environment variable to indicate we're building docs
os.environ['READTHEDOCS'] = 'True'

# Import and install mock modules for documentation
try:
    from api.mock_extensions import install_mock_modules
    install_mock_modules()
except ImportError:
    pass

# -- Project information -----------------------------------------------------
project = 'SemiPRO'
copyright = '2024, Dr. Mazharuddin Mohammed'
author = 'Dr. Mazharuddin Mohammed'
release = '1.0.0'

# -- General configuration ---------------------------------------------------
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx.ext.napoleon',
    'sphinx.ext.intersphinx',
    'sphinx.ext.mathjax',
    'myst_parser',
]

templates_path = ['_templates']
exclude_patterns = []

# Autodoc configuration
autodoc_default_options = {
    'members': True,
    'undoc-members': True,
    'show-inheritance': True,
    'special-members': '__init__',
}

# Mock imports for modules that can't be imported during doc build
autodoc_mock_imports = [
    'numpy',
    'matplotlib',
    'scipy',
    'pyside6',
    'h5py',
    'vtk',
    'yaml',
    'cv2',
    'PIL',
    'skimage',
    'networkx',
    'pandas',
    'cython',
    'pybind11',
    'flask',
    'websockets',
    'flask_cors',
    'flask_limiter',
    'requests',
    'asyncio',
    'threading',
    'uuid',
    'pathlib',
    'jsonschema',
]

# -- Options for HTML output -------------------------------------------------
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

# Theme options
html_theme_options = {
    'canonical_url': '',
    'analytics_id': '',
    'logo_only': False,
    'display_version': True,
    'prev_next_buttons_location': 'bottom',
    'style_external_links': False,
    'vcs_pageview_mode': '',
    'style_nav_header_background': '#2980B9',
    'collapse_navigation': True,
    'sticky_navigation': True,
    'navigation_depth': 4,
    'includehidden': True,
    'titles_only': False
}

# Napoleon settings
napoleon_google_docstring = True
napoleon_numpy_docstring = True
napoleon_include_init_with_doc = False
napoleon_include_private_with_doc = False
napoleon_include_special_with_doc = True
napoleon_use_admonition_for_examples = False
napoleon_use_admonition_for_notes = False
napoleon_use_admonition_for_references = False
napoleon_use_ivar = False
napoleon_use_param = True
napoleon_use_rtype = True

# Intersphinx mapping
intersphinx_mapping = {
    'python': ('https://docs.python.org/3/', None),
    'numpy': ('https://numpy.org/doc/stable/', None),
    'matplotlib': ('https://matplotlib.org/stable/', None),
    'scipy': ('https://docs.scipy.org/doc/scipy/', None),
}
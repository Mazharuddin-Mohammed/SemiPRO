# Author: Dr. Mazharuddin Mohammed
from setuptools import setup, Extension
from Cython.Build import cythonize
import numpy
import os

# Get the absolute path to the build directory
build_dir = os.path.abspath("../../build")
cpp_dir = os.path.abspath("../cpp")

# Common settings for all extensions
common_include_dirs = [
    numpy.get_include(),
    os.path.join(cpp_dir, "core"),
    "/usr/include/eigen3",
]

common_library_dirs = [build_dir]
common_libraries = ["simulator_lib", "yaml-cpp"]
common_extra_compile_args = ["-std=c++17", "-O3"]
common_extra_link_args = ["-std=c++17"]

extensions = [
    Extension(
        "geometry",
        ["geometry.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/geometry")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "oxidation",
        ["oxidation.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/oxidation")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "doping",
        ["doping.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/doping")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "photolithography",
        ["photolithography.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/photolithography")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "deposition",
        ["deposition.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/deposition")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "etching",
        ["etching.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/etching")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "metallization",
        ["metallization.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/metallization")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "packaging",
        ["packaging.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/packaging")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "thermal",
        ["thermal.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/thermal")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "reliability",
        ["reliability.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/reliability")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "renderer",
        ["renderer.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "renderer")],
        library_dirs=common_library_dirs,
        libraries=common_libraries + ["vulkan", "glfw"],
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "multi_die",
        ["multi_die.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/multi_die")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "drc",
        ["drc.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/design_rule_check")],
        library_dirs=common_library_dirs,
        libraries=common_libraries,
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
    Extension(
        "advanced_visualization",
        ["advanced_visualization.pyx"],
        language="c++",
        include_dirs=common_include_dirs + [os.path.join(cpp_dir, "modules/advanced_visualization")],
        library_dirs=common_library_dirs,
        libraries=common_libraries + ["vulkan", "glfw"],
        extra_compile_args=common_extra_compile_args,
        extra_link_args=common_extra_link_args,
    ),
]

setup(
    ext_modules=cythonize(extensions),
)
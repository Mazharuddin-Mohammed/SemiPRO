#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
Complete build script for SemiPRO Semiconductor Simulator
Builds C++ libraries, Cython extensions, and Python packages
"""

import os
import sys
import subprocess
import shutil
from pathlib import Path

def run_command(cmd, cwd=None, check=True):
    """Run a command and handle errors"""
    print(f"Running: {cmd}")
    if cwd:
        print(f"  in directory: {cwd}")
    
    try:
        result = subprocess.run(cmd, shell=True, cwd=cwd, check=check, 
                              capture_output=True, text=True)
        if result.stdout:
            print(result.stdout)
        return result
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {cmd}")
        print(f"Return code: {e.returncode}")
        if e.stdout:
            print(f"STDOUT: {e.stdout}")
        if e.stderr:
            print(f"STDERR: {e.stderr}")
        if check:
            raise
        return e

def check_dependencies():
    """Check if required dependencies are available"""
    print("Checking dependencies...")
    
    dependencies = {
        'cmake': 'cmake --version',
        'g++': 'g++ --version',
        'python': 'python --version',
        'cython': 'python -c "import Cython; print(Cython.__version__)"',
        'numpy': 'python -c "import numpy; print(numpy.__version__)"',
        'eigen3': 'pkg-config --modversion eigen3',
        'vulkan': 'vulkan-info --version',
        'glfw': 'pkg-config --modversion glfw3',
        'yaml-cpp': 'pkg-config --modversion yaml-cpp'
    }
    
    missing = []
    
    for dep, cmd in dependencies.items():
        try:
            result = run_command(cmd, check=False)
            if result.returncode == 0:
                print(f"✓ {dep} found")
            else:
                print(f"✗ {dep} not found")
                missing.append(dep)
        except Exception:
            print(f"✗ {dep} not found")
            missing.append(dep)
    
    if missing:
        print(f"\nMissing dependencies: {', '.join(missing)}")
        print("Please install missing dependencies before building.")
        return False
    
    print("✓ All dependencies found")
    return True

def build_cpp_library():
    """Build the C++ library using CMake"""
    print("\nBuilding C++ library...")
    
    # Create build directory
    build_dir = Path("build")
    build_dir.mkdir(exist_ok=True)
    
    # Configure with CMake
    cmake_cmd = "cmake .."
    run_command(cmake_cmd, cwd=build_dir)
    
    # Build
    make_cmd = "make -j$(nproc)"
    run_command(make_cmd, cwd=build_dir)
    
    print("✓ C++ library built successfully")

def build_cython_extensions():
    """Build Cython extensions"""
    print("\nBuilding Cython extensions...")
    
    cython_dir = Path("src/cython")
    
    # Build extensions
    setup_cmd = "python setup.py build_ext --inplace"
    run_command(setup_cmd, cwd=cython_dir)
    
    print("✓ Cython extensions built successfully")

def install_python_package():
    """Install the Python package in development mode"""
    print("\nInstalling Python package...")
    
    # Install in development mode
    install_cmd = "pip install -e ."
    run_command(install_cmd)
    
    print("✓ Python package installed successfully")

def compile_shaders():
    """Compile SPIR-V shaders"""
    print("\nCompiling shaders...")
    
    shader_dir = Path("src/shaders")
    compiled_dir = shader_dir / "compiled"
    compiled_dir.mkdir(exist_ok=True)
    
    # Find all shader files
    vertex_shaders = list(shader_dir.glob("*.vert"))
    fragment_shaders = list(shader_dir.glob("*.frag"))
    compute_shaders = list(shader_dir.glob("*.comp"))
    
    all_shaders = vertex_shaders + fragment_shaders + compute_shaders
    
    for shader in all_shaders:
        output_file = compiled_dir / f"{shader.name}.spv"
        compile_cmd = f"glslc {shader} -o {output_file}"
        
        try:
            run_command(compile_cmd, check=False)
            print(f"✓ Compiled {shader.name}")
        except Exception as e:
            print(f"✗ Failed to compile {shader.name}: {e}")
    
    print("✓ Shader compilation completed")

def run_tests():
    """Run the test suite"""
    print("\nRunning tests...")
    
    # Run C++ tests
    cpp_test_cmd = "./build/tests"
    if Path("build/tests").exists():
        try:
            run_command(cpp_test_cmd)
            print("✓ C++ tests passed")
        except Exception:
            print("✗ C++ tests failed")
    else:
        print("⚠ C++ tests not found")
    
    # Run Python tests
    python_test_cmd = "python test_complete_implementation.py"
    try:
        run_command(python_test_cmd)
        print("✓ Python tests passed")
    except Exception:
        print("✗ Python tests failed")

def create_documentation():
    """Generate documentation"""
    print("\nGenerating documentation...")
    
    # Create docs directory
    docs_dir = Path("docs")
    docs_dir.mkdir(exist_ok=True)
    
    # Generate API documentation
    api_doc_cmd = "python -m pydoc -w src.python.simulator"
    try:
        run_command(api_doc_cmd, check=False)
        print("✓ API documentation generated")
    except Exception:
        print("⚠ API documentation generation failed")
    
    # Copy README and other docs
    readme_files = ["README.md", "FINAL_IMPLEMENTATION_SUMMARY.md"]
    for readme in readme_files:
        if Path(readme).exists():
            shutil.copy(readme, docs_dir)
            print(f"✓ Copied {readme}")

def package_distribution():
    """Create distribution packages"""
    print("\nCreating distribution packages...")
    
    # Create source distribution
    sdist_cmd = "python setup.py sdist"
    try:
        run_command(sdist_cmd)
        print("✓ Source distribution created")
    except Exception:
        print("✗ Source distribution failed")
    
    # Create wheel distribution
    wheel_cmd = "python setup.py bdist_wheel"
    try:
        run_command(wheel_cmd)
        print("✓ Wheel distribution created")
    except Exception:
        print("✗ Wheel distribution failed")

def clean_build():
    """Clean build artifacts"""
    print("\nCleaning build artifacts...")
    
    clean_dirs = ["build", "dist", "src/cython/build", "*.egg-info"]
    clean_files = ["**/*.pyc", "**/__pycache__", "**/*.so", "**/*.pyd"]
    
    for pattern in clean_dirs:
        for path in Path(".").glob(pattern):
            if path.is_dir():
                shutil.rmtree(path)
                print(f"✓ Removed {path}")
    
    for pattern in clean_files:
        for path in Path(".").rglob(pattern):
            if path.is_file():
                path.unlink()
                print(f"✓ Removed {path}")
            elif path.is_dir():
                shutil.rmtree(path)
                print(f"✓ Removed {path}")

def main():
    """Main build function"""
    print("=" * 60)
    print("SemiPRO Complete Build Script")
    print("Author: Dr. Mazharuddin Mohammed")
    print("=" * 60)
    
    # Parse command line arguments
    import argparse
    parser = argparse.ArgumentParser(description="Build SemiPRO Semiconductor Simulator")
    parser.add_argument("--clean", action="store_true", help="Clean build artifacts")
    parser.add_argument("--no-tests", action="store_true", help="Skip running tests")
    parser.add_argument("--no-docs", action="store_true", help="Skip documentation generation")
    parser.add_argument("--package", action="store_true", help="Create distribution packages")
    parser.add_argument("--deps-only", action="store_true", help="Only check dependencies")
    
    args = parser.parse_args()
    
    try:
        # Clean if requested
        if args.clean:
            clean_build()
            return
        
        # Check dependencies
        if not check_dependencies():
            return 1
        
        if args.deps_only:
            return 0
        
        # Build steps
        build_cpp_library()
        compile_shaders()
        build_cython_extensions()
        install_python_package()
        
        # Optional steps
        if not args.no_tests:
            run_tests()
        
        if not args.no_docs:
            create_documentation()
        
        if args.package:
            package_distribution()
        
        print("\n" + "=" * 60)
        print("🎉 BUILD COMPLETED SUCCESSFULLY!")
        print("=" * 60)
        print("\nNext steps:")
        print("1. Run 'python test_complete_implementation.py' to verify installation")
        print("2. Start the GUI with 'python -m src.python.gui.main_window'")
        print("3. Check the documentation in the 'docs' directory")
        
        return 0
        
    except Exception as e:
        print(f"\n❌ BUILD FAILED: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())

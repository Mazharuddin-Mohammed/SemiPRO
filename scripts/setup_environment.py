#!/usr/bin/env python3
# Author: Dr. Mazharuddin Mohammed
"""
SemiPRO Environment Setup Script
===============================

This script sets up the complete SemiPRO development and runtime environment.
It handles dependency installation, compilation, and configuration.
"""

import os
import sys
import subprocess
import platform
import shutil
from pathlib import Path
import argparse
import logging

# Setup logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)

class SemiPROSetup:
    """SemiPRO environment setup manager"""
    
    def __init__(self, project_root: Path = None):
        self.project_root = project_root or Path(__file__).parent.parent
        self.system = platform.system().lower()
        self.python_version = f"{sys.version_info.major}.{sys.version_info.minor}"
        
        # Paths
        self.build_dir = self.project_root / "build"
        self.src_dir = self.project_root / "src"
        self.venv_dir = self.project_root / "venv"
        
        logger.info(f"Setting up SemiPRO in: {self.project_root}")
        logger.info(f"System: {self.system}")
        logger.info(f"Python: {self.python_version}")
    
    def check_system_requirements(self):
        """Check system requirements and dependencies"""
        logger.info("Checking system requirements...")
        
        # Check Python version
        if sys.version_info < (3, 8):
            logger.error("Python 3.8 or higher is required")
            return False
        
        # Check for required system tools
        required_tools = {
            'git': 'Git version control',
            'cmake': 'CMake build system',
            'make': 'Make build tool',
        }
        
        if self.system == 'windows':
            required_tools.update({
                'cl': 'Microsoft Visual C++ compiler',
                'vcpkg': 'vcpkg package manager'
            })
        else:
            required_tools.update({
                'gcc': 'GCC compiler',
                'g++': 'G++ compiler'
            })
        
        missing_tools = []
        for tool, description in required_tools.items():
            if not shutil.which(tool):
                missing_tools.append(f"{tool} ({description})")
        
        if missing_tools:
            logger.error("Missing required tools:")
            for tool in missing_tools:
                logger.error(f"  - {tool}")
            return False
        
        logger.info("✓ All system requirements satisfied")
        return True
    
    def setup_python_environment(self):
        """Setup Python virtual environment"""
        logger.info("Setting up Python environment...")
        
        try:
            # Create virtual environment
            if not self.venv_dir.exists():
                logger.info("Creating virtual environment...")
                subprocess.run([
                    sys.executable, "-m", "venv", str(self.venv_dir)
                ], check=True)
            
            # Get pip executable
            if self.system == 'windows':
                pip_exe = self.venv_dir / "Scripts" / "pip.exe"
                python_exe = self.venv_dir / "Scripts" / "python.exe"
            else:
                pip_exe = self.venv_dir / "bin" / "pip"
                python_exe = self.venv_dir / "bin" / "python"
            
            # Upgrade pip
            logger.info("Upgrading pip...")
            subprocess.run([
                str(python_exe), "-m", "pip", "install", "--upgrade", "pip"
            ], check=True)
            
            # Install build dependencies
            logger.info("Installing build dependencies...")
            build_deps = [
                "wheel",
                "setuptools>=45",
                "cython>=0.29",
                "pybind11>=2.6",
                "numpy>=1.19",
                "cmake>=3.16"
            ]
            
            subprocess.run([
                str(pip_exe), "install"
            ] + build_deps, check=True)
            
            # Install requirements
            requirements_file = self.project_root / "requirements.txt"
            if requirements_file.exists():
                logger.info("Installing project requirements...")
                subprocess.run([
                    str(pip_exe), "install", "-r", str(requirements_file)
                ], check=True)
            
            logger.info("✓ Python environment setup complete")
            return True
            
        except subprocess.CalledProcessError as e:
            logger.error(f"Python environment setup failed: {e}")
            return False
    
    def install_system_dependencies(self):
        """Install system-level dependencies"""
        logger.info("Installing system dependencies...")
        
        try:
            if self.system == 'linux':
                # Ubuntu/Debian
                if shutil.which('apt-get'):
                    deps = [
                        'libeigen3-dev',
                        'libyaml-cpp-dev',
                        'libvulkan-dev',
                        'libglfw3-dev',
                        'libhdf5-dev',
                        'libopenmpi-dev'
                    ]
                    
                    logger.info("Installing dependencies with apt-get...")
                    subprocess.run([
                        'sudo', 'apt-get', 'update'
                    ], check=True)
                    
                    subprocess.run([
                        'sudo', 'apt-get', 'install', '-y'
                    ] + deps, check=True)
                
                # CentOS/RHEL
                elif shutil.which('yum'):
                    deps = [
                        'eigen3-devel',
                        'yaml-cpp-devel',
                        'vulkan-devel',
                        'glfw-devel',
                        'hdf5-devel',
                        'openmpi-devel'
                    ]
                    
                    logger.info("Installing dependencies with yum...")
                    subprocess.run([
                        'sudo', 'yum', 'install', '-y'
                    ] + deps, check=True)
            
            elif self.system == 'darwin':  # macOS
                if shutil.which('brew'):
                    deps = [
                        'eigen',
                        'yaml-cpp',
                        'vulkan-headers',
                        'glfw',
                        'hdf5',
                        'open-mpi'
                    ]
                    
                    logger.info("Installing dependencies with Homebrew...")
                    subprocess.run([
                        'brew', 'install'
                    ] + deps, check=True)
                else:
                    logger.warning("Homebrew not found. Please install dependencies manually.")
            
            elif self.system == 'windows':
                if shutil.which('vcpkg'):
                    deps = [
                        'eigen3',
                        'yaml-cpp',
                        'vulkan',
                        'glfw3',
                        'hdf5'
                    ]
                    
                    logger.info("Installing dependencies with vcpkg...")
                    for dep in deps:
                        subprocess.run([
                            'vcpkg', 'install', f'{dep}:x64-windows'
                        ], check=True)
                else:
                    logger.warning("vcpkg not found. Please install dependencies manually.")
            
            logger.info("✓ System dependencies installed")
            return True
            
        except subprocess.CalledProcessError as e:
            logger.error(f"System dependency installation failed: {e}")
            return False
    
    def compile_cpp_extensions(self):
        """Compile C++ extensions"""
        logger.info("Compiling C++ extensions...")
        
        try:
            # Create build directory
            self.build_dir.mkdir(exist_ok=True)
            
            # Configure with CMake
            logger.info("Configuring with CMake...")
            cmake_args = [
                'cmake',
                str(self.src_dir),
                '-DCMAKE_BUILD_TYPE=Release',
                '-DWITH_PYTHON_BINDINGS=ON',
                '-DWITH_VULKAN=ON',
                '-DWITH_HDF5=ON'
            ]
            
            subprocess.run(cmake_args, cwd=self.build_dir, check=True)
            
            # Build
            logger.info("Building C++ extensions...")
            subprocess.run([
                'cmake', '--build', '.', '--config', 'Release', '-j'
            ], cwd=self.build_dir, check=True)
            
            logger.info("✓ C++ extensions compiled successfully")
            return True
            
        except subprocess.CalledProcessError as e:
            logger.error(f"C++ compilation failed: {e}")
            return False
    
    def compile_shaders(self):
        """Compile SPIR-V shaders"""
        logger.info("Compiling SPIR-V shaders...")
        
        try:
            shader_script = self.project_root / "scripts" / "compile_shaders.sh"
            if shader_script.exists():
                if self.system == 'windows':
                    # Use WSL or Git Bash on Windows
                    subprocess.run(['bash', str(shader_script)], check=True)
                else:
                    subprocess.run(['bash', str(shader_script)], check=True)
                
                logger.info("✓ Shaders compiled successfully")
            else:
                logger.warning("Shader compilation script not found")
            
            return True
            
        except subprocess.CalledProcessError as e:
            logger.error(f"Shader compilation failed: {e}")
            return False
    
    def setup_configuration(self):
        """Setup default configuration files"""
        logger.info("Setting up configuration...")
        
        try:
            config_dir = self.project_root / "config"
            config_dir.mkdir(exist_ok=True)
            
            # Create default simulation config
            default_config = {
                'simulation': {
                    'solver': {
                        'tolerance': 1e-8,
                        'max_iterations': 1000
                    },
                    'physics': {
                        'enable_thermal': True,
                        'enable_electrical': True,
                        'enable_mechanical': False
                    }
                },
                'execution': {
                    'mode': 'sequential',
                    'parallel_steps': 4,
                    'enable_checkpointing': True
                },
                'paths': {
                    'input_directory': 'input',
                    'output_directory': 'output',
                    'temp_directory': 'temp'
                }
            }
            
            import yaml
            config_file = config_dir / "default.yaml"
            with open(config_file, 'w') as f:
                yaml.dump(default_config, f, default_flow_style=False)
            
            logger.info(f"✓ Created default configuration: {config_file}")
            return True
            
        except Exception as e:
            logger.error(f"Configuration setup failed: {e}")
            return False
    
    def run_tests(self):
        """Run test suite to verify installation"""
        logger.info("Running test suite...")
        
        try:
            # Get python executable from venv
            if self.system == 'windows':
                python_exe = self.venv_dir / "Scripts" / "python.exe"
            else:
                python_exe = self.venv_dir / "bin" / "python"
            
            # Run pytest
            test_cmd = [
                str(python_exe), "-m", "pytest",
                str(self.project_root / "tests"),
                "-v", "--tb=short"
            ]
            
            result = subprocess.run(test_cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                logger.info("✓ All tests passed")
                return True
            else:
                logger.warning("Some tests failed:")
                logger.warning(result.stdout)
                logger.warning(result.stderr)
                return False
                
        except Exception as e:
            logger.error(f"Test execution failed: {e}")
            return False
    
    def setup_complete_environment(self, skip_tests=False, skip_system_deps=False):
        """Setup complete SemiPRO environment"""
        logger.info("🚀 Starting SemiPRO environment setup...")
        
        steps = [
            ("System Requirements", self.check_system_requirements),
            ("Python Environment", self.setup_python_environment),
            ("Configuration", self.setup_configuration),
            ("C++ Extensions", self.compile_cpp_extensions),
            ("SPIR-V Shaders", self.compile_shaders),
        ]
        
        if not skip_system_deps:
            steps.insert(2, ("System Dependencies", self.install_system_dependencies))
        
        if not skip_tests:
            steps.append(("Test Suite", self.run_tests))
        
        failed_steps = []
        
        for step_name, step_func in steps:
            logger.info(f"\n📋 {step_name}...")
            try:
                if not step_func():
                    failed_steps.append(step_name)
                    logger.error(f"❌ {step_name} failed")
                else:
                    logger.info(f"✅ {step_name} completed")
            except Exception as e:
                failed_steps.append(step_name)
                logger.error(f"❌ {step_name} failed with exception: {e}")
        
        if failed_steps:
            logger.error(f"\n💥 Setup failed. Failed steps: {', '.join(failed_steps)}")
            return False
        else:
            logger.info(f"\n🎉 SemiPRO environment setup completed successfully!")
            logger.info(f"📁 Project root: {self.project_root}")
            logger.info(f"🐍 Virtual environment: {self.venv_dir}")
            logger.info(f"🔧 Build directory: {self.build_dir}")
            return True

def main():
    """Main setup function"""
    parser = argparse.ArgumentParser(description="Setup SemiPRO environment")
    parser.add_argument("--skip-tests", action="store_true",
                       help="Skip running test suite")
    parser.add_argument("--skip-system-deps", action="store_true",
                       help="Skip system dependency installation")
    parser.add_argument("--project-root", type=Path,
                       help="Project root directory")
    
    args = parser.parse_args()
    
    setup = SemiPROSetup(args.project_root)
    
    try:
        success = setup.setup_complete_environment(
            skip_tests=args.skip_tests,
            skip_system_deps=args.skip_system_deps
        )
        
        if success:
            print("\n🎯 Next steps:")
            print("1. Activate virtual environment:")
            if setup.system == 'windows':
                print(f"   {setup.venv_dir}\\Scripts\\activate")
            else:
                print(f"   source {setup.venv_dir}/bin/activate")
            print("2. Run example simulation:")
            print("   python examples/end_to_end/mosfet_fabrication.py")
            print("3. Start API server:")
            print("   python -m src.python.api.rest_api")
            
            sys.exit(0)
        else:
            sys.exit(1)
            
    except KeyboardInterrupt:
        logger.info("\n⚠️  Setup interrupted by user")
        sys.exit(1)
    except Exception as e:
        logger.error(f"\n💥 Unexpected error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()

# Contributing to SemiPRO Semiconductor Simulator

Thank you for your interest in contributing to SemiPRO! This document provides guidelines and information for contributors.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Contributing Guidelines](#contributing-guidelines)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Documentation](#documentation)

## Code of Conduct

We are committed to providing a welcoming and inclusive environment for all contributors. Please be respectful and professional in all interactions.

## Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally:
   ```bash
   git clone https://github.com/yourusername/SemiPRO.git
   cd SemiPRO
   ```
3. **Set up the development environment** (see Development Setup below)
4. **Create a feature branch** for your changes:
   ```bash
   git checkout -b feature/your-feature-name
   ```

## Development Setup

### Prerequisites

- **C++ Compiler**: GCC 9+ or Clang 10+
- **CMake**: Version 3.10 or higher
- **Python**: Version 3.8 or higher
- **Dependencies**: Vulkan SDK, GLFW, Eigen3, YAML-cpp, Catch2

### Build Instructions

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y g++ cmake libeigen3-dev libvulkan-dev libglfw3-dev \
                        python3-dev python3-pip libyaml-cpp-dev libcatch2-dev

# Build the project
mkdir build && cd build
cmake ..
make -j$(nproc)

# Run tests
./tests
```

### Python Environment

```bash
# Create virtual environment
python3 -m venv venv
source venv/bin/activate

# Install Python dependencies
pip install -r requirements.txt
pip install -r tests/requirements.txt
```

## Contributing Guidelines

### Types of Contributions

We welcome the following types of contributions:

- **Bug fixes**: Fix issues in existing code
- **New features**: Add new simulation modules or capabilities
- **Performance improvements**: Optimize existing algorithms
- **Documentation**: Improve or add documentation
- **Tests**: Add or improve test coverage
- **Examples**: Add new examples or tutorials

### Before You Start

1. **Check existing issues** to see if your idea is already being worked on
2. **Open an issue** to discuss major changes before implementing
3. **Search existing pull requests** to avoid duplicate work

## Pull Request Process

1. **Ensure your code builds** without warnings:
   ```bash
   mkdir build && cd build
   cmake .. && make
   ```

2. **Run all tests** and ensure they pass:
   ```bash
   ./tests
   python -m pytest tests/python/
   ```

3. **Update documentation** if you're adding new features

4. **Add tests** for new functionality

5. **Follow the commit message format**:
   ```
   type(scope): brief description
   
   Detailed explanation of changes (if needed)
   
   Fixes #issue_number (if applicable)
   ```

6. **Submit your pull request** with:
   - Clear title and description
   - Reference to related issues
   - Screenshots/examples if applicable

## Coding Standards

### C++ Guidelines

- **Standard**: Use C++17 features
- **Naming**: 
  - Classes: `PascalCase` (e.g., `WaferManager`)
  - Functions/variables: `snake_case` (e.g., `simulate_oxidation`)
  - Constants: `UPPER_SNAKE_CASE` (e.g., `MAX_GRID_SIZE`)
- **Headers**: Use include guards or `#pragma once`
- **Memory**: Prefer smart pointers over raw pointers
- **Error handling**: Use exceptions for error conditions

### Python Guidelines

- **Standard**: Follow PEP 8
- **Type hints**: Use type annotations for function signatures
- **Documentation**: Use docstrings for all public functions
- **Testing**: Write unit tests using pytest

### General Guidelines

- **Comments**: Write clear, concise comments explaining why, not what
- **Functions**: Keep functions small and focused on a single task
- **Dependencies**: Minimize external dependencies
- **Performance**: Consider performance implications of changes

## Testing

### C++ Tests

```bash
# Build and run C++ tests
cd build
make tests
./tests
```

### Python Tests

```bash
# Run Python tests
python -m pytest tests/python/ -v
```

### Test Coverage

- Aim for >80% test coverage for new code
- Include both unit tests and integration tests
- Test edge cases and error conditions

### Performance Tests

- Add benchmarks for performance-critical code
- Ensure changes don't regress performance
- Document performance characteristics

## Documentation

### Code Documentation

- **C++**: Use Doxygen-style comments
- **Python**: Use Google-style docstrings
- **Examples**: Include usage examples in documentation

### User Documentation

- Update relevant documentation files
- Add new tutorials for significant features
- Keep README.md up to date

### API Documentation

- Document all public interfaces
- Include parameter descriptions and return values
- Provide usage examples

## Module-Specific Guidelines

### Simulation Modules

When adding new simulation modules:

1. **Interface**: Implement the appropriate interface class
2. **Testing**: Add comprehensive unit tests
3. **Examples**: Provide usage examples
4. **Documentation**: Document the physical model and parameters
5. **Performance**: Consider computational complexity

### Visualization

For graphics and visualization changes:

1. **Shaders**: Use SPIR-V compiled shaders
2. **Resources**: Properly manage GPU resources
3. **Compatibility**: Test on different graphics hardware
4. **Performance**: Profile rendering performance

### Python Bindings

For Python interface changes:

1. **Cython**: Update .pyx files for C++ bindings
2. **API**: Maintain backward compatibility when possible
3. **Documentation**: Update Python docstrings
4. **Examples**: Add Python usage examples

## Release Process

### Version Numbering

We use semantic versioning (MAJOR.MINOR.PATCH):

- **MAJOR**: Incompatible API changes
- **MINOR**: New functionality (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

### Release Checklist

- [ ] All tests pass
- [ ] Documentation updated
- [ ] Version numbers updated
- [ ] Changelog updated
- [ ] Performance benchmarks run
- [ ] Examples tested

## Getting Help

- **Issues**: Open a GitHub issue for bugs or feature requests
- **Discussions**: Use GitHub Discussions for questions
- **Email**: Contact maintainers for security issues

## Recognition

Contributors will be acknowledged in:

- CONTRIBUTORS.md file
- Release notes
- Documentation credits

Thank you for contributing to SemiPRO!

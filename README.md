![GOLDE](docs/logo.gif)

# Game of Life Designer and Engine (GOLDE)

<div align="center">

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.25+-blue.svg)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform: Windows | Linux | macOS](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-brightgreen.svg)

</div>

## Overview

GOLDE is a high-performance Conway's Game of Life simulator with an intuitive GUI and advanced algorithms. Features **HashLife** — a revolutionary algorithm that can advance billions of generations in seconds — alongside the classic **SparseLife** algorithm for everyday simulation needs.

Perfect for exploring complex cellular automata patterns, researching emergent behavior, and experiencing the beauty of algorithmic art.

## Key Features

- **Dual Algorithms**: Choose between blazing-fast HashLife for massive patterns or standard SparseLife for everyday use
- **Interactive GUI**: Full-featured ImGui-based interface with intuitive controls
- **Simulation Control**: Play, pause, step, and adjust speed in real-time
- **Pattern Editor**: Create and edit patterns with an interactive grid
- **Preset Library**: Pre-loaded classic Game of Life patterns
- **Multi-threaded**: Dedicated worker thread for responsiveness
- **Cross-platform**: Windows, Linux, and macOS support
- **Production Quality**: Comprehensive test suite with GTest

## Architecture

GOLDE follows a clean layered architecture:

```
┌─────────────────────────────────────────────────┐
│           GOLExecutable (App Entry)             │
└────────────────────┬────────────────────────────┘
                     │
┌─────────────────────▼────────────────────────────┐
│        GOLCoreLib (Game Orchestration)          │
│  • Simulation Control  • Pattern Management     │
└──────┬──────────────────┬──────────┬─────────────┘
       │                  │          │
   ┌───▼─────┐    ┌──────▼──┐   ┌───▼──────┐
   │ GOLAlgo │    │ GOLGame │   │ GOLGuiLib│
   │  Lib    │    │ raphics │   │(ImGui UI)│
   │• HashLi-│    │ Lib     │   │          │
   │  fe     │    │         │   └──────────┘
   │• Sparse │    └─────────┘
   │  Life   │
   └────┬────┘       
        │
   ┌────▼──────────────────────────────┐
   │    GOLLoggingLib (Logging)        │
   └───────────────────────────────────┘
```

## Quick Start

### Prerequisites

- **C++23** compiler (MSVC, Clang, or GCC)
- **CMake 3.25+**
- **Ninja** (recommended) or other CMake-compatible build system
- **OpenGL 4.5+**

### Build and Run

```sh
# Configure (Ninja single-config)
cmake -B build -G Ninja -D CMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure

# Launch application
./build/GOLExecutable/GOLExecutable.exe
```

For **multi-config generators** (Visual Studio, Xcode):
```sh
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
./build/GOLExecutable/Release/GOLExecutable.exe
```
## Usage Guide

### Launching the Application
Start GOLDE and you'll be greeted with an interactive grid and control panel.

### Controls
- **Play/Pause**: Start and stop the simulation
- **Step**: Advance one generation at a time
- **Speed Control**: Adjust simulation speed
- **Zoom**: Use mouse wheel to zoom in/out
- **Pan**: Click and drag to navigate the grid

### Selecting Patterns
- Access the **Preset Library** to load classic patterns (Glider, Blinker, Gosper Gun, etc.)
- Use the **Editor** to create custom patterns
- **Import/Export** patterns in a custom GOLDE format

### Algorithm Selection
- **HashLife**: For large patterns and exploring distant future states
- **SparseLife**: For general use and everyday patterns

## Project Structure

```
GameOfLife/
├── GOLAlgoLib/          # Core algorithms (HashLife, SparseLife, Quadtree)
├── GOLCoreLib/          # Game state & orchestration
├── GOLExecutable/       # Application entry point
├── GOLGraphicsLib/      # OpenGL rendering engine
├── GOLGuiLib/           # ImGui-based UI
├── GOLLoggingLib/       # Logging system
├── GOLTest/             # Unit and integration tests
├── Dependencies/        # Third-party libraries (ImGui, FontAwesome)
├── cmake/               # CMake utilities and compiler options
├── docs/                # Documentation and assets
└── build/               # Build output (generated)
```

### Dependencies
- OpenGL 4.5+
- [ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://www.glfw.org/)
- [GLEW](https://glew.sourceforge.net/)
- [GLM](https://github.com/g-truc/glm)
- [GoogleTest](https://github.com/google/googletest)
- [unordered_dense](https://github.com/martinus/unordered_dense/)
- [Native File Dialog Extended](https://github.com/btzy/nativefiledialog-extended)

## Development

### Code Standards
- **Language**: C++23 with modern idioms
- **Code Format**: Clang-format (run with `cmake --build build --target Format`)
- **Testing**: Comprehensive GTest suite
- **Compilation**: All warnings enabled and treated as errors in Release builds

### Running Tests Locally

```sh
# Run all tests
ctest --test-dir build --output-on-failure

# Run specific test
ctest --test-dir build -R TestName --output-on-failure

# Run with verbose output
ctest --test-dir build --verbose
```

### Understanding the Algorithms

**HashLife**: Advanced algorithm using quadtree spatial partitioning for massive performance gains. Ideal for:
- Exploring patterns millions of generations into the future
- Detecting blinkers and oscillators
- Pattern analysis and research

**SparseLife**: Hash-set based algorithm optimized for typical patterns. Best for:
- Interactive simulation and editing
- Real-time exploration
- Patterns with limited density

## Gallery

*Add GIF demonstrations here:*
- ***HashLife Performance Demo*** — millions of generations advancing in seconds
- ***Pattern Editor in Action*** — creating and modifying patterns
- ***Interactive Navigation*** — zooming and panning across the grid

## License

This project is licensed under the terms of the [MIT license](LICENSE)

# LunateEpsilon

**LunateEpsilon** is a high-performance desktop utility for converting and repairing M3U playlists with consistent Windows-native path formatting.
The application is built with **modern C++20** and **Qt 6 Widgets**, focusing on deterministic behavior, strong architecture, and a responsive user interface.

The project emphasizes **clean architecture, strict memory safety, and professional desktop UX**, while remaining lightweight and dependency-minimal.

————————————————————————————————————————————————————

# Overview

LunateEpsilon solves a common problem with media playlists: inconsistent file paths, incompatible slash formatting, and broken entries across systems.

The application processes playlists and produces corrected output with **strict Windows-compatible path normalization**, making large music libraries easier to maintain and migrate.

The program is designed to handle **large playlist files without freezing the UI**, ensuring a smooth experience even during heavy operations.

————————————————————————————————————————————————————

# Key Features

## Modern C++20 Implementation

Built using the **MSVC 2026 toolchain** with modern C++ idioms including:

* RAII-driven memory management
* Smart pointer usage
* Deterministic resource ownership
* No unnecessary raw pointer usage

The codebase prioritizes maintainability and predictable runtime behavior.

————————————————————————————————————————————————————

## Qt 6 Native Desktop UI

The interface uses **Qt 6 Widgets**, providing:

* Native Windows rendering
* High DPI compatibility
* Clean layout control
* Stable state transitions

The UI is intentionally minimal and functional.

————————————————————————————————————————————————————

## Asynchronous Processing

Playlist conversion runs outside the UI thread using **QtConcurrent**, preventing interface blocking during large playlist operations.

Benefits:

* Responsive UI during heavy conversions
* Safe cross-thread communication using Qt signals
* Deterministic processing pipeline

————————————————————————————————————————————————————

## Strict Windows Path Normalization

All playlist entries are normalized to enforce **single backslash paths**.

Example:

```
Input:
Music//Album///Track.mp3

Output:
Music\Album\Track.mp3
```

This ensures compatibility with Windows media players and file systems.

————————————————————————————————————————————————————

## Dynamic Theme System

The application includes a built-in theme switcher:

* **Light Theme**
* **Dark Theme**
* **AMOLED Theme**

The UI avoids layout shifting by anchoring the theme toggle to the **top-right corner with fixed dimensions**.

Application icons automatically change based on the active theme.

————————————————————————————————————————————————————

# Architecture

The project follows a strict separation of responsibilities.

```
UI Layer
 └── MainWindow

Business Logic
 └── Converter
```

### Design Principles

* No business logic inside UI classes
* No file processing inside the interface layer
* Threading never blocks the UI thread
* UI updates only through Qt signals/slots
* Deterministic state transitions

————————————————————————————————————————————————————

# Technical Specifications

**Version**

v2.0.0

**Platform**

Windows 10 / Windows 11
x64 architecture only

**Language**

C++20

**Framework**

Qt 6 Widgets

**Build System**

CMake + Ninja

**Encoding**

Full UTF-8 support using `QTextStream` for international file paths.

**Memory Management**

Strict RAII with modern smart pointer usage.

————————————————————————————————————————————————————

# Installation

1. Download the installer:

```
LunateEpsilon-setup-2.0.0-windows-x64.exe
```

2. Run the installer.

3. The installer will deploy:

* LunateEpsilon executable
* Required Qt 6 runtime dependencies

No additional configuration is required.

————————————————————————————————————————————————————

# Building From Source

## Requirements

* MSVC 2026 toolchain
* CMake ≥ 3.26
* Ninja
* Qt 6

## Build Steps

```powershell
git clone https://github.com/LAXMINARAYANBHARDWAJ/LunateEpsilon.git
cd LunateEpsilon

& "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x64 "-vcvars_ver=14.50.35717"

cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.10.2/msvc2022_64" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

& "C:\Qt\6.10.2\msvc2022_64\bin\windeployqt.exe" --no-translations --compiler-runtime release\LunateEpsilon.exe

& "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" installer.iss
```

The compiled binary will appear in:

```
build/Release
```

————————————————————————————————————————————————————

# Project Goals

LunateEpsilon is built around several long-term goals:

* Clean modern C++ architecture
* Deterministic desktop application behavior
* Minimal runtime overhead
* Native Windows experience
* Maintainable and extensible codebase

————————————————————————————————————————————————————

# License

This project is released under **The Unlicense**.

You are free to use, modify, distribute, and integrate the software without restriction.

See the LICENSE file for details.

————————————————————————————————————————————————————

<div align="center">

# ⚡ ApexReader

**A fast, lightweight, open-source PDF reader built as a modern Adobe Acrobat alternative.**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C++-20-00599C?logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![Qt](https://img.shields.io/badge/Qt-6-41CD52?logo=qt)](https://www.qt.io/)
[![CMake](https://img.shields.io/badge/CMake-3.20+-064F8C?logo=cmake)](https://cmake.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows)](https://www.microsoft.com/)

</div>

---

## 📖 About

**ApexReader** is a high-performance PDF reader for Windows, built from the ground up with modern **C++20** and **Qt 6**. It uses **PDFium** (Google's battle-tested PDF rendering engine) as its core rendering backend, delivering pixel-perfect document rendering with a clean, distraction-free interface.

The project was born out of frustration with bloated PDF readers — ApexReader is lean, fast, and puts the document front and center.

---

## ✨ Features

### Core Viewing
- 📄 **High-fidelity PDF rendering** via the PDFium engine
- 🔍 **Smooth zoom** with fit-to-width and fit-to-page modes
- 📑 **Thumbnail panel** for quick page navigation
- 🌙 **Night / Dark mode** for comfortable low-light reading
- 🖥️ **Distraction-free full-screen mode** (F11)
- ⌨️ **Keyboard navigation** (arrow keys, Page Up/Down, Home/End)

### Annotation & Markup
- 🖊️ **Text highlighting** with multiple colour options
- ___ **Underline annotations**
- 💬 **Sticky notes** (text comments) on any page
- 🗑️ **Annotation management** — view, edit, and delete remarks

### Document Operations
- 🖨️ **Print support** via Qt's native print pipeline
- 📋 **Page metadata** display (dimensions, page count)
- ⚡ **Asynchronous rendering** — UI never blocks during page load

---

## 🛠️ Tech Stack

| Component | Technology |
|-----------|-----------|
| Language | C++20 |
| UI Framework | Qt 6 (Widgets, Concurrent, PrintSupport) |
| PDF Engine | PDFium (Google) |
| Build System | CMake 3.20+ |
| Dependency Manager | vcpkg |
| Annotations | Custom `AnnotationManager` |
| Target Platform | Windows (x64) |

---

## 📸 Screenshots

> _Screenshots coming soon — stay tuned!_

---

## 🚀 Building from Source

### Prerequisites

| Tool | Version |
|------|---------|
| MSVC (Visual Studio Build Tools) | 2022 or later |
| CMake | 3.20+ |
| Qt 6 | 6.5+ (add to `PATH`) |
| vcpkg | latest |

### 1. Clone the repository

```bash
git clone https://github.com/YOUR_USERNAME/ApexReader.git
cd ApexReader
```

### 2. Obtain PDFium

Download the pre-built PDFium binary for Windows x64 and extract it into `third_party/`:

```
third_party/
├── bin/
│   └── pdfium.dll
├── lib/
│   └── pdfium.dll.lib
└── include/
    └── fpdfview.h  (+ other PDFium headers)
```

### 3. Configure & Build

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The executable `ApexReader.exe` will be placed inside `build/Release/`.

### 4. Run

```bash
.\build\Release\ApexReader.exe
```

> **Note:** `pdfium.dll` is automatically copied next to the executable by the CMake post-build step.

---

## 📁 Project Structure

```
ApexReader/
├── assets/          # Qt resource files (icons, etc.)
├── custom-ports/    # vcpkg custom port overrides
├── include/         # Public C++ headers
│   ├── MainWindow.h
│   ├── PdfView.h
│   ├── PdfDocument.h
│   ├── PdfRenderer.h
│   ├── AnnotationManager.h
│   └── PdfiumWorker.h
├── libs/            # Pre-built static/shared libraries
├── scripts/         # Helper build/deployment scripts
├── src/             # C++ source files
│   ├── main.cpp
│   ├── MainWindow.cpp
│   ├── PdfView.cpp
│   ├── PdfDocument.cpp
│   ├── PdfRenderer.cpp
│   ├── PdfiumWorker.cpp
│   └── AnnotationManager.cpp
├── third_party/     # PDFium binaries & headers (not tracked)
├── vcpkg/           # vcpkg submodule
└── CMakeLists.txt
```

---

## 🗺️ Roadmap

- [ ] Text search (Ctrl+F) with highlighted matches
- [ ] Copy selected text to clipboard
- [ ] Bookmark manager
- [ ] Recent files history
- [ ] PDF form field support
- [ ] Linux / macOS builds

---

## 🤝 Contributing

Contributions, issues and feature requests are welcome!  
Feel free to open an issue or submit a pull request.

1. Fork the project
2. Create your feature branch (`git checkout -b feat/amazing-feature`)
3. Commit your changes (`git commit -m 'feat: add amazing feature'`)
4. Push to the branch (`git push origin feat/amazing-feature`)
5. Open a Pull Request

---

## 📜 License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

<div align="center">
  Made with ❤️ and C++20
</div>

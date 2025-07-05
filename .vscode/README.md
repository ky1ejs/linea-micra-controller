# VS Code Configuration

This directory contains VS Code workspace configuration files to enhance the development experience for this PlatformIO ESP32-C6 project.

## Files

### `settings.json`
Configures VS Code to:
- Use clang-format for automatic code formatting (using the project's `.clang-format` file)
- Enable format-on-save for C/C++ files
- Configure cppcheck integration for static analysis
- Set up PlatformIO integration
- Hide build artifacts and temporary files

### `extensions.json`
Recommends essential extensions:
- **ms-vscode.cpptools**: C/C++ IntelliSense and debugging
- **platformio.platformio-ide**: PlatformIO IDE integration
- **ms-vscode.cmake-tools**: CMake support
- **cschlosser.doxdocgen**: Documentation generation
- **ms-vscode.vscode-clangd**: Advanced C++ language server (optional)

### `c_cpp_properties.json`
Configures IntelliSense for:
- ESP32-C6 specific includes and defines
- PlatformIO library dependencies
- Proper compiler paths and standards

## Benefits

With this configuration, VS Code will:
1. **Auto-format code** on save using the same rules as CI
2. **Show linting errors** in real-time using cppcheck
3. **Provide accurate IntelliSense** for ESP32 and PlatformIO libraries
4. **Suggest relevant extensions** for new contributors
5. **Maintain consistency** with the CI pipeline

## Getting Started

1. Install recommended extensions when prompted
2. VS Code will automatically apply the formatting and linting rules
3. Code will be auto-formatted on save to match CI requirements
4. Static analysis warnings will appear inline as you code
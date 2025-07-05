# Development Guidelines for Linea Micra Controller

This document provides comprehensive guidelines for building, analyzing, and maintaining the Linea Micra Controller project.

## Project Overview

The Linea Micra Controller is an ESP32-C6 based Arduino project that controls a Linea Micra espresso machine. It uses PlatformIO as the build system and includes automated code quality checks.

### Key Technologies
- **Platform**: ESP32-C6 (DevKit-C-1 board)
- **Framework**: Arduino
- **Build System**: PlatformIO
- **Code Quality**: clang-format + cppcheck
- **CI/CD**: GitHub Actions

## Quick Start Commands

### Essential Commands
```bash
# Set up the project (first time)
cp src/config.example.h src/config.h

# Fix all code quality issues
./scripts/fix-lint.sh

# Build the project
pio run --environment esp32-c6-devkitc-1

# Clean and rebuild
pio run --target clean && pio run --environment esp32-c6-devkitc-1

# Upload to device (with device connected)
pio run --target upload --environment esp32-c6-devkitc-1

# Monitor serial output
pio device monitor --environment esp32-c6-devkitc-1
```

### Code Quality Commands
```bash
# Format code automatically
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Run static analysis
cppcheck --enable=warning,performance,portability --std=c++11 --platform=unix32 \
  --suppress=missingIncludeSystem --suppress=unmatchedSuppression \
  --error-exitcode=1 --verbose --include=src/config.h src/ include/

# Check formatting without changing files
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror
```

## Project Structure

```
linea-micra-controller/
├── .github/workflows/ci.yml    # CI/CD pipeline
├── .vscode/                    # VS Code workspace settings
│   ├── settings.json           # Editor settings for clang-format integration
│   └── extensions.json         # Recommended extensions
├── docs/                       # Documentation
│   ├── LINTING.md              # Code quality and linting guide
│   └── DEVELOPMENT_GUIDELINES.md # This file
├── src/                        # Source code
│   ├── main.cpp                # Arduino main entry point
│   ├── config.example.h        # Configuration template
│   ├── config.h                # Local configuration (generated/gitignored)
│   ├── LineaMicra.*            # Main controller class
│   ├── WiFiManager.*           # WiFi connection management
│   ├── HomeAssistantClient.*   # Home Assistant integration
│   ├── RotaryEncoder.*         # Rotary encoder handling (template)
│   └── i2cScanner.*           # I2C device discovery utility
├── include/                    # Header files (if any)
├── scripts/fix-lint.sh         # Automated code quality fix script
├── platformio.ini              # PlatformIO configuration
├── .clang-format              # Code formatting rules
└── .gitignore                 # Git ignore rules
```

## PlatformIO Configuration

### Environment: esp32-c6-devkitc-1
- **Platform**: Custom ESP32 platform for C6 support
- **Framework**: Arduino
- **Monitor Speed**: 115200 baud
- **Custom Build Flags**: Debug level 3, USB CDC on boot, custom I2C pins (SDA=6, SCL=7)

### Key Dependencies
```ini
lib_deps =
    adafruit/Adafruit SH110X@^2.1.13      # OLED display
    adafruit/Adafruit BusIO@^1.17.1       # I2C/SPI bus abstraction
    adafruit/Adafruit GFX Library@^1.12.1 # Graphics library
    adafruit/Adafruit seesaw Library@^1.7.9 # GPIO expander
    gilmaimon/ArduinoWebsockets@^0.5.4    # WebSocket client
    bblanchon/ArduinoJson@^7.0.4          # JSON parsing
```

## Development Workflow

### 1. Initial Setup
```bash
# Copy configuration template
cp src/config.example.h src/config.h

# Edit config.h with your settings
# - WiFi credentials
# - Home Assistant configuration
# - Hardware pin assignments
```

### 2. Code Development
```bash
# Write code following project style guidelines
# Use VS Code with recommended extensions for best experience

# Before committing, always run:
./scripts/fix-lint.sh

# Review changes
git diff

# Test build
pio run --environment esp32-c6-devkitc-1
```

### 3. Testing
```bash
# Upload and monitor for hardware testing
pio run --target upload --environment esp32-c6-devkitc-1
pio device monitor --environment esp32-c6-devkitc-1

# Look for serial output, error messages, etc.
```

## Code Quality Standards

### Formatting Rules (.clang-format)
- **Style**: Google C++ with modifications
- **Indentation**: 2 spaces
- **Line Length**: 120 characters maximum
- **Braces**: Attach style (K&R)
- **Pointers**: Left-aligned (`int* ptr`)

### Static Analysis Rules
- **Tool**: cppcheck (offline analysis)
- **Enabled Checks**: warnings, performance, portability
- **Severity**: Fails on high-severity issues
- **Suppressions**: System includes, style-only issues

### Quality Gates
1. **Format Check**: All code must pass clang-format validation
2. **Static Analysis**: Must pass cppcheck analysis (warnings/errors only)
3. **Build Check**: Must compile successfully
4. **No Debug Code**: No debug print statements in production code

## CI/CD Pipeline

### GitHub Actions Workflow (.github/workflows/ci.yml)

**Triggers**: Push to main, Pull Requests to main

**Jobs**:
1. **format-check**: Validates code formatting with clang-format
2. **static-analysis**: Runs cppcheck + additional checks
3. **build**: Compiles project with PlatformIO

### Network Considerations
The CI environment has firewall restrictions that block PlatformIO registry access. The workflow is designed to work with system-available tools:
- Uses `cppcheck` instead of `pio check` for static analysis
- Caches PlatformIO dependencies when possible
- Falls back gracefully when network tools are unavailable

## Troubleshooting

### Build Issues
```bash
# Clean everything and rebuild
pio run --target clean
pio run --environment esp32-c6-devkitc-1

# Check for missing config.h
ls -la src/config.h || cp src/config.example.h src/config.h

# Verify platform/framework versions in platformio.ini
```

### Code Quality Issues
```bash
# See what would be changed by formatting
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror

# Run static analysis manually
cppcheck --enable=all --std=c++11 src/ include/

# Check for specific issues
grep -r "TODO\|FIXME" src/ include/
```

### CI/CD Issues
- **PlatformIO registry blocked**: Use offline tools (cppcheck instead of pio check)
- **Build cache issues**: Clear cache in GitHub Actions settings
- **Format failures**: Run `./scripts/fix-lint.sh` locally first

### Hardware Issues
```bash
# Scan I2C devices
# Use the built-in i2cScanner functionality

# Check serial connection
pio device list

# Monitor with timestamp
pio device monitor --environment esp32-c6-devkitc-1 --filter time
```

## VS Code Integration

### Recommended Extensions (.vscode/extensions.json)
- **PlatformIO IDE**: Complete PlatformIO integration
- **C/C++**: IntelliSense and debugging
- **Clang-Format**: Automatic code formatting
- **C/C++ Themes**: Better syntax highlighting

### Key Settings (.vscode/settings.json)
- Format on save enabled
- clang-format as default formatter
- Integrated terminal for PlatformIO commands
- File associations for PlatformIO config files

## Best Practices

### Code Organization
1. Keep `main.cpp` minimal - delegate to class methods
2. Use header guards or `#pragma once`
3. Separate interface (.h) from implementation (.cpp)
4. Use meaningful variable and function names
5. Add comments for complex logic, not obvious code

### Git Workflow
1. Always run `./scripts/fix-lint.sh` before committing
2. Review changes with `git diff` after formatting
3. Test build after formatting to ensure no breaking changes
4. Keep commits focused and atomic
5. Use descriptive commit messages

### Testing
1. Test on actual hardware when possible
2. Use serial output for debugging
3. Verify I2C devices are detected correctly
4. Test WiFi connectivity and Home Assistant integration
5. Monitor memory usage and performance

### Documentation
1. Update relevant documentation when making changes
2. Add inline comments for non-obvious code
3. Keep README and docs up to date
4. Document configuration options in config.example.h

## Hardware-Specific Notes

### ESP32-C6 DevKit-C-1
- **I2C Pins**: SDA=6, SCL=7 (custom assignment)
- **USB**: CDC on boot enabled
- **Debug Level**: Set to 3 for verbose output
- **WebSocket**: Configured for local network (no SSL)

### Connected Devices
- OLED Display (SH110X)
- Rotary Encoder
- GPIO Expander (seesaw)
- Linea Micra hardware interfaces

## File Management

### Files to Ignore (.gitignore)
- `src/config.h` (contains local configuration)
- `.vscode/c_cpp_properties.json` (auto-generated by PlatformIO)
- `cppcheck-*.txt` (static analysis output files)
- Build artifacts and PlatformIO cache

### Files to Commit
- All source code (.cpp, .h files)
- Configuration templates (config.example.h)
- Documentation and scripts
- CI/CD configuration
- VS Code workspace settings (but not auto-generated files)

## Common Maintenance Tasks

### Updating Dependencies
```bash
# Check for updates
pio pkg update

# Update specific library
pio pkg update adafruit/Adafruit\ SH110X

# Clean and rebuild after updates
pio run --target clean && pio run
```

### Code Refactoring
1. Run `./scripts/fix-lint.sh` before starting
2. Make incremental changes
3. Test build after each change
4. Run full script again after refactoring
5. Test on hardware if available

### Adding New Features
1. Follow existing code patterns
2. Add appropriate includes and dependencies to platformio.ini
3. Update config.example.h if new configuration is needed
4. Add documentation for new functionality
5. Test thoroughly before submitting PR

This guide should be updated as the project evolves and new patterns or requirements emerge.
# Claude AI Development Reference

This file provides quick reference for AI assistants working on the Linea Micra Controller project.

## Primary Reference

**See [docs/DEVELOPMENT_GUIDELINES.md](docs/DEVELOPMENT_GUIDELINES.md) for comprehensive development guidelines.**

This document contains everything needed to:
- Build and analyze the codebase
- Understand project structure and dependencies
- Follow code quality standards
- Use CI/CD pipeline effectively
- Troubleshoot common issues

## Quick Commands

```bash
# Essential setup
cp src/config.example.h src/config.h

# Fix all linting issues
./scripts/fix-lint.sh

# Build project
pio run --environment esp32-c6-devkitc-1

# Static analysis
cppcheck --enable=warning,performance,portability --std=c++11 --platform=unix32 \
  --suppress=missingIncludeSystem --suppress=unmatchedSuppression \
  --error-exitcode=1 --verbose --include=src/config.h src/ include/
```

## Key Project Details

- **Platform**: ESP32-C6 DevKit-C-1
- **Framework**: Arduino with PlatformIO
- **Code Quality**: clang-format + cppcheck
- **CI/CD**: GitHub Actions with offline static analysis
- **Network Limitations**: Use system tools instead of PlatformIO registry

## Always Remember

1. Run `./scripts/fix-lint.sh` before any commits
2. Test builds after code changes
3. Use offline tools (cppcheck) instead of `pio check` in restricted environments
4. Keep changes minimal and focused
5. Reference the full development guidelines for detailed procedures
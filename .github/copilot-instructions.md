# GitHub Copilot Instructions

This repository contains development guidelines that should be referenced when working on the Linea Micra Controller project.

## Development Guidelines Reference

**Primary Documentation**: [docs/DEVELOPMENT_GUIDELINES.md](../docs/DEVELOPMENT_GUIDELINES.md)

This comprehensive guide covers:

### Project Overview
- ESP32-C6 based Arduino project using PlatformIO
- Automated code quality checks with clang-format and cppcheck
- GitHub Actions CI/CD pipeline

### Essential Commands
- Setup: `cp src/config.example.h src/config.h`
- Linting: `./scripts/fix-lint.sh`
- Building: `pio run --environment esp32-c6-devkitc-1`
- Static Analysis: Use cppcheck (not `pio check` due to network restrictions)

### Code Quality Standards
- **Formatting**: Google C++ style with 2-space indentation, 120 char lines
- **Analysis**: cppcheck with warnings, performance, and portability checks
- **Quality Gates**: Format validation, static analysis, successful build

### Network Considerations
- CI environment blocks PlatformIO registry access
- Use offline tools: cppcheck instead of `pio check`
- System-available tools preferred for analysis

### File Management
- `src/config.h` is gitignored (copy from config.example.h)
- `.vscode/c_cpp_properties.json` is auto-generated, should be gitignored
- `cppcheck-*.txt` files are build artifacts, should be gitignored

### Best Practices
1. Always run lint script before commits
2. Make minimal, focused changes
3. Test builds after modifications
4. Use VS Code with recommended extensions for best experience
5. Reference full guidelines for detailed procedures
6. **Log development work**: Always create detailed build logs in `docs/copilot/build_logs/` with numeric prefixes (0001_, 0002_, etc.) for context preservation

### Development Logging
- Create markdown files in `docs/copilot/build_logs/` for each major development session
- Include implementation details, architecture decisions, and context
- Use these logs as context reminders for future development work
- **Index files with numeric prefixes**: Start with `0001_`, `0002_`, etc. (e.g., `0001_cloud_api_implementation_context.md`)
- Name files descriptively after the numeric prefix

## Quick Reference for AI Assistants

When working on this codebase:
- Follow the development guidelines in docs/DEVELOPMENT_GUIDELINES.md
- Use offline static analysis tools due to network restrictions
- Prioritize code quality and minimal changes
- Test builds and linting after modifications
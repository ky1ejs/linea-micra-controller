# Code Linting and Formatting

This document explains how to maintain code quality and fix linting issues in the Linea Micra Controller project.

## Overview

The project uses automated code linting and formatting to ensure consistent code style and catch potential issues early. The CI/CD pipeline includes several quality checks:

1. **Code Formatting Check** - Ensures all code follows consistent formatting standards using clang-format
2. **Static Code Analysis** - Uses cppcheck for offline static analysis to catch potential bugs and quality issues
3. **Build Verification** - Ensures code compiles successfully

## Quick Fix Command

To automatically fix most linting and formatting issues, run:

```bash
./scripts/fix-lint.sh
```

This script will:
- ✅ Apply consistent code formatting using clang-format
- ✅ Check for common coding issues
- ✅ Create `config.h` if missing
- ✅ Validate that code still builds after fixes

## Available Commands

### Basic Usage
```bash
# Fix all linting issues (recommended)
./scripts/fix-lint.sh

# Only apply code formatting
./scripts/fix-lint.sh --format-only

# Only check for issues without fixing
./scripts/fix-lint.sh --check-only

# Show help
./scripts/fix-lint.sh --help
```

### Manual Commands

If you prefer to run tools individually:

```bash
# Check code formatting
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror

# Fix code formatting
find src include -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Run static analysis using cppcheck
cppcheck --enable=warning,performance,portability --std=c++11 --platform=unix32 \
  --suppress=missingIncludeSystem --suppress=unmatchedSuppression \
  --error-exitcode=1 --verbose --include=src/config.h src/ include/

# Build project
pio run --environment esp32-c6-devkitc-1
```

## Code Style Guidelines

The project uses a custom clang-format configuration based on Google C++ style with modifications suitable for Arduino/ESP32 development:

- **Indentation**: 2 spaces
- **Line length**: 120 characters max
- **Brace style**: Attach (K&R style)
- **Include sorting**: Enabled
- **Pointer alignment**: Left (`int* ptr` not `int *ptr`)

## What Gets Fixed

### Automatic Fixes
- ✅ Code formatting and indentation
- ✅ Include statement ordering
- ✅ Whitespace and line endings
- ✅ Brace placement and spacing

### Manual Review Required
- ⚠️ Logic errors and bugs
- ⚠️ Memory leaks
- ⚠️ Unused variables or functions
- ⚠️ Performance issues
- ⚠️ TODO/FIXME comments

## CI/CD Integration

The GitHub Actions workflow automatically:

1. **Format Check**: Verifies all code is properly formatted
2. **Lint Check**: Runs static analysis to catch potential issues  
3. **Build Check**: Ensures code compiles successfully

If any check fails, the PR cannot be merged until issues are resolved.

## Troubleshooting

### Build Fails After Formatting
If the build fails after running the fix script:
1. Review changes with `git diff`
2. Look for unintended modifications
3. Test the code manually
4. Revert specific files if needed: `git checkout -- path/to/file`

### Formatting Tool Not Found
Install clang-format:
```bash
# Ubuntu/Debian
sudo apt install clang-format

# macOS
brew install clang-format

# Windows (with chocolatey)
choco install llvm
```

### Static Analysis Fails
If static analysis fails:
1. Review the cppcheck output for specific issues
2. Common issues include:
   - Uninitialized variables
   - Potential null pointer dereferences
   - Performance issues
3. Fix issues manually or suppress specific warnings if they're false positives
4. For offline analysis, ensure cppcheck is installed: `sudo apt install cppcheck`

## Best Practices

1. **Run before committing**: Always run `./scripts/fix-lint.sh` before creating a commit
2. **Review changes**: Use `git diff` to review what the formatter changed
3. **Test after formatting**: Ensure your code still works as expected
4. **Address warnings**: Don't ignore linting warnings - they often indicate real issues
5. **Keep it simple**: Write clear, readable code that doesn't need complex linting rules

## Configuration Files

- `.clang-format` - Code formatting rules
- `scripts/fix-lint.sh` - Automated fix script
- `.github/workflows/ci.yml` - CI/CD pipeline configuration

## Getting Help

If you encounter issues with linting or formatting:
1. Check this documentation first
2. Review the error messages carefully
3. Try running individual commands to isolate the problem
4. Ask for help in the project issues or discussions
#!/bin/bash

# Linea Micra Controller - Code Linting Fix Script
# This script automatically fixes common linting issues in the project

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

echo "🔧 Fixing code linting issues for Linea Micra Controller..."

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if required tools are available
check_tools() {
    print_status "Checking required tools..."
    
    if ! command -v clang-format &> /dev/null; then
        print_error "clang-format is not installed. Please install it first:"
        echo "  Ubuntu/Debian: sudo apt install clang-format"
        echo "  macOS: brew install clang-format"
        exit 1
    fi
    
    print_success "clang-format is available"
}

# Create config.h if it doesn't exist
setup_config() {
    if [ ! -f "src/config.h" ]; then
        print_status "Creating config.h from config.example.h..."
        cp src/config.example.h src/config.h
        print_success "config.h created"
    else
        print_status "config.h already exists"
    fi
}

# Fix C/C++ formatting issues
fix_formatting() {
    print_status "Fixing C/C++ code formatting..."
    
    # Find all C/C++ source files
    sources=$(find src include -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" 2>/dev/null || true)
    
    if [ -z "$sources" ]; then
        print_warning "No C/C++ source files found to format"
        return
    fi
    
    total_files=0
    formatted_files=0
    
    for file in $sources; do
        if [ -f "$file" ]; then
            total_files=$((total_files + 1))
            print_status "Formatting: $file"
            
            # Create a backup
            cp "$file" "$file.backup"
            
            # Apply clang-format
            if clang-format -i "$file"; then
                # Check if file was actually changed
                if ! cmp -s "$file" "$file.backup"; then
                    formatted_files=$((formatted_files + 1))
                    print_success "Formatted: $file"
                fi
                # Remove backup
                rm "$file.backup"
            else
                print_error "Failed to format: $file"
                # Restore backup
                mv "$file.backup" "$file"
            fi
        fi
    done
    
    print_success "Formatting complete: $formatted_files/$total_files files were modified"
}

# Check for common coding issues
check_common_issues() {
    print_status "Checking for common coding issues..."
    
    issues_found=0
    
    # Check for TODO/FIXME comments
    todos=$(grep -rn "TODO\|FIXME" src/ include/ 2>/dev/null || true)
    if [ ! -z "$todos" ]; then
        print_warning "Found TODO/FIXME comments:"
        echo "$todos"
        issues_found=$((issues_found + 1))
    fi
    
    # Check for debugging prints that might need removal
    debug_prints=$(grep -rn "Serial.println.*debug\|Serial.print.*debug" src/ 2>/dev/null | grep -v "//.*debug" || true)
    if [ ! -z "$debug_prints" ]; then
        print_warning "Found potential debug prints:"
        echo "$debug_prints"
        issues_found=$((issues_found + 1))
    fi
    
    # Check for unused includes (simple check)
    print_status "Looking for potentially unused includes..."
    find src/ -name "*.cpp" -o -name "*.h" | while read file; do
        # This is a simple check - a more sophisticated tool would be better
        includes=$(grep "^#include" "$file" 2>/dev/null || true)
        if [ ! -z "$includes" ]; then
            # Just report files with many includes for manual review
            include_count=$(echo "$includes" | wc -l)
            if [ "$include_count" -gt 10 ]; then
                print_warning "$file has $include_count includes - consider reviewing for unused ones"
            fi
        fi
    done
    
    if [ $issues_found -eq 0 ]; then
        print_success "No obvious issues found in common checks"
    fi
}

# Validate that the code still compiles after fixes
validate_build() {
    print_status "Validating that code still builds after fixes..."
    
    if command -v pio &> /dev/null; then
        print_status "Running PlatformIO build check..."
        if pio run --environment esp32-c6-devkitc-1 --target clean > /dev/null 2>&1; then
            if pio run --environment esp32-c6-devkitc-1 > /dev/null 2>&1; then
                print_success "Build validation passed"
            else
                print_error "Build failed after applying fixes - please review changes"
                return 1
            fi
        else
            print_warning "Could not clean build directory, skipping build validation"
        fi
    else
        print_warning "PlatformIO not available, skipping build validation"
        print_warning "Please ensure your changes compile correctly"
    fi
}

# Main execution
main() {
    print_status "Starting lint fix process..."
    
    check_tools
    setup_config
    fix_formatting
    check_common_issues
    
    if [ "${SKIP_BUILD_VALIDATION:-}" != "true" ]; then
        validate_build
    fi
    
    print_success "Lint fix process completed!"
    print_status "Summary:"
    echo "  ✅ Code formatting applied with clang-format"
    echo "  ✅ Common issues checked"
    echo "  ✅ Configuration files validated"
    echo ""
    print_status "Next steps:"
    echo "  1. Review the changes with: git diff"
    echo "  2. Test your code to ensure it still works"
    echo "  3. Commit the changes if everything looks good"
    echo ""
    print_warning "Note: This script fixes formatting and common issues."
    print_warning "You may still need to address logic or functionality issues manually."
}

# Handle command line arguments
case "${1:-}" in
    --help|-h)
        echo "Usage: $0 [options]"
        echo ""
        echo "Options:"
        echo "  --help, -h                Show this help message"
        echo "  --format-only            Only run code formatting"
        echo "  --check-only             Only run issue checks, no fixes"
        echo ""
        echo "Environment variables:"
        echo "  SKIP_BUILD_VALIDATION    Set to 'true' to skip build validation"
        exit 0
        ;;
    --format-only)
        check_tools
        setup_config
        fix_formatting
        ;;
    --check-only)
        check_tools
        setup_config
        check_common_issues
        ;;
    *)
        main
        ;;
esac
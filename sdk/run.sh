#!/bin/bash

# LINX SDK Build Script
# This script clones and builds third-party dependencies (mongoose, opus) and builds the main project

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_info() {
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

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRD_DIR="${SCRIPT_DIR}/third"
CMAKE_DIR="${SCRIPT_DIR}/cmake"

print_info "Starting LINX SDK build process..."
print_info "Script directory: ${SCRIPT_DIR}"
print_info "Third-party directory: ${THIRD_DIR}"

# Create third directory if it doesn't exist
mkdir -p "${THIRD_DIR}"

# Function to clone or update repository
clone_or_update_repo() {
    local repo_url="$1"
    local repo_name="$2"
    local repo_dir="${THIRD_DIR}/${repo_name}"
    
    if [ -d "${repo_dir}" ]; then
        print_info "Repository ${repo_name} already exists, updating..."
        cd "${repo_dir}"
        git pull origin main || git pull origin master || {
            print_warning "Failed to update ${repo_name}, continuing with existing version"
        }
    else
        print_info "Cloning ${repo_name} from ${repo_url}..."
        cd "${THIRD_DIR}"
        git clone "${repo_url}" "${repo_name}" || {
            print_error "Failed to clone ${repo_name}"
            exit 1
        }
    fi
}

# Function to build mongoose
build_mongoose() {
    print_info "Building mongoose..."
    local mongoose_dir="${THIRD_DIR}/mongoose"
    
    if [ ! -d "${mongoose_dir}" ]; then
        print_error "Mongoose directory not found"
        exit 1
    fi
    
    cd "${mongoose_dir}"
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure with cmake
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="${mongoose_dir}/install" \
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
        || {
        print_error "Failed to configure mongoose"
        exit 1
    }
    
    # Build
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || {
        print_error "Failed to build mongoose"
        exit 1
    }
    
    # Install
    make install || {
        print_error "Failed to install mongoose"
        exit 1
    }
    
    print_success "Mongoose built successfully"
}

# Function to build opus
build_opus() {
    print_info "Building opus..."
    local opus_dir="${THIRD_DIR}/opus"
    
    if [ ! -d "${opus_dir}" ]; then
        print_error "Opus directory not found"
        exit 1
    fi
    
    cd "${opus_dir}"
    
    # Check if configure script exists, if not run autogen.sh
    if [ ! -f "configure" ]; then
        print_info "Running autogen.sh for opus..."
        ./autogen.sh || {
            print_error "Failed to run autogen.sh for opus"
            exit 1
        }
    fi
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Configure
    ../configure \
        --prefix="${opus_dir}/install" \
        --enable-static \
        --enable-shared \
        --disable-doc \
        --disable-extra-programs \
        || {
        print_error "Failed to configure opus"
        exit 1
    }
    
    # Build
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || {
        print_error "Failed to build opus"
        exit 1
    }
    
    # Install
    make install || {
        print_error "Failed to install opus"
        exit 1
    }
    
    print_success "Opus built successfully"
}

# Function to build main project
build_main_project() {
    print_info "Building main LINX SDK project..."
    
    cd "${SCRIPT_DIR}"
    
    # Create build directory
    mkdir -p build
    cd build
    
    # Set up environment variables for finding third-party libraries
    export PKG_CONFIG_PATH="${THIRD_DIR}/opus/install/lib/pkgconfig:${PKG_CONFIG_PATH}"
    export CMAKE_PREFIX_PATH="${THIRD_DIR}/mongoose/install:${THIRD_DIR}/opus/install:${CMAKE_PREFIX_PATH}"
    
    # Configure with cmake using toolchains
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_MODULE_PATH="${CMAKE_DIR}" \
        -DMongoose_ROOT="${THIRD_DIR}/mongoose/install" \
        -DOPUS_ROOT="${THIRD_DIR}/opus/install" \
        || {
        print_error "Failed to configure main project"
        exit 1
    }
    
    # Build
    make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4) || {
        print_error "Failed to build main project"
        exit 1
    }
    
    print_success "Main project built successfully"
}

# Main execution flow
main() {
    print_info "=== LINX SDK Build Process Started ==="
    
    # Clone/update repositories
    print_info "=== Step 1: Cloning/Updating Dependencies ==="
    clone_or_update_repo "https://github.com/Automattic/mongoose.git" "mongoose"
    clone_or_update_repo "https://github.com/xiph/opus.git" "opus"
    
    # Build dependencies
    print_info "=== Step 2: Building Dependencies ==="
    build_mongoose
    build_opus
    
    # Build main project
    print_info "=== Step 3: Building Main Project ==="
    build_main_project
    
    print_success "=== LINX SDK Build Process Completed Successfully ==="
    print_info "Build artifacts are located in: ${SCRIPT_DIR}/build"
}

# Check if script is being sourced or executed
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
# CMake Modules

This directory contains CMake modules used by the container_system build system.

## UnifiedDependencies.cmake

A reusable CMake module for unified dependency resolution with multiple search strategies and automatic fallback mechanisms.

### Features

- **Multi-strategy search**: Searches for dependencies in multiple locations in priority order
- **FetchContent fallback**: Automatically downloads missing dependencies from Git
- **REQUIRED mode**: Supports both optional and required dependencies
- **Detailed logging**: Provides clear status messages for debugging

### Usage

```cmake
# Include the module
include(cmake/UnifiedDependencies.cmake)

# Find a required dependency
unified_find_dependency(common_system REQUIRED)

# Use the resolved include directory
target_include_directories(my_target PUBLIC
    $<BUILD_INTERFACE:${common_system_INCLUDE_DIR}>
)
```

### Search Order

The module searches for dependencies in the following order:

1. **Cache variable**: `${DEP_NAME}_ROOT` (e.g., `COMMON_SYSTEM_ROOT`)
2. **Environment variable**: `$ENV{${DEP_NAME}_ROOT}` (e.g., `$ENV{COMMON_SYSTEM_ROOT}`)
3. **Sibling directory**: `${CMAKE_CURRENT_SOURCE_DIR}/../${dep_name}`
4. **Subdirectory**: `${CMAKE_CURRENT_SOURCE_DIR}/${dep_name}`
5. **FetchContent**: Downloads from configured Git repository (if `UNIFIED_ALLOW_FETCHCONTENT=ON`)

### Output Variables

After calling `unified_find_dependency()`, the following variables are set:

| Variable | Description |
|----------|-------------|
| `${dep_name}_FOUND` | TRUE if dependency was found |
| `${dep_name}_INCLUDE_DIR` | Include directory path |
| `${dep_name}_SOURCE_DIR` | Source directory path |
| `${dep_name}_TARGET` | Target name (if applicable) |

### Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `UNIFIED_ALLOW_FETCHCONTENT` | ON | Enable automatic download via FetchContent |
| `UNIFIED_COMMON_SYSTEM_GIT_URL` | GitHub URL | Git URL for common_system |
| `UNIFIED_COMMON_SYSTEM_GIT_TAG` | main | Git tag/branch for common_system |

### Examples

#### Using Cache Variable

```bash
cmake -DCOMMON_SYSTEM_ROOT=/path/to/common_system ..
```

#### Using Environment Variable

```bash
export COMMON_SYSTEM_ROOT=/path/to/common_system
cmake ..
```

#### Disabling FetchContent

```bash
cmake -DUNIFIED_ALLOW_FETCHCONTENT=OFF ..
```

### Adding New Dependencies

To add support for a new dependency, modify the `unified_find_dependency` macro:

1. Add header patterns for the new dependency
2. Configure FetchContent URL and tag

```cmake
# In the macro, add:
if("${_dep_name}" STREQUAL "new_dependency")
    set(_header_patterns
        "include/new_dependency/header.h"
    )
endif()

# For FetchContent:
set(UNIFIED_NEW_DEPENDENCY_GIT_URL "https://github.com/org/new_dependency.git" CACHE STRING "")
set(UNIFIED_NEW_DEPENDENCY_GIT_TAG "main" CACHE STRING "")
```

## Other Modules

| Module | Description |
|--------|-------------|
| `ContainerSystemConfig.cmake.in` | Package configuration template for find_package() |
| `Version.cmake` | Version information and semantic versioning utilities |
| `toolchain-arm64-linux.cmake` | Cross-compilation toolchain for ARM64 Linux |

## Contributing

When adding or modifying CMake modules:

1. Follow CMake best practices and modern CMake idioms
2. Add comprehensive documentation
3. Test on all supported platforms (Linux, macOS, Windows)
4. Update this README with any new modules

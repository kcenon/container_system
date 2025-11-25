## Container System {VERSION}

### Highlights

{HIGHLIGHTS}

### Changes

{CHANGELOG}

### Downloads

| Platform | Architecture | Download |
|----------|--------------|----------|
| Linux | x64 | [container-system-{VERSION}-linux-x64.tar.gz](../../releases/download/{VERSION}/container-system-{VERSION}-linux-x64.tar.gz) |
| macOS | ARM64 | [container-system-{VERSION}-macos-arm64.tar.gz](../../releases/download/{VERSION}/container-system-{VERSION}-macos-arm64.tar.gz) |
| Windows | x64 | [container-system-{VERSION}-windows-x64.zip](../../releases/download/{VERSION}/container-system-{VERSION}-windows-x64.zip) |
| Headers | All | [container-system-{VERSION}-headers.tar.gz](../../releases/download/{VERSION}/container-system-{VERSION}-headers.tar.gz) |

### Installation

**Linux/macOS:**
```bash
# Extract library
tar -xzvf container-system-{VERSION}-linux-x64.tar.gz -C /usr/local/lib

# Extract headers
tar -xzvf container-system-{VERSION}-headers.tar.gz -C /usr/local/include
```

**Windows:**
1. Extract the ZIP file to your desired location
2. Add the `lib` directory to your library path
3. Add the `include` directory to your include path

### CMake Integration

```cmake
# Option 1: FetchContent (recommended)
include(FetchContent)
FetchContent_Declare(
    container_system
    GIT_REPOSITORY https://github.com/kcenon/container_system.git
    GIT_TAG {VERSION}
)
FetchContent_MakeAvailable(container_system)

target_link_libraries(your_target PRIVATE container_system)

# Option 2: find_package (if installed)
find_package(ContainerSystem REQUIRED)
target_link_libraries(your_target PRIVATE ContainerSystem::container)
```

### Requirements

- **Compiler**: C++20 compatible (GCC 13+, Clang 15+, MSVC 2022+)
- **Build System**: CMake 3.16+
- **Dependencies**: Automatically managed via vcpkg or system packages

### Compatibility

This release is compatible with:
- Linux (x64, ARM64)
- macOS (ARM64)
- Windows (x64)

### Documentation

- [API Documentation](https://kcenon.github.io/container_system/)
- [Migration Guide](docs/MIGRATION.md)
- [Examples](examples/)

### Support

- [Issue Tracker](../../issues)
- [Discussions](../../discussions)

---

**Full Changelog**: [{PREV_VERSION}...{VERSION}](../../compare/{PREV_VERSION}...{VERSION})

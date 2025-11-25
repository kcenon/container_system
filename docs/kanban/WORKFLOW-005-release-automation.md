# WORKFLOW-005: Release Automation

## Metadata
- **ID**: WORKFLOW-005
- **Priority**: LOW
- **Estimated Time**: 1 day
- **Dependencies**: None (WORKFLOW-001 completed)
- **Status**: DONE
- **Assignee**: TBD
- **Related Documents**: [WORKFLOW_IMPROVEMENTS.md](../../.github/workflows/WORKFLOW_IMPROVEMENTS.md)

---

## Overview

### What are we trying to change?

Build an automated pipeline that automatically creates a GitHub Release when a Git tag is created and attaches build artifacts from all platforms.

**Current State**:
- Manual release process
- Manual artifact upload

**Goals**:
- Tag-based automatic releases
- Multi-platform binary packaging
- Automatic changelog generation
- Automatic release notes creation

---

## Changes

### How are we going to implement this?

#### 1. Release Workflow

**`.github/workflows/release.yaml`**:
```yaml
name: Release

on:
  push:
    tags:
      - 'v*'

permissions:
  contents: write

jobs:
  # Build jobs
  build-linux:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build
      - name: Package
        run: |
          mkdir -p artifacts
          tar -czvf artifacts/container-system-linux-x64.tar.gz \
            -C build/lib . \
            -C ../build/bin .
      - uses: actions/upload-artifact@v4
        with:
          name: linux-x64
          path: artifacts/

  build-macos:
    runs-on: macos-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: |
          cmake -B build -DCMAKE_BUILD_TYPE=Release
          cmake --build build
      - name: Package
        run: |
          mkdir -p artifacts
          tar -czvf artifacts/container-system-macos-arm64.tar.gz \
            -C build/lib . \
            -C ../build/bin .
      - uses: actions/upload-artifact@v4
        with:
          name: macos-arm64
          path: artifacts/

  build-windows:
    runs-on: windows-latest
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: |
          cmake -B build -G "Visual Studio 17 2022" -A x64
          cmake --build build --config Release
      - name: Package
        run: |
          mkdir artifacts
          Compress-Archive -Path build/bin/Release/*,build/lib/Release/* `
            -DestinationPath artifacts/container-system-windows-x64.zip
      - uses: actions/upload-artifact@v4
        with:
          name: windows-x64
          path: artifacts/

  # Create release
  create-release:
    needs: [build-linux, build-macos, build-windows]
    runs-on: ubuntu-latest

    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0

      - name: Download All Artifacts
        uses: actions/download-artifact@v4
        with:
          path: artifacts

      - name: Generate Changelog
        id: changelog
        run: |
          # Find previous tag
          PREV_TAG=$(git describe --tags --abbrev=0 HEAD^ 2>/dev/null || echo "")

          if [ -n "$PREV_TAG" ]; then
            echo "Generating changelog from $PREV_TAG to ${{ github.ref_name }}"
            git log $PREV_TAG..HEAD --pretty=format:"- %s (%h)" > changelog.md
          else
            echo "First release"
            echo "Initial release" > changelog.md
          fi

      - name: Create Release
        uses: softprops/action-gh-release@v1
        with:
          name: Release ${{ github.ref_name }}
          body_path: changelog.md
          draft: false
          prerelease: ${{ contains(github.ref_name, '-rc') || contains(github.ref_name, '-beta') }}
          files: |
            artifacts/linux-x64/*
            artifacts/macos-arm64/*
            artifacts/windows-x64/*
```

#### 2. Version Management

**`cmake/Version.cmake`**:
```cmake
# Extract version from Git tag
find_package(Git QUIET)

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if(GIT_VERSION MATCHES "^v([0-9]+)\\.([0-9]+)\\.([0-9]+)")
        set(VERSION_MAJOR ${CMAKE_MATCH_1})
        set(VERSION_MINOR ${CMAKE_MATCH_2})
        set(VERSION_PATCH ${CMAKE_MATCH_3})
    endif()
endif()

# Default version
if(NOT DEFINED VERSION_MAJOR)
    set(VERSION_MAJOR 2)
    set(VERSION_MINOR 0)
    set(VERSION_PATCH 0)
endif()

set(PROJECT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
```

#### 3. Release Notes Template

**`.github/RELEASE_TEMPLATE.md`**:
```markdown
## Container System $VERSION

### Highlights
- [Auto-generated highlights]

### Changes
$CHANGELOG

### Downloads
| Platform | Architecture | Download |
|----------|-------------|----------|
| Linux | x64 | [container-system-linux-x64.tar.gz]() |
| macOS | ARM64 | [container-system-macos-arm64.tar.gz]() |
| Windows | x64 | [container-system-windows-x64.zip]() |

### Installation

**Linux/macOS:**
```bash
tar -xzvf container-system-*.tar.gz
```

**Windows:**
Extract the ZIP file to your desired location.

### Compatibility
- C++17 or later
- CMake 3.16+
- vcpkg for dependency management
```

#### 4. Pre-release Support

```yaml
# Tags like v1.2.3-rc1, v1.2.3-beta are treated as pre-release
prerelease: ${{ contains(github.ref_name, '-rc') || contains(github.ref_name, '-beta') || contains(github.ref_name, '-alpha') }}
```

---

## Test Plan

### How will we test this?

#### 1. Draft Release Test

```yaml
# Set draft: true for testing
draft: true
```

Create tag and verify draft release

#### 2. Local Packaging Test

```bash
# Linux packaging test
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
mkdir -p artifacts
tar -czvf artifacts/container-system-linux-x64.tar.gz \
  -C build/lib . -C ../build/bin .

# Verify archive contents
tar -tzvf artifacts/container-system-linux-x64.tar.gz
```

#### 3. Changelog Generation Test

```bash
# Commit log from previous tag to current
git log v1.0.0..HEAD --pretty=format:"- %s (%h)"
```

#### Success Criteria
- [ ] All platform build artifacts generated
- [ ] GitHub Release automatically created
- [ ] Changelog automatically included
- [ ] Pre-release correctly marked

---

## Release Process

### Standard Release

```bash
# 1. Verify version and create tag
git tag -a v2.0.0 -m "Release v2.0.0"

# 2. Push tag
git push origin v2.0.0

# 3. GitHub Actions automatically:
#    - Builds for all platforms
#    - Packages artifacts
#    - Creates Release
#    - Attaches changelog
```

### Pre-release

```bash
# RC version
git tag -a v2.0.0-rc1 -m "Release Candidate 1"
git push origin v2.0.0-rc1

# Beta version
git tag -a v2.0.0-beta -m "Beta Release"
git push origin v2.0.0-beta
```

---

## Checklist

- [x] Write release.yaml workflow
- [x] Write Version.cmake
- [x] Write release notes template
- [x] Linux packaging test
- [x] macOS packaging test
- [x] Windows packaging test
- [x] Changelog generation test
- [x] Draft release test
- [x] Actual release test

---

**Created**: 2025-11-23
**Last Modified**: 2025-11-25
**Completed**: 2025-11-25

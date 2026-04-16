# Changelog

> **Note**: The [docs/CHANGELOG.md](docs/CHANGELOG.md) file carries the registry `doc_id: CNT-PROJ-002` (SSOT tag), but this root-level `CHANGELOG.md` remains the file normally consumed by downstream tooling (GitHub releases, vcpkg ports, crates). Entries should be mirrored between both files until the SSOT is consolidated. See also [docs/README.md](docs/README.md) for the registry.

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.0.0] - 2026-04-16

### Added

- Add `recommended_container` alias using `indexed_storage_policy` for O(1) key lookups ([#446](https://github.com/kcenon/container_system/issues/446))
- Add vcpkg port validation workflow for cross-platform install and consumer-test verification ([#458](https://github.com/kcenon/container_system/issues/458))
- Add `testing`, `samples`, and `docs` features to vcpkg port manifest, mapped to CMake options via `vcpkg_check_features` ([#501](https://github.com/kcenon/container_system/issues/501))
- Add Result\<T\>-based deserialization for `value_store` (`deserialize_result`, `deserialize_binary_result`) ([#517](https://github.com/kcenon/container_system/issues/517))

### Changed

- Remove vestigial `is_variant_mode()` and `enable_variant_mode()` no-op methods ([#516](https://github.com/kcenon/container_system/issues/516))
- Remove unused `CONTAINER_LEGACY_API` CMake option — legacy API was fully removed in prior releases ([#516](https://github.com/kcenon/container_system/issues/516))
- Remove unused `POLICY_CONTAINER_HAS_COMMON_RESULT` compatibility macro ([#516](https://github.com/kcenon/container_system/issues/516))

### Fixed

- Enable pointer range validation in release builds for `fixed_block_pool` ([#447](https://github.com/kcenon/container_system/issues/447))
- Correct vcpkg port `PACKAGE_NAME` and `CONFIG_PATH` from `ContainerSystem` to `container_system` so `vcpkg_cmake_config_fixup` matches the actual install layout (`lib/cmake/container_system`) ([#501](https://github.com/kcenon/container_system/issues/501))
- Apply Korean documentation parity with the mechanical fixes from [#513](https://github.com/kcenon/container_system/pull/513): remove fabricated `v2.0.0` version claim in `README.kr.md`, drop non-existent `BUILD_GUIDE.md` references in `docs/PROJECT_STRUCTURE.kr.md`, and align `docs/API_REFERENCE.kr.md` version to `0.1.0` matching `vcpkg.json` and `CMakeLists.txt` ([#514](https://github.com/kcenon/container_system/issues/514))

## [0.1.0] - 2026-03-13

### Added
- Type-safe value container with strongly-typed value system
- SIMD-accelerated serialization (1.8M serializations/sec, 25M SIMD ops/sec)
- Thread-safe concurrent operations
- Binary and JSON serialization formats
- Policy-based container variants (optimized, typed, policy)
- gRPC isolated module support
- C++20 module support
- CMake install/export for find_package support (#410)
- Dependabot and OSV-Scanner vulnerability monitoring (#400)
- SBOM generation and CVE scanning workflows (#396)
- IEC 62304 SOUP compliance documentation

### Infrastructure
- GitHub Actions CI/CD with sanitizer testing
- Doxygen documentation workflow
- vcpkg manifest with feature-based configuration
- codecov.io integration
- Security scan (dependency-security-scan.yml)
- Cross-platform support (Linux, macOS, Windows, ARM64)

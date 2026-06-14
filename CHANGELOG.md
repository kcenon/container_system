# Changelog

> **Note**: The [docs/CHANGELOG.md](docs/CHANGELOG.md) file carries the registry `doc_id: CNT-PROJ-002` (SSOT tag), but this root-level `CHANGELOG.md` remains the file normally consumed by downstream tooling (GitHub releases, vcpkg ports, crates). Entries should be mirrored between both files until the SSOT is consolidated. See also [docs/README.md](docs/README.md) for the registry.

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- **BREAKING (error-code values)**: Renumber all `error_codes::` and `validation_codes::` constants from their old positive bands (1xx/2xx/3xx/4xx/5xx) to **negative** values inside common_system's reserved `container_system` range `[-499, -400]`. The shared `kcenon::common` error registry classifies any non-negative code as `Success`/`Invalid`, so the previous positive codes were mis-classified once stored in `common::error_info.code`; the new values classify correctly as `ContainerSystem`. Sub-bands: value `-400..-409`, validation `-410..-419`, serialization `-420..-429`, resource `-430..-439`, thread-safety `-440..-449`, schema-level validation `-450..-459`. Obvious codes are aligned with `kcenon::common::error::codes::container_system::` (`type_mismatch == value_type_mismatch == -400`, `invalid_value == invalid_value_type == -401`, `serialization_failed == -420`, `deserialization_failed == -421`, `invalid_format == -422`). `get_category()` was updated to the negative sub-bands (same returned category strings). The numeric value of every container error code changes — any consumer comparing against literal codes, persisting them, or transmitting them across the wire MUST update. Constant names are unchanged; code that uses the named constants needs no change. Ships as **v1.1.0**, coordinated with the ecosystem (version not bumped in this change — owner's release decision). ([#551](https://github.com/kcenon/container_system/issues/551))
- Repoint Doxygen `INPUT`/`INCLUDE_PATH` and `cmake/documentation.cmake` `DOXYGEN_INPUT` to the canonical `include/kcenon/container/` + `src/` layout. Stale legacy references to `core/`, `internal/`, `integration/`, `values/` were removed; root-level `mainpage.dox` was repointed to the actual `docs/mainpage.dox` and the remaining tutorial/FAQ `.dox` pages were added explicitly. ([#536](https://github.com/kcenon/container_system/issues/536))
- Tidy `cmake/install.cmake` documentation block to reflect the post-migration install layout (`include/kcenon/container/` canonical + `include/container/` deprecated). The deprecated forwarding tree's removal milestone (v1.2.0) is now recorded as a TODO marker next to its install rule. ([#536](https://github.com/kcenon/container_system/issues/536))

### Deprecated

- Legacy forwarding header `include/container/optimizations/fast_parser.h` is deprecated; downstream consumers should include `<kcenon/container/optimizations/fast_parser.h>` instead. The legacy header now emits a build-time `#pragma message` warning. Scheduled for removal in the next minor release after v1.1.0. ([#534](https://github.com/kcenon/container_system/issues/534))

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

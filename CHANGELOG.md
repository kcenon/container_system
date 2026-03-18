# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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

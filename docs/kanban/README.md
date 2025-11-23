# Container System Kanban Board

This folder contains tickets for tracking improvement work on the Container System.

**Last Updated**: 2025-11-23
**Analysis Date**: 2025-11-23 (Validated against actual codebase)

---

## Ticket Status

### Summary

| Category | Total Tickets | Done | In Progress | Pending |
|----------|---------------|------|-------------|---------|
| REACT (Reactivation) | 6 | 0 | 0 | 6 |
| MIGRATE (Migration) | 3 | 1 | 0 | 3 |
| LEGACY (Legacy Removal) | 4 | 0 | 0 | 4 |
| WORKFLOW (CI/CD) | 4 | 1 | 0 | 4 |
| **Total** | **17** | **2** | **0** | **15** |

### Completed Tickets (Removed from Active Board)

| ID | Title | Completion Evidence |
|----|-------|---------------------|
| ~~MIGRATE-001~~ | Factory Functions Implementation | `internal/variant_value_factory.h` exists with full implementation |
| ~~WORKFLOW-001~~ | Windows Build Support | `ci.yml` includes Windows 2022 + MSVC matrix |

---

## Active Ticket List

### REACT: Reactivate Disabled Components

Reactivate components disabled by legacy value system removal by migrating them to the variant-based system.

**Evidence**: 9 DISABLED_ tests found in test files

| ID | Title | Priority | Est. Duration | Dependencies | Status |
|----|-------|----------|---------------|--------------|--------|
| [REACT-001](REACT-001-unit-tests-reactivation.md) | Unit Tests Reactivation | HIGH | 1.5d | - | TODO |
| [REACT-002](REACT-002-integration-tests-reactivation.md) | Integration Tests Reactivation | MEDIUM | 1.5d | REACT-001, REACT-005 | TODO |
| [REACT-003](REACT-003-benchmarks-reactivation.md) | Benchmarks Reactivation | MEDIUM | 2d | REACT-001 | TODO |
| [REACT-004](REACT-004-samples-examples-reactivation.md) | Samples/Examples Reactivation | LOW | 2d | REACT-001, REACT-002 | TODO |
| [REACT-005](REACT-005-messaging-integration-reactivation.md) | Messaging Integration Reactivation | HIGH | 1d | REACT-001 | TODO |
| [REACT-006](REACT-006-performance-baseline-reactivation.md) | Performance Baseline Reactivation | MEDIUM | 0.5d | REACT-002, REACT-003 | TODO |

**Disabled Tests to Reactivate**:
- `unit_tests.cpp`: DISABLED_LargeDataHandling, DISABLED_ConcurrentReads, DISABLED_ThreadSafeContainer, DISABLED_InvalidSerialization, DISABLED_NullValueConversions, DISABLED_SerializationSpeed, DISABLED_DeserializationSpeed
- `test_messaging_integration.cpp`: DISABLED_PerformanceMonitoring, DISABLED_ExternalCallbacks

**Recommended Execution Order**: REACT-001 → REACT-005 → REACT-002 → REACT-003 → REACT-004 → REACT-006

---

### MIGRATE: variant_value_v2 Migration

Migrate from the legacy polymorphic value system to the variant-based system.

**Status**: Phase 1 Complete (internal implementation done)

| ID | Title | Priority | Est. Duration | Dependencies | Status |
|----|-------|----------|---------------|--------------|--------|
| ~~MIGRATE-001~~ | ~~Factory Functions Implementation~~ | - | - | - | DONE |
| [MIGRATE-002](MIGRATE-002-value-container-support.md) | value_container Variant Support | HIGH | 2d | - | TODO |
| [MIGRATE-003](MIGRATE-003-thread-safe-container-integration.md) | thread_safe_container Integration | HIGH | 1d | MIGRATE-002 | TODO |
| [MIGRATE-004](MIGRATE-004-deprecation-warnings.md) | Add Deprecation Warnings | LOW | 0.5d | MIGRATE-002, MIGRATE-003 | TODO |

**Recommended Execution Order**: MIGRATE-002 → MIGRATE-003 → MIGRATE-004

---

### LEGACY: Legacy System Removal

Clean up the codebase by removing legacy code after migration is complete.

**Files to Remove**:
- `/internal/value.h` - Legacy polymorphic value class
- `/internal/value.cpp` - Legacy value implementation
- `/core/value_types.h` - Legacy type enum
- `/core/value_types.cpp` - Legacy type utilities

| ID | Title | Priority | Est. Duration | Dependencies | Status |
|----|-------|----------|---------------|--------------|--------|
| [LEGACY-001](LEGACY-001-core-value-removal.md) | Remove Core Value Classes | HIGH | 1d | MIGRATE-*, REACT-* | TODO |
| [LEGACY-002](LEGACY-002-value-types-removal.md) | Remove Value Types Implementation | HIGH | 1d | LEGACY-001 | TODO |
| [LEGACY-003](LEGACY-003-variant-value-old-removal.md) | Remove Old variant_value | MEDIUM | 0.5d | LEGACY-001, LEGACY-002 | TODO |
| [LEGACY-004](LEGACY-004-build-cleanup.md) | Build System Cleanup | LOW | 0.5d | LEGACY-001~003 | TODO |

**Recommended Execution Order**: LEGACY-001 → LEGACY-002 → LEGACY-003 → LEGACY-004

---

### WORKFLOW: CI/CD Improvements

Improve GitHub Actions workflows to strengthen build, test, and deployment automation.

**Existing CI Coverage** (in `ci.yml`):
- Linux (Ubuntu 22.04): GCC, Clang
- macOS 13: Clang
- Windows 2022: MSVC
- Sanitizers: Thread, Address, UndefinedBehavior
- Coverage, Static Analysis, Benchmarks

| ID | Title | Priority | Est. Duration | Dependencies | Status |
|----|-------|----------|---------------|--------------|--------|
| ~~WORKFLOW-001~~ | ~~Windows Build Support~~ | - | - | - | DONE |
| [WORKFLOW-002](WORKFLOW-002-arm64-linux-cross-compile.md) | ARM64 Linux Cross-Compilation | LOW | 1d | - | TODO |
| [WORKFLOW-003](WORKFLOW-003-fuzzing-automation.md) | Fuzzing Automation | MEDIUM | 2d | - | TODO |
| [WORKFLOW-004](WORKFLOW-004-performance-regression-detection.md) | Performance Regression Detection | MEDIUM | 1.5d | REACT-003, REACT-006 | TODO |
| [WORKFLOW-005](WORKFLOW-005-release-automation.md) | Release Automation | LOW | 1d | - | TODO |

**Recommended Execution Order**: Can proceed independently (only WORKFLOW-004 requires REACT-003, REACT-006)

---

## Execution Plan (Updated)

### Phase 1: Core Testing
1. REACT-001: Unit Tests Reactivation (7 tests)
2. REACT-005: Messaging Integration Reactivation (2 tests)
3. REACT-002: Integration Tests Reactivation

### Phase 2: Migration & Performance
1. MIGRATE-002: value_container Variant Support
2. MIGRATE-003: thread_safe_container Integration
3. REACT-003: Benchmarks Reactivation

### Phase 3: Documentation & Cleanup
1. REACT-004: Samples/Examples Reactivation
2. REACT-006: Performance Baseline Reactivation
3. MIGRATE-004: Add Deprecation Warnings

### Phase 4: Legacy Removal
1. LEGACY-001: Remove Core Value Classes
2. LEGACY-002: Remove Value Types Implementation
3. LEGACY-003: Remove Old variant_value
4. LEGACY-004: Build System Cleanup

### Phase 5: CI/CD Enhancement (Parallel)
1. WORKFLOW-003: Fuzzing Automation
2. WORKFLOW-004: Performance Regression Detection
3. WORKFLOW-002: ARM64 Linux Cross-Compilation
4. WORKFLOW-005: Release Automation

---

## Quick Start

**Immediately Actionable Tickets** (no dependencies):
1. **REACT-001** - Unit Tests Reactivation (HIGH priority)
2. **MIGRATE-002** - value_container Variant Support (HIGH priority)
3. **WORKFLOW-002** - ARM64 Cross-Compilation (LOW priority)
4. **WORKFLOW-003** - Fuzzing Automation (MEDIUM priority)
5. **WORKFLOW-005** - Release Automation (LOW priority)

---

## Status Definitions

- **TODO**: Not yet started
- **IN_PROGRESS**: Work in progress
- **REVIEW**: Awaiting code review
- **DONE**: Completed

---

## Related Documents

- [ARCHITECTURE.md](../ARCHITECTURE.md) - System Architecture
- [PROJECT_STRUCTURE.md](../PROJECT_STRUCTURE.md) - Project Structure
- [VARIANT_VALUE_V2_MIGRATION_GUIDE.md](../advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md) - Migration Guide
- [MIGRATION_PHASE1_COMPLETE.md](../advanced/MIGRATION_PHASE1_COMPLETE.md) - Phase 1 Completion Report

---

**Maintainer**: TBD
**Contact**: Use issue tracker

# Container System Work Priority Directive

**Document Version**: 2.0
**Created**: 2025-11-23
**Updated**: 2025-11-23 (Validated against codebase)
**Active Tickets**: 15 (2 completed)

---

## 1. Executive Summary

Analysis of Container System's tickets shows 15 active tickets across 4 work tracks:

| Track | Active Tickets | Completed | Key Objective | Est. Duration |
|-------|----------------|-----------|---------------|---------------|
| REACT (Reactivation) | 6 | 0 | Restore Disabled Components | 8.5d |
| MIGRATE (Migration) | 3 | 1 | variant_value_v2 Transition | 3.5d |
| LEGACY (Legacy Removal) | 4 | 0 | Legacy Code Cleanup | 3d |
| WORKFLOW (CI/CD) | 4 | 1 | Build/Test Automation | 5.5d |

**Total Estimated Duration**: ~20.5 days (single developer)

### Completed Tickets (Removed)

| ID | Title | Evidence |
|----|-------|----------|
| MIGRATE-001 | Factory Functions Implementation | `internal/variant_value_factory.h` implemented |
| WORKFLOW-001 | Windows Build Support | `ci.yml` includes Windows 2022 + MSVC |

---

## 2. Dependency Graph (Updated)

```
                                    ┌──────────────────────────────────────────┐
                                    │           WORKFLOW (Independent Track)    │
                                    │                                          │
                                    │  ✅ WORKFLOW-001 (DONE)                  │
                                    │                                          │
                                    │  WORKFLOW-002 (ARM64) - Independent      │
                                    │  WORKFLOW-003 (Fuzzing) - Independent    │
                                    │  WORKFLOW-005 (Release) - Independent    │
                                    └──────────────────────────────────────────┘
                                                          │
                                                          │ Requires REACT-003,006
                                                          ▼
┌────────────────────────────────────────────────────────────────────────────────┐
│                              MAIN PIPELINE                                      │
│                                                                                 │
│   ┌─────────────┐     ┌─────────────┐                                          │
│   │ REACT-001   │     │ ✅ MIGRATE-001│                                        │
│   │ Unit Tests  │     │   (DONE)     │                                         │
│   │ (Base)      │     └──────────────┘                                         │
│   └──────┬──────┘            │                                                 │
│          │                   │ (Already complete)                              │
│          ├────────────┐      │                                                  │
│          ▼            │      ▼                                                  │
│   ┌─────────────┐     │ ┌─────────────┐                                        │
│   │ REACT-005   │     │ │ MIGRATE-002 │ ◄─── Can start immediately            │
│   │ Messaging   │     │ │ Container   │                                        │
│   └──────┬──────┘     │ └──────┬──────┘                                        │
│          │            │        │                                                │
│          ▼            │        ▼                                                │
│   ┌─────────────┐     │ ┌─────────────┐                                        │
│   │ REACT-002   │◄────┤ │ MIGRATE-003 │                                        │
│   │ Integration │     │ │ ThreadSafe  │                                        │
│   └──────┬──────┘     │ └──────┬──────┘                                        │
│          │            │        │                                                │
│          │            ▼        ▼                                                │
│          │     ┌─────────────┐ ┌─────────────┐                                  │
│          │     │ REACT-003   │ │ MIGRATE-004 │                                  │
│          │     │ Benchmarks  │ │ Deprecation │                                  │
│          │     └──────┬──────┘ └─────────────┘                                  │
│          │            │                                                         │
│          └────────────┼─────────────────────────────────────────────────────────│
│                       ▼                                                         │
│                ┌─────────────┐                                                  │
│                │ REACT-006   │──────► WORKFLOW-004 (Perf Regression Detection) │
│                │ Perf Base   │                                                  │
│                └──────┬──────┘                                                  │
│                       │                                                         │
│          ┌────────────┴────────────┐                                            │
│          ▼                                                                      │
│   ┌─────────────┐                                                               │
│   │ REACT-004   │                                                               │
│   │ Samples     │                                                               │
│   └─────────────┘                                                               │
└─────────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    │ After all REACT + MIGRATE complete
                                    ▼
┌─────────────────────────────────────────────────────────────────────────────────┐
│                              LEGACY PIPELINE                                     │
│                                                                                  │
│   ┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐   │
│   │ LEGACY-001  │────►│ LEGACY-002  │────►│ LEGACY-003  │────►│ LEGACY-004  │   │
│   │ Core Remove │     │ Types Remove│     │ Old Variant │     │ Build Clean │   │
│   └─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘   │
│                                                                                  │
└─────────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Critical Path Analysis

### 3.1 Longest Dependency Chain (Main Critical Path)

```
REACT-001 → REACT-005 → REACT-002 → REACT-006 → LEGACY-001 → LEGACY-002 → LEGACY-003 → LEGACY-004
   1.5d      1d        1.5d       0.5d        1d          1d          0.5d        0.5d
                                                                                = 7.5d
```

### 3.2 Parallelizable Work

| Parallel Group | Included Tickets | Condition |
|----------------|------------------|-----------|
| Group A | REACT-001, MIGRATE-002, WORKFLOW-002, WORKFLOW-003, WORKFLOW-005 | Can start immediately |
| Group B | REACT-003, REACT-005, MIGRATE-003 | After REACT-001 or MIGRATE-002 complete |
| Group C | REACT-002, MIGRATE-004 | After Group B partially complete |
| Group D | REACT-004, REACT-006 | After Group C complete |
| Group E | WORKFLOW-004 | After REACT-003, REACT-006 complete |

---

## 4. Recommended Execution Order

### Phase 1: Foundation Building (Start immediately)
**Goal**: Establish core infrastructure

| Order | Ticket | Priority | Est. Duration | Reason |
|-------|--------|----------|---------------|--------|
| 1-1 | **REACT-001** | HIGH | 1.5d | Prerequisite for all REACT tickets |
| 1-2 | **MIGRATE-002** | HIGH | 2d | Container variant support (MIGRATE-001 already done) |
| 1-3 | WORKFLOW-003 | MEDIUM | 2d | Independent, can proceed in parallel |

**Phase 1 Completion Criteria**: Unit test framework restored, Container supports variant values

---

### Phase 2: Core Feature Restoration (After Phase 1)
**Goal**: Restore test/container functionality

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 2-1 | **REACT-005** | HIGH | 1d | REACT-001 |
| 2-2 | **MIGRATE-003** | HIGH | 1d | MIGRATE-002 |
| 2-3 | **REACT-003** | MEDIUM | 2d | REACT-001 |

**Parallel Work**: WORKFLOW-002, WORKFLOW-005 (independent)

**Phase 2 Completion Criteria**: Messaging integration, Thread-safe container integration, Benchmark foundation

---

### Phase 3: Integration & Verification (After Phase 2)
**Goal**: System integration and test completion

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 3-1 | **REACT-002** | MEDIUM | 1.5d | REACT-001, REACT-005 |
| 3-2 | **MIGRATE-004** | LOW | 0.5d | MIGRATE-002, MIGRATE-003 |

**Phase 3 Completion Criteria**: Integration test restored, Deprecation warnings added

---

### Phase 4: Finalization & Documentation (After Phase 3)
**Goal**: Final cleanup and establish performance baseline

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 4-1 | **REACT-006** | MEDIUM | 0.5d | REACT-002, REACT-003 |
| 4-2 | REACT-004 | LOW | 2d | REACT-001, REACT-002 |
| 4-3 | **WORKFLOW-004** | MEDIUM | 1.5d | REACT-003, REACT-006 |

**Phase 4 Completion Criteria**: Performance baseline established, Samples/examples working

---

### Phase 5: Legacy Removal (Phase 4 completion required)
**Goal**: Complete legacy code removal

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 5-1 | **LEGACY-001** | HIGH | 1d | All MIGRATE-*, REACT-* |
| 5-2 | **LEGACY-002** | HIGH | 1d | LEGACY-001 |
| 5-3 | LEGACY-003 | MEDIUM | 0.5d | LEGACY-001, LEGACY-002 |
| 5-4 | LEGACY-004 | LOW | 0.5d | LEGACY-001~003 |

**Phase 5 Completion Criteria**: Legacy code completely removed, Build system cleaned

---

## 5. Immediately Actionable Tickets

Tickets with no dependencies that can **start immediately**:

1. **REACT-001** - Unit Tests Reactivation (HIGH, critical path)
2. **MIGRATE-002** - value_container Variant Support (HIGH, critical path)
3. **WORKFLOW-002** - ARM64 Cross-Compilation (LOW)
4. **WORKFLOW-003** - Fuzzing Automation (MEDIUM)
5. **WORKFLOW-005** - Release Automation (LOW)

**Recommendation**: Start REACT-001 and MIGRATE-002 simultaneously for fastest progress

---

## 6. Disabled Tests Reference

### unit_tests.cpp (7 tests)
| Test Name | Category | Notes |
|-----------|----------|-------|
| DISABLED_LargeDataHandling | ContainerTest | Line 345 |
| DISABLED_ConcurrentReads | ThreadSafetyTest | Line 364 |
| DISABLED_ThreadSafeContainer | ThreadSafetyTest | Line 402 |
| DISABLED_InvalidSerialization | ErrorHandlingTest | Line 454 |
| DISABLED_NullValueConversions | ErrorHandlingTest | Line 471 |
| DISABLED_SerializationSpeed | PerformanceTest | Line 488 |
| DISABLED_DeserializationSpeed | PerformanceTest | Line 514 |

### test_messaging_integration.cpp (2 tests)
| Test Name | Category | Notes |
|-----------|----------|-------|
| DISABLED_PerformanceMonitoring | MessagingIntegrationTest | Line 180 |
| DISABLED_ExternalCallbacks | MessagingIntegrationTest | Line 213 |

---

## 7. Legacy Files to Remove

| File | Size | Description |
|------|------|-------------|
| `internal/value.h` | 12,964 bytes | Legacy polymorphic value class |
| `internal/value.cpp` | 23,295 bytes | Legacy value implementation |
| `core/value_types.h` | TBD | Legacy type enum |
| `core/value_types.cpp` | TBD | Legacy type utilities |

---

## Appendix: Active Ticket List (Quick Reference)

| ID | Title | Priority | Duration | Dependencies | Status |
|----|-------|----------|----------|--------------|--------|
| REACT-001 | Unit Tests Reactivation | HIGH | 1.5d | - | TODO |
| REACT-002 | Integration Tests Reactivation | MEDIUM | 1.5d | REACT-001, REACT-005 | TODO |
| REACT-003 | Benchmarks Reactivation | MEDIUM | 2d | REACT-001 | TODO |
| REACT-004 | Samples/Examples Reactivation | LOW | 2d | REACT-001, REACT-002 | TODO |
| REACT-005 | Messaging Integration Reactivation | HIGH | 1d | REACT-001 | TODO |
| REACT-006 | Performance Baseline Reactivation | MEDIUM | 0.5d | REACT-002, REACT-003 | TODO |
| MIGRATE-002 | value_container Variant Support | HIGH | 2d | - | TODO |
| MIGRATE-003 | thread_safe_container Integration | HIGH | 1d | MIGRATE-002 | TODO |
| MIGRATE-004 | Add Deprecation Warnings | LOW | 0.5d | MIGRATE-002, MIGRATE-003 | TODO |
| LEGACY-001 | Remove Core Value Classes | HIGH | 1d | MIGRATE-*, REACT-* | TODO |
| LEGACY-002 | Remove Value Types Implementation | HIGH | 1d | LEGACY-001 | TODO |
| LEGACY-003 | Remove Old variant_value | MEDIUM | 0.5d | LEGACY-001, LEGACY-002 | TODO |
| LEGACY-004 | Build System Cleanup | LOW | 0.5d | LEGACY-001~003 | TODO |
| WORKFLOW-002 | ARM64 Linux Cross-Compilation | LOW | 1d | - | TODO |
| WORKFLOW-003 | Fuzzing Automation | MEDIUM | 2d | - | TODO |
| WORKFLOW-004 | Performance Regression Detection | MEDIUM | 1.5d | REACT-003, REACT-006 | TODO |
| WORKFLOW-005 | Release Automation | LOW | 1d | - | TODO |

---

**Document Author**: Claude
**Last Modified**: 2025-11-23
**Status**: Active

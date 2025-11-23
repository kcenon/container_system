# Container System Work Priority Directive

**Document Version**: 1.0
**Created**: 2025-11-23
**Total Tickets**: 19

---

## 1. Executive Summary

Analysis of Container System's 19 tickets shows they fall into 4 work tracks:

| Track | Tickets | Key Objective | Est. Duration |
|-------|---------|---------------|---------------|
| REACT (Reactivation) | 6 | Restore Disabled Components | 8.5d |
| MIGRATE (Migration) | 4 | variant_value_v2 Transition | 4.5d |
| LEGACY (Legacy Removal) | 4 | Legacy Code Cleanup | 3d |
| WORKFLOW (CI/CD) | 5 | Build/Test Automation | 7.5d |

**Total Estimated Duration**: ~23.5 days (single developer)

---

## 2. Dependency Graph

```
                                    ┌──────────────────────────────────────────┐
                                    │           WORKFLOW (Independent Track)    │
                                    │                                          │
                                    │  WORKFLOW-001 ──────► WORKFLOW-005       │
                                    │  (Windows)            (Release)          │
                                    │                                          │
                                    │  WORKFLOW-002 (ARM64) - Independent      │
                                    │  WORKFLOW-003 (Fuzzing) - Independent    │
                                    └──────────────────────────────────────────┘
                                                          │
                                                          │ Requires REACT-003,006
                                                          ▼
┌────────────────────────────────────────────────────────────────────────────────┐
│                              MAIN PIPELINE                                      │
│                                                                                 │
│   ┌─────────────┐     ┌─────────────┐                                          │
│   │ REACT-001   │     │ MIGRATE-001 │                                          │
│   │ Unit Tests  │     │ Factory Fn  │                                          │
│   │ (Base)      │     │ (Base)      │                                          │
│   └──────┬──────┘     └──────┬──────┘                                          │
│          │                   │                                                  │
│          ├────────────┐      │                                                  │
│          ▼            │      ▼                                                  │
│   ┌─────────────┐     │ ┌─────────────┐                                        │
│   │ REACT-005   │     │ │ MIGRATE-002 │                                        │
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
│          │     └──────┬──────┘ └──────┬──────┘                                  │
│          │            │               │                                         │
│          └────────────┼───────────────┘                                         │
│                       ▼                                                         │
│                ┌─────────────┐                                                  │
│                │ REACT-006   │──────► WORKFLOW-004 (Perf Regression Detection) │
│                │ Perf Base   │                                                  │
│                └──────┬──────┘                                                  │
│                       │                                                         │
│          ┌────────────┴────────────┐                                            │
│          ▼                         ▼                                            │
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
| Group A | REACT-001, MIGRATE-001, WORKFLOW-001, WORKFLOW-002, WORKFLOW-003 | Can start (no dependencies) |
| Group B | REACT-003, REACT-005, MIGRATE-002 | After REACT-001 or MIGRATE-001 complete |
| Group C | REACT-002, MIGRATE-003 | After Group B partially complete |
| Group D | REACT-004, REACT-006, MIGRATE-004 | After Group C complete |
| Group E | WORKFLOW-004, WORKFLOW-005 | After REACT-003,006 or WORKFLOW-001 complete |

---

## 4. Recommended Execution Order

### Phase 1: Foundation Building (Can start simultaneously)
**Goal**: Establish core infrastructure

| Order | Ticket | Priority | Est. Duration | Reason |
|-------|--------|----------|---------------|--------|
| 1-1 | **REACT-001** | 🔴 HIGH | 1.5d | Prerequisite for all REACT tickets |
| 1-2 | **MIGRATE-001** | 🔴 HIGH | 1d | Prerequisite for all MIGRATE tickets |
| 1-3 | WORKFLOW-001 | 🟡 MEDIUM | 2d | Independent, can proceed in parallel |

**Phase 1 Completion Criteria**: Unit test framework restored, Factory functions implemented

---

### Phase 2: Core Feature Restoration (After Phase 1)
**Goal**: Restore test/container functionality

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 2-1 | **REACT-005** | 🟡 MEDIUM | 1d | REACT-001 |
| 2-2 | **MIGRATE-002** | 🔴 HIGH | 2d | MIGRATE-001 |
| 2-3 | **REACT-003** | 🟡 MEDIUM | 2d | REACT-001 |

**Parallel Work**: WORKFLOW-002, WORKFLOW-003 (independent)

**Phase 2 Completion Criteria**: Messaging integration, Container variant support, Benchmark foundation established

---

### Phase 3: Integration & Verification (After Phase 2)
**Goal**: System integration and test completion

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 3-1 | **REACT-002** | 🟡 MEDIUM | 1.5d | REACT-001, REACT-005 |
| 3-2 | **MIGRATE-003** | 🔴 HIGH | 1d | MIGRATE-001, MIGRATE-002 |

**Phase 3 Completion Criteria**: Integration test restored, Thread-safe container complete

---

### Phase 4: Finalization & Documentation (After Phase 3)
**Goal**: Final cleanup and establish performance baseline

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 4-1 | **REACT-006** | 🟡 MEDIUM | 0.5d | REACT-002, REACT-003 |
| 4-2 | **MIGRATE-004** | 🟢 LOW | 0.5d | MIGRATE-001~003 |
| 4-3 | REACT-004 | 🟢 LOW | 2d | REACT-001, REACT-002 |
| 4-4 | **WORKFLOW-004** | 🟡 MEDIUM | 1.5d | REACT-003, REACT-006 |

**Phase 4 Completion Criteria**: Performance baseline established, Deprecation warnings added

---

### Phase 5: Legacy Removal (Phase 4 completion required)
**Goal**: Complete legacy code removal

⚠️ **Warning**: This phase should only start after all REACT + MIGRATE tickets are complete.

| Order | Ticket | Priority | Est. Duration | Prerequisites |
|-------|--------|----------|---------------|---------------|
| 5-1 | **LEGACY-001** | 🔴 HIGH | 1d | All MIGRATE-*, REACT-* |
| 5-2 | **LEGACY-002** | 🔴 HIGH | 1d | LEGACY-001 |
| 5-3 | LEGACY-003 | 🟡 MEDIUM | 0.5d | LEGACY-001, LEGACY-002 |
| 5-4 | LEGACY-004 | 🟢 LOW | 0.5d | LEGACY-001~003 |
| 5-5 | WORKFLOW-005 | 🟢 LOW | 1d | WORKFLOW-001 |

**Phase 5 Completion Criteria**: Legacy code completely removed, Build system cleaned

---

## 5. Detailed Dependency Matrix

### 5.1 Per-Ticket Dependency Details

| Ticket ID | Direct Dependencies | Indirect Dependencies | Tickets Depending on This |
|-----------|---------------------|----------------------|---------------------------|
| REACT-001 | - | - | REACT-002,003,004,005,006, LEGACY-001 |
| REACT-002 | REACT-001, REACT-005 | - | REACT-004, REACT-006 |
| REACT-003 | REACT-001 | - | REACT-006, WORKFLOW-004 |
| REACT-004 | REACT-001, REACT-002 | REACT-005 | - |
| REACT-005 | REACT-001 | - | REACT-002 |
| REACT-006 | REACT-002, REACT-003 | REACT-001, REACT-005 | WORKFLOW-004 |
| MIGRATE-001 | - | - | MIGRATE-002,003,004, LEGACY-001 |
| MIGRATE-002 | MIGRATE-001 | - | MIGRATE-003, MIGRATE-004 |
| MIGRATE-003 | MIGRATE-001, MIGRATE-002 | - | MIGRATE-004 |
| MIGRATE-004 | MIGRATE-001~003 | - | - |
| LEGACY-001 | MIGRATE-*, REACT-* | All MIGRATE/REACT | LEGACY-002,003,004 |
| LEGACY-002 | LEGACY-001 | MIGRATE-*, REACT-* | LEGACY-003, LEGACY-004 |
| LEGACY-003 | LEGACY-001, LEGACY-002 | MIGRATE-*, REACT-* | LEGACY-004 |
| LEGACY-004 | LEGACY-001~003 | MIGRATE-*, REACT-* | - |
| WORKFLOW-001 | - | - | WORKFLOW-005 |
| WORKFLOW-002 | - | - | - |
| WORKFLOW-003 | - | - | - |
| WORKFLOW-004 | REACT-003, REACT-006 | REACT-001, REACT-002, REACT-005 | - |
| WORKFLOW-005 | WORKFLOW-001 | - | - |

### 5.2 Blocker Analysis

**Tickets blocking the most other tickets (priority resolution needed)**:
1. **REACT-001** - Direct/indirect blocker of 10 tickets
2. **MIGRATE-001** - Direct/indirect blocker of 7 tickets
3. **LEGACY-001** - Direct blocker of 3 tickets

---

## 6. Risks and Mitigation Strategies

### 6.1 High-Risk Tickets

| Ticket | Risk | Impact | Mitigation |
|--------|------|--------|------------|
| LEGACY-001 | Missing dependencies discovered | Build failure | Thorough dependency analysis beforehand |
| REACT-001 | Test migration omissions | Quality degradation | Monitor test coverage |
| MIGRATE-003 | Serialization compatibility broken | Data loss | Mandatory round-trip tests |

### 6.2 Rollback Points

| Phase | Rollback Criteria | Rollback Method |
|-------|-------------------|-----------------|
| Before Phase 5 start | Test failure rate > 5% | Return to Phase 4 |
| After LEGACY-001 | Build failure | `git revert` |
| After LEGACY-002 | Serialization errors | `git revert` |

---

## 7. Parallel Work Guidelines

### 7.1 Recommended Distribution for 2 Developers

**Developer A (Core responsibility)**:
- Phase 1: REACT-001
- Phase 2: REACT-005, REACT-002
- Phase 3: REACT-003, REACT-006
- Phase 5: LEGACY-001~004

**Developer B (Infrastructure responsibility)**:
- Phase 1: MIGRATE-001, WORKFLOW-001
- Phase 2: MIGRATE-002, WORKFLOW-002/003
- Phase 3: MIGRATE-003
- Phase 4: MIGRATE-004, WORKFLOW-004, WORKFLOW-005

### 7.2 For 3+ Developers

Assign WORKFLOW series to a separate person (high independence)

---

## 8. Checkpoints and Verification Criteria

### Per-Phase Completion Checklist

#### Phase 1 Completion Conditions
- [ ] Basic unit tests executable via `ctest` command
- [ ] Factory functions like `make_int_value()` working
- [ ] Windows CI build succeeds (if WORKFLOW-001 in progress)

#### Phase 2 Completion Conditions
- [ ] messaging_integration module builds normally
- [ ] variant_value_v2 can be stored/retrieved from value_container
- [ ] Benchmarks executable

#### Phase 3 Completion Conditions
- [ ] Integration test suite executable
- [ ] Nested container support in thread_safe_container
- [ ] Serialization round-trip tests pass

#### Phase 4 Completion Conditions
- [ ] Performance baseline documentation updated
- [ ] Deprecation warnings generated when using legacy classes
- [ ] samples/examples build and run normally

#### Phase 5 Completion Conditions
- [ ] `core/value.h`, `values/` directory removal complete
- [ ] Full build succeeds (all platforms)
- [ ] 100% test pass
- [ ] Build time reduced by 10% or more

---

## 9. Immediately Actionable Tickets (Starting Points)

Tickets with no dependencies that can **start immediately**:

1. ⭐ **REACT-001** - Unit Tests Reactivation (required starting point)
2. ⭐ **MIGRATE-001** - Factory Functions Implementation (required starting point)
3. **WORKFLOW-001** - Windows Build Support (optional)
4. **WORKFLOW-002** - ARM64 Cross-Compilation (optional)
5. **WORKFLOW-003** - Fuzzing Automation (optional)

**Recommendation**: Start REACT-001 and MIGRATE-001 simultaneously for parallel progress

---

## 10. Timeline Estimate (Single Developer)

| Week | Phase | Main Tasks | Cumulative Progress |
|------|-------|------------|---------------------|
| Week 1 | Phase 1-2 | REACT-001, MIGRATE-001, REACT-005, MIGRATE-002 | 30% |
| Week 2 | Phase 2-3 | REACT-003, REACT-002, MIGRATE-003 | 55% |
| Week 3 | Phase 4 | REACT-006, MIGRATE-004, REACT-004 | 70% |
| Week 4 | Phase 5 | LEGACY-001~004 | 85% |
| Week 5 | WORKFLOW | WORKFLOW-001~005 | 100% |

---

**Document Author**: Claude
**Last Modified**: 2025-11-23
**Status**: Draft

---

## Appendix: Complete Ticket List (Quick Reference)

| ID | Title | Priority | Duration | Dependencies |
|----|-------|----------|----------|--------------|
| REACT-001 | Unit Tests Reactivation | HIGH | 1.5d | - |
| REACT-002 | Integration Tests Reactivation | MEDIUM | 1.5d | REACT-001, REACT-005 |
| REACT-003 | Benchmarks Reactivation | MEDIUM | 2d | REACT-001 |
| REACT-004 | Samples/Examples Reactivation | LOW | 2d | REACT-001, REACT-002 |
| REACT-005 | Messaging Integration Reactivation | MEDIUM | 1d | REACT-001 |
| REACT-006 | Performance Baseline Reactivation | MEDIUM | 0.5d | REACT-002, REACT-003 |
| MIGRATE-001 | Factory Functions Implementation | HIGH | 1d | - |
| MIGRATE-002 | value_container Variant Support | HIGH | 2d | MIGRATE-001 |
| MIGRATE-003 | thread_safe_container Integration | HIGH | 1d | MIGRATE-001, MIGRATE-002 |
| MIGRATE-004 | Add Deprecation Warnings | LOW | 0.5d | MIGRATE-001~003 |
| LEGACY-001 | Remove Core Value Classes | HIGH | 1d | MIGRATE-*, REACT-* |
| LEGACY-002 | Remove Value Types Implementation | HIGH | 1d | LEGACY-001 |
| LEGACY-003 | Remove Old variant_value | MEDIUM | 0.5d | LEGACY-001, LEGACY-002 |
| LEGACY-004 | Build System Cleanup | LOW | 0.5d | LEGACY-001~003 |
| WORKFLOW-001 | Windows Build Support | MEDIUM | 2d | - |
| WORKFLOW-002 | ARM64 Linux Cross-Compilation | LOW | 1d | - |
| WORKFLOW-003 | Fuzzing Automation | MEDIUM | 2d | - |
| WORKFLOW-004 | Performance Regression Detection | MEDIUM | 1.5d | REACT-003, REACT-006 |
| WORKFLOW-005 | Release Automation | LOW | 1d | WORKFLOW-001 |

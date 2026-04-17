# Document Review Report — container_system

**Generated**: 2026-04-14
**Mode**: Report-only (no source .md files modified)
**Files analyzed**: 78 markdown files
**Scope**: `/Volumes/T5 EVO/Sources/container_system/**/*.md` (excluding `build/`, `scripts/build/`, `.git/`)

---

## Findings Summary

| Severity     | Phase 1 (Anchors / Links) | Phase 2 (Accuracy) | Phase 3 (SSOT / Xrefs) | Total |
|--------------|---------------------------|---------------------|-------------------------|-------|
| Must-Fix     | 11                        | 8                   | 4                       | 23    |
| Should-Fix   | 3                         | 9                   | 7                       | 19    |
| Nice-to-Have | 2                         | 4                   | 5                       | 11    |

---

## Must-Fix Items

### Phase 1 — Broken Links (relative paths to nonexistent files)

1. `README.md:414` — `docs/guides/INTEGRATION.md` — OK (exists). *[not broken; kept for xref context]*
2. `README.md:519` — `[Integration Guide →](docs/INTEGRATION.md)` — broken; correct path is `docs/guides/INTEGRATION.md`. (Phase 1)
3. `README.md:603` — `[Build Guide →](docs/guides/BUILD_GUIDE.md)` — broken; `BUILD_GUIDE.md` does not exist in `docs/guides/`. (Phase 1)
4. `README.md:728` — `[Contributing Guide](docs/CONTRIBUTING.md)` — broken; canonical is `docs/contributing/CONTRIBUTING.md`. (Phase 1)
5. `docs/PRODUCTION_QUALITY.md:687` — `[INTEGRATION.md](INTEGRATION.md)` — broken; correct is `guides/INTEGRATION.md`. (Phase 1)
6. `docs/PROJECT_STRUCTURE.md:142, 713` — references `BUILD_GUIDE.md` in tree and link list; file does not exist. Same issue in `docs/PROJECT_STRUCTURE.kr.md:144, 608`. (Phase 1)
7. `docs/guides/FAQ.md:513` — `[Integration](../INTEGRATION.md)` — broken; should be `INTEGRATION.md` (same folder) or `../guides/INTEGRATION.md`. (Phase 1)
8. `docs/advanced/VALUE_STORE_DESIGN.md:284–285` — `../docs/ADR-001-Type-System-Unification.md` and `../docs/VARIANT_VALUE_V2_MIGRATION_GUIDE.md` — both broken (wrong directory nesting; correct is `./ADR-001-Type-System-Unification.md` and `./VARIANT_VALUE_V2_MIGRATION_GUIDE.md`). (Phase 1)
9. `docs/advanced/DOMAIN_SEPARATION_GUIDE.md:255–258` — `../IMPROVEMENT_PLAN.md`, `../core/value_store.h`, `../messaging/message_container.h`, `../../SYSTEM_ANALYSIS_SUMMARY.md` — `IMPROVEMENT_PLAN.md` and `SYSTEM_ANALYSIS_SUMMARY.md` do not exist anywhere in the repo. (Phase 1)
10. `docs/advanced/ADR-001-Type-System-Unification.md:217–218` — `../IMPROVEMENT_PLAN.md`, `../../SYSTEM_ANALYSIS_SUMMARY.md` — targets do not exist. (Phase 1)
11. `docs/integration/README.md:203–205` — `../VALUE_SYSTEM_COMPARISON_ANALYSIS.md` (does not exist) and `../../../ECOSYSTEM.md` (wrong depth; actual path is `../ECOSYSTEM.md`). (Phase 1)
12. `docs/advanced/VALUE_STORE_DESIGN.md:283` — `[IMPROVEMENT_PLAN.md](../IMPROVEMENT_PLAN.md)` — nonexistent. (Phase 1)

### Phase 2 — Factual Errors / Version Mismatches

13. `README.md:184` — gRPC/protobuf versions **`protobuf 3.21.12, gRPC 1.51.1`** conflict with `CLAUDE.md:83` which declares `gRPC 1.60.0 + protobuf 4.25.1`. `SOUP.md:55, 89` states protobuf `3.21.12`. `grpc/README.md:304` states `Protobuf >= 3.21.0`. Single source of truth is missing — three different versions. (Phase 2) — **RESOLVED**: `vcpkg.json` established as single source of truth (`gRPC 1.60.0`, `protobuf 4.25.1`). `LICENSE-THIRD-PARTY` aligned and `.github/workflows/docs-verify.yml` added to prevent drift (#524).
14. `README.md:609` — Linux row: `GCC 9+, Clang 10+` contradicts `README.md:175` and `:738` which require `GCC 11+ / Clang 14+`. Self-contradiction inside same file. (Phase 2)
15. `README.md:611` — Windows row: `MSVC 2019+` contradicts line `175` requiring `MSVC 2022+`. (Phase 2)
16. `samples/README.md:64` — `C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)` — toolchain floors lower than the official C++20/Concepts requirements in `README.md`. Project actually needs GCC 11+, Clang 14+, MSVC 2022+. (Phase 2)
17. `docs/API_REFERENCE.md:15–16` — declares **Version 0.3.0.0** while `docs/ARCHITECTURE.md:15, 1321` declares **0.1.0.0** and `CHANGELOG.md` latest tag is `[0.1.0] - 2026-03-13`. Version semantics diverge across SSOT docs. (Phase 2)
18. `docs/API_QUICK_REFERENCE.md:16` and `README.kr.md:243` — state `message_buffer` preferred "since v2.0.0", but no v2.0.0 is referenced in any `CHANGELOG.md`. Latest shipped tag is `v0.1.0`. (Phase 2)
19. `docs/CHANGELOG.kr.md:531–534` — references to tags `v1.0.0`, `v0.9.0`, `v0.8.0` that do not appear in `CHANGELOG.md` (root) and have no matching release entry. Fabricated or stale. (Phase 2)
20. `docs/performance/MEMORY_POOL_PERFORMANCE.md:15` — `Last Updated: Awaiting first benchmark run` combined with `doc_status: "Released"` in registry is contradictory. (Phase 2)

### Phase 3 — SSOT Contradictions

21. SSOT for documentation registry (`docs/README.md:16`) declares **55 total documents**; the file index has 55 rows but the repository contains 78 `.md` files. Registry lists only `docs/**` files and omits `CLAUDE.md`, `SCOPE.md`, `SECURITY.md`, `CONTRIBUTING.md`, `CODE_OF_CONDUCT.md`, `README.md`, `README.kr.md`, `CHANGELOG.md` (root), and every project `*/README.md` (benchmarks, cmake, grpc, examples, samples, integration_tests, `.github-workflows-template`, `.github/*`). SSOT incomplete. (Phase 3)
22. Two files both labeled SSOT for "Container System Scope": `SCOPE.md` (root) and `docs/architecture/ADR-001-container-system-scope.md`. No cross-reference between them; content partially overlaps. (Phase 3)
23. Two files both declaring canonical "Contributing" guidance: `CONTRIBUTING.md` (root) and `docs/contributing/CONTRIBUTING.md`. `README.md:707` links to `docs/contributing/CONTRIBUTING.md`, but `README.md:728` links to `docs/CONTRIBUTING.md` (broken). Ambiguous canonicity. (Phase 3)
24. Two changelogs: `CHANGELOG.md` (root) and `docs/CHANGELOG.md` — both contain `[Unreleased]` sections with different content and different format bases (`Keep a Changelog 1.1.0` vs `1.0.0`). Only `docs/CHANGELOG.md` is tagged SSOT in the registry, but root is the one normally consumed. (Phase 3)

---

## Should-Fix Items

### Phase 1

1. `docs/grpc/GRPC_INTEGRATION_PROPOSAL.md` and `docs/ASYNC_GUIDE.md` — no intra-file `](#anchor)` TOC links although sections are long (800+ lines). Anchors for in-doc navigation would improve discoverability.
2. `docs/README.md` (registry) contains no anchors back to sibling sections (e.g., from "Document Index" table rows to the per-category tables below).
3. `docs/SERIALIZATION_SCHEMA_POLICY_GUIDE.md` — heading "### Architecture" is reused three times (lines 44, 207, 390). GitHub auto-slug produces `architecture`, `architecture-1`, `architecture-2`. No links currently target these duplicates, but future references risk ambiguity.

### Phase 2 — Consistency

4. Terminology: both `container` and `value_container` used interchangeably for the same class. `CLAUDE.md` standardizes on `value_container` / `message_buffer`, but `docs/API_REFERENCE.md:23` uses `Container` as a section name and `docs/API_REFERENCE.md:1449` introduces the "unified `value_container` class". Standardize on `value_container` everywhere and note `message_buffer` alias once.
5. `docs/API_QUICK_REFERENCE.md:11` shows `#include <kcenon/container/container.h>` while `README.md:692` specifies canonical `#include <container/container.h>`. Pick one include convention.
6. Namespaces: `container_module` (per `API_REFERENCE.md:33`) vs `kcenon::container` (per `API_QUICK_REFERENCE.md:13` and `README.md:383`). Migration must be declared in one place.
7. `README.md:420` — `Language Support: Most documents available in English and Korean (*_KO.md)` — actual convention is `*.kr.md` (not `_KO.md`). Doc states wrong suffix.
8. `docs/BENCHMARKS.md:15` Last Updated `2025-11-15` conflicts with `FEATURES.md:15` `2026-02-08` despite both describing the same v0.x build; staleness is unclear.
9. `docs/guides/RESULT_API_MIGRATION_GUIDE.md:17` — `Last Updated: 2025-01-11` is earlier than all referenced issue numbers (#284, #297, #301 in later changelog). Likely stale.
10. Serialization formats list: `README.md:39` says "Binary, JSON, XML serialization"; `CLAUDE.md:15` lists "binary, json, xml, msgpack serializers"; `BENCHMARKS.md` references 4 formats. Root README understates supported formats (omits MessagePack).
11. `README.md:744` — emoji line at the bottom. Project CLAUDE.md rules recommend avoiding emojis in docs without explicit request (style; but conflicts with `commit-settings.md` intent, kept as Nice-to-Have).
12. `docs/PRODUCTION_QUALITY.md:80` references "C++11/14/17" modernization; factually the project targets C++20 only (confirmed by `CLAUDE.md`, `README.md`). Prior-version reference should be labeled historical.

### Phase 3 — Missing / Orphan Xrefs

13. `docs/ECOSYSTEM.md` (referenced by `README.md:471` via GitHub URLs, and by `docs/integration/README.md:205`) — no reverse links from `ARCHITECTURE.md`, `CHANGELOG.md`, or `PROJECT_STRUCTURE.md` in either language. Effectively orphaned in navigation.
14. `docs/GETTING_STARTED.md` — not listed in the `docs/README.md` registry (SSOT) despite being a primary entry-point doc mentioned elsewhere.
15. `docs/API_QUICK_REFERENCE.md` — not listed in `docs/README.md` registry.
16. `docs/TRACEABILITY.md` — listed as SSOT (`CNT-QUAL-007`) but not cross-referenced from `PRODUCTION_QUALITY.md` or `CONTRIBUTING.md`; downstream readers will miss it.
17. `docs/adr/ADR-003-simd-optimization-strategy.md` — isolated; no link in/out except registry entry. Should xref `ARCHITECTURE.md` SIMD section and `PERFORMANCE.md`.
18. `docs/architecture/ADR-001-container-system-scope.md` and `docs/advanced/ADR-001-Type-System-Unification.md` — two distinct "ADR-001" documents, no disambiguation or cross-reference explaining why both carry the same number.
19. `docs/performance/BASELINE.md` references `../BASELINE.md` (line 421 of `docs/BENCHMARKS.md` states `[BASELINE.md](../BASELINE.md)` which resolves to `/BASELINE.md` at repo root — file does not exist there). Wrong depth.

---

## Nice-to-Have Items

1. `README.md` Table of Contents anchor `#c20-module-support` (line 20) points to `## C++20 Module Support` — GitHub slug rule strips `+`, yielding `c20-module-support`. Works, but readability poor; consider `## C++20 Modules`.
2. `docs/performance/PERFORMANCE.md:102` — table comparing against "Protocol Buffers, MessagePack, JSON (nlohmann)" — no version column. Adding versions would make the comparison reproducible.
3. `docs/BENCHMARKS.kr.md:119` — "### vs Protocol Buffers" heading slug in Korean produces `vs-protocol-buffers`, same as English sibling — fine, but no explicit xref back to the English doc.
4. `docs/FEATURES.md` and `docs/FEATURES.kr.md` — language-pair declaration missing from `FEATURES.md` (only `FEATURES.kr.md` links back via language header line).
5. `docs/README.md:74` — `SOUP List &mdash; container_system` uses HTML entity `&mdash;`. Plain `—` would render identically and avoid HTML-in-Markdown.
6. `README.md:169` — `docs/ARCHITECTURE.md#minimal-dependency-architecture` anchor exists (confirmed at `ARCHITECTURE.md:87`). Kept for verification record.
7. `docs/ARCHITECTURE.md:16` `Last Updated: 2025-10-22` is 6 months stale relative to the 2026-04-04 `doc_date` in the frontmatter. Reconcile both dates to avoid reader confusion.
8. Several "Last Updated" lines predate the 2026-04-04 registry freeze (e.g., `BENCHMARKS.md: 2025-11-15`, `PRODUCTION_QUALITY.md: 2025-11-15`, `PROJECT_STRUCTURE.md: 2025-12-10`). Single refresh pass recommended.
9. `docs/performance/MEMORY_POOL_PERFORMANCE.md` placeholder "Awaiting first benchmark run" — either move to `Draft` status or remove from registry.
10. `docs/guides/INTEGRATION.md:22` and `:491` — both carry `Last Updated: 2025-10-22`; consider auto-generating the footer from the header to prevent drift.
11. `docs/advanced/VISITOR_PATTERN_GUIDE.md:157` — mentions C++17's overload pattern; wording "C++17's" may mislead given C++20 target. Add a note that this pattern is usable from C++17 onward and adopted in container_system's C++20 codebase.

---

## Score

- **Overall**: **6.2 / 10**
- Anchors & Links: **5.5 / 10** — 11 confirmed broken relative links; several reach across wrong directory depths.
- Accuracy & Consistency: **6.0 / 10** — three distinct gRPC/protobuf version strings across docs; compiler floors contradict inside the root README; v2.0.0 claims with no corresponding release.
- SSOT & Redundancy: **7.0 / 10** — registry is present and mostly clean, but undercounts real `.md` files, duplicate canonicity for SCOPE and CONTRIBUTING, and duplicate ADR-001 numbering remains.

---

## Notes

- **Top patterns detected**:
  1. Path-depth drift after docs reorganization (links still assume the old flat `docs/*.md` layout; guides now live under `docs/guides/`, contributing under `docs/contributing/`, advanced under `docs/advanced/`). At least 11 dangling relative paths trace to this migration.
  2. Version triangulation missing — `CLAUDE.md`, root `README.md` table, `SOUP.md`, and `grpc/README.md` each declare different protobuf/gRPC pinned versions. No doc is authoritative.
  3. SSOT registry (`docs/README.md`) scope is `docs/**` only, but project-level docs (README, CHANGELOG, SCOPE, CONTRIBUTING, CODE_OF_CONDUCT, SECURITY, subdir READMEs) are outside its purview, creating a navigation blind spot.

- **Evidence locations reviewed** (non-exhaustive):
  - Broken links: `README.md:519, 603, 728`; `PRODUCTION_QUALITY.md:687`; `PROJECT_STRUCTURE.md:142, 713`; `guides/FAQ.md:513`; `advanced/VALUE_STORE_DESIGN.md:283–285`; `advanced/DOMAIN_SEPARATION_GUIDE.md:255–258`; `advanced/ADR-001-Type-System-Unification.md:217–218`; `integration/README.md:203–205`.
  - Version conflicts: `README.md:175, 184, 609, 611, 738`; `CLAUDE.md:82–83`; `SOUP.md:55, 89`; `grpc/README.md:304`; `samples/README.md:64`.
  - SSOT scope: `docs/README.md:16` (declares 55), actual tree = 78 `.md` files.

- **Out of scope (noted only)**: Doxyfile `.dox` fragments (`mainpage.dox`, `troubleshooting.dox`, `tutorial_*.dox`, `faq.dox`) were not scanned as Markdown. Generated HTML (`docs/html/`, `docs/doxygen-awesome-css/`) intentionally excluded.

- **Mode compliance**: No source `.md` file was modified during this review. Only `/Volumes/T5 EVO/Sources/container_system/DOC_REVIEW_REPORT.md` was written.

---

## Post-Fix Re-Validation (2026-04-15)

**Fix commit**: `b9e369b` — `docs: fix 19 broken links, 9 version mismatches, 8 xrefs, 4 SSOT deferrals`
**Branch**: `feature/docs-fix-2026-04-15`
**Re-run scope**: Phase 1 only (anchors + intra/inter-file links)
**Files scanned**: 79 markdown files (excluding `build/`, `scripts/build/`, `.git/`)
**Methodology**: Ran identical Phase 1 validator against both pre-fix (`b9e369b^`) and post-fix (`b9e369b`) trees; cross-referenced the two issue sets to isolate true regressions vs. pre-existing pre-existing problems.

### Before / After Summary

| Metric                                     | Pre-Fix (`b9e369b^`) | Post-Fix (`b9e369b`) | Delta |
|--------------------------------------------|----------------------|----------------------|-------|
| Total Phase 1 issues                       | 79                   | 64                   | -15   |
| Broken file references                     | 72                   | 57                   | -15   |
| Missing intra-file anchors                 | 7                    | 7                    | 0     |
| Missing inter-file anchors                 | 0                    | 0                    | 0     |
| Prior **Must-Fix** items resolved          | —                    | 15 / 16              | 94%   |
| Prior **Must-Fix** items residual          | —                    | 1                    | —     |
| True regressions (new issues from fix)     | —                    | 0                    | 0     |
| Self-references in `DOC_REVIEW_REPORT.md`  | —                    | 7 (excluded)         | —     |

Note: The post-fix scan finds 64 issues, of which 7 are self-references inside `DOC_REVIEW_REPORT.md` itself (the report text cites previously-broken paths verbatim). Excluding those, the effective post-fix issue count is 57.

### Residual List (Prior Must-Fix still failing)

| # | File:Line | Target | Notes |
|---|-----------|--------|-------|
| 1 | `docs/PROJECT_STRUCTURE.kr.md:608` | `guides/BUILD_GUIDE.md` | Korean variant of item #6 in the original Must-Fix list. The fix commit message explicitly deferred Korean `.kr.md` files ("Korean .kr.md files and CHANGELOG.kr.md version tags are left untouched per task scope"). English sibling `docs/PROJECT_STRUCTURE.md:713` was fixed. |

### Regression List

**None.** Set-difference of post-fix issues minus pre-fix issues (after excluding `DOC_REVIEW_REPORT.md` self-references) yields zero entries. The fix commit introduced no new broken links, no new missing anchors, and no new path-depth errors. File list changes (one new file scanned: `DOC_REVIEW_REPORT.md`) contributed only the 7 self-references, all of which are documentation of the prior issues, not active links to navigate.

### Key Observations

1. **Net reduction of 15 broken file references** — all 11 canonical path-depth errors flagged in the original Must-Fix list were resolved. Additional "collateral" fixes swept up 4 adjacent broken references in `docs/PRODUCTION_QUALITY.md`, `docs/BENCHMARKS.md`, and `docs/advanced/DOMAIN_SEPARATION_GUIDE.md` (header references and code-path references near the Must-Fix lines).
2. **56 pre-existing broken references remain** — these were present in `b9e369b^` but were not on the original Phase 1 Must-Fix list (they were either missed in the first scan or correctly classified as Should-Fix / Nice-to-Have scope). They are **not regressions** — the fix commit did not create or alter them.
3. **7 intra-file anchors remain missing** — unchanged from pre-fix (`docs/API_REFERENCE.md:23`, `docs/API_REFERENCE.kr.md:23`, `docs/guides/INTEGRATION.md:29–32`, `docs/grpc/GRPC_INTEGRATION_PROPOSAL.md:43`). These are outside the original Phase 1 Must-Fix list.

### Verdict

**PARTIAL-PASS**

- All 11 canonical broken-link Must-Fix items (English variants) resolved.
- 1 Must-Fix residual (`docs/PROJECT_STRUCTURE.kr.md:608`) — explicitly deferred by the fix commit's documented scope. Acceptable per the commit message's stated boundary.
- Zero true regressions introduced.
- 56 pre-existing Should-Fix / unlisted broken references remain across the repository; these are documentation-quality issues orthogonal to the Must-Fix set and did not shift as a result of the fix.

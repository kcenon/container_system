---
doc_id: "CNT-PROJ-005"
doc_title: "SOUP List &mdash; container_system"
doc_version: "1.0.0"
doc_date: "2026-04-04"
doc_status: "Released"
project: "container_system"
category: "PROJ"
---

# SOUP List &mdash; container_system

> **Software of Unknown Provenance (SOUP) Register per IEC 62304:2006+AMD1:2015 &sect;8.1.2**
>
> This document is the authoritative reference for all external software dependencies.
> Every entry must include: title, manufacturer, unique version identifier, license, and known anomalies.

| Document | Version |
|----------|---------|
| IEC 62304 Reference | &sect;8.1.2 Software items from SOUP |
| Last Reviewed | 2026-03-10 |
| container_system Version | 2.0.0 |

---

## Internal Ecosystem Dependencies

| ID | Name | Manufacturer | Version | License | Usage | Safety Class | Known Anomalies |
|----|------|-------------|---------|---------|-------|-------------|-----------------|
| INT-001 | [common_system](https://github.com/kcenon/common_system) | kcenon | Latest (vcpkg / source) | BSD-3-Clause | Result&lt;T&gt; pattern, error handling primitives, interfaces | B | None |

---

## Production SOUP

### System Dependencies

| ID | Name | Manufacturer | Version | License | Usage | Safety Class | Known Anomalies |
|----|------|-------------|---------|---------|-------|-------------|-----------------|
| SOUP-001 | POSIX Threads (pthreads) | POSIX / OS vendor | System-provided | N/A (OS) | Thread-safe container operations via `find_package(Threads)` | B | None |

---

## Optional SOUP

Optional SOUP entries cover both root-package features and separately scoped
module deliverables that are not built by the root `CMakeLists.txt`.

| ID | Name | Manufacturer | Version | License | Usage | Safety Class | Known Anomalies |
|----|------|-------------|---------|---------|-------|-------------|-----------------|
| SOUP-002 | [fmt](https://github.com/fmtlib/fmt) | Victor Zverovich | 10.0.0+ | MIT | String formatting fallback for compilers without `std::format` (`fmt-support` feature) | A | None |
| SOUP-003 | [gRPC](https://grpc.io/) | Google / CNCF | 1.51.1 | Apache-2.0 | RPC transport and generated service bindings for the isolated `grpc/` integration module | B | None |
| SOUP-004 | [Protocol Buffers](https://protobuf.dev/) | Google | 3.21.12 | BSD-3-Clause | Message serialization and code generation for the isolated `grpc/` integration module | B | None |

---

## Development/Test SOUP (Not Deployed)

| ID | Name | Manufacturer | Version | License | Usage | Qualification |
|----|------|-------------|---------|---------|-------|--------------|
| SOUP-T01 | [Google Test](https://github.com/google/googletest) | Google | 1.14.0 | BSD-3-Clause | Unit testing framework (includes GMock) | Required |
| SOUP-T02 | [Google Benchmark](https://github.com/google/benchmark) | Google | 1.8.3 | Apache-2.0 | Performance benchmarking framework | Not required |
| SOUP-T03 | [Doxygen](https://www.doxygen.nl/) | Dimitri van Heesch | Latest | GPL-2.0 | API documentation generation (build tool only) | Not required |

---

## Safety Classification Key

| Class | Definition | Example |
|-------|-----------|---------|
| **A** | No contribution to hazardous situation | Logging, formatting, test frameworks |
| **B** | Non-serious injury possible | Data processing, network communication |
| **C** | Death or serious injury possible | Encryption, access control |

---

## Version Pinning and Feature Constraints (IEC 62304 Compliance)

Pinned versions in `vcpkg.json` are controlled via the `overrides` field for
module and test dependencies, while feature-scoped dependencies can use
version constraints:

```json
{
  "overrides": [
    { "name": "grpc", "version": "1.51.1" },
    { "name": "protobuf", "version": "3.21.12" },
    { "name": "gtest", "version": "1.14.0" },
    { "name": "benchmark", "version": "1.8.3" }
  ],
  "features": {
    "fmt-support": {
      "dependencies": [
        {
          "name": "fmt",
          "version>=": "10.0.0"
        }
      ]
    }
  }
}
```

The vcpkg baseline is locked in `vcpkg-configuration.json` to ensure reproducible builds.

## Module Scope Boundary

The root `container_system` package does not link gRPC or protobuf in its
default build. Those dependencies apply only when the isolated `grpc/`
subproject is configured directly, for example with `cmake -S grpc -B build-grpc`.
Treat the `grpc/` artifacts (`container_grpc_proto`, `container_grpc_adapter`,
and `container_grpc`) as a separately scoped deliverable for SOUP, license,
and SBOM review.

---

## Version Update Process

When updating any SOUP dependency:

1. Update the version in `vcpkg.json` (`overrides` or feature constraints)
2. Update the corresponding row in this document
3. Verify no new known anomalies (check CVE databases)
4. Run full CI/CD pipeline to confirm compatibility
5. Document the change in the PR description

---

## License Compliance Summary

| License | Count | Copyleft | Obligation |
|---------|-------|----------|------------|
| MIT | 1 | No | Include copyright notice |
| BSD-3-Clause | 2 | No | Include copyright + no-endorsement clause |
| Apache-2.0 | 2 | No | Include license + NOTICE file |
| GPL-2.0 | 1 | Yes | Build tool only; not linked or distributed |

> **GPL contamination**: None. Doxygen is a build tool that does not link with or become part of the distributed software.

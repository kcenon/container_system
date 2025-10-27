# Container System Documentation

> **Version:** 1.0.0
> **Last Updated:** 2025-10-27

This directory contains comprehensive documentation for the Container System project.

## 📚 Documentation Index

### Core Documentation

| Document | Description | Language |
|----------|-------------|----------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | Detailed system architecture and design philosophy | EN |
| [ARCHITECTURE_KO.md](ARCHITECTURE_KO.md) | 상세한 시스템 아키텍처 및 설계 철학 | KO |
| [STRUCTURE.md](STRUCTURE.md) | Project structure and file organization | EN |

### Integration & Migration

| Document | Description | Language |
|----------|-------------|----------|
| [INTEGRATION.md](INTEGRATION.md) | Integration guide with other systems | EN |
| [MIGRATION.md](MIGRATION.md) | Migration guide from messaging_system | EN |

### Performance & Quality

| Document | Description | Language |
|----------|-------------|----------|
| [BASELINE.md](BASELINE.md) | Performance baseline metrics | EN |
| [BASELINE_KO.md](BASELINE_KO.md) | 성능 기준 메트릭 | KO |
| [PERFORMANCE.md](PERFORMANCE.md) | Detailed performance analysis | EN |
| [PERFORMANCE_KO.md](PERFORMANCE_KO.md) | 상세 성능 분석 | KO |
| [SANITIZER_TEST_RESULTS.md](SANITIZER_TEST_RESULTS.md) | Sanitizer test results (TSAN, ASAN, UBSAN) | EN |
| [SANITIZER_TEST_RESULTS_KO.md](SANITIZER_TEST_RESULTS_KO.md) | 새니타이저 테스트 결과 | KO |

### Development & Planning

| Document | Description | Language |
|----------|-------------|----------|
| [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) | Implementation summary and test coverage | EN |
| [IMPLEMENTATION_SUMMARY_KO.md](IMPLEMENTATION_SUMMARY_KO.md) | 구현 요약 및 테스트 커버리지 | KO |
| [IMPROVEMENTS.md](IMPROVEMENTS.md) | Planned improvements and roadmap | EN |
| [IMPROVEMENTS_KO.md](IMPROVEMENTS_KO.md) | 계획된 개선사항 및 로드맵 | KO |

### gRPC Integration (Experimental)

| Document | Description | Language |
|----------|-------------|----------|
| [grpc/GRPC_INTEGRATION_PROPOSAL.md](grpc/GRPC_INTEGRATION_PROPOSAL.md) | gRPC integration proposal | EN |
| [grpc/GRPC_INTEGRATION_PROPOSAL_KO.md](grpc/GRPC_INTEGRATION_PROPOSAL_KO.md) | gRPC 통합 제안서 | KO |
| [grpc/GRPC_MINIMAL_INTEGRATION_PLAN.md](grpc/GRPC_MINIMAL_INTEGRATION_PLAN.md) | Minimal gRPC integration plan | EN |
| [grpc/GRPC_MINIMAL_INTEGRATION_PLAN_KO.md](grpc/GRPC_MINIMAL_INTEGRATION_PLAN_KO.md) | 최소 gRPC 통합 계획 | KO |
| [grpc/GRPC_ADAPTER_IMPLEMENTATION.md](grpc/GRPC_ADAPTER_IMPLEMENTATION.md) | gRPC adapter implementation guide | EN |
| [grpc/GRPC_ADAPTER_IMPLEMENTATION_KO.md](grpc/GRPC_ADAPTER_IMPLEMENTATION_KO.md) | gRPC 어댑터 구현 가이드 | KO |
| [grpc/GRPC_PROJECT_PLAN.md](grpc/GRPC_PROJECT_PLAN.md) | gRPC project plan | EN |
| [grpc/GRPC_PROJECT_PLAN_KO.md](grpc/GRPC_PROJECT_PLAN_KO.md) | gRPC 프로젝트 계획 | KO |

---

## 🚀 Quick Start Guide

### For New Users

1. Start with [../README.md](../README.md) for project overview
2. Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand system design
3. Follow [INTEGRATION.md](INTEGRATION.md) to integrate with your project
4. Check [PERFORMANCE.md](PERFORMANCE.md) for optimization tips

### For Contributors

1. Review [STRUCTURE.md](STRUCTURE.md) to understand project organization
2. Read [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) for implementation details
3. Check [IMPROVEMENTS.md](IMPROVEMENTS.md) for planned features
4. See [SANITIZER_TEST_RESULTS.md](SANITIZER_TEST_RESULTS.md) for quality metrics

### For Migrating from messaging_system

1. Read [MIGRATION.md](MIGRATION.md) for step-by-step migration guide
2. Review [INTEGRATION.md](INTEGRATION.md) for integration patterns
3. Check [../CHANGELOG.md](../CHANGELOG.md) for breaking changes

---

## 📊 Document Categories

### Architecture & Design
- System architecture and design principles
- Component organization and dependencies
- Design patterns and best practices

### Integration & Usage
- Integration with messaging, network, and database systems
- Migration guides and compatibility information
- API usage examples and patterns

### Performance & Quality
- Performance benchmarks and baselines
- Optimization strategies and SIMD usage
- Test results and quality metrics
- RAII compliance and memory safety

### Development & Planning
- Implementation summaries and test coverage
- Planned improvements and feature roadmap
- Development guidelines and coding standards

### Experimental Features
- gRPC integration proposals and plans
- Adapter implementations for gRPC
- Future integration possibilities

---

## 🌐 Language Support

Most documents are available in both English and Korean:
- English documents: `*.md`
- Korean documents: `*_KO.md`

If a Korean version is not available, please refer to the English version.

---

## 📝 Contributing to Documentation

When updating documentation:

1. **Maintain consistency**: Follow existing document structure
2. **Update both languages**: Provide English and Korean versions when possible
3. **Update this index**: Add new documents to the appropriate section
4. **Cross-reference**: Link related documents where appropriate
5. **Version information**: Update version and date in document headers

---

## 🔍 Finding Information

Use the following guide to find specific information:

| Topic | Document |
|-------|----------|
| System design principles | [ARCHITECTURE.md](ARCHITECTURE.md) |
| File organization | [STRUCTURE.md](STRUCTURE.md) |
| Performance metrics | [BASELINE.md](BASELINE.md), [PERFORMANCE.md](PERFORMANCE.md) |
| Integration patterns | [INTEGRATION.md](INTEGRATION.md) |
| Migration steps | [MIGRATION.md](MIGRATION.md) |
| Test coverage | [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) |
| Quality metrics | [SANITIZER_TEST_RESULTS.md](SANITIZER_TEST_RESULTS.md) |
| Future plans | [IMPROVEMENTS.md](IMPROVEMENTS.md) |
| gRPC integration | [grpc/](grpc/) directory |

---

## 📧 Contact

- **Maintainer:** kcenon@naver.com
- **Issues:** [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **License:** BSD 3-Clause

---

**Last Updated:** 2025-10-27
**Documentation Version:** 1.0.0

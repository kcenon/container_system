---
doc_id: "CNT-GUID-002"
doc_title: "Container System Documentation"
doc_version: "1.0.0"
doc_date: "2026-04-04"
doc_status: "Released"
project: "container_system"
category: "GUID"
---

# Container System Documentation

> **Language:** **English** | [한국어](README.kr.md)

**Version:** 0.1.0.0
**Last Updated:** 2025-11-11
**Status:** Comprehensive

Welcome to the container_system documentation! A type-safe, high-performance container and serialization system for C++20 with SIMD optimization and multiple format support.

---

## 🚀 Quick Navigation

| I want to... | Document |
|--------------|----------|
| ⚡ Understand containers | [Architecture](01-ARCHITECTURE.md) |
| ❓ Find answers to common questions | [FAQ](guides/FAQ.md) (25+ Q&A) |
| 🔧 Integrate into my project | [Integration Guide](guides/INTEGRATION.md) |
| 🚀 Migrate from messaging_system | [Migration Guide](guides/MIGRATION.md) |
| 📊 Review performance | [Baseline](performance/BASELINE.md) |

---

## Documentation Structure

### 📘 Core Documentation

| Document | Description | Korean | Lines |
|----------|-------------|--------|-------|
| [01-ARCHITECTURE.md](01-ARCHITECTURE.md) | System architecture, design patterns, SIMD optimization | [🇰🇷](01-ARCHITECTURE.kr.md) | 800+ |

### 📗 User Guides

| Document | Description | Lines |
|----------|-------------|-------|
| [FAQ.md](guides/FAQ.md) | 25 Q&A covering containers, serialization, performance | 516 |
| [INTEGRATION.md](guides/INTEGRATION.md) | Integration with messaging, network, database systems | 300+ |
| [MIGRATION.md](guides/MIGRATION.md) | Migration from messaging_system containers | 200+ |

### 📙 Advanced Topics

| Document | Description | Lines |
|----------|-------------|-------|
| [ADR-001-Type-System-Unification.md](advanced/ADR-001-Type-System-Unification.md) | Architecture decision record | 200+ |
| [VALUE_STORE_DESIGN.md](advanced/VALUE_STORE_DESIGN.md) | Value store design | 150+ |
| [VARIANT_VALUE_V2_MIGRATION_GUIDE.md](advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md) | V2 migration | 200+ |
| [IMPLEMENTATION_SUMMARY.md](advanced/IMPLEMENTATION_SUMMARY.md) | Implementation status | [🇰🇷](advanced/IMPLEMENTATION_SUMMARY.kr.md) |
| [IMPROVEMENTS.md](advanced/IMPROVEMENTS.md) | Planned improvements | [🇰🇷](advanced/IMPROVEMENTS.kr.md) |

### 📊 Performance

| Document | Description | Korean | Lines |
|----------|-------------|--------|-------|
| [BASELINE.md](performance/BASELINE.md) | 5M containers/s, 2M serialize/s | [🇰🇷](performance/BASELINE.kr.md) | 300+ |
| [PERFORMANCE.md](performance/PERFORMANCE.md) | Detailed performance analysis | [🇰🇷](performance/PERFORMANCE.kr.md) | 400+ |
| [SANITIZER_TEST_RESULTS.md](performance/SANITIZER_TEST_RESULTS.md) | TSan, ASan, UBSan results | [🇰🇷](performance/SANITIZER_TEST_RESULTS.kr.md) | 200+ |

### 🤝 Contributing

| Document | Description | Lines |
|----------|-------------|-------|
| [TESTING.md](contributing/TESTING.md) | Testing strategy | 300+ |

---

## Project Information

### Current Status
- **Version**: 0.1.0.0 (Type-safe variant system)
- **C++ Standard**: C++20
- **License**: BSD 3-Clause

### Key Features
- ✅ **Type-safe containers** - `std::variant` based
- ✅ **Multiple formats** - Binary, JSON, XML
- ✅ **SIMD optimization** - NEON (ARM), AVX2 (x86)
- ✅ **High performance** - 5M containers/s creation
- ✅ **Memory pooling** - Reduced allocation overhead
- ✅ **Thread-safe** - Verified with TSan

---

## 📞 Getting Help

- **Issues**: [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **Discussions**: [GitHub Discussions](https://github.com/kcenon/container_system/discussions)

---

**Last Updated**: 2025-11-11

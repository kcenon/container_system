# Ecosystem Integration

**container_system** provides type-safe containers and SIMD serialization for the kcenon ecosystem.

> **See also**: [ARCHITECTURE.md](ARCHITECTURE.md) for internal architecture, [PROJECT_STRUCTURE.md](PROJECT_STRUCTURE.md) for code layout, [integration/README.md](integration/README.md) for integration patterns.

## Dependencies
| System | Relationship |
|--------|-------------|
| common_system | Core interfaces and patterns |

## Dependent Systems
| System | Usage |
|--------|-------|
| network_system | Serialized message containers |
| pacs_system | DICOM data containers |

## All Systems
| System | Description | Docs |
|--------|------------|------|
| common_system | Foundation — interfaces, patterns, utilities | [Docs](https://kcenon.github.io/common_system/) |
| thread_system | Thread pool, DAG scheduling, work stealing | [Docs](https://kcenon.github.io/thread_system/) |
| logger_system | Async logging, decorators, OpenTelemetry | [Docs](https://kcenon.github.io/logger_system/) |
| **container_system** | **Type-safe containers, SIMD serialization** | **Current** |
| monitoring_system | Metrics, tracing, alerts, plugins | [Docs](https://kcenon.github.io/monitoring_system/) |
| database_system | Multi-backend DB | [Docs](https://kcenon.github.io/database_system/) |
| network_system | TCP/UDP/WebSocket/HTTP2/QUIC/gRPC | [Docs](https://kcenon.github.io/network_system/) |
| pacs_system | DICOM medical imaging | [Docs](https://kcenon.github.io/pacs_system/) |

# 변경 이력

Container System 프로젝트의 모든 주요 변경 사항이 이 파일에 문서화됩니다.

이 문서는 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/) 형식을 따르며,
프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

> **언어:** [English](CHANGELOG.md) | **한국어**

---

## [Unreleased]

### Changed
- fmt 라이브러리 의존성 제거 및 C++20 std::format 전용 사용 (#168)
- 조건부 컴파일 분기 제거로 formatter.h 간소화
- C++20 std::format 지원을 필수로 하는 CMake 설정 업데이트

### Added
- 생태계 전반의 포괄적인 문서 통일화
- 버전 추적 개선을 위한 CHANGELOG.md 및 MIGRATION.md

---

## [1.0.0] - 2025-10-21

### Added
- **15가지 Value 타입**: int, uint, long, ulong, double, bool, string, blob, array, map 등
- **SIMD 최적화**: ARM NEON 및 x86 AVX 지원으로 고성능 연산
- **다중 직렬화 형식**: Binary, JSON, XML 지원
- **타입 안전 컨테이너**: 컴파일 타임 타입 안전성과 런타임 유연성
- **Zero-Copy 연산**: 대용량 데이터를 위한 효율적인 메모리 관리
- **Builder 패턴 API**: 컨테이너 생성을 위한 Fluent 인터페이스
- **메모리 풀**: Hot path를 위한 최적화된 메모리 할당
- **스레드 안전 연산**: Lock-free 읽기와 mutex 보호 쓰기
- **통합 테스트**: 49개의 포괄적인 테스트 케이스 (Phase 5)

### Performance
- 컨테이너 생성: 2M containers/sec
- 직렬화 (binary): 1.8M containers/sec
- SIMD 연산: 25M ops/sec
- 메모리 오버헤드: 컨테이너당 ~32 bytes

### Integration
- messaging_system: 원활한 메시지 컨테이너 매핑
- network_system: 네트워크 전송 통합
- database_system: 데이터 영속성 지원

---

## [0.9.0] - 2025-09-15

### Added
- messaging_system으로부터 초기 분리
- 핵심 value 타입 구현
- 기본 직렬화 지원

### Changed
- 모듈화 아키텍처 설계
- 독립 빌드 시스템

---

## [0.8.0] - 2025-08-01

### Added
- Binary 직렬화 형식
- Value 타입 확장성 프레임워크

### Fixed
- array/map 타입의 메모리 누수
- 동시 접근 시 스레드 안전성 문제

---

## 버전 번호 체계

이 프로젝트는 Semantic Versioning을 사용합니다:
- **MAJOR** 버전: 호환되지 않는 API 변경
- **MINOR** 버전: 하위 호환되는 기능 추가
- **PATCH** 버전: 하위 호환되는 버그 수정

---

## 마이그레이션 가이드

### 1.0.0으로 마이그레이션

자세한 마이그레이션 지침은 [MIGRATION.md](MIGRATION_KO.md)를 참조하세요.

#### 빠른 마이그레이션 예제

**이전** (messaging_system 0.x):
```cpp
#include <messaging/container.h>
auto container = messaging::create_container();
```

**이후** (container_system 1.0):
```cpp
#include <container/container.h>
auto container = container_system::create_container();
```

---

## 주요 변경 사항

### Version 1.0.0
- 네임스페이스 변경: `messaging::container` → `container_system::container`
- 헤더 경로가 독립 구조로 변경
- CMake 타겟 이름 변경: `messaging_container` → `kcenon::container_system`

---

## 성능 개선

### Version 1.0.0
- SIMD 최적화: +150% 성능 향상
- 메모리 풀: -40% 할당 오버헤드
- Lock-free 읽기: +200% 동시 읽기 성능

---

## 알려진 이슈

### Version 1.0.0
- 대형 타입 (>64 bytes)은 캐시 효율성에 영향을 줄 수 있음
- JSON 직렬화가 binary 형식보다 느림 (~4배)
- Windows 플랫폼은 완전한 SIMD 지원을 위해 C++20 필요

---

## 지원

- **이슈**: [GitHub Issues](https://github.com/kcenon/container_system/issues)
- **토론**: [GitHub Discussions](https://github.com/kcenon/container_system/discussions)
- **이메일**: kcenon@naver.com

---

## 라이선스

이 프로젝트는 BSD 3-Clause 라이선스를 따릅니다 - 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

---

[Unreleased]: https://github.com/kcenon/container_system/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/kcenon/container_system/releases/tag/v1.0.0
[0.9.0]: https://github.com/kcenon/container_system/releases/tag/v0.9.0
[0.8.0]: https://github.com/kcenon/container_system/releases/tag/v0.8.0

# 변경 이력

Container System 프로젝트의 모든 주요 변경 사항이 이 파일에 문서화됩니다.

이 문서는 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/) 형식을 따르며,
프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

> **언어:** [English](CHANGELOG.md) | **한국어**

---

## [Unreleased]

### Added
- **C++20 모듈 지원** (#222): `kcenon.container` C++20 모듈 파일 추가
  - `src/modules/container.cppm` 기본 모듈 인터페이스 생성
  - 모든 공개 타입 export: `value_types`, `value_variant`, `optimized_value`, `pool_stats`, `value_container`
  - `variant_helpers` 네임스페이스와 JSON/XML 인코딩 유틸리티 export
  - `Result<T>` 패턴 지원을 위해 `kcenon.common` 모듈 import
  - 모듈 빌드를 위한 `CONTAINER_BUILD_MODULES` CMake 옵션 추가
  - 모듈 컴파일에 CMake 3.28+ 및 Ninja 생성기 필요
  - 전환 기간 동안 헤더 기반 빌드 계속 지원
- **messaging_container_builder에 통합 set() 메서드 추가** (#218): API 일관성을 위해 `messaging_container_builder`에 통합 `set()` 메서드 추가
  - `value_container::set()` API와 일치하는 `set()` 템플릿 메서드 추가
  - fluent 인터페이스를 위한 메서드 체이닝 지원
  - `[[deprecated]]` 속성으로 `add_value()` deprecated 처리

### Changed
- **테스트 파일을 통합 set() API로 마이그레이션** (#218): 테스트 파일에서 deprecated `add_value()`를 `set()`으로 교체
  - `test_messaging_integration.cpp`에서 23개 호출 마이그레이션
  - `performance_tests.cpp`에서 6개 호출 마이그레이션
  - 모든 테스트가 회귀 없이 통과

### Removed
- **deprecation.h 제거** (#221): C++20 모듈 준비를 위해 deprecation 경고 억제 매크로 제거
  - 사용자 정의 경고 억제 매크로가 있는 `core/deprecation.h` 파일 제거
  - 테스트 프레임워크 파일에서 `CONTAINER_SUPPRESS_DEPRECATION_START/END` 매크로 제거
  - 호출처가 없었던 deprecated 래퍼 함수 (`IsCiEnvironment`, `AdjustPerformanceThreshold`, `AdjustDurationThreshold`) 제거
  - 사용자 정의 매크로 대신 표준 `[[deprecated]]` 속성 사용
  - Kent Beck의 "Fewest Elements" 원칙 준수
- **Deprecated 스레드 안전성 메서드 제거** (#217): `value_store`에서 deprecated 스레드 안전성 제어 메서드 제거
  - `enable_thread_safety()` 제거: v0.2.0 이후 no-op이었음
  - `disable_thread_safety()` 제거: v0.2.0 이후 no-op이었음
  - `is_thread_safe()` 제거: v0.2.0 이후 항상 true 반환했음
  - 스레드 안전성은 v0.2.0 이후 항상 활성화됨 (TOCTOU 보안 수정 #190 참조)
  - **Breaking Change**: 이 메서드를 호출하는 코드는 더 이상 컴파일되지 않음

### Changed
- **미사용 통계 카운터 제거** (#209): Simple Design 준수를 위한 데드 코드 제거
  - `read_count_`, `write_count_`, `serialization_count_` atomic 카운터 제거
  - `heap_allocations_`와 `stack_allocations_`는 유지 (`memory_stats()`로 노출됨)
  - 컨테이너 인스턴스당 메모리 사용량 24바이트 감소
  - lock guard와 serialize()에서 불필요한 atomic 연산 제거
- **헤더 모듈화** (#191): container.h (902줄)를 집중된 서브 헤더로 분리
  - `core/container/fwd.h`: 컨테이너 타입 전방 선언
  - `core/container/types.h`: value_variant, optimized_value, pool_stats 정의
  - `core/container/variant_helpers.h`: JSON/XML 인코딩 유틸리티
  - container.h는 이제 하위 호환성을 위한 umbrella 헤더로 동작
  - 컴파일 시간 및 코드 탐색 개선

### Security
- **TOCTOU 스레드 안전성 수정** (#190): Time-Of-Check-Time-Of-Use 취약점 제거
  - `read_lock_guard`와 `write_lock_guard`에서 항상 락 획득
  - 런타임 플래그 기반 조건부 락 제거
  - `enable_thread_safety()` 및 `disable_thread_safety()` 메서드 deprecated (v0.3.0에서 제거 예정)
  - 모든 작업이 기본적으로 스레드 안전함

### Fixed
- **XML 엔티티 인코딩** (#187): `to_xml()`에 XML 1.0 엔티티 인코딩 추가
  - XML 특수 문자 인코딩: `&` -> `&amp;`, `<` -> `&lt;`, `>` -> `&gt;`, `"` -> `&quot;`, `'` -> `&apos;`
  - 제어 문자 (0x00-0x1F, 탭/줄바꿈/캐리지리턴 제외)를 `&#xNN;` 형식으로 인코딩
  - 문자열 값과 헤더 필드에 인코딩 적용
  - 잘못된 XML 출력 및 잠재적 XML 인젝션 공격 방지
- **JSON 문자열 이스케이핑** (#186): `to_json()`에 RFC 8259 준수 이스케이핑 추가
  - 특수 문자 이스케이프: `"`, `\`, 줄바꿈, 캐리지리턴, 탭, 백스페이스, 폼피드
  - 제어 문자 (U+0000-U+001F)를 `\uXXXX` 형식으로 이스케이프
  - 문자열 값과 필드 이름 모두에 이스케이프 적용
  - 헤더 필드 (source_id, target_id, message_type, version)에도 이스케이프 적용
  - 잘못된 JSON 출력 및 잠재적 인젝션 공격 방지

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

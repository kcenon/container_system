# 변경 이력

Container System 프로젝트의 모든 주요 변경 사항이 이 파일에 문서화됩니다.

이 문서는 [Keep a Changelog](https://keepachangelog.com/ko/1.0.0/) 형식을 따르며,
프로젝트는 [Semantic Versioning](https://semver.org/lang/ko/)을 준수합니다.

> **언어:** [English](CHANGELOG.md) | **한국어**

---

## [Unreleased]

### Added
- **배치 연산 API** (#229): 대량 삽입, 조회, 수정을 위한 최적화된 배치 API 추가
  - `bulk_insert(vector&&)`: 이동 기반 대량 삽입 (가장 효율적)
  - `bulk_insert(span, reserve_hint)`: 사전 할당을 통한 복사 기반 대량 삽입
  - `get_batch(keys)`: 단일 락 획득으로 여러 값 조회 (optional 벡터 반환)
  - `get_batch_map(keys)`: 찾은 키-값 쌍을 맵으로 반환
  - `contains_batch(keys)`: 여러 키 존재 여부 동시 확인
  - `remove_batch(keys)`: 여러 키 동시 삭제 (삭제 개수 반환)
  - `update_if(key, expected, new_value)`: compare-and-swap 스타일 조건부 업데이트
  - `update_batch_if(updates)`: 대량 조건부 업데이트
  - 배치 업데이트 명세를 위한 `update_spec` 구조체 추가
  - Result 반환 변형 추가: `bulk_insert_result()`, `get_batch_result()`, `remove_batch_result()`
  - 모든 배치 연산은 효율성을 위해 단일 락 획득 사용
  - 반복적 연산 대비 3-5배 성능 향상 예상
  - 모든 배치 연산에 대한 포괄적인 단위 테스트 추가
- **AVX-512 SIMD 지원** (#227): 최신 Intel/AMD 프로세서용 AVX-512 지원 추가
  - AVX-512F 감지 매크로 및 512비트 SIMD 타입 추가 (`float_simd_512`, `double_simd_512`, `int32_simd_512`)
  - AVX-512 최적화 연산 구현 (`sum_floats_avx512`, `min_float_avx512`, `max_float_avx512`)
  - 배정밀도 SIMD 연산을 위한 `sum_doubles_avx512` 추가
  - AVX-512 하위 기능 런타임 감지 추가 (`has_avx512f()`, `has_avx512dq()`, `has_avx512bw()`, `has_avx512vl()`)
  - SIMD 기능 표현을 위한 `simd_level` 열거형 추가
  - 런타임 SIMD 지원 조회를 위한 `get_best_simd_level()` 추가
  - AVX-512 우선 디스패치 로직 업데이트 (AVX-512 > AVX2 > SSE > NEON)
  - 컴파일 타임 및 런타임 SIMD 지원 표시를 위한 `get_simd_info()` 개선
  - 포괄적인 SIMD 테스트 스위트 추가 (`tests/simd_tests.cpp`)
  - AVX-512 지원 하드웨어에서 2배 성능 향상 예상
- **Result<T> 패턴을 모든 Public API로 확장** (#231): 포괄적인 Result 기반 에러 처리 추가
  - `error_codes` 네임스페이스에 표준화된 에러 코드 추가 (core/container/error_codes.h)
    - 값 연산 (1xx): key_not_found, type_mismatch, empty_key 등
    - 직렬화 (2xx): serialization_failed, deserialization_failed 등
    - 유효성 검사 (3xx): schema_validation_failed, constraint_violated 등
    - 리소스 (4xx): memory_allocation_failed, file_not_found 등
    - 스레드 안전성 (5xx): lock_acquisition_failed, concurrent_modification
  - Result를 반환하는 값 API 추가:
    - `get_result(key)`: `Result<optimized_value>` 반환
    - `set_result(key, value)`: `VoidResult` 반환
    - `set_all_result(vals)`: `VoidResult` 반환
    - `remove_result(key)`: `VoidResult` 반환
  - Result를 반환하는 직렬화 API 추가:
    - `serialize_result()`: `Result<string>` 반환
    - `serialize_array_result()`: `Result<vector<uint8_t>>` 반환
    - `to_json_result()`: `Result<string>` 반환
    - `to_xml_result()`: `Result<string>` 반환
  - Result를 반환하는 파일 연산 API 추가:
    - `load_packet_result(path)`: `VoidResult` 반환
    - `save_packet_result(path)`: `VoidResult` 반환
  - 마이그레이션 가이드 추가: docs/guides/RESULT_PATTERN_MIGRATION.md
  - 모든 새 메서드에 `[[nodiscard]]` 속성 적용
- **Zero-Copy 역직렬화** (#226): Zero-copy 역직렬화 구현 완료
  - 직렬화된 데이터에 대한 비소유 접근을 위한 `value_view` 클래스 추가
  - 온디맨드 값 조회를 위한 lazy parsing 인덱스 구현
  - zero-copy 값 접근을 위한 `get_view()` 메서드 추가
  - `is_zero_copy_mode()` 및 `ensure_index_built()` 메서드 추가
  - 문자열 값을 복사 없이 `string_view`로 접근 가능
  - 대용량 메시지 파싱 시 10-30배 성능 향상 제공
- **메모리 풀 통합** (#225): 소규모 할당을 위해 memory pool을 value_container에 통합
  - thread-local pool 인스턴스가 포함된 `internal/pool_allocator.h` 추가
  - 두 가지 크기 클래스 지원: small (<=64 bytes), medium (<=256 bytes)
  - `pool_allocate<T>()` 및 `pool_deallocate<T>()` 템플릿 함수 추가
  - `CONTAINER_USE_MEMORY_POOL` CMake 옵션 추가 (기본값: ON)
  - 상세한 할당 통계를 포함하도록 `pool_stats` 구조체 확장
  - 풀 효율성 계산을 위한 `hit_rate()` 메서드 추가
  - 소규모 할당에 대해 10-20배 성능 향상 예상
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
- **메모리 풀 테스트 Clang 호환성** (#225): macOS Clang 컴파일 에러 수정
  - PoolAllocateTemplate 테스트의 SmallStruct에 명시적 생성자 추가
  - Clang은 GCC와 달리 C++20의 aggregate 괄호 초기화를 지원하지 않음
  - memory_efficiency_bench.cpp에서 `hit_rate()` 함수 호출 수정
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

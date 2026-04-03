[![CI](https://github.com/kcenon/container_system/actions/workflows/ci.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/ci.yml)
[![Code Coverage](https://github.com/kcenon/container_system/actions/workflows/coverage.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/kcenon/container_system/branch/main/graph/badge.svg)](https://codecov.io/gh/kcenon/container_system)
[![Static Analysis](https://github.com/kcenon/container_system/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/static-analysis.yml)
[![Security Scan](https://github.com/kcenon/container_system/actions/workflows/dependency-security-scan.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/dependency-security-scan.yml)
[![Documentation](https://github.com/kcenon/container_system/actions/workflows/build-Doxygen.yaml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/build-Doxygen.yaml)
[![License](https://img.shields.io/github/license/kcenon/container_system)](https://github.com/kcenon/container_system/blob/main/LICENSE)

# Container System

> **Language:** [English](README.md) | **한국어**

메시징 시스템과 범용 애플리케이션을 위한 고성능 C++20 타입 안전 컨테이너 프레임워크입니다.

## 목차

- [개요](#개요)
- [주요 기능](#주요-기능)
- [요구사항](#요구사항)
- [빠른 시작](#빠른-시작)
- [설치](#설치)
- [아키텍처](#아키텍처)
- [핵심 개념](#핵심-개념)
- [API 개요](#api-개요)
- [예제](#예제)
- [성능](#성능)
- [생태계 통합](#생태계-통합)
- [기여하기](#기여하기)
- [라이선스](#라이선스)

---

## 개요

Container System은 SIMD 최적화와 원활한 생태계 통합을 특징으로 하는 모듈식 인터페이스 기반 아키텍처로 구축된 고성능 직렬화 가능 데이터 컨테이너 라이브러리입니다.

**핵심 가치**:
- **타입 안전성**: 컴파일 타임 검사를 갖춘 강력한 타입 값 시스템
- **고성능**: SIMD 가속 연산 (초당 1.8M 직렬화, 25M SIMD ops/sec)
- **철저한 테스트**: 완벽한 RAII 점수 (20/20), 데이터 레이스 제로
- **크로스 플랫폼**: Linux, macOS, Windows 네이티브 지원
- **다중 포맷**: Binary, JSON, XML 직렬화 (자동 포맷 감지)

### 미션

고성능 데이터 직렬화를 전 세계 개발자에게 **접근 가능하고**, **타입 안전하며**, **효율적으로** 만드는 것입니다.

---

## 주요 기능

| 기능 | 설명 | 상태 |
|------|------|------|
| **타입 안전 값 시스템** | 16개 내장 타입, 컴파일 타임 검사 | 안정 |
| **Binary 직렬화** | 초당 1.8M ops, ~10% 오버헤드 | 안정 |
| **JSON 직렬화** | 초당 950K ops, 사람이 읽기 가능 | 안정 |
| **XML 직렬화** | 초당 720K ops, 스키마 검증 | 안정 |
| **SIMD 가속** | ARM NEON / x86 AVX2 자동 감지 | 안정 |
| **스레드 안전 컨테이너** | RCU 기반 lock-free 읽기 | 안정 |
| **메모리 풀** | 소형 할당 최적화 | 안정 |
| **빌더 패턴** | 메시지 컨테이너 빌더 | 안정 |
| **자동 포맷 감지** | 직렬화 포맷 자동 식별 | 안정 |
| **코루틴 비동기** | C++20 코루틴 지원 | 안정 |
| **C++20 모듈** | 헤더 기반 인터페이스 대안 | 실험적 |

---

## 요구사항

### 컴파일러 매트릭스

| 컴파일러 | 최소 버전 | 비고 |
|----------|----------|------|
| GCC | 11+ | C++20 Concepts 필수 |
| Clang | 14+ | C++20 Concepts 필수 |
| Apple Clang | 14+ | macOS 지원 |
| MSVC | 2022+ | C++20 Concepts 필수 |

### 빌드 도구 및 의존성

| 의존성 | 버전 | 필수 | 설명 |
|--------|------|------|------|
| CMake | 3.20+ | 예 | 빌드 시스템 |
| [common_system](https://github.com/kcenon/common_system) | latest | 예 | C++20 Concepts 및 공통 인터페이스 |
| vcpkg | latest | 아니오 | 패키지 관리 (권장) |

> container_system은 생태계에서 가장 깔끔한 **최소 의존성 아키텍처**를 갖추고 있습니다 - common_system만 필수입니다.

---

## 빠른 시작

```cpp
#include <container/container.h>

using namespace container_module;

int main() {
    // 빌더 패턴으로 컨테이너 생성
    auto container = messaging_container_builder()
        .source("trading_engine", "session_001")
        .target("risk_monitor", "main")
        .message_type("market_data")
        .add_value("symbol", "AAPL")
        .add_value("price", 175.50)
        .add_value("volume", 1000000)
        .optimize_for_speed()
        .build();

    // 고성능 바이너리 직렬화
    std::string binary_data = container->serialize();  // 1.8M ops/sec

    // 사람이 읽을 수 있는 JSON
    std::string json_data = container->to_json();

    // 역직렬화
    auto restored = std::make_shared<value_container>(binary_data);
    auto price = restored->get_value("price");

    return 0;
}
```

---

## 설치

### vcpkg를 통한 설치

```bash
vcpkg install kcenon-container-system
```

`CMakeLists.txt`에서:
```cmake
find_package(container_system CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE container_system::container)
```

### 소스에서 빌드

```bash
# container_system 클론
git clone https://github.com/kcenon/container_system.git
cd container_system

# 빌드 (common_system은 자동으로 가져옴)
./scripts/build.sh           # Linux/macOS
scripts\build.bat            # Windows (CMD)
```

### 로컬 개발 (common_system과 함께)

```bash
# 두 리포지토리를 나란히 클론
git clone https://github.com/kcenon/common_system.git
git clone https://github.com/kcenon/container_system.git

cd container_system
# CMake가 ../common_system을 자동 감지
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 의존성 해결 우선순위

1. **캐시 변수**: `COMMON_SYSTEM_ROOT` (CMake 옵션)
2. **환경 변수**: `$COMMON_SYSTEM_ROOT`
3. **형제 디렉토리**: `../common_system` (로컬 개발)
4. **하위 디렉토리**: `./common_system` (CI/CD)
5. **FetchContent**: GitHub에서 자동 다운로드 (기본값)

---

## 아키텍처

### 모듈 구조

```
include/kcenon/container/
  core/             - value_types (16 타입), value_store, container_schema
  core/serializers/ - binary, json, xml, msgpack 직렬화기 + serializer_factory
  internal/         - value, variant_value_factory, memory_pool, pool_allocator,
                      thread_safe_container, simd_processor, rcu_value, epoch_manager
  internal/async/   - C++20 코루틴 지원 (task, generator, async_container)
  messaging/        - message_container (도메인 특화)
  integration/      - messaging_integration
  utilities/        - 문자열 변환, 핵심 유틸리티
```

### 의존성 구조

```
container_system
       |
       v
common_system (유일한 필수 생태계 의존성)
```

---

## 핵심 개념

### 값 타입 (Value Types)

16개의 내장 타입을 지원합니다: null, bool, short/ushort, int/uint, long/ulong, llong/ullong, float, double, string, bytes, container, array.

컴파일 타임 `constexpr` 타입 매핑으로 타입 안전성을 보장합니다.

### 직렬화 전략 (Serializer Strategy)

Strategy 패턴으로 구현되며, `serializer_factory`가 런타임에 binary/json/xml/msgpack을 해석합니다:

```cpp
// 자동 포맷 감지
auto container = std::make_shared<value_container>(data);

// 명시적 포맷 지정
auto json = container->serialize(serialization_format::json);
```

### 스레드 안전성

- **lock-free 읽기**: `rcu_value`, `epoch_manager`를 사용한 동시 읽기
- **스레드 안전 래퍼**: `thread_safe_container`로 쓰기 동기화
- **선형 스케일링**: 8 스레드에서 7.5x 처리량

### 빌더 패턴

`messaging_container_builder`로 메시지 컨테이너를 플루언트하게 구성합니다.

### 메모리 풀

소형 객체 할당을 위한 커스텀 `pool_allocator`로 할당 성능을 최적화합니다.

---

## API 개요

| API | 헤더 | 설명 |
|-----|------|------|
| `value_container` | `container.h` | 핵심 직렬화 가능 컨테이너 |
| `message_buffer` | `container.h` | v2.0.0부터 권장 이름 |
| `messaging_container_builder` | `messaging/message_container.h` | 빌더 패턴 생성 |
| `thread_safe_container` | `internal/thread_safe_container.h` | 스레드 안전 래퍼 |
| `simd_processor` | `internal/simd_processor.h` | SIMD 가속 처리 |
| `serializer_factory` | `core/serializers/serializer_factory.h` | 직렬화 포맷 팩토리 |
| `memory_pool` | `internal/memory_pool.h` | 커스텀 메모리 할당 |

---

## 예제

| 예제 | 난이도 | 설명 |
|------|--------|------|
| [basic_container_example](examples/basic_container_example) | 초급 | 기본 컨테이너 사용 |
| [messaging_example](examples/messaging_example) | 중급 | 메시징 시스템 통합 |
| [simd_batch_example](examples/simd_batch_example) | 고급 | SIMD 배치 처리 |

---

## 성능

**플랫폼**: Apple M1 @ 3.2GHz, 16GB RAM

| 메트릭 | 값 | 비고 |
|--------|------|------|
| **Binary 직렬화** | 1.8M ops/s | ~10% 오버헤드 |
| **JSON 직렬화** | 950K ops/s | 사람이 읽기 가능 |
| **XML 직렬화** | 720K ops/s | 스키마 검증 |
| **SIMD 처리** | 25M ops/s | 숫자 배열 |
| **SIMD 가속** | 3.2x | Apple Silicon (NEON) |
| **스레드 스케일링** | 7.5x | 8 스레드 |

### 품질 메트릭

- RAII 점수: 20/20 (A+ 등급, 생태계 최고)
- AddressSanitizer: 메모리 누수 제로
- ThreadSanitizer: 데이터 레이스 제로
- 123+ 테스트, 85%+ 커버리지

---

## 생태계 통합

### 의존성 계층

```
common_system (Tier 0) [필수]
container_system (Tier 1) <-- 이 프로젝트
  -> 사용처: network_system, database_system, pacs_system
```

### 통합 프로젝트

| 프로젝트 | container_system 역할 |
|----------|----------------------|
| [common_system](https://github.com/kcenon/common_system) | 필수 의존성 |
| [network_system](https://github.com/kcenon/network_system) | 데이터 교환 포맷 |
| [database_system](https://github.com/kcenon/database_system) | 데이터 직렬화 |
| [pacs_system](https://github.com/kcenon/pacs_system) | DICOM 직렬화 |

### 플랫폼 지원

| 플랫폼 | 컴파일러 | 상태 |
|--------|----------|------|
| **Linux** | GCC 11+, Clang 14+ | 완전 지원 |
| **macOS** | Apple Clang 14+ | 완전 지원 |
| **Windows** | MSVC 2022+ | 완전 지원 |

---

## 기여하기

기여를 환영합니다! 자세한 내용은 [기여 가이드](docs/contributing/CONTRIBUTING.md)를 참조하세요.

1. 리포지토리 포크
2. 기능 브랜치 생성
3. 테스트와 함께 변경 사항 작성
4. 로컬에서 테스트 실행
5. Pull Request 열기

---

## 라이선스

이 프로젝트는 BSD 3-Clause 라이선스에 따라 배포됩니다 - 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

---

<p align="center">
  Made with ❤️ by 🍀☀🌕🌥 🌊
</p>

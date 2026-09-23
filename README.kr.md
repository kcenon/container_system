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
| `message_buffer` | `container.h` | `value_container`의 권장 별칭 (v0.1 기준선에서 도입; 자세한 내용은 `CLAUDE.md` 참조) |
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

## 목적과 적용 범위

Container System은 서로 다른 종류의 값을 하나의 데이터 모델로 표현하고,
그 값을 저장하거나 시스템 사이에 전달할 때 사용하는 C++ 라이브러리입니다.
애플리케이션의 통신 프로토콜이나 데이터베이스 연결 자체를 구현하는 대신,
이들 계층에서 함께 사용할 값 표현, 직렬화 전략, 컨테이너 조작 기능을 제공합니다.
단순한 구조체만 필요한 프로그램이라면 표준 라이브러리만으로 충분할 수 있습니다.
여러 값 타입을 동적으로 다루거나 여러 직렬화 형식을 선택해야 하는 경우에
컨테이너의 타입 검사와 공통 인터페이스를 검토하는 것이 좋습니다.

`common_system`은 결과와 오류 표현 등 공통 인터페이스를 제공하는 필수 의존성입니다.
컨테이너를 포함하는 프로젝트는 공통 헤더도 함께 제공해야 합니다.
데이터의 의미와 메시지 처리 순서는 애플리케이션에서 정의하며,
컨테이너에 값을 넣는 것만으로 네트워크 전송, 영속성, 트랜잭션이 보장되지는 않습니다.
메시징 전용 기능과 일반적인 값 저장 기능의 구분은
[공개 헤더](include/kcenon/container/)와 [예제](examples/)에서 확인할 수 있습니다.

### 의존성 선택과 기존 빌드 호환성

새 CMake 이름은 `KCENON_WITH_COMMON_SYSTEM`입니다. 기존
`BUILD_WITH_COMMON_SYSTEM`도 호환 별칭으로 계속 사용할 수 있습니다.
두 이름을 동시에 지정하고 값이 다르면 새 이름이 우선하며 CMake가 경고합니다.
공통 시스템은 필수이므로 어느 이름으로 지정하더라도 비활성화할 수 없습니다.
이 옵션 이름의 정리는 애플리케이션에 공개하는 C++ 타입이나 헤더 이름을
바꾸는 작업이 아닙니다. 기존 컴파일 정의도 필요한 호환성을 유지합니다.

기존 빌드 디렉토리를 다시 설정할 때는 캐시에 직접 지정한 값을 확인하세요.
새 이름을 명시적으로 캐시에 저장했다면 그 값이 계속 우선합니다.
기존 별칭으로 다시 제어하려면 해당 새 이름의 캐시 항목을 제거해야 합니다.
프로젝트를 하위 디렉토리로 포함하는 경우에도 부모 프로젝트의 명시적 설정을
자식 프로젝트의 기본값으로 강제로 덮어쓰지 않습니다.
자세한 동작과 검증 절차는 [일관성 검사 안내](ci/README.md)에 설명되어 있습니다.

의존성을 찾았다는 메시지만으로 원하는 소스가 사용된다고 판단하지 마세요.
설치된 패키지, 형제 디렉토리의 체크아웃, FetchContent의 소스가 서로 다른 버전일 수 있습니다.
재현이 필요한 빌드에서는 사용한 소스 커밋과 실제 포함 경로를 기록하고,
여러 시스템을 함께 변경했다면 같은 조합으로 소비자 빌드도 확인하세요.
릴리스 포트는 새 옵션을 모르는 과거 태그를 참조할 수 있으므로
포트의 호환 별칭은 해당 릴리스의 지원 여부를 확인한 뒤 제거해야 합니다.

## 모듈과 헤더 방식의 구분

기본 사용 방식은 C++ 헤더를 포함하고 CMake 대상에 연결하는 것입니다.
`CONTAINER_BUILD_MODULES`는 별도의 실험적 C++20 모듈 빌드를 선택합니다.
컴파일러가 C++20 언어 기능을 지원한다는 사실과 이름 있는 모듈의 의존성 탐색을
지원한다는 사실은 서로 다릅니다. 모듈을 사용하려면 CMake, 컴파일러,
생성기와 의존성 스캐너의 조합까지 확인해야 합니다.
헤더 방식에 대한 빌드 성공을 모듈 import에 대한 검증으로 해석하면 안 됩니다.

애플리케이션에서 모듈을 선택할 때는 라이브러리와 소비자에 같은 C++ 표준을
설정하고, 실제 모듈 대상이 생성되었는지 구성 로그에서 확인하세요.
지원되지 않는 환경에서 헤더 방식으로 되돌아간 경우에는 그 사실을 기록해야 합니다.
모듈 관련 선언은 [CMake 모듈 설정](cmake/modules.cmake)에 있고,
현재 소스 빌드의 일반 옵션은 [옵션 정의](cmake/options.cmake)에 있습니다.
공개 릴리스의 사용법을 작성할 때는 개발 브랜치의 대상이나 옵션이
그 릴리스에도 존재하는지 별도로 확인하세요.

## 동시 접근과 값의 수명

읽기 전용으로 공유하는 값과 값을 변경하는 컨테이너는 접근 조건이 다릅니다.
여러 스레드에서 동일한 컨테이너를 수정하려면 사용 중인 API가 제공하는
동기화 방식과 참조의 수명을 함께 검토해야 합니다.
스레드 안전한 래퍼를 사용하더라도, 래퍼 밖으로 전달한 참조를 다른 스레드가
수정하는 애플리케이션 코드까지 자동으로 보호하지는 않습니다.
콜백이나 비동기 작업에 값을 전달할 때는 소유권을 이전하는지,
복사본을 만드는지, 호출자가 객체의 수명을 유지해야 하는지 구분하세요.

메모리 풀, RCU 및 SIMD 관련 구현은 사용 목적이 서로 다릅니다.
메모리 풀은 할당 경로에 영향을 주고, RCU는 읽기와 갱신 시점의 관리에 관여하며,
SIMD는 지원되는 데이터 연산을 벡터 명령으로 수행합니다.
어느 기능 하나가 모든 워크로드의 지연 시간이나 메모리 사용량을 결정하지는 않습니다.
기능을 선택할 때는 입력 크기, 값 타입, 갱신 빈도와 실제 대상 플랫폼으로 검증하세요.
세부 구현은 [내부 구성 요소](include/kcenon/container/internal/)에 있습니다.

## 오류 처리와 경계 설계

직렬화 결과와 외부 입력은 정상 경로와 실패 경로를 모두 처리해야 합니다.
공통 Result 기반 인터페이스를 사용할 때는 성공 여부를 먼저 확인하고,
오류 코드와 메시지를 호출자가 이해할 수 있는 경계까지 전달하세요.
외부 형식을 읽는 코드에서는 잘못된 값 타입, 잘린 데이터,
지원하지 않는 형식을 정상적인 성공 값으로 바꾸지 않는 것이 중요합니다.
예제의 짧은 오류 처리 코드는 애플리케이션의 복구 정책을 대신하지 않습니다.

라이브러리 경계를 넘을 때는 값의 도메인 의미와 전송 형식을 별도로 정의하세요.
네트워크 계층과 데이터베이스 계층이 같은 컨테이너를 사용하더라도,
전송 재시도, 저장 실패, 부분 갱신에 대한 정책은 각각의 계층에서 결정합니다.
내부 구현 헤더에 직접 의존하기 전에 공개 API로 같은 작업을 표현할 수 있는지
확인하면 이후 버전 변경에 대응하기 쉽습니다.
현재 공개 헤더 경로와 대상 이름은 [최상위 빌드 정의](CMakeLists.txt)가 기준입니다.

## 예제, 튜토리얼과 검증

루트의 `samples/`에 있던 교육용 소스는 `examples/tutorials/`로 통합되었습니다.
기존 샘플 빌드 옵션과 실행 대상 이름은 유지하므로 애플리케이션 빌드 설정을
동시에 변경할 필요는 없습니다. 설치된 샘플의 위치와 빌드 결과 디렉토리는
소스 디렉토리의 이름과 별개이므로 실제 CMake 설정을 확인하세요.
[튜토리얼](examples/tutorials/)은 기본 사용법과 동시 접근을 살펴볼 때 사용하고,
[예제 모음](examples/)은 스키마, 직렬화 전략과 다른 구성 요소의 조합을 살펴볼 때 사용합니다.

변경을 검증할 때는 수정한 코드가 선택되는 구성으로 빌드해야 합니다.
예제를 비활성화한 라이브러리 빌드만으로 예제 소스의 컴파일 여부를 알 수 없습니다.
선택적 기능은 켠 구성과 끈 구성을 구분하고, 설치 패키지 소비와
소스 디렉토리 직접 포함도 서로 다른 경로로 확인하세요.
일관성 검사는 메타데이터와 소스 구조의 편차를 찾는 도구이며,
애플리케이션의 모든 동작이나 성능을 증명하는 시험은 아닙니다.

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

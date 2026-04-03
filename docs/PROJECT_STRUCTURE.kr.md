---
doc_id: "CNT-PROJ-003"
doc_title: "Container System 프로젝트 구조"
doc_version: "1.0.0"
doc_date: "2026-04-04"
doc_status: "Released"
project: "container_system"
category: "PROJ"
---

# Container System 프로젝트 구조

> **SSOT**: This document is the single source of truth for **Container System 프로젝트 구조**.

**언어:** [English](PROJECT_STRUCTURE.md) | **한국어**

**최종 업데이트**: 2025-12-10

## 개요

이 문서는 Container System 프로젝트 구조에 대한 포괄적인 정보를 제공하며, 디렉토리 구성, 파일 설명, 모듈 의존성 및 빌드 아티팩트를 포함합니다.

## 디렉토리 트리

```
container_system/
├── 📁 core/                        # 코어 컨테이너 기능
│   ├── concepts.h                  # 타입 검증을 위한 C++20 concepts (신규)
│   ├── container.h                 # 메인 컨테이너 클래스
│   ├── container.cpp               # 컨테이너 구현
│   ├── simd_batch.h                # SIMD 친화적 배치 컨테이너 (typed_container에서 이름 변경, TriviallyCopyable concept 사용)
│   ├── typed_container.h           # Deprecated: simd_batch.h로 리다이렉트
│   ├── value_types.h               # Value 타입 열거형
│   ├── value_types.cpp             # Value 타입 구현
│   ├── value_store.h               # Value 저장소 추상화
│   └── value_store.cpp             # Value 저장소 구현
├── 📁 internal/                    # 내부 구현
│   ├── value.h                     # Value 클래스 (ValueVariantType, ValueVisitor concepts 사용)
│   ├── value.cpp                   # Value 구현
│   ├── thread_safe_container.h     # 스레드 안전 컨테이너 (KeyValueCallback, MutableKeyValueCallback concepts 사용)
│   ├── thread_safe_container.cpp   # 스레드 안전 구현
│   ├── variant_value_factory.h     # Variant value 팩토리 (Arithmetic concept 사용)
│   ├── memory_pool.h               # 할당을 위한 메모리 풀
│   ├── simd_processor.h            # SIMD 최적화 유틸리티
│   └── simd_processor.cpp          # SIMD 구현
├── 📁 integration/                 # 통합 헬퍼
│   ├── messaging_integration.h     # 메시징 통합 (IntegralType, FloatingPointType, StringLike concepts 사용)
│   └── messaging_integration.cpp   # 메시징 구현
├── 📁 include/container/           # 공개 API 헤더 (호환성)
│   ├── 📁 core/                    # 코어 컨테이너 기능
│   │   ├── container.h             # 메인 컨테이너 클래스
│   │   ├── value.h                 # 추상 value 베이스 클래스
│   │   ├── value_types.h           # Value 타입 열거형
│   │   └── value_factory.h         # 타입 안전 value 생성
│   ├── 📁 values/                  # 구체적 value 구현
│   │   ├── primitive_values.h      # bool, char, string
│   │   ├── numeric_values.h        # int8-int64, float, double
│   │   ├── container_value.h       # 중첩 컨테이너 지원
│   │   └── bytes_value.h           # 원시 바이트 배열
│   ├── 📁 advanced/                # 고급 기능
│   │   ├── variant_value.h         # 다형성 value 저장소
│   │   ├── thread_safe_container.h # 스레드 안전 래퍼
│   │   └── simd_processor.h        # SIMD 최적화 유틸리티
│   ├── 📁 serialization/           # 직렬화 지원
│   │   ├── binary_serializer.h     # 바이너리 형식
│   │   ├── json_serializer.h       # JSON 형식
│   │   ├── xml_serializer.h        # XML 형식
│   │   └── format_detector.h       # 자동 형식 감지
│   ├── 📁 integration/             # 통합 헬퍼
│   │   ├── messaging_builder.h     # 메시징 시스템 빌더
│   │   ├── network_serializer.h    # 네트워크 최적화
│   │   └── database_adapter.h      # 데이터베이스 스토리지 어댑터
│   └── 📁 adapters/                # 오류 처리 어댑터
│       ├── common_result_adapter.h # Result<T> 어댑터
│       └── error_codes.h           # 오류 코드 정의
├── 📁 src/                         # 구현 파일
│   ├── 📁 core/
│   │   ├── container.cpp
│   │   ├── value.cpp
│   │   └── value_factory.cpp
│   ├── 📁 values/
│   │   ├── primitive_values.cpp
│   │   ├── numeric_values.cpp
│   │   ├── container_value.cpp
│   │   └── bytes_value.cpp
│   ├── 📁 advanced/
│   │   ├── variant_value.cpp
│   │   ├── thread_safe_container.cpp
│   │   └── simd_processor.cpp
│   ├── 📁 serialization/
│   │   ├── binary_serializer.cpp
│   │   ├── json_serializer.cpp
│   │   ├── xml_serializer.cpp
│   │   └── format_detector.cpp
│   ├── 📁 integration/
│   │   ├── messaging_builder.cpp
│   │   ├── network_serializer.cpp
│   │   └── database_adapter.cpp
│   └── 📁 adapters/
│       └── common_result_adapter.cpp
├── 📁 examples/                    # 예제 애플리케이션
│   ├── 📁 basic_container_example/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   ├── 📁 advanced_container_example/
│   │   ├── CMakeLists.txt
│   │   └── main.cpp
│   ├── 📁 real_world_scenarios/
│   │   ├── financial_trading.cpp
│   │   ├── iot_sensors.cpp
│   │   └── web_api.cpp
│   └── 📁 messaging_integration_example/
│       ├── CMakeLists.txt
│       └── main.cpp
├── 📁 tests/                       # 테스트 스위트
│   ├── 📁 unit/                    # 유닛 테스트
│   │   ├── container_test.cpp
│   │   ├── value_factory_test.cpp
│   │   ├── serialization_test.cpp
│   │   ├── thread_safety_test.cpp
│   │   └── simd_test.cpp
│   ├── 📁 integration/             # 통합 테스트
│   │   ├── messaging_integration_test.cpp
│   │   ├── network_integration_test.cpp
│   │   └── database_integration_test.cpp
│   └── 📁 performance/             # 성능 벤치마크
│       ├── benchmark_main.cpp
│       ├── container_benchmark.cpp
│       ├── serialization_benchmark.cpp
│       └── simd_benchmark.cpp
├── 📁 docs/                        # 문서
│   ├── README.md
│   ├── FEATURES.md
│   ├── BENCHMARKS.md
│   ├── PROJECT_STRUCTURE.md
│   ├── PRODUCTION_QUALITY.md
│   ├── ARCHITECTURE.md
│   ├── API_REFERENCE.md
│   ├── USER_GUIDE.md
│   ├── PERFORMANCE.md
│   ├── MIGRATION.md
│   ├── INTEGRATION.md
│   ├── 📁 guides/
│   │   ├── BUILD_GUIDE.md
│   │   ├── QUICK_START.md
│   │   ├── BEST_PRACTICES.md
│   │   ├── TROUBLESHOOTING.md
│   │   └── FAQ.md
│   ├── 📁 advanced/
│   │   ├── SIMD_OPTIMIZATION.md
│   │   ├── THREAD_SAFETY.md
│   │   └── MEMORY_MANAGEMENT.md
│   └── 📁 performance/
│       ├── BASELINE.md
│       └── MEMORY_POOL_PERFORMANCE.md
├── 📁 cmake/                       # CMake 모듈
│   ├── FindUtilities.cmake
│   ├── CompilerOptions.cmake
│   └── Dependencies.cmake
├── 📁 scripts/                     # 빌드 스크립트
│   ├── dependency.sh
│   ├── dependency.bat
│   ├── dependency.ps1
│   ├── build.sh
│   ├── build.bat
│   └── build.ps1
├── 📁 benchmarks/                  # 벤치마크 데이터
│   ├── BASELINE.md
│   └── results/
├── 📁 .github/                     # GitHub 워크플로우
│   └── workflows/
│       ├── ci.yml
│       ├── coverage.yml
│       ├── static-analysis.yml
│       ├── dependency-security-scan.yml
│       └── build-Doxygen.yaml
├── 📄 CMakeLists.txt
├── 📄 vcpkg.json
├── 📄 .gitignore
├── 📄 LICENSE
├── 📄 README.md
└── 📄 README.kr.md
```

## 코어 모듈 파일

### Container Core (`core/`)

#### `concepts.h`
**목적**: container_system 타입 검증을 위한 C++20 concepts

**주요 기능**:
- 명확한 오류 메시지와 함께 컴파일 타임 타입 제약
- SFINAE 기반 제약 대체
- common_system concepts와 통합

**요구사항**:
- C++20 concepts를 지원하는 컴파일러
- GCC 10+, Clang 10+, MSVC 2022+

**정의된 Concepts**:

| 카테고리 | Concepts |
|----------|----------|
| 타입 제약 | `Arithmetic`, `IntegralType`, `FloatingPointType`, `SignedIntegral`, `UnsignedIntegral`, `TriviallyCopyable` |
| 값 타입 | `ValueVariantType`, `NumericValueType`, `StringLike`, `ByteContainer` |
| 콜백 | `ValueVisitor`, `KeyValueCallback`, `MutableKeyValueCallback`, `ValueMapCallback`, `ConstValueMapCallback` |
| 직렬화 | `Serializable`, `JsonSerializable` |
| 컨테이너 | `ContainerValue` |

**사용 예시**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

// SIMD 친화적 컨테이너를 위한 TriviallyCopyable 사용
template<TriviallyCopyable TValue>
class simd_batch { /* ... */ };  // typed_container에서 이름 변경 (Issue #328)

// 반복을 위한 KeyValueCallback 사용
template<KeyValueCallback Func>
void for_each(Func&& func) const;
```

**통합 지점**:
- `simd_batch.h` - `TriviallyCopyable` concept 사용
- `thread_safe_container.h` - `KeyValueCallback`, `MutableKeyValueCallback`, `ValueMapCallback`, `ConstValueMapCallback` 사용
- `value.h` - `ValueVariantType`, `ValueVisitor` 사용
- `variant_value_factory.h` - `Arithmetic` 사용
- `messaging_integration.h` - `IntegralType`, `FloatingPointType`, `StringLike` 사용

#### `container.h` / `container.cpp`
**목적**: 헤더 관리 및 value 저장소를 제공하는 메인 컨테이너 클래스

**주요 기능**:
- 헤더 관리 (source, target, message_type)
- 해시 맵 조회를 사용한 value 저장소
- 직렬화/역직렬화 지원
- 복사 연산 (얕은 복사 및 깊은 복사)
- 요청/응답 패턴을 위한 헤더 스왑

**공개 인터페이스**:
```cpp
class value_container {
    // 헤더 관리
    void set_source(const std::string& id, const std::string& sub_id = "");
    void set_target(const std::string& id, const std::string& sub_id = "");
    void set_message_type(const std::string& type);
    void swap_header();

    // Value 관리
    void add_value(std::shared_ptr<value> val);
    void set_values(const std::vector<std::shared_ptr<value>>& values);
    std::shared_ptr<value> get_value(const std::string& key) const;
    void reserve_values(size_t count);

    // 직렬화
    std::string serialize() const;
    std::vector<uint8_t> serialize_array() const;
    std::string to_json() const;
    std::string to_xml() const;
};
```

#### `value.h` / `value.cpp`
**목적**: 모든 value 타입을 위한 추상 베이스 클래스

**주요 기능**:
- 타입 열거형 및 식별
- 직렬화를 위한 가상 인터페이스
- 문자열 변환 지원
- 키-값 페어링

**공개 인터페이스**:
```cpp
class value {
    virtual value_types type() const = 0;
    virtual std::string key() const = 0;
    virtual std::string to_string() const = 0;
    virtual std::string serialize() const = 0;
    virtual std::shared_ptr<value> copy() const = 0;
};
```

#### `value_types.h`
**목적**: 타입 열거형 및 타입 식별

**주요 기능**:
- 15가지 구별되는 value 타입 (null ~ string)
- 타입 코드 매핑 ('0' ~ 'f')
- 타입 이름 변환 유틸리티

**열거형**:
```cpp
enum class value_types : char {
    null_value = '0',
    bool_value = '1',
    char_value = '2',
    // ... (총 15가지 타입)
    string_value = 'f'
};
```

#### `value_factory.h` / `value_factory.cpp`
**목적**: 타입 안전 value 생성을 위한 팩토리 패턴

**공개 인터페이스**:
```cpp
class value_factory {
    static std::shared_ptr<value> create(
        const std::string& key,
        value_types type,
        const std::string& data
    );

    static std::shared_ptr<bool_value> create_bool(const std::string& key, bool val);
    static std::shared_ptr<string_value> create_string(const std::string& key, const std::string& val);
    static std::shared_ptr<int64_value> create_int64(const std::string& key, int64_t val);
    static std::shared_ptr<double_value> create_double(const std::string& key, double val);
};
```

## Value 구현 파일

### Primitive Values (`include/container/values/primitive_values.h`)

**클래스**: `null_value`, `bool_value`, `char_value`, `string_value`

**기능**:
- 짧은 문자열을 위한 Small String Optimization (SSO)
- null value를 위한 제로 할당
- bool/char를 위한 싱글 바이트 저장소

### Numeric Values (`include/container/values/numeric_values.h`)

**클래스**: `int8_value`, `uint8_value`, `int16_value`, `uint16_value`, `int32_value`, `uint32_value`, `int64_value`, `uint64_value`, `float_value`, `double_value`

**기능**:
- SIMD 최적화 연산
- 효율적인 바이너리 직렬화
- 타입 안전 변환

### Thread-Safe Container (`include/container/advanced/thread_safe_container.h`)

**목적**: 동시 접근을 위한 스레드 안전 래퍼

**기능**:
- 안전한 동시 접근을 위한 읽기-쓰기 락
- 락프리 읽기 연산
- 동기화된 쓰기 연산

**공개 인터페이스**:
```cpp
class thread_safe_container {
    std::shared_ptr<value> get_value(const std::string& key) const;
    void set_value(const std::string& key, value_types type, const std::string& data);
    void add_value(std::shared_ptr<value> val);
    std::string serialize() const;
};
```

### SIMD Processor (`include/container/advanced/simd_processor.h`)

**목적**: 숫자 연산을 위한 SIMD 가속

**기능**:
- ARM NEON 지원 (Apple Silicon)
- x86 AVX2 지원 (Intel/AMD)
- 자동 플랫폼 감지
- 스칼라 연산으로 폴백

**공개 인터페이스**:
```cpp
class simd_processor {
    std::vector<double> process_array(const std::vector<double>& input);
    std::vector<float> process_array(const std::vector<float>& input);
    std::vector<int64_t> process_array(const std::vector<int64_t>& input);
    bool is_simd_available() const;
    std::string get_simd_type() const;
};
```

## 직렬화 모듈

### Binary Serializer (`include/container/serialization/binary_serializer.h`)

**목적**: 고성능 바이너리 직렬화

**기능**:
- 최소 오버헤드 (~10%)
- 빠른 직렬화 (1.8M ops/s)
- 효율적인 역직렬화 (2.1M ops/s)

### JSON Serializer (`include/container/serialization/json_serializer.h`)

**목적**: 사람이 읽을 수 있는 JSON 형식

**기능**:
- Pretty-printing 지원
- 타입 안전 JSON 생성
- nlohmann::json 통합

### XML Serializer (`include/container/serialization/xml_serializer.h`)

**목적**: 스키마 검증된 XML 형식

**기능**:
- 네임스페이스 지원
- 스키마 검증
- pugixml 통합

### Format Detector (`include/container/serialization/format_detector.h`)

**목적**: 역직렬화를 위한 자동 형식 감지

**기능**:
- 바이너리/JSON/XML 감지
- 매직 넘버 검증
- 바이너리 형식으로 폴백

## 통합 모듈

### Messaging Builder (`include/container/integration/messaging_builder.h`)

**목적**: 메시징 시스템 통합을 위한 플루언트 API

**기능**:
- 메서드 체이닝 빌더 패턴
- 타입 안전 구성
- 최적화 힌트 (speed, network)
- 중첩 컨테이너 지원

**공개 인터페이스**:
```cpp
class messaging_container_builder {
    messaging_container_builder& source(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& target(const std::string& id, const std::string& sub_id = "");
    messaging_container_builder& message_type(const std::string& type);
    messaging_container_builder& add_value(const std::string& key, const auto& value);
    messaging_container_builder& optimize_for_speed();
    messaging_container_builder& optimize_for_network();
    std::shared_ptr<value_container> build();
};
```

## 테스트 구성

### 유닛 테스트 (`tests/unit/`)

**커버리지**: 코어 기능, 개별 컴포넌트

**주요 테스트**:
- `container_test.cpp`: 컨테이너 연산, 헤더 관리
- `value_factory_test.cpp`: Value 생성, 타입 안전성
- `serialization_test.cpp`: 형식 직렬화/역직렬화
- `thread_safety_test.cpp`: 동시 접근 패턴
- `simd_test.cpp`: SIMD 연산, 플랫폼 감지

**테스트 프레임워크**: Google Test
**총 테스트 케이스**: 123+

### 성능 테스트 (`tests/performance/`)

**커버리지**: 벤치마크, 성능 회귀 감지

**주요 벤치마크**:
- `container_benchmark.cpp`: 컨테이너 생성, value 연산
- `serialization_benchmark.cpp`: 형식 성능 비교
- `simd_benchmark.cpp`: SIMD vs 스칼라 성능

**벤치마크 프레임워크**: Google Benchmark

## 빌드 아티팩트

### Debug 빌드

```
build/
├── bin/
│   ├── examples/
│   │   ├── basic_container_example
│   │   ├── advanced_container_example
│   │   └── messaging_integration_example
│   └── tests/
│       ├── unit_tests
│       ├── integration_tests
│       └── performance_tests
└── lib/
    ├── libcontainer.a              # 정적 라이브러리
    └── libcontainer.so             # 공유 라이브러리 (Linux)
```

### Release 빌드

```
build/
├── bin/
│   └── [debug와 동일]
├── lib/
│   └── [최적화된 라이브러리]
└── docs/
    └── html/                       # Doxygen 문서
        └── index.html
```

## 모듈 의존성

### 내부 의존성

```
utilities_module (외부 의존성)
    │
    └──> core (container, value, value_types, value_factory)
            │
            ├──> values (primitive, numeric, container, bytes)
            │      │
            │      └──> advanced (variant, thread_safe, simd)
            │
            ├──> serialization (binary, json, xml, format_detector)
            │
            ├──> integration (messaging_builder, network, database)
            │
            └──> adapters (common_result_adapter, error_codes)
```

### 외부 의존성

| 의존성 | 목적 | 버전 | 필수 |
|-----------|---------|---------|----------|
| **utilities_module** | 문자열 변환, 시스템 유틸리티 | 최신 | 예 |
| **nlohmann_json** | JSON 직렬화 | 3.11+ | 예 |
| **pugixml** | XML 직렬화 | 1.13+ | 예 |
| **googletest** | 유닛 테스팅 프레임워크 | 1.14+ | 테스트만 |
| **google-benchmark** | 성능 벤치마킹 | 1.8+ | 테스트만 |

> **참고**: 이 프로젝트는 C++20 기능을 필요로 합니다:
> - **C++20 Concepts**: 타입 검증 (GCC 10+, Clang 10+, 또는 MSVC 2022+ 필요)
> - **`std::format`**: 문자열 포매팅 (GCC 13+, Clang 14+, 또는 MSVC 19.29+ 필요)

### 선택적 통합 의존성

| 의존성 | 목적 | 통합 지점 |
|-----------|---------|------------------|
| **messaging_system** | 메시지 캡슐화 | `messaging_builder.h` |
| **network_system** | 네트워크 전송 | `network_serializer.h` |
| **database_system** | 영속 저장소 | `database_adapter.h` |

## 빌드 설정

### CMake 옵션

| 옵션 | 설명 | 기본값 |
|--------|-------------|---------|
| `ENABLE_MESSAGING_FEATURES` | 향상된 메시징 최적화 | ON |
| `ENABLE_PERFORMANCE_METRICS` | 실시간 성능 모니터링 | OFF |
| `ENABLE_EXTERNAL_INTEGRATION` | 외부 시스템 콜백 | ON |
| `BUILD_CONTAINER_EXAMPLES` | 예제 애플리케이션 빌드 | ON |
| `BUILD_CONTAINER_SAMPLES` | 샘플 프로그램 빌드 | ON |
| `USE_THREAD_SAFE_OPERATIONS` | 스레드 안전 연산 활성화 | ON |
| `ENABLE_SIMD` | SIMD 최적화 활성화 | ON |
| `BUILD_TESTING` | 테스트 스위트 빌드 | ON |

### 플랫폼별 빌드

#### Linux/macOS
```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_SIMD=ON \
  -DBUILD_TESTING=ON
cmake --build build -j$(nproc)
```

#### Windows (PowerShell)
```powershell
cmake -S . -B build `
  -G "Visual Studio 17 2022" `
  -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DENABLE_SIMD=ON
cmake --build build --config Release
```

## 파일 명명 규칙

### 헤더 파일
- **공개 API**: `include/container/<module>/<name>.h`
- **비공개 헤더**: `src/<module>/<name>_internal.h`

### 구현 파일
- **소스 파일**: `src/<module>/<name>.cpp`

### 테스트 파일
- **유닛 테스트**: `tests/unit/<module>_test.cpp`
- **통합 테스트**: `tests/integration/<module>_integration_test.cpp`
- **벤치마크**: `tests/performance/<module>_benchmark.cpp`

### 문서 파일
- **Markdown**: `docs/<TOPIC>.md`
- **한국어 번역**: `docs/<TOPIC>.kr.md`

## 참고 문서

- [FEATURES.md](FEATURES.md) / [FEATURES.kr.md](FEATURES.kr.md) - 완전한 기능 문서
- [BENCHMARKS.md](BENCHMARKS.md) / [BENCHMARKS.kr.md](BENCHMARKS.kr.md) - 성능 벤치마크
- [PRODUCTION_QUALITY.md](PRODUCTION_QUALITY.md) / [PRODUCTION_QUALITY.kr.md](PRODUCTION_QUALITY.kr.md) - 품질 메트릭
- [ARCHITECTURE.md](ARCHITECTURE.md) - 아키텍처 가이드
- [BUILD_GUIDE.md](guides/BUILD_GUIDE.md) - 상세 빌드 지침

---

**최종 업데이트**: 2025-12-10
**버전**: 0.1.1

---

Made with ❤️ by 🍀☀🌕🌥 🌊

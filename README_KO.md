[![CI](https://github.com/kcenon/container_system/actions/workflows/ci.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/ci.yml)
[![Code Coverage](https://github.com/kcenon/container_system/actions/workflows/coverage.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/coverage.yml)
[![codecov](https://codecov.io/gh/kcenon/container_system/branch/main/graph/badge.svg)](https://codecov.io/gh/kcenon/container_system)
[![Static Analysis](https://github.com/kcenon/container_system/actions/workflows/static-analysis.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/static-analysis.yml)
[![Security Scan](https://github.com/kcenon/container_system/actions/workflows/dependency-security-scan.yml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/dependency-security-scan.yml)
[![Documentation](https://github.com/kcenon/container_system/actions/workflows/build-Doxygen.yaml/badge.svg)](https://github.com/kcenon/container_system/actions/workflows/build-Doxygen.yaml)

# Container System

> **Language:** [English](README.md) | **한국어**

## 개요

Container System은 메시징 시스템과 범용 애플리케이션을 위한 포괄적인 데이터 관리를 위해 설계된 프로덕션 준비 완료, 고성능 C++17 타입 안전 컨테이너 프레임워크입니다. SIMD 최적화와 원활한 생태계 통합을 특징으로 하는 모듈식 인터페이스 기반 아키텍처로 구축되었습니다.

**주요 특징**:
- **타입 안전성**: 컴파일 타임 검사를 포함한 강력한 타입 시스템
- **고성능**: SIMD 가속 연산 (초당 1.8M 직렬화, 초당 25M SIMD 연산)
- **프로덕션 준비**: 완벽한 RAII 점수 (20/20), 데이터 레이스 제로, 포괄적인 테스팅
- **크로스 플랫폼**: Linux, macOS, Windows 네이티브 지원 및 최적화된 빌드 스크립트
- **다중 포맷**: Binary, JSON, XML 직렬화 및 자동 포맷 감지

### 미션

전 세계 개발자들에게 고성능 데이터 직렬화를 **접근 가능**하고, **타입 안전**하며, **효율적**으로 만듭니다.

## 빠른 시작

### 기본 사용 예제

```cpp
#include <kcenon/container/core/container.h>
#include <kcenon/container/integration/messaging_builder.h>

using namespace container_module;

int main() {
    // 빌더 패턴을 사용한 컨테이너 생성
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

### 사전 요구사항

- **컴파일러**: C++17 지원 (GCC 7+, Clang 5+, MSVC 2017+)
- **빌드 시스템**: CMake 3.16+
- **패키지 매니저**: vcpkg (스크립트로 자동 설치)

### 설치

```bash
# 저장소 복제
git clone https://github.com/kcenon/container_system.git
cd container_system

# 의존성 설치 (크로스 플랫폼)
./scripts/dependency.sh      # Linux/macOS
# 또는
scripts\dependency.bat       # Windows (CMD)
.\scripts\dependency.ps1     # Windows (PowerShell)

# 프로젝트 빌드
./scripts/build.sh           # Linux/macOS
# 또는
scripts\build.bat            # Windows (CMD)
.\scripts\build.ps1          # Windows (PowerShell)

# 예제 실행
./build/examples/basic_container_example
```

## 핵심 기능

### 타입 안전 값 시스템
- **15가지 내장 타입**: null부터 중첩 컨테이너까지
- **컴파일 타임 검사**: 템플릿 메타프로그래밍으로 타입 안전성 보장
- **런타임 검증**: 역직렬화 시 타입 검사
- **제로 오버헤드**: 타입 안전성에 대한 런타임 비용 없음

### 고성능 직렬화
- **바이너리 포맷**: 초당 1.8M 연산, ~10% 오버헤드
- **JSON 포맷**: 초당 950K 연산, 사람이 읽을 수 있음
- **XML 포맷**: 초당 720K 연산, 스키마 검증 포함
- **자동 감지**: 자동 포맷 식별

### SIMD 가속
- **ARM NEON**: Apple Silicon (M1/M2/M3)에서 3.2배 속도 향상
- **x86 AVX2**: Intel/AMD 프로세서에서 2.5배 속도 향상
- **자동 감지**: 플랫폼별 최적화 선택
- **25M ops/sec**: 숫자 배열 처리 처리량

### 스레드 안전성
- **락프리 읽기**: 동기화 없이 동시 읽기 연산
- **스레드 안전 래퍼**: 쓰기를 위한 선택적 `thread_safe_container`
- **선형 확장**: 8 스레드로 7.5배 처리량
- **데이터 레이스 제로**: ThreadSanitizer 검증 완료

### 프로덕션 품질
- **완벽한 RAII**: 20/20 점수 (A+ 등급), 생태계 내 최고
- **메모리 누수 제로**: AddressSanitizer 클린
- **데이터 레이스 제로**: ThreadSanitizer 검증 완료
- **123개 이상 테스트**: 포괄적인 테스트 커버리지
- **85% 이상 커버리지**: 지속적인 모니터링과 함께 라인 커버리지

📚 **[전체 기능 →](docs/FEATURES.md)**

## 성능 하이라이트

*Apple M1 (8코어, ARM NEON), macOS 26.1, 릴리스 빌드 벤치마크*

| 연산 | 처리량 | 비고 |
|-----------|-----------|-------|
| **컨테이너 생성** | 2M/초 | 헤더가 있는 빈 컨테이너 |
| **값 추가** | 4.5M/초 | SIMD를 사용한 숫자 값 |
| **바이너리 직렬화** | 1.8M/초 | 1KB 컨테이너, ~10% 오버헤드 |
| **JSON 직렬화** | 950K/초 | 프리티 프린트 활성화 |
| **SIMD 연산** | 25M/초 | ARM NEON 가속 |
| **이동 연산** | 4.2M/초 | 제로 카피 시맨틱 |

**플랫폼 비교**:

| 플랫폼 | 아키텍처 | 바이너리 직렬화 | SIMD 속도 향상 |
|----------|-------------|-------------|--------------|
| Apple M1 | ARM64 | 1.8M/초 | 3.2배 (NEON) |
| Intel i7-12700K | x64 | 1.6M/초 | 2.5배 (AVX2) |
| AMD Ryzen 9 | x64 | 1.55M/초 | 2.3배 (AVX2) |

⚡ **[전체 벤치마크 →](docs/BENCHMARKS.md)**

## 아키텍처 개요

```
                    ┌─────────────────┐
                    │ utilities_module│
                    │   (Foundation)  │
                    └────────┬────────┘
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
┌────────────────┐  ┌────────────────┐  ┌────────────────┐
│ container_system│  │messaging_system│  │ network_system │
│  (이 프로젝트)   │◄─│   (소비자)      │◄─│  (전송)        │
└────────────────┘  └────────────────┘  └────────────────┘
         │
         └──────────────────────┐
                                ▼
                    ┌────────────────────┐
                    │  database_system   │
                    │     (저장소)        │
                    └────────────────────┘
```

**통합 이점**:
- **타입 안전 메시징**: 런타임 오류 방지
- **성능 최적화**: 높은 처리량을 위한 SIMD 가속
- **범용 직렬화**: 다양한 요구를 위한 Binary, JSON, XML
- **통합 데이터 모델**: 생태계 전체에서 일관된 구조

🏗️ **[아키텍처 가이드 →](docs/ARCHITECTURE.md)**

## 문서

### 시작하기
- 📖 [빠른 시작 가이드](docs/guides/QUICK_START.md)
- 🔧 [빌드 가이드](docs/guides/BUILD_GUIDE.md)
- ✅ [모범 사례](docs/guides/BEST_PRACTICES.md)
- 🔍 [문제 해결](docs/guides/TROUBLESHOOTING.md)

### 핵심 문서
- 📚 [기능](docs/FEATURES.md) - 전체 기능 문서
- ⚡ [벤치마크](docs/BENCHMARKS.md) - 성능 분석
- 📦 [프로젝트 구조](docs/PROJECT_STRUCTURE.md) - 코드 구성
- 🏆 [프로덕션 품질](docs/PRODUCTION_QUALITY.md) - 품질 메트릭

### 기술 가이드
- 🏛️ [아키텍처](docs/ARCHITECTURE.md) - 시스템 설계 및 패턴
- 📘 [API 레퍼런스](docs/API_REFERENCE.md) - 전체 API 문서
- 📗 [사용자 가이드](docs/USER_GUIDE.md) - 사용 패턴 및 값 타입
- 🚀 [성능](docs/PERFORMANCE.md) - SIMD 최적화 가이드
- 🔗 [통합](docs/INTEGRATION.md) - 생태계 통합

### 문제 해결
- 🔁 [문제 해결 가이드](docs/guides/TROUBLESHOOTING.md)는 직렬화 실패, SIMD 비활성화, 통합 오류 시 `result.error()`를 어떻게 해석할지 단계별로 안내합니다.

### 개발
- 🤝 [기여하기](docs/CONTRIBUTING.md) - 기여 가이드라인
- 🔄 [마이그레이션](docs/MIGRATION.md) - messaging_system에서 마이그레이션
- 📋 [FAQ](docs/guides/FAQ.md) - 자주 묻는 질문

**언어 지원**: 대부분의 문서는 영어와 한국어(`*_KO.md`)로 제공됩니다.

## 값 타입

컨테이너 시스템은 15가지 고유한 값 타입을 지원합니다:

| 카테고리 | 타입 | 크기 |
|----------|-------|------|
| **기본 타입** | null, bool, char | 0-1 바이트 |
| **정수** | int8, uint8, int16, uint16, int32, uint32, int64, uint64 | 1-8 바이트 |
| **부동소수점** | float, double | 4-8 바이트 |
| **복합 타입** | bytes, container, string | 가변 |

**예제**:
```cpp
using namespace container_module;

// 팩토리를 사용한 값 생성
auto int_val = value_factory::create_int64("id", 12345);
auto str_val = value_factory::create_string("name", "홍길동");
auto dbl_val = value_factory::create_double("balance", 1500.75);
auto bool_val = value_factory::create_bool("active", true);

// 컨테이너에 추가
container->add_value(int_val);
container->add_value(str_val);
container->add_value(dbl_val);
container->add_value(bool_val);
```

📚 **[값 타입 상세 →](docs/FEATURES.md#value-types)**

## 생태계 통합

### 메시징 시스템과의 통합
```cpp
#include <messaging_system/messaging_client.h>

auto client = std::make_shared<messaging_client>("client_01");

auto message = messaging_container_builder()
    .source("client_01", "session_123")
    .target("server", "main")
    .message_type("request")
    .add_value("action", "query")
    .build();

client->send_container(message);
```

### 네트워크 시스템과의 통합
```cpp
#include <network_system/tcp_client.h>

auto tcp_client = network_system::TcpClient::create("localhost", 8080);

// 직렬화 및 전송
std::string data = container->serialize();
tcp_client->send(data);

// 수신 및 역직렬화
auto received = tcp_client->receive();
auto restored = std::make_shared<value_container>(received);
```

### 데이터베이스 시스템과의 통합
```cpp
#include <database_system/database_manager.h>

database_manager db;
db.connect("host=localhost dbname=app");

// BLOB로 저장
std::string data = container->serialize();
db.insert_query("INSERT INTO messages (data) VALUES (?)", data);
```

🌐 **[통합 가이드 →](docs/INTEGRATION.md)**

## 빌드

### 기본 빌드

```bash
# 구성
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 빌드
cmake --build build -j$(nproc)

# 테스트 실행
cd build && ctest
```

### 빌드 옵션

| 옵션 | 설명 | 기본값 |
|--------|-------------|---------|
| `ENABLE_MESSAGING_FEATURES` | 메시징 최적화 | ON |
| `ENABLE_PERFORMANCE_METRICS` | 성능 모니터링 | OFF |
| `ENABLE_SIMD` | SIMD 최적화 | ON |
| `BUILD_CONTAINER_EXAMPLES` | 예제 빌드 | ON |
| `BUILD_TESTING` | 테스트 빌드 | ON |

### CMake 통합

```cmake
# 하위 디렉터리로 추가
add_subdirectory(container_system)
target_link_libraries(your_target PRIVATE ContainerSystem::container)

# 또는 FetchContent 사용
include(FetchContent)
FetchContent_Declare(
    container_system
    GIT_REPOSITORY https://github.com/kcenon/container_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(container_system)
```

🔧 **[빌드 가이드 →](docs/guides/BUILD_GUIDE.md)**

## 플랫폼 지원

| 플랫폼 | 아키텍처 | 컴파일러 | SIMD | 상태 |
|----------|-------------|----------|------|--------|
| **Linux** | x86_64, ARM64 | GCC 9+, Clang 10+ | AVX2, NEON | ✅ |
| **macOS** | x86_64, ARM64 (Apple Silicon) | Apple Clang, Clang | AVX2, NEON | ✅ |
| **Windows** | x86, x64 | MSVC 2019+, Clang | AVX2 | ✅ |

**네이티브 빌드 스크립트**:
- Linux/macOS: `scripts/dependency.sh`, `scripts/build.sh`
- Windows: `scripts/dependency.bat`, `scripts/build.bat`, `scripts/dependency.ps1`, `scripts/build.ps1`

## 스레드 안전성

### 동시 읽기 연산

**보장**: 동기화 없이 스레드 안전

```cpp
// 여러 스레드가 안전하게 읽기 가능
std::vector<std::thread> readers;
for (int i = 0; i < 8; ++i) {
    readers.emplace_back([&container]() {
        auto value = container->get_value("price");
        // 안전한 동시 읽기
    });
}
```

**성능**: 8 스레드로 7.5배 속도 향상 (선형 확장)

### 동시 쓰기 연산

**사용**: 동기화된 쓰기를 위한 `thread_safe_container`

```cpp
#include <kcenon/container/advanced/thread_safe_container.h>

auto safe_container = std::make_shared<thread_safe_container>(container);

std::vector<std::thread> workers;
for (int i = 0; i < 4; ++i) {
    workers.emplace_back([&safe_container, i]() {
        safe_container->set_value("thread_" + std::to_string(i),
                                 int32_value,
                                 std::to_string(i));
    });
}
```

**검증**: 데이터 레이스 제로 (ThreadSanitizer 검증 완료)

## 오류 처리

### 하이브리드 어댑터 패턴

- **내부 연산**: 성능을 위한 C++ 예외
- **외부 API**: 타입 안전성을 위한 Result&lt;T&gt;

```cpp
#include <kcenon/container/adapters/common_result_adapter.h>
using namespace container::adapters;

// Result<T>를 사용한 직렬화
auto result = serialization_result_adapter::serialize(*container);
if (!result) {
    std::cerr << "오류: " << result.error().message << "\n";
    return -1;
}
auto data = result.value();

// Result<T>를 사용한 컨테이너 연산
auto get_result = container_result_adapter::get_value<double>(container, "price");
if (!get_result) {
    std::cerr << "오류: " << get_result.error().message << "\n";
}
```

**오류 코드**: -400 ~ -499 (common_system에 중앙화)

## 기여하기

기여를 환영합니다! 자세한 내용은 [기여 가이드](docs/CONTRIBUTING.md)를 참조하세요.

### 개발 설정

1. 저장소 포크
2. 기능 브랜치 생성 (`git checkout -b feature/amazing-feature`)
3. 변경사항 커밋 (`git commit -m 'Add amazing feature'`)
4. 브랜치에 푸시 (`git push origin feature/amazing-feature`)
5. Pull Request 열기

### 코드 스타일

- 현대적인 C++ 모범 사례 따르기
- RAII 및 스마트 포인터 사용
- 포맷 유지 (clang-format 제공)
- 포괄적인 테스트 작성

## 지원 및 기여

- 💬 [GitHub Discussions](https://github.com/kcenon/container_system/discussions)
- 🐛 [이슈 트래커](https://github.com/kcenon/container_system/issues)
- 🤝 [기여 가이드](docs/CONTRIBUTING.md)
- 📧 이메일: kcenon@naver.com

## 라이선스

이 프로젝트는 BSD 3-Clause License에 따라 라이선스가 부여됩니다 - 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

## 감사의 말

- 현대적인 직렬화 프레임워크 및 고성능 컴퓨팅 사례에서 영감을 받음
- kcenon@naver.com이 유지 관리

---

<p align="center">
  Made with ❤️ by 🍀☀🌕🌥 🌊
</p>

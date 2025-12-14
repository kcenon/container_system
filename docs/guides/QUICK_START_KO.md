# 빠른 시작 가이드

5분 만에 container_system을 시작하세요.

## 사전 요구사항

| 의존성 | 버전 | 필수 | 설명 |
|--------|------|------|------|
| C++20 컴파일러 | GCC 10+ / Clang 10+ / Apple Clang 12+ / MSVC 2022+ | 예 | C++20 Concepts 지원 필요 |
| CMake | 3.20+ | 예 | 빌드 시스템 |
| common_system | latest | 예 | C++20 Concepts 및 공통 인터페이스 |
| vcpkg | latest | 선택 | 패키지 관리 (권장) |

## 빠른 설치

### 1단계: 의존성 복제

```bash
# common_system 복제 (필수 의존성)
git clone https://github.com/kcenon/common_system.git

# container_system 복제
git clone https://github.com/kcenon/container_system.git
```

### 2단계: 의존성 설치 및 빌드

**Linux/macOS:**
```bash
cd container_system

# 의존성 설치
./scripts/dependency.sh

# 빌드
./scripts/build.sh
```

**Windows (CMD):**
```batch
cd container_system

rem 의존성 설치
scripts\dependency.bat

rem 빌드
scripts\build.bat
```

**Windows (PowerShell):**
```powershell
cd container_system

# 의존성 설치
.\scripts\dependency.ps1

# 빌드
.\scripts\build.ps1
```

### 3단계: 설치 확인

```bash
# 예제 실행
./build/examples/basic_container_example
```

## 기본 사용법

### 컨테이너 생성 및 사용

```cpp
#include <kcenon/container/core/container.h>
#include <kcenon/container/integration/messaging_builder.h>

using namespace container_module;

int main() {
    // 빌더 패턴을 사용한 컨테이너 생성
    auto container = messaging_container_builder()
        .source("my_app", "session_001")
        .target("service", "main")
        .message_type("data")
        .add_value("name", "example")
        .add_value("count", 42)
        .add_value("ratio", 3.14)
        .build();

    // 바이너리로 직렬화 (고성능)
    std::string binary_data = container->serialize();

    // JSON으로 직렬화 (사람이 읽을 수 있음)
    std::string json_data = container->to_json();

    // 바이너리에서 역직렬화
    auto restored = std::make_shared<value_container>(binary_data);

    // 값 접근
    auto name = restored->get_value("name");
    auto count = restored->get_value("count");

    return 0;
}
```

### Value Factory 사용

```cpp
#include <kcenon/container/core/container.h>

using namespace container_module;

int main() {
    // 컨테이너 생성
    auto container = std::make_shared<value_container>();

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

    // 직렬화
    std::string data = container->serialize();

    return 0;
}
```

## CMake 통합

### 옵션 1: 하위 디렉터리로 추가

```cmake
# CMakeLists.txt에서
add_subdirectory(path/to/container_system)
target_link_libraries(your_target PRIVATE ContainerSystem::container)
```

### 옵션 2: FetchContent 사용

```cmake
include(FetchContent)
FetchContent_Declare(
    container_system
    GIT_REPOSITORY https://github.com/kcenon/container_system.git
    GIT_TAG main
)
FetchContent_MakeAvailable(container_system)
target_link_libraries(your_target PRIVATE ContainerSystem::container)
```

## 다음 단계

- [기능](../FEATURES.md) - 모든 기능 살펴보기
- [API 레퍼런스](../API_REFERENCE.md) - 전체 API 문서
- [아키텍처](../ARCHITECTURE.md) - 시스템 설계 및 패턴
- [벤치마크](../BENCHMARKS.md) - 성능 분석
- [FAQ](FAQ.md) - 자주 묻는 질문

## 문제 해결

문제가 발생하면:

1. **common_system을 찾을 수 없음**: common_system이 동일한 상위 디렉터리에 복제되어 있는지 확인하세요
2. **C++20 기능을 사용할 수 없음**: C++20 Concepts를 지원하는 버전으로 컴파일러를 업데이트하세요
3. **CMake 버전이 너무 낮음**: CMake를 3.20 이상으로 업데이트하세요

자세한 내용은 [문제 해결 가이드](TROUBLESHOOTING.md)를 참조하세요.

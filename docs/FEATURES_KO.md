# Container System - 상세 기능

**언어:** [English](README.md) | **한국어**

**최종 업데이트**: 2025-11-28
**버전**: 0.1.0

이 문서는 container_system의 모든 기능에 대한 포괄적인 세부 정보를 제공합니다.

---

## 목차

- [핵심 기능](#핵심-기능)
- [타입 안전 컨테이너](#타입-안전-컨테이너)
- [직렬화 시스템](#직렬화-시스템)
- [SIMD 최적화](#simd-최적화)
- [메모리 풀링](#메모리-풀링)
- [스레드 안전성](#스레드-안전성)
- [통합 기능](#통합-기능)

---

## 핵심 기능

### 설계 철학

Container System은 세 가지 기본 원칙을 중심으로 설계되었습니다:

### 1. 성능 저하 없는 타입 안전성

시스템은 C++20 concepts 및 `std::variant`를 통해 컴파일 타임 타입 안전성을 제공하여, 일반적인 프로그래밍 오류를 방지하면서 타입 검사에 대한 런타임 오버헤드가 제로입니다.

**주요 설계 결정:**
- 다형적 값 저장을 위한 `std::variant` 사용 (컴파일 타임 타입 안전성)
- 타입 안전 값 팩토리를 위한 템플릿 메타프로그래밍
- 컴파일 타임 타입 매핑을 위한 constexpr 함수
- 인라인 함수 및 이동 시맨틱을 통한 제로 비용 추상화

### 2. 기본 성능 최적화

모든 컴포넌트는 사용자의 수동 최적화 없이 고처리량 시나리오에 최적화되어 있습니다.

**성능 특성:**
- 컨테이너 생성: 5M 컨테이너/초 (빈 컨테이너)
- 바이너리 직렬화: 2M 컨테이너/초 (평균 1KB 크기)
- SIMD 가속 연산: 25M 숫자 연산/초
- 메모리 효율: variant 저장으로 ~128 바이트 기준

### 3. 통합 우선 설계

아키텍처는 독립적인 사용성을 유지하면서 다른 에코시스템 컴포넌트와의 원활한 통합을 우선시합니다.

**통합 이점:**
- 메시징, 네트워크 및 데이터베이스 시스템 전반에 걸친 통합 데이터 모델
- 각 전송 계층에 최적화된 네이티브 직렬화 형식
- 레거시 messaging_system 컨테이너와의 하위 호환성
- 커스텀 통합을 위한 확장 가능한 값 타입 시스템

---

## 타입 안전 컨테이너

### variant_value_v2

`std::variant` 기반의 타입 안전 값 저장:

```cpp
#include <container/variant_value_v2.h>

using namespace kcenon::container;

// 타입 안전 값 생성
variant_value_v2 int_val(42);
variant_value_v2 str_val("Hello, World!");
variant_value_v2 vec_val(std::vector<int>{1, 2, 3, 4, 5});

// 타입 검사
if (int_val.is<int>()) {
    int value = int_val.get<int>();
}

// 방문자 패턴
int_val.visit([](auto&& value) {
    std::cout << "값: " << value << std::endl;
});
```

### 지원 타입

| 카테고리 | 타입 |
|----------|------|
| **정수** | `int8_t`, `int16_t`, `int32_t`, `int64_t`, `uint8_t`, `uint16_t`, `uint32_t`, `uint64_t` |
| **부동소수점** | `float`, `double` |
| **불리언** | `bool` |
| **문자열** | `std::string`, `std::string_view` |
| **바이트** | `std::vector<uint8_t>`, `std::span<const uint8_t>` |
| **컨테이너** | `std::vector<T>`, `std::map<K,V>`, 중첩 컨테이너 |
| **시간** | `std::chrono::system_clock::time_point` |

### container 클래스

키-값 쌍의 컬렉션:

```cpp
#include <container/container.h>

container data;

// 값 설정
data.set("user_id", 12345);
data.set("username", "john_doe");
data.set("is_active", true);
data.set("scores", std::vector<int>{95, 87, 92});

// 값 가져오기
auto user_id = data.get<int>("user_id");
auto username = data.get<std::string>("username");
auto scores = data.get<std::vector<int>>("scores");

// 존재 여부 확인
if (data.contains("email")) {
    auto email = data.get<std::string>("email");
}

// 기본값으로 가져오기
auto level = data.get_or("level", 1);  // 없으면 1 반환

// 반복
for (const auto& [key, value] : data) {
    std::cout << key << ": " << value.to_string() << std::endl;
}
```

---

## 직렬화 시스템

### 지원 형식

| 형식 | 처리량 | 특징 |
|------|--------|------|
| **Binary** | 2M ops/s | 최고 성능, 네트워크 전송에 최적 |
| **JSON** | 500K ops/s | 사람이 읽을 수 있음, REST API에 적합 |
| **XML** | 200K ops/s | 레거시 시스템 통합 |

### Binary 직렬화

```cpp
#include <container/serialization/binary_serializer.h>

container data;
data.set("message", "Hello");
data.set("count", 42);

// 직렬화
auto bytes = binary_serializer::serialize(data);

// 역직렬화
auto result = binary_serializer::deserialize(bytes);
if (result) {
    container restored = result.value();
}
```

### JSON 직렬화

```cpp
#include <container/serialization/json_serializer.h>

container data;
data.set("name", "John");
data.set("age", 30);

// JSON 문자열로 직렬화
std::string json = json_serializer::serialize(data);
// 결과: {"name":"John","age":30}

// JSON에서 역직렬화
auto result = json_serializer::deserialize(json);
```

### XML 직렬화

```cpp
#include <container/serialization/xml_serializer.h>

container data;
data.set("title", "Document");
data.set("version", 1);

// XML로 직렬화
std::string xml = xml_serializer::serialize(data);

// XML에서 역직렬화
auto result = xml_serializer::deserialize(xml);
```

---

## SIMD 최적화

### SIMD 가속 연산

ARM NEON 및 x86 AVX2를 활용한 고성능 숫자 연산:

```cpp
#include <container/simd/simd_operations.h>

std::vector<float> data(1000000, 1.0f);

// SIMD 가속 합계
float sum = simd::sum(data);

// SIMD 가속 최대/최소
float max_val = simd::max(data);
float min_val = simd::min(data);

// SIMD 가속 변환
simd::transform(data, [](float x) { return x * 2.0f; });
```

### 성능 비교

| 연산 | 스칼라 | SIMD (NEON) | SIMD (AVX2) |
|------|--------|-------------|-------------|
| 합계 (1M floats) | 2.5ms | 0.6ms | 0.4ms |
| 최대값 | 2.8ms | 0.7ms | 0.5ms |
| 변환 | 3.2ms | 0.9ms | 0.6ms |

---

## 메모리 풀링

### 메모리 풀 사용

할당 오버헤드 감소를 위한 메모리 풀:

```cpp
#include <container/memory/memory_pool.h>

// 메모리 풀 생성
memory_pool<container> pool(1000);  // 1000개 객체 사전 할당

// 풀에서 획득
auto* container_ptr = pool.acquire();
container_ptr->set("key", "value");

// 풀에 반환
pool.release(container_ptr);

// 풀 통계
std::cout << "사용 중: " << pool.in_use() << std::endl;
std::cout << "사용 가능: " << pool.available() << std::endl;
```

### 성능 영향

| 메트릭 | 표준 할당 | 메모리 풀 | 개선 |
|--------|----------|----------|------|
| 할당 시간 | 150ns | 15ns | 10x |
| 해제 시간 | 120ns | 10ns | 12x |
| 메모리 단편화 | 높음 | 낮음 | - |

---

## 스레드 안전성

### 스레드 안전 컨테이너

```cpp
#include <container/thread_safe_container.h>

thread_safe_container data;

// 여러 스레드에서 안전하게 접근
std::thread writer([&]() {
    for (int i = 0; i < 1000; ++i) {
        data.set("counter", i);
    }
});

std::thread reader([&]() {
    for (int i = 0; i < 1000; ++i) {
        auto value = data.get<int>("counter");
    }
});

writer.join();
reader.join();
```

### 동시성 보장

- **std::shared_mutex** 기반 읽기-쓰기 잠금
- 다중 동시 읽기 허용
- 단일 쓰기 독점 접근
- TSan으로 검증된 데이터 레이스 없음

---

## 통합 기능

### messaging_system 통합

```cpp
#include <container/integration/messaging_adapter.h>

// 메시지에서 컨테이너 생성
auto container = messaging_adapter::from_message(message);

// 컨테이너를 메시지로 변환
auto message = messaging_adapter::to_message(container);
```

### network_system 통합

```cpp
#include <container/integration/network_serializer.h>

// 네트워크 전송을 위한 최적화된 직렬화
auto bytes = network_serializer::serialize(container);

// 수신 데이터 역직렬화
auto container = network_serializer::deserialize(received_bytes);
```

### database_system 통합

```cpp
#include <container/integration/database_adapter.h>

// 컨테이너를 데이터베이스 행으로 변환
auto row = database_adapter::to_row(container);

// 데이터베이스 행에서 컨테이너 생성
auto container = database_adapter::from_row(result_row);
```

---

## 성능 벤치마크

### 컨테이너 생성

| 시나리오 | 처리량 | 메모리 |
|----------|--------|--------|
| 빈 컨테이너 | 5M/s | 128B |
| 10 필드 | 2M/s | 512B |
| 100 필드 | 500K/s | 4KB |
| 중첩 컨테이너 | 200K/s | 8KB |

### 직렬화 성능

| 형식 | 직렬화 | 역직렬화 | 크기 효율 |
|------|--------|----------|----------|
| Binary | 2M/s | 2.5M/s | 100% (기준) |
| JSON | 500K/s | 400K/s | 150% |
| XML | 200K/s | 150K/s | 250% |

---

## 사용 예시

### 기본 사용

```cpp
#include <container/container.h>

using namespace kcenon::container;

int main() {
    // 컨테이너 생성
    container user;
    user.set("id", 12345);
    user.set("name", "John Doe");
    user.set("email", "john@example.com");
    user.set("age", 30);
    user.set("active", true);

    // 값 읽기
    auto name = user.get<std::string>("name");
    auto age = user.get<int>("age");

    // JSON으로 직렬화
    auto json = json_serializer::serialize(user);
    std::cout << json << std::endl;

    return 0;
}
```

### 중첩 컨테이너

```cpp
container order;
order.set("order_id", 98765);
order.set("customer", "Jane Smith");

// 중첩된 아이템 목록
std::vector<container> items;

container item1;
item1.set("product", "Widget");
item1.set("quantity", 5);
item1.set("price", 19.99);
items.push_back(item1);

container item2;
item2.set("product", "Gadget");
item2.set("quantity", 2);
item2.set("price", 49.99);
items.push_back(item2);

order.set("items", items);
order.set("total", 199.93);
```

---

## 참고사항

### 스레드 안전성

- **variant_value_v2**: 생성 후 읽기 전용 (공유 안전)
- **container**: 기본적으로 비스레드 안전, `thread_safe_container` 사용
- **직렬화**: 스레드 안전 (상태 없음)
- **메모리 풀**: 내부 동기화 있음

### 권장 사항

- **고성능 필요**: Binary 직렬화 + 메모리 풀
- **디버깅/로깅**: JSON 직렬화
- **레거시 통합**: XML 직렬화
- **다중 스레드**: `thread_safe_container` 사용

---

**최종 업데이트**: 2025-11-28
**버전**: 0.1.0

---

Made with ❤️ by 🍀☀🌕🌥 🌊

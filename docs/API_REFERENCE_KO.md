# container_system API 레퍼런스

> **버전**: 2.1
> **최종 업데이트**: 2025-12-10
> **상태**: C++20 Concepts 통합 완료, variant_value_v2로 마이그레이션 중 (Phase 2 진행 중)

## 목차

1. [네임스페이스](#네임스페이스)
2. [C++20 Concepts](#c20-concepts)
3. [variant_value_v2 (권장)](#variant_value_v2-권장)
4. [Container](#container)
5. [직렬화/역직렬화](#직렬화역직렬화)

---

## 네임스페이스

### `container_module`

container_system의 모든 공개 API는 이 네임스페이스에 포함됩니다.

**하위 네임스페이스**:
- `container_module::concepts` - 타입 검증을 위한 C++20 concepts

**포함 항목**:
- `variant_value_v2` - 차세대 Value 시스템 (권장)
- `container` - 값 컨테이너
- 직렬화 함수들
- 컴파일 타임 타입 검증을 위한 C++20 concepts

---

## C++20 Concepts

### 개요

**헤더**: `#include <container/core/concepts.h>`

**설명**: container_system 타입 검증을 위한 C++20 concepts. SFINAE 기반 제약을 대체하여 명확한 오류 메시지와 함께 컴파일 타임 검증을 제공합니다.

**요구사항**:
- C++20 concepts를 지원하는 컴파일러
- GCC 10+, Clang 10+, MSVC 2022+

### 타입 제약 Concepts

#### `Arithmetic`

```cpp
template<typename T>
concept Arithmetic = std::is_arithmetic_v<T>;
```

**설명**: 산술 타입 (정수 또는 부동소수점).

**예시**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

template<Arithmetic T>
value make_numeric_value(std::string_view name, T val);

// 사용법
auto v1 = make_numeric_value("count", 42);      // OK: int는 산술 타입
auto v2 = make_numeric_value("rate", 3.14);     // OK: double은 산술 타입
// auto v3 = make_numeric_value("name", "test"); // 오류: const char*는 산술 타입이 아님
```

#### `TriviallyCopyable`

```cpp
template<typename T>
concept TriviallyCopyable = std::is_trivially_copyable_v<T>;
```

**설명**: memcpy로 안전하게 복사할 수 있는 타입. SIMD 최적화 연산이나 결정적 메모리 레이아웃이 필요한 타입에 사용합니다.

**예시**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

// SIMD 친화적 배치를 위한 typed_container에서 사용
template<TriviallyCopyable TValue>
class typed_container {
    std::vector<TValue> values_;
public:
    void push(const TValue& value) { values_.push_back(value); }
};

// 사용법
typed_container<int> int_container;       // OK: int는 trivially copyable
typed_container<double> double_container; // OK: double은 trivially copyable
// typed_container<std::string> str_container; // 오류: std::string은 trivially copyable이 아님
```

### 값 타입 Concepts

#### `ValueVariantType`

**설명**: ValueVariant에 저장할 수 있는 유효한 타입.

**유효한 타입**:
- `std::monostate` (null)
- `bool`
- `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
- `float`, `double`
- `std::string`
- `std::vector<uint8_t>` (bytes)
- `std::shared_ptr<thread_safe_container>`
- `array_variant`

### 콜백 Concepts

#### `KeyValueCallback`

```cpp
template<typename F>
concept KeyValueCallback = std::invocable<F, const std::string&, const value&>;
```

**설명**: const 키-값 쌍 반복을 위한 호출 가능 타입.

**예시**:
```cpp
#include <container/core/concepts.h>
using namespace container_module::concepts;

template<KeyValueCallback Func>
void for_each(Func&& func) const;

// 사용법
container.for_each([](const std::string& key, const value& val) {
    std::cout << key << ": " << val.to_string() << std::endl;
});
```

### Concepts 요약 표

| Concept | 설명 | 사용처 |
|---------|------|-------|
| `Arithmetic` | 정수 또는 부동소수점 타입 | 숫자 값 생성을 위한 템플릿 제약 |
| `IntegralType` | 정수 타입 | 메시징 통합에서 타입 검사 |
| `FloatingPointType` | 부동소수점 타입 | 메시징 통합에서 타입 검사 |
| `TriviallyCopyable` | memcpy/SIMD 안전 타입 | `typed_container` 템플릿 제약 |
| `ValueVariantType` | `ValueVariant`에 유효한 타입 | 타입 안전 값 연산 |
| `StringLike` | 문자열 변환 가능 타입 | 문자열 값 처리 |
| `KeyValueCallback` | const 반복 콜백 | `for_each()` 함수 |
| `MutableKeyValueCallback` | 가변 반복 콜백 | `for_each_mut()` 함수 |
| `Serializable` | 직렬화 메서드 보유 타입 | 직렬화 지원 |
| `JsonSerializable` | JSON 직렬화 타입 | JSON 출력 지원 |

---

## variant_value_v2 (권장)

### 개요

**헤더**: `#include <container/values/variant_value_v2.h>`

**설명**: 고성능 타입 안전 variant value 구현 (4.39배 성능 향상)

**지원 타입**:
- 기본 타입: `bool`, `int`, `uint`, `long`, `float`, `double`, `string`
- 컨테이너 타입: `vector`, `map`
- 바이너리 타입: `bytes`

### 생성 및 접근

```cpp
#include <container/values/variant_value_v2.h>

using namespace container_module;

// 생성
variant_value_v2 val_int(42);
variant_value_v2 val_str("hello");
variant_value_v2 val_double(3.14);

// 타입 검사
if (val_int.is<int>()) {
    // int 타입
}

// 값 접근
int i = val_int.get<int>();                    // 42
std::string s = val_str.get<std::string>();    // "hello"
```

### 컨테이너 타입

```cpp
// Vector
variant_value_v2 vec;
vec.set<std::vector<variant_value_v2>>({
    variant_value_v2(1),
    variant_value_v2(2),
    variant_value_v2(3)
});

auto& v = vec.get<std::vector<variant_value_v2>>();
for (const auto& item : v) {
    std::cout << item.get<int>() << std::endl;
}

// Map
variant_value_v2 map;
std::map<std::string, variant_value_v2> m;
m["name"] = variant_value_v2("Alice");
m["age"] = variant_value_v2(30);
map.set(m);
```

### 핵심 메서드

#### `is<T>()`

```cpp
template <typename T>
bool is() const;
```

**설명**: 타입 검사

**예시**:
```cpp
variant_value_v2 val(42);
if (val.is<int>()) {
    // int 타입
}
```

#### `get<T>()`

```cpp
template <typename T>
T& get();

template <typename T>
const T& get() const;
```

**설명**: 값 접근

**예외**:
- `std::bad_variant_access`: 타입 불일치 시 발생

#### `set<T>()`

```cpp
template <typename T>
void set(const T& value);
```

**설명**: 값 설정

---

## Container

### container 클래스

**헤더**: `#include <container/container.h>`

**설명**: Key-Value 저장소

#### 생성 및 사용

```cpp
#include <container/container.h>

using namespace container_module;

container c;

// 값 추가
c.add("name", variant_value_v2("Alice"));
c.add("age", variant_value_v2(30));
c.add("score", variant_value_v2(95.5));

// 값 조회
if (c.contains("name")) {
    auto name = c.get("name").get<std::string>();
    std::cout << "이름: " << name << std::endl;
}

// 값 제거
c.remove("score");
```

#### 핵심 메서드

##### `add()`

```cpp
void add(const std::string& key, const variant_value_v2& value);
```

**설명**: Key-Value 쌍 추가

##### `get()`

```cpp
variant_value_v2& get(const std::string& key);
const variant_value_v2& get(const std::string& key) const;
```

**설명**: 값 조회

**예외**:
- `std::out_of_range`: 키가 존재하지 않을 때 발생

##### `contains()`

```cpp
bool contains(const std::string& key) const;
```

**설명**: 키 존재 여부 확인

##### `remove()`

```cpp
bool remove(const std::string& key);
```

**설명**: Key-Value 쌍 제거

**반환값**:
- `true`: 성공적으로 제거됨
- `false`: 키가 존재하지 않음

---

## 직렬화/역직렬화

### JSON 직렬화

```cpp
#include <container/serialization.h>

using namespace container_module;

container c;
c.add("name", variant_value_v2("Alice"));
c.add("age", variant_value_v2(30));

// JSON으로 직렬화
std::string json = serialize_to_json(c);
// {"name":"Alice","age":30}

// JSON에서 역직렬화
container c2 = deserialize_from_json(json);
```

### 바이너리 직렬화

```cpp
// 바이너리로 직렬화
std::vector<uint8_t> binary = serialize_to_binary(c);

// 바이너리에서 역직렬화
container c2 = deserialize_from_binary(binary);
```

---

## 성능

### variant_value_v2 벤치마크

| 작업 | v1 (레거시) | v2 (신규) | 개선 |
|-----------|-------------|----------|-------------|
| 생성 | 45 ns | 10 ns | 4.5배 |
| 복사 | 52 ns | 12 ns | 4.33배 |
| 접근 | 8 ns | 2 ns | 4.0배 |
| 타입 검사 | 5 ns | 1 ns | 5.0배 |

**전체 평균**: 4.39배 성능 향상

**테스트 환경**: Apple M1 Max, 10 코어, macOS 14

---

## 마이그레이션 가이드

### 레거시 value에서 variant_value_v2로

**v1 (레거시)**:
```cpp
value val_old;
val_old.set_int(42);
int i = val_old.get_int();
```

**v2 (신규, 권장)**:
```cpp
variant_value_v2 val_new(42);
int i = val_new.get<int>();
```

**장점**:
- 타입 안전성 (컴파일 타임 검사)
- 성능 (4.39배 향상)
- 현대적인 C++ API

**상세 가이드**: [MIGRATION_GUIDE.md](advanced/VARIANT_VALUE_V2_MIGRATION_GUIDE.md)

---

## 참고사항

### 마이그레이션 진행 중

- **Phase 1**: ✅ 완료 (2025-11-06)
  - variant_value_v2 구현
  - 19/19 테스트 통과
- **C++20 Concepts**: ✅ 완료 (2025-12-09)
  - 타입 검증을 위한 C++20 concepts 통합
  - SFINAE 기반 제약을 명확한 오류 메시지로 대체
  - `core/concepts.h`에 18개 concepts 추가
- **Phase 2**: 🔄 진행 중
  - 핵심 컨테이너 마이그레이션
  - 팩토리 함수 구현
- **Phase 3-5**: ⏳ 대기 중

### 권장사항

- **새 코드**: variant_value_v2 사용 (권장)
- **기존 코드**: 점진적 마이그레이션 (MIGRATION_GUIDE 참조)
- **타입 제약**: 컴파일 타임 검증을 위해 C++20 concepts 사용

---

**작성일**: 2025-11-21
**수정일**: 2025-12-10
**버전**: 2.1
**관리자**: kcenon@naver.com

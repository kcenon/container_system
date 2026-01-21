# container_system API 레퍼런스

> **버전**: 0.2.1
> **최종 업데이트**: 2026-01-10
> **상태**: C++20 Concepts 통합 완료, variant_value_v2로 마이그레이션 중 (Phase 2 진행 중)

## 목차

1. [네임스페이스](#네임스페이스)
2. [C++20 Concepts](#c20-concepts)
3. [variant_value_v2 (권장)](#variant_value_v2-권장)
4. [Container](#container)
5. [비동기 API (C++20 코루틴)](#비동기-api-c20-코루틴)
6. [에러 코드](#에러-코드)
7. [직렬화/역직렬화](#직렬화역직렬화)

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

// SIMD 친화적 배치를 위한 simd_batch에서 사용 (Issue #328에서 typed_container에서 이름 변경)
template<TriviallyCopyable TValue>
class simd_batch {
    std::vector<TValue> values_;
public:
    void push(const TValue& value) { values_.push_back(value); }
};

// 사용법
simd_batch<int> int_container;       // OK: int는 trivially copyable
simd_batch<double> double_container; // OK: double은 trivially copyable
// simd_batch<std::string> str_container; // 오류: std::string은 trivially copyable이 아님
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
| `TriviallyCopyable` | memcpy/SIMD 안전 타입 | `simd_batch` 템플릿 제약 |
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

## 비동기 API (C++20 코루틴)

### 개요

**헤더**: `#include <container/internal/async/async.h>`

**네임스페이스**: `container_module::async`

**설명**: 논블로킹 컨테이너 연산을 위한 C++20 코루틴 기반 비동기 API.

**요구사항**:
- C++20 코루틴 지원 컴파일러
- GCC 10+ (11+에서 완전 지원), Clang 13+, MSVC 2019 16.8+
- CMake 옵션: `CONTAINER_ENABLE_COROUTINES=ON` (기본값)

### task<T>

비동기 계산을 나타내는 지연 코루틴 태스크 타입.

#### 메서드

##### `valid()`

```cpp
[[nodiscard]] bool valid() const noexcept;
```

**설명**: 태스크가 유효한 코루틴 핸들을 보유하고 있는지 확인.

##### `done()`

```cpp
[[nodiscard]] bool done() const noexcept;
```

**설명**: 코루틴이 완료되었는지 확인.

##### `get()`

```cpp
T get();                    // task<T>의 경우
void get();                 // task<void>의 경우
```

**설명**: 태스크의 결과를 가져옴. 캡처된 예외를 재발생시킴.

#### 팩토리 함수

##### `make_ready_task()`

```cpp
template<typename T>
task<T> make_ready_task(T value);

task<void> make_ready_task();
```

**설명**: 즉시 완료된 태스크를 생성.

##### `make_exceptional_task()`

```cpp
template<typename T>
task<T> make_exceptional_task(std::exception_ptr ex);
```

**설명**: 주어진 예외를 재발생시킬 태스크를 생성.

#### 예시

```cpp
#include <container/internal/async/async.h>
using namespace container_module::async;

task<int> compute() {
    co_return 42;
}

task<int> chained() {
    int value = co_await compute();
    co_return value * 2;
}

// 사용법
auto t = chained();
while (!t.done()) {
    std::this_thread::sleep_for(1ms);
}
int result = t.get();  // 84
```

---

### generator<T>

값 시퀀스를 지연 생성하는 코루틴 제너레이터.

#### 메서드

##### `valid()`

```cpp
[[nodiscard]] bool valid() const noexcept;
```

**설명**: 제너레이터가 유효한 코루틴 핸들을 보유하고 있는지 확인.

##### `begin()` / `end()`

```cpp
iterator begin();
std::default_sentinel_t end();
```

**설명**: 범위 기반 for 루프 지원.

#### 유틸리티 함수

##### `take()`

```cpp
template<typename T>
generator<T> take(generator<T> gen, size_t count);
```

**설명**: 제너레이터가 최대 `count`개의 값만 생성하도록 제한.

#### 예시

```cpp
generator<int> range(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}

// 사용법
for (int value : range(0, 10)) {
    std::cout << value << "\n";
}

// 무한 시퀀스에 take() 사용
generator<int> infinite() {
    int i = 0;
    while (true) co_yield i++;
}

for (int value : take(infinite(), 5)) {
    process(value);
}
```

---

### async_container

코루틴 기반 연산이 가능한 `value_container`의 비동기 래퍼.

#### 생성자

```cpp
async_container();                                          // 빈 컨테이너
explicit async_container(std::shared_ptr<value_container> container);  // 기존 컨테이너 래핑
async_container(async_container&& other) noexcept;         // 이동 생성자
```

#### 컨테이너 접근

##### `get_container()`

```cpp
[[nodiscard]] std::shared_ptr<value_container> get_container() const noexcept;
```

**설명**: 기본 컨테이너를 가져옴.

##### `set_container()`

```cpp
void set_container(std::shared_ptr<value_container> container) noexcept;
```

**설명**: 기본 컨테이너를 교체.

#### 값 연산

##### `set()`

```cpp
template<typename T>
async_container& set(std::string_view key, T&& value);
```

**설명**: 컨테이너에 값을 설정. 체이닝을 위해 `*this`를 반환.

##### `get()`

```cpp
template<typename T>
[[nodiscard]] std::optional<T> get(std::string_view key) const;
```

**설명**: 컨테이너에서 타입이 지정된 값을 가져옴.

##### `contains()`

```cpp
[[nodiscard]] bool contains(std::string_view key) const noexcept;
```

**설명**: 키가 존재하는지 확인.

#### 비동기 직렬화

##### `serialize_async()`

```cpp
[[nodiscard]] task<Result<std::vector<uint8_t>>> serialize_async() const;
```

**설명**: 컨테이너를 바이트 배열로 비동기 직렬화.

##### `serialize_string_async()`

```cpp
[[nodiscard]] task<Result<std::string>> serialize_string_async() const;
```

**설명**: 컨테이너를 문자열로 비동기 직렬화.

#### 비동기 역직렬화

##### `deserialize_async()`

```cpp
[[nodiscard]] static task<Result<std::shared_ptr<value_container>>>
    deserialize_async(std::span<const uint8_t> data);
```

**설명**: 바이트 배열에서 비동기 역직렬화.

##### `deserialize_string_async()`

```cpp
[[nodiscard]] static task<Result<std::shared_ptr<value_container>>>
    deserialize_string_async(std::string_view data);
```

**설명**: 문자열에서 비동기 역직렬화.

#### 비동기 파일 I/O

##### `load_async()`

```cpp
[[nodiscard]] task<VoidResult> load_async(
    std::string_view path,
    progress_callback callback = nullptr);
```

**설명**: 파일에서 컨테이너를 비동기 로드.

**파라미터**:
- `path`: 로드할 파일 경로
- `callback`: 선택적 프로그레스 콜백 `void(size_t bytes, size_t total)`

##### `save_async()`

```cpp
[[nodiscard]] task<VoidResult> save_async(
    std::string_view path,
    progress_callback callback = nullptr);
```

**설명**: 컨테이너를 파일에 비동기 저장.

#### 스트리밍

##### `serialize_chunked()`

```cpp
[[nodiscard]] generator<std::vector<uint8_t>> serialize_chunked(
    size_t chunk_size = 64 * 1024) const;
```

**설명**: 제너레이터를 사용하여 청크로 직렬화.

##### `deserialize_streaming()`

```cpp
[[nodiscard]] static task<Result<std::shared_ptr<value_container>>>
    deserialize_streaming(std::span<const uint8_t> data, bool is_final = true);
```

**설명**: 스트리밍 지원으로 역직렬화.

#### 예시

```cpp
#include <container/internal/async/async.h>
using namespace container_module::async;

task<void> example() {
    // 생성 및 채우기
    async_container cont;
    cont.set("name", std::string("test"))
        .set("value", static_cast<int64_t>(42));

    // 프로그레스와 함께 비동기 저장
    auto save_result = co_await cont.save_async("data.bin",
        [](size_t bytes, size_t total) {
            std::cout << bytes << "/" << total << "\n";
        });

    if (!save_result.is_ok()) {
        std::cerr << "저장 실패: " << save_result.error().message << "\n";
        co_return;
    }

    // 비동기 로드
    async_container loaded;
    auto load_result = co_await loaded.load_async("data.bin");

    if (load_result.is_ok()) {
        auto name = loaded.get<std::string>("name");
        // 로드된 데이터 사용
    }
}
```

---

### 독립 파일 유틸리티

#### `read_file_async()`

```cpp
[[nodiscard]] task<Result<std::vector<uint8_t>>> read_file_async(
    std::string_view path,
    progress_callback callback = nullptr);
```

**설명**: 파일 내용을 비동기로 읽음.

#### `write_file_async()`

```cpp
[[nodiscard]] task<VoidResult> write_file_async(
    std::string_view path,
    std::span<const uint8_t> data,
    progress_callback callback = nullptr);
```

**설명**: 데이터를 파일에 비동기로 쓰기.

---

### thread_pool_executor

**헤더**: `#include <container/internal/async/thread_pool_executor.h>`

효율적인 비동기 태스크 실행을 위한 스레드 풀.

#### 생성자

```cpp
explicit thread_pool_executor(size_t thread_count = std::thread::hardware_concurrency());
```

**설명**: 지정된 스레드 수로 executor를 생성.

#### 메서드

##### `submit()`

```cpp
template<typename F>
auto submit(F&& func) -> std::future<std::invoke_result_t<F>>;
```

**설명**: 스레드 풀에 작업을 제출.

##### `spawn()`

```cpp
template<typename T>
void spawn(task<T> t);
```

**설명**: 스레드 풀에서 코루틴 태스크를 실행.

##### `stop()`

```cpp
void stop();
```

**설명**: executor를 중지하고 모든 태스크가 완료될 때까지 대기.

#### 예시

```cpp
#include <container/internal/async/thread_pool_executor.h>
using namespace container_module::async;

thread_pool_executor executor(4);

// 람다 제출
auto future = executor.submit([]() {
    return expensive_computation();
});
auto result = future.get();

// 코루틴 실행
task<int> async_work() {
    co_return 42;
}
executor.spawn(async_work());
```

---

### 기능 감지

```cpp
namespace container_module::async {
    inline constexpr bool has_coroutine_support = /* 코루틴 사용 가능 시 true */;
}
```

**사용법**:
```cpp
#if CONTAINER_HAS_COROUTINES
    // 비동기 API 사용
#else
    // 동기 API로 폴백
#endif
```

---

## 에러 코드

### 개요

**헤더**: `#include <container/core/container/error_codes.h>`

**설명**: container_system Result<T> 패턴을 위한 표준화된 에러 코드입니다. 에러 코드는 쉬운 식별과 처리를 위해 백의 자리 숫자로 카테고리가 구분됩니다.

**네임스페이스**: `container_module::error_codes`

### 에러 코드 카테고리

| 카테고리 | 범위 | 설명 |
|----------|------|------|
| 값 연산 | 1xx (100-199) | 키/값 접근 및 수정 오류 |
| 직렬화 | 2xx (200-299) | 데이터 형식 및 인코딩 오류 |
| 검증 | 3xx (300-399) | 스키마 및 제약 검증 오류 |
| 리소스 | 4xx (400-499) | 파일 및 메모리 리소스 오류 |
| 스레드 안전성 | 5xx (500-599) | 동시성 및 락 오류 |

### 에러 코드 참조

#### 값 연산 (1xx)

| 코드 | 이름 | 설명 |
|------|------|------|
| 100 | `key_not_found` | 요청한 키가 컨테이너에 존재하지 않음 |
| 101 | `type_mismatch` | 값 타입이 요청한 타입과 일치하지 않음 |
| 102 | `value_out_of_range` | 숫자 값이 유효 범위를 벗어남 |
| 103 | `invalid_value` | 연산에 유효하지 않은 값 |
| 104 | `key_already_exists` | 고유 키가 필요한데 키가 이미 존재함 |
| 105 | `empty_key` | 빈 키 이름이 제공됨 |

#### 직렬화 (2xx)

| 코드 | 이름 | 설명 |
|------|------|------|
| 200 | `serialization_failed` | 직렬화 연산 실패 |
| 201 | `deserialization_failed` | 역직렬화 연산 실패 |
| 202 | `invalid_format` | 데이터 형식이 유효하지 않거나 인식할 수 없음 |
| 203 | `version_mismatch` | 데이터 버전이 예상 버전과 일치하지 않음 |
| 204 | `corrupted_data` | 데이터가 손상되었거나 불완전함 |
| 205 | `header_parse_failed` | 헤더 파싱 실패 |
| 206 | `value_parse_failed` | 값 파싱 실패 |
| 207 | `encoding_error` | 인코딩/디코딩 오류 (예: 유효하지 않은 UTF-8) |

#### 검증 (3xx)

| 코드 | 이름 | 설명 |
|------|------|------|
| 300 | `schema_validation_failed` | 스키마 검증 실패 |
| 301 | `missing_required_field` | 필수 필드가 누락됨 |
| 302 | `constraint_violated` | 제약 조건 위반 |
| 303 | `type_constraint_violated` | 타입 제약 조건 불충족 |
| 304 | `max_size_exceeded` | 최대 크기 초과 |

#### 리소스 (4xx)

| 코드 | 이름 | 설명 |
|------|------|------|
| 400 | `memory_allocation_failed` | 메모리 할당 실패 |
| 401 | `file_not_found` | 파일을 찾을 수 없음 |
| 402 | `file_read_error` | 파일 읽기 오류 |
| 403 | `file_write_error` | 파일 쓰기 오류 |
| 404 | `permission_denied` | 권한 거부 |
| 405 | `resource_exhausted` | 리소스 고갈 |
| 406 | `io_error` | I/O 연산 실패 |

#### 스레드 안전성 (5xx)

| 코드 | 이름 | 설명 |
|------|------|------|
| 500 | `lock_acquisition_failed` | 락 획득 실패 |
| 501 | `concurrent_modification` | 동시 수정 감지됨 |
| 502 | `lock_timeout` | 데드락 감지 또는 타임아웃 |

### 유틸리티 함수

#### `get_message(int code)`

에러 코드에 대한 사람이 읽을 수 있는 메시지를 반환합니다.

```cpp
#include <container/core/container/error_codes.h>

using namespace container_module::error_codes;

auto msg = get_message(key_not_found);  // "Key not found" 반환
auto msg2 = get_message(999);           // "Unknown error" 반환
```

#### `get_category(int code)`

에러 코드의 카테고리 이름을 반환합니다.

```cpp
auto cat = get_category(100);   // "value_operation" 반환
auto cat2 = get_category(200);  // "serialization" 반환
auto cat3 = get_category(300);  // "validation" 반환
auto cat4 = get_category(400);  // "resource" 반환
auto cat5 = get_category(500);  // "thread_safety" 반환
```

#### 카테고리 확인 함수

```cpp
// 에러가 특정 카테고리에 속하는지 확인
bool is_value_error(int code);         // 1xx 범위인지 확인
bool is_serialization_error(int code); // 2xx 범위인지 확인
bool is_validation_error(int code);    // 3xx 범위인지 확인
bool is_resource_error(int code);      // 4xx 범위인지 확인
bool is_thread_error(int code);        // 5xx 범위인지 확인

// 사용 예시
if (is_resource_error(err.code)) {
    // 리소스 관련 에러 처리
}
```

#### `make_message(int code, std::string_view detail = "")`

컨텍스트가 포함된 상세 에러 메시지를 생성합니다.

```cpp
auto msg = make_message(file_not_found, "/path/to/file.txt");
// "File not found: /path/to/file.txt" 반환
```

### 사용 예시

```cpp
#include <container/core/container/error_codes.h>

using namespace container_module;
using namespace container_module::error_codes;

Result<int> get_value(const value_container& c, std::string_view key) {
    auto val = c.get_value(key);
    if (!val) {
        return error(error_info{
            key_not_found,
            make_message(key_not_found, key),
            "container_system"
        });
    }
    // 값 처리...
}

// 에러 처리
auto result = get_value(container, "user_id");
if (!result) {
    const auto& err = result.error();
    if (is_value_error(err.code)) {
        // 값 관련 에러 처리
    } else if (is_resource_error(err.code)) {
        // 리소스 관련 에러 처리
    }
}
```

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
**버전**: 0.2.1
**관리자**: kcenon@naver.com

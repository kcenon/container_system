# 네임스페이스 마이그레이션 가이드

> **Language:** [English](NAMESPACE_MIGRATION.md) | **한국어**

## 개요

container_system은 향후 메이저 버전에서 `container_module`에서 `kcenon::container` 네임스페이스로 전환하여 에코시스템 규칙에 맞출 예정입니다.

**마이그레이션 경로**: container_module (v1.x) → kcenon::container (v2.0+)
**호환성 파괴**: 예 (네임스페이스, 인클루드 경로)
**호환성**: 전환 기간 동안 별칭 제공

---

## 변경 이유

### 현재 에코시스템 네임스페이스

| 시스템 | 현재 네임스페이스 | 패턴 |
|--------|-------------------|------|
| common_system | `kcenon::common` | ✅ 일관됨 |
| thread_system | `kcenon::thread` | ✅ 일관됨 |
| logger_system | `kcenon::logger` | ✅ 일관됨 |
| network_system | `kcenon::network` | ✅ 일관됨 |
| **container_system** | `container_module` | ❌ 불일치 |

`container_module` 네임스페이스는 `kcenon::<system>` 명명 규칙을 따르지 않아 에코시스템 통합이 직관적이지 않고 사용자 코드에서 특별한 처리가 필요합니다.

---

## 현재 vs 미래

| 버전 | 네임스페이스 | 인클루드 경로 |
|------|-----------|--------------|
| v1.x (현재) | `container_module` | `<container/...>` |
| v2.0 (계획) | `kcenon::container` | `<kcenon/container/...>` |

---

## 타임라인

| 버전 | 상태 | 설명 |
|------|------|------|
| **v1.x** | 현재 | 현재 네임스페이스 (`container_module`), 완전 지원 |
| **v1.x+1** | 계획 | 구 네임스페이스에 대한 사용 중단 경고 도입 |
| **v2.0** | 계획 | 새 네임스페이스가 기본값이 되고, 구 네임스페이스는 별칭으로 제공 |
| **v3.0** | 미래 | 구 네임스페이스 제거 |

---

## 마이그레이션 단계

### 이전 (v1.x)

```cpp
#include <container/container.h>
#include <container/values/int_value.h>
#include <container/values/string_value.h>

using namespace container_module;

auto container = value_container();
container.add("name", "John");
container.add("age", 30);

auto name = container.get_value<std::string>("name");
```

### 이후 (v2.0)

```cpp
#include <kcenon/container/container.h>
#include <kcenon/container/values/int_value.h>
#include <kcenon/container/values/string_value.h>

using namespace kcenon::container;

auto container = value_container();
container.add("name", "John");
container.add("age", 30);

auto name = container.get_value<std::string>("name");
```

---

## 호환성 레이어 (v2.0)

전환 기간 동안 기존 코드를 깨뜨리지 않고 점진적 마이그레이션을 위한 호환성 별칭이 제공됩니다:

```cpp
// kcenon/container/compat.h 내
namespace container_module = kcenon::container;
```

### 호환성 레이어 사용

즉시 마이그레이션할 수 없는 경우, 호환성 헤더를 포함하세요:

```cpp
#include <kcenon/container/compat.h>

// 기존 코드가 계속 작동합니다
using namespace container_module;  // kcenon::container의 별칭
```

> **참고**: 호환성 레이어는 전환 지원을 위한 것입니다. v3.0 이전에 새 네임스페이스로 업데이트할 계획을 세우세요.

---

## CMake 타겟 변경

### 이전 (v1.x)

```cmake
find_package(container_system CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE kcenon::container_system)
```

### 이후 (v2.0)

```cmake
find_package(container_system CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE kcenon::container)
```

> **참고**: CMake 타겟 이름 변경은 네임스페이스 변경에 맞춰 계획되어 있습니다.

---

## 마이그레이션 체크리스트

코드베이스를 마이그레이션할 때 이 체크리스트를 사용하세요:

- [ ] 인클루드 경로를 `<container/...>`에서 `<kcenon/container/...>`로 업데이트
- [ ] `using namespace container_module`을 `using namespace kcenon::container`로 업데이트
- [ ] 명시적 네임스페이스 참조 업데이트 (예: `container_module::value_container` → `kcenon::container::value_container`)
- [ ] 해당되는 경우 CMake 타겟 업데이트
- [ ] 기능 확인을 위한 테스트 실행
- [ ] 전체 마이그레이션 후 호환성 헤더 인클루드 제거

---

## 자동화된 마이그레이션 (예정)

자동 코드 업데이트를 지원하는 마이그레이션 스크립트가 v2.0과 함께 제공될 예정입니다:

```bash
# 미래 명령어 (아직 사용 불가)
./scripts/migrate-namespace.sh /path/to/your/project
```

---

## FAQ

### Q: v2.0은 언제 출시되나요?

A: v2.0 출시 일정은 아직 발표되지 않았습니다. 저장소를 팔로우하여 업데이트를 확인하세요.

### Q: v2.0이 출시되면 v1.x 코드가 즉시 깨지나요?

A: 아니요. 호환성 레이어가 기존 코드가 계속 작동하도록 보장합니다. 다만 마이그레이션을 권장하는 사용 중단 경고가 표시됩니다.

### Q: 호환성 레이어는 얼마나 오래 지원되나요?

A: 호환성 레이어는 v2.x 전체에서 사용 가능합니다. v3.0에서 제거됩니다.

### Q: v2.0 이전에 새 네임스페이스를 사용할 수 있나요?

A: 아니요. 새 네임스페이스와 인클루드 경로는 v2.0부터만 사용 가능합니다.

---

## 관련 문서

- [일반 마이그레이션 가이드](MIGRATION.md) - messaging_system에서 마이그레이션
- [API 레퍼런스](../API_REFERENCE.kr.md) - 전체 API 문서
- [변경 로그](../CHANGELOG.kr.md) - 버전 이력 및 변경 사항

---

## 지원

마이그레이션 중 문제가 발생하면:

1. [FAQ](FAQ.md)에서 일반적인 문제 확인
2. [기존 이슈](https://github.com/kcenon/container_system/issues) 검색
3. `migration` 라벨로 새 이슈 생성

---

*마지막 업데이트: 2026-02*

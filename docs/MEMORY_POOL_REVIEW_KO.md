# Container System 메모리 풀 검토

> **Language:** [English](MEMORY_POOL_REVIEW.md) | **한국어**

## 범위

- container_system을 위한 할당 패턴을 평가하고 선택적 메모리 풀 전략을 제안합니다.

## 발견사항

- Hot paths: value_container 생성, 중첩된 container_value 생성, string/bytes 저장.
- 할당 패턴: 급증하는 워크로드 하에서 많은 작은, 짧은 수명의 할당.

## 옵션

- A) std::pmr resources (monotonic_buffer_resource, unsynchronized_pool_resource)
  - 장점: 표준, 낮은 통합 비용, 배치 수명에 적합
  - 단점: PMR이 value containers에 연결되어야 함
- B) Fixed-block pool (커스텀)
  - 장점: 예측 가능한 지연 시간; 빈번한 작은 값을 위해 블록 재사용
  - 단점: 단편화 위험; 튜닝 필요; 유지관리할 코드
- C) 컨테이너당 Slab allocator
  - 장점: 우수한 locality; 쉬운 대량 해제
  - 단점: Slab 크기 절충; 최악의 경우 오버헤드

## 권장 전략

- Phase 1: PMR 준비 생성자 및 typedef 도입 (non-breaking)
- Phase 2: 작은 객체를 위한 선택적 fixed-block pool 제공 (64/128/256B 클래스)
- Phase 3: 벤치마크 + 휴리스틱: 워크로드 프로필별 전환

## 통합 지점

- value_container 내부 (node/object allocations)
- container_value (children vector, metadata)
- string_value / bytes_value buffers

## API 스케치

```cpp
// Phase 1 – PMR hooks
namespace container_module {
  struct container_resources {
    std::pmr::memory_resource* general{std::pmr::get_default_resource()};
    std::pmr::memory_resource* small_objects{std::pmr::get_default_resource()};
  };

  class value_container {
  public:
    explicit value_container(container_resources r = {});
    // …
  };
}
```

## 벤치마크 계획

- 다양한 크기로 Construct/Serialize/Deserialize 마이크로벤치마크
- 비교: default, PMR mono, PMR pool, fixed-block pool
- 메트릭: alloc count, bytes, 지연 시간 백분위수, 최대 RSS

## 위험

- 풀을 과도하게 튜닝하면 일반 워크로드가 퇴보할 수 있음
- PMR 전파는 신중한 API 추가가 필요함

## 다음 단계

- PMR hook 추가 (동작 변경 없음)
- 실험을 위한 프로토타입 fixed-block pool header 제공
- 벤치마크 추가 및 보고

---

*Last Updated: 2025-10-20*

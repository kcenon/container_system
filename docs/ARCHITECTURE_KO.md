# 아키텍처 개요

> **Language:** [English](ARCHITECTURE.md) | **한국어**

## 목적
- container_system은 SIMD 가속 연산과 효율적인 직렬화(binary/JSON/XML)를 지원하는 타입 안전한 고성능 value/container 프레임워크입니다.
- messaging/network/database 하위 시스템을 위한 공유 데이터 모델로 제공됩니다.

## 계층

- Core: value, container, value_types, variant storage
- Internal: thread_safe_container, SIMD processors
- Utilities: 문자열 변환, format helpers (하드 의존성을 피하기 위한 로컬 stub)
- Integration: 메시지 조립/직렬화를 위한 messaging_integration

## 통합 토폴로지

```
container_system ──► messaging_system ──► network_system
        │                                   │
        └──────────────► database_system ◄──┘
```

## 데이터 모델

- Primitive values (bool/int/float), bytes, string, nested container.
- 가능한 경우 Zero-copy; 압축 바이너리 인코딩; 선택적 JSON/XML 내보내기.

## 동시성 및 성능

- thread_safe_container에서 예측 가능한 동작을 위한 Lock-free/mutex hybrid.
- SIMD (NEON/AVX) 가속 숫자 연산 (타겟별 선택 사항).

## 설계 원칙

- 타입별 단일 책임; 조립을 위한 builder pattern; 전역 상태 없음.
- 무거운 외부 의존성 방지; 명확한 seam을 가진 로컬 유틸리티 stub 제공.

## 빌드

- C++20, CMake. 선택적 sanitizer/benchmark 대상. 독립 실행 가능.

## 통합 및 빌드 플래그

- network_system과 함께: container adapter를 사용하려면 network_system에서 `BUILD_WITH_CONTAINER_SYSTEM` 활성화.
- Adapters: `container_system_adapter`는 network_system의 `container_manager`에서 (역)직렬화를 위해 사용됩니다.

---

*Last Updated: 2025-10-20*

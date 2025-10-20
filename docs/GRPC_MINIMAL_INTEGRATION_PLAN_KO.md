# Container System gRPC 통합 - 최소 수정 계획

> **Language:** [English](GRPC_MINIMAL_INTEGRATION_PLAN.md) | **한국어**

## 목차

- [요약](#요약)
  - [핵심 원칙: "추가만, 수정 금지"](#핵심-원칙-추가만-수정-금지)
- [1. 아키텍처 개요](#1-아키텍처-개요)
  - [1.1 계층화된 접근 방식](#11-계층화된-접근-방식)
  - [1.2 새로운 디렉토리 구조 (추가만)](#12-새로운-디렉토리-구조-추가만)
- [2. 구현 전략](#2-구현-전략)
  - [2.1 Proto 정의 (독립형)](#21-proto-정의-독립형)
  - [2.2 어댑터 레이어 (읽기 전용 접근)](#22-어댑터-레이어-읽기-전용-접근)
  - [2.3 서비스 구현 (래퍼 패턴)](#23-서비스-구현-래퍼-패턴)
- [3. 빌드 구성 (격리됨)](#3-빌드-구성-격리됨)
  - [3.1 별도 CMake 구성](#31-별도-cmake-구성)
  - [3.2 메인 빌드는 변경되지 않음](#32-메인-빌드는-변경되지-않음)
- [4. 최소 구현 일정](#4-최소-구현-일정)
  - [1주차: 기반 작업 (3일)](#1주차-기반-작업-3일)
  - [2주차: 구현 (4일)](#2주차-구현-4일)
  - [3주차: 테스트 및 문서화 (3일)](#3주차-테스트-및-문서화-3일)
- [5. 사용 예제 (통합 후)](#5-사용-예제-통합-후)
  - [5.1 gRPC 서버 시작](#51-grpc-서버-시작)
  - [5.2 gRPC 클라이언트 사용](#52-grpc-클라이언트-사용)
- [6. 테스트 전략 (비침투적)](#6-테스트-전략-비침투적)
  - [6.1 테스트 격리](#61-테스트-격리)
  - [6.2 통합 테스트](#62-통합-테스트)
- [7. 위험 분석 (최소 접근 방식)](#7-위험-분석-최소-접근-방식)
  - [7.1 위험 평가](#71-위험-평가)
  - [7.2 롤백 계획](#72-롤백-계획)
- [8. 최소 접근 방식의 장점](#8-최소-접근-방식의-장점)
  - [8.1 제로 위험](#81-제로-위험)
  - [8.2 빠른 구현](#82-빠른-구현)
  - [8.3 쉬운 유지보수](#83-쉬운-유지보수)
  - [8.4 점진적 채택](#84-점진적-채택)
- [9. 제한사항 및 향후 개선 사항](#9-제한사항-및-향후-개선-사항)
  - [9.1 현재 제한사항](#91-현재-제한사항)
  - [9.2 향후 개선 사항 (성공 시)](#92-향후-개선-사항-성공-시)
- [10. 의사결정 매트릭스](#10-의사결정-매트릭스)
- [11. 구현 체크리스트](#11-구현-체크리스트)
  - [구현 전](#구현-전)
  - [1주차](#1주차)
  - [2주차](#2주차)
  - [3주차](#3주차)
  - [구현 후](#구현-후)
- [12. 결론](#12-결론)
  - [권장 조치](#권장-조치)

**문서 버전:** 1.0
**날짜:** 2024-01-20
**접근 방식:** Zero Core Modification
**기간:** 3주
**위험 수준:** 낮음

---

## 요약

이 계획은 기존 소스 파일을 수정하지 않고 Container System에 gRPC 기능을 추가하는 **비침투적 접근 방식**을 제시합니다. 통합은 현재 코드베이스와 함께 존재하는 순수 어댑터 패턴을 사용하여 기존 기능에 대한 위험을 제로로 보장합니다.

### 핵심 원칙: "추가만, 수정 금지"
- ✅ 기존 헤더 파일 변경 없음
- ✅ 기존 소스 파일 변경 없음
- ✅ 기존 API 변경 없음
- ✅ 기존 빌드 대상 변경 없음
- ✅ 100% 하위 호환성

---

## 1. 아키텍처 개요

### 1.1 계층화된 접근 방식

```
┌─────────────────────────────────────────┐
│         External gRPC Clients           │
└─────────────────┬───────────────────────┘
                  │
┌─────────────────▼───────────────────────┐
│         NEW: gRPC Service Layer         │  ← 새로운 추가만
│         (grpc/ directory)               │
├─────────────────────────────────────────┤
│         NEW: Protocol Adapter           │  ← 새로운 추가만
│         (grpc/adapters/)                │
├─────────────────────────────────────────┤
│     EXISTING: Container System          │  ← 수정 없음
│     (core/, values/, internal/)         │
└─────────────────────────────────────────┘
```

### 1.2 새로운 디렉토리 구조 (추가만)

```
container_system/
├── [EXISTING DIRECTORIES - UNCHANGED]
│
└── grpc/                           # NEW: 모든 gRPC 코드가 여기에 격리됨
    ├── CMakeLists.txt             # 별도 빌드 구성
    ├── proto/
    │   └── container_service.proto
    ├── adapters/
    │   ├── container_adapter.h    # 어댑터 인터페이스
    │   ├── container_adapter.cpp  # 변환 로직
    │   ├── value_mapper.h         # 타입 매핑
    │   └── value_mapper.cpp
    ├── server/
    │   ├── grpc_server.h
    │   ├── grpc_server.cpp
    │   └── service_impl.cpp
    ├── client/
    │   ├── grpc_client.h
    │   └── grpc_client.cpp
    └── examples/
        ├── server_example.cpp
        └── client_example.cpp
```

---

## 2. 구현 전략

### 2.1 Proto 정의 (독립형)

```protobuf
// grpc/proto/container_service.proto
syntax = "proto3";
package container_grpc;

// 독립적인 메시지 정의 - 코어에 대한 의존성 없음
message GrpcValue {
  string key = 1;
  oneof value_type {
    bool bool_val = 2;
    int32 int_val = 3;
    int64 long_val = 4;
    float float_val = 5;
    double double_val = 6;
    bytes bytes_val = 7;
    string string_val = 8;
    GrpcContainer nested_container = 9;
  }
}

message GrpcContainer {
  string source_id = 1;
  string target_id = 2;
  string message_type = 3;
  repeated GrpcValue values = 4;
}

service ContainerService {
  rpc Process(GrpcContainer) returns (GrpcContainer);
  rpc Stream(stream GrpcContainer) returns (stream GrpcContainer);
}
```

### 2.2 어댑터 레이어 (읽기 전용 접근)

```cpp
// grpc/adapters/container_adapter.h
#pragma once
#include "container/core/container.h"  // 읽기 전용 include
#include "container_service.pb.h"

namespace container_grpc {

// 순수 어댑터 - container_module에 대한 수정 없음
class container_adapter {
public:
    // 기존 container에서 proto로 변환 (읽기 전용)
    static GrpcContainer to_grpc(
        const container_module::value_container& container);

    // proto에서 새 container 생성
    static std::shared_ptr<container_module::value_container>
        from_grpc(const GrpcContainer& grpc);

private:
    // 내부 매핑 함수
    static GrpcValue map_value(
        const container_module::value& val);
    static std::shared_ptr<container_module::value>
        create_value(const GrpcValue& grpc_val);
};

}
```

### 2.3 서비스 구현 (래퍼 패턴)

```cpp
// grpc/server/service_impl.cpp
#include "grpc/adapters/container_adapter.h"

namespace container_grpc {

class container_service_impl final
    : public ContainerService::Service {

    grpc::Status Process(
        grpc::ServerContext* context,
        const GrpcContainer* request,
        GrpcContainer* response) override {

        // gRPC → Container 변환
        auto container = container_adapter::from_grpc(*request);

        // 기존 container 로직을 사용하여 처리
        // (컨테이너 시스템에 대한 수정 없음)
        process_container(container);

        // Container → gRPC 변환
        *response = container_adapter::to_grpc(*container);

        return grpc::Status::OK;
    }

private:
    void process_container(
        std::shared_ptr<container_module::value_container> c) {
        // 기존 public API만 사용
        auto type = c->message_type();
        auto values = c->value_array("");
        // 처리...
    }
};

}
```

---

## 3. 빌드 구성 (격리됨)

### 3.1 별도 CMake 구성

```cmake
# grpc/CMakeLists.txt (새 파일 - 주 빌드에 영향 없음)
cmake_minimum_required(VERSION 3.16)
project(container_grpc)

# 의존성 찾기
find_package(Protobuf REQUIRED)
find_package(gRPC REQUIRED)

# 기존 container 헤더 포함 (읽기 전용)
include_directories(${CMAKE_SOURCE_DIR})

# protobuf/gRPC 코드 생성
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
    proto/container_service.proto)
grpc_generate_cpp(GRPC_SRCS GRPC_HDRS
    proto/container_service.proto)

# gRPC 라이브러리 빌드 (메인 container와 분리)
add_library(container_grpc STATIC
    ${PROTO_SRCS}
    ${GRPC_SRCS}
    adapters/container_adapter.cpp
    adapters/value_mapper.cpp
    server/grpc_server.cpp
    server/service_impl.cpp
    client/grpc_client.cpp
)

# 기존 container 라이브러리와 링크 (수정 없음)
target_link_libraries(container_grpc
    container_system  # 기존 라이브러리
    protobuf::libprotobuf
    gRPC::grpc++
)

# 선택사항: 예제 빌드
add_executable(grpc_server_example examples/server_example.cpp)
target_link_libraries(grpc_server_example container_grpc)

add_executable(grpc_client_example examples/client_example.cpp)
target_link_libraries(grpc_client_example container_grpc)
```

### 3.2 메인 빌드는 변경되지 않음

```cmake
# CMakeLists.txt (기존 - 수정 불필요)
# ... 기존 구성 ...

# 선택사항: gRPC를 선택적 컴포넌트로 추가
option(BUILD_GRPC "Build gRPC support" OFF)
if(BUILD_GRPC)
    add_subdirectory(grpc)  # 추가된 유일한 줄
endif()
```

---

## 4. 최소 구현 일정

### 1주차: 기반 작업 (3일)
| 일차 | 작업 | 생성된 파일 | 수정된 기존 파일 |
|------|------|-------------|------------------|
| Day 1 | Proto 정의 | grpc/proto/container_service.proto | 없음 |
| Day 2 | 빌드 설정 | grpc/CMakeLists.txt | 없음 |
| Day 3 | 어댑터 인터페이스 | grpc/adapters/container_adapter.h | 없음 |

### 2주차: 구현 (4일)
| 일차 | 작업 | 생성된 파일 | 수정된 기존 파일 |
|------|------|-------------|------------------|
| Day 4 | Value mapper | grpc/adapters/value_mapper.cpp | 없음 |
| Day 5 | Container adapter | grpc/adapters/container_adapter.cpp | 없음 |
| Day 6 | 서비스 구현 | grpc/server/service_impl.cpp | 없음 |
| Day 7 | 클라이언트 구현 | grpc/client/grpc_client.cpp | 없음 |

### 3주차: 테스트 및 문서화 (3일)
| 일차 | 작업 | 생성된 파일 | 수정된 기존 파일 |
|------|------|-------------|------------------|
| Day 8 | 예제 | grpc/examples/*.cpp | 없음 |
| Day 9 | 테스트 | grpc/tests/*.cpp | 없음 |
| Day 10 | 문서화 | grpc/README.md | 없음 |

**총계: 10 작업일, 수정된 기존 파일 0개**

---

## 5. 사용 예제 (통합 후)

### 5.1 gRPC 서버 시작

```cpp
// grpc/examples/server_example.cpp
#include "grpc/server/grpc_server.h"

int main() {
    // gRPC 서버 생성 (container 시스템 건드리지 않음)
    container_grpc::grpc_server server("0.0.0.0:50051");

    // 서버는 내부적으로 기존 container 시스템 사용
    server.start();
    server.wait();

    return 0;
}
```

### 5.2 gRPC 클라이언트 사용

```cpp
// grpc/examples/client_example.cpp
#include "grpc/client/grpc_client.h"
#include "container/core/container.h"

int main() {
    // 표준 container 생성 (변경 없음)
    auto container = std::make_shared<container_module::value_container>();
    container->set_message_type("test");

    // gRPC를 통해 전송 (어댑터가 변환 처리)
    container_grpc::grpc_client client("localhost:50051");
    auto response = client.process(container);

    // 응답은 표준 container
    std::cout << response->message_type() << std::endl;

    return 0;
}
```

---

## 6. 테스트 전략 (비침투적)

### 6.1 테스트 격리

```cpp
// grpc/tests/adapter_test.cpp
#include <gtest/gtest.h>
#include "grpc/adapters/container_adapter.h"

TEST(AdapterTest, RoundTripConversion) {
    // 기존 API를 사용하여 container 생성
    auto original = std::make_shared<container_module::value_container>();
    original->set_message_type("test");

    // proto로 변환하고 다시 복원
    auto grpc_msg = container_grpc::container_adapter::to_grpc(*original);
    auto restored = container_grpc::container_adapter::from_grpc(grpc_msg);

    // 기존 API를 사용하여 검증
    EXPECT_EQ(original->message_type(), restored->message_type());
}
```

### 6.2 통합 테스트

```bash
# 기존 테스트 실행 - 모두 통과해야 함 (변경 없음)
./build/bin/container_test

# 새로운 gRPC 테스트를 별도로 실행
./build/grpc/bin/grpc_adapter_test
./build/grpc/bin/grpc_service_test
```

---

## 7. 위험 분석 (최소 접근 방식)

### 7.1 위험 평가

| 위험 | 확률 | 영향 | 완화 방안 |
|------|------|------|----------|
| 기존 코드 손상 | **제로** | N/A | 기존 코드 수정 없음 |
| 성능 오버헤드 | 낮음 | 낮음 | 어댑터 패턴, 필요시 최적화 |
| 유지보수 부담 | 낮음 | 낮음 | 격리된 코드베이스 |
| 버전 충돌 | 낮음 | 낮음 | 별도 네임스페이스 및 빌드 |

### 7.2 롤백 계획

```bash
# 1개 명령으로 완전 롤백
rm -rf grpc/

# 시스템은 이전과 정확히 동일하게 계속 작동
```

---

## 8. 최소 접근 방식의 장점

### 8.1 제로 위험
- ✅ 프로덕션 코드 수정 없음
- ✅ API 변경 없음
- ✅ 빌드 시스템 변경 없음 (선택적 포함)
- ✅ 기존 테스트가 계속 통과

### 8.2 빠른 구현
- 10일 vs 8주
- 단일 개발자 vs 팀
- 기존 개발과의 조정 불필요

### 8.3 쉬운 유지보수
- 완전히 격리된 코드베이스
- 별도로 유지보수 가능
- 흔적 없이 제거 가능

### 8.4 점진적 채택
- 서비스가 선택적으로 gRPC 채택 가능
- 강제 마이그레이션 없음
- 위험 없이 학습 및 반복 가능

---

## 9. 제한사항 및 향후 개선 사항

### 9.1 현재 제한사항
- 추가 직렬화 단계 (어댑터 오버헤드)
- zero-copy 최적화 없음
- container 시스템의 public API로 제한됨

### 9.2 향후 개선 사항 (성공 시)
1. **Phase 2**: 어댑터의 성능 최적화
2. **Phase 3**: 네이티브 protobuf 지원 (정당화되는 경우)
3. **Phase 4**: 더 깊은 통합 (필요한 경우)

---

## 10. 의사결정 매트릭스

| 기준 | 최소 접근 방식 | 완전 통합 |
|------|---------------|----------|
| 구현 시간 | **3주** ✅ | 8주 |
| 위험 수준 | **제로** ✅ | 중간 |
| 코드 변경 | **0개 파일** ✅ | 50개 이상 파일 |
| 성능 | 양호 | 우수 |
| 유지보수 | **격리됨** ✅ | 통합됨 |
| 롤백 난이도 | **간단** ✅ | 복잡 |
| 팀 규모 | **1명 개발자** ✅ | 3-4명 개발자 |
| 비용 | **$15,000** ✅ | $65,000 |

---

## 11. 구현 체크리스트

### 구현 전
- [ ] gRPC/Protobuf 의존성 사용 가능 확인
- [ ] grpc/ 디렉토리 생성
- [ ] 기존 코드 변경 없음 확인

### 1주차
- [ ] proto 정의 생성
- [ ] 격리된 빌드 설정
- [ ] 어댑터 인터페이스 설계

### 2주차
- [ ] 어댑터 구현
- [ ] 서비스 구현
- [ ] 클라이언트 구현

### 3주차
- [ ] 예제 생성
- [ ] 테스트 작성
- [ ] 사용법 문서화

### 구현 후
- [ ] 기존 코드에 대한 변경 제로 검증
- [ ] 모든 기존 테스트 실행 (통과 필수)
- [ ] 성능 벤치마크
- [ ] 배포 가이드

---

## 12. 결론

이 최소 수정 접근 방식은 다음과 같이 gRPC 기능을 제공합니다:
- 기존 시스템에 대한 **제로 위험**
- 현재 코드에 대한 **제로 변경**
- **3주 구현**
- **단일 개발자** 요구사항
- 프로덕션 코드로부터 **완전 격리**

어댑터 패턴은 Container System이 건드려지지 않은 상태로 유지되면서 필요한 클라이언트에게 완전한 gRPC 기능을 제공하도록 보장합니다. 이 접근 방식은 기존 시스템에 대한 어떠한 약속이나 위험 없이 평가 및 점진적 채택을 가능하게 합니다.

### 권장 조치
1. 즉시 구현을 위한 최소 접근 방식 승인
2. 프로덕션 사용 3개월 후 평가
3. 메트릭으로 정당화되는 경우에만 더 깊은 통합 고려

---

**문서 승인**

| 역할 | 이름 | 서명 | 날짜 |
|------|------|------|------|
| Technical Lead | | | |
| Project Manager | | | |
| Risk Manager | | | |

---

**최소 통합 계획 종료**

---

*Last Updated: 2025-10-20*

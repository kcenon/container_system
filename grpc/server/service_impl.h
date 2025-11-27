/*****************************************************************************
BSD 3-Clause License

Copyright (c) 2024, kcenon
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#pragma once

/**
 * @file service_impl.h
 * @brief gRPC service implementation for container system
 */

#include "container_service.grpc.pb.h"
#include "grpc_server.h"

#include <grpcpp/grpcpp.h>
#include <atomic>
#include <mutex>

namespace container_grpc {

/**
 * @class container_service_impl
 * @brief Implementation of ContainerService gRPC service
 *
 * This class implements all RPC methods defined in container_service.proto.
 * It uses the adapter layer to convert between native containers and proto.
 */
class container_service_impl final : public ContainerService::Service {
public:
    /**
     * @brief Default constructor
     */
    container_service_impl();

    /**
     * @brief Set custom container processor
     * @param processor Processing function
     */
    void set_processor(container_processor processor);

    /**
     * @brief Get request statistics
     */
    size_t request_count() const noexcept;
    size_t error_count() const noexcept;

    // =========================================================================
    // Unary RPCs
    // =========================================================================

    /**
     * @brief Send a single container and receive response
     */
    grpc::Status SendContainer(
        grpc::ServerContext* context,
        const SendContainerRequest* request,
        SendContainerResponse* response) override;

    /**
     * @brief Process a container and receive transformed result
     */
    grpc::Status ProcessContainer(
        grpc::ServerContext* context,
        const GrpcContainer* request,
        GrpcContainer* response) override;

    // =========================================================================
    // Streaming RPCs
    // =========================================================================

    /**
     * @brief Server streaming: Subscribe to containers
     */
    grpc::Status StreamContainers(
        grpc::ServerContext* context,
        const SendContainerRequest* request,
        grpc::ServerWriter<GrpcContainer>* writer) override;

    /**
     * @brief Client streaming: Send multiple containers
     */
    grpc::Status CollectContainers(
        grpc::ServerContext* context,
        grpc::ServerReader<GrpcContainer>* reader,
        BatchContainerResponse* response) override;

    /**
     * @brief Bidirectional streaming: Full duplex exchange
     */
    grpc::Status ProcessStream(
        grpc::ServerContext* context,
        grpc::ServerReaderWriter<GrpcContainer, GrpcContainer>* stream) override;

    /**
     * @brief Get stream status
     */
    grpc::Status GetStreamStatus(
        grpc::ServerContext* context,
        const SendContainerRequest* request,
        StreamStatus* response) override;

private:
    container_processor processor_;
    mutable std::mutex processor_mutex_;

    std::atomic<size_t> request_count_{0};
    std::atomic<size_t> error_count_{0};
    std::atomic<int64_t> messages_sent_{0};
    std::atomic<int64_t> messages_received_{0};
};

} // namespace container_grpc

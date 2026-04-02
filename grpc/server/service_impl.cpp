// BSD 3-Clause License
// Copyright (c) 2024, 🍀☀🌕🌥 🌊
// See the LICENSE file in the project root for full license information.

#include "service_impl.h"
#include "../adapters/container_adapter.h"

#include <stdexcept>

namespace kcenon::container_grpc {

container_service_impl::container_service_impl() = default;

void container_service_impl::set_processor(container_processor processor) {
    std::lock_guard<std::mutex> lock(processor_mutex_);
    processor_ = std::move(processor);
}

size_t container_service_impl::request_count() const noexcept {
    return request_count_.load(std::memory_order_relaxed);
}

size_t container_service_impl::error_count() const noexcept {
    return error_count_.load(std::memory_order_relaxed);
}

// =============================================================================
// Unary RPCs
// =============================================================================

grpc::Status container_service_impl::SendContainer(
    grpc::ServerContext* context,
    const SendContainerRequest* request,
    SendContainerResponse* response) {

    request_count_.fetch_add(1, std::memory_order_relaxed);
    messages_received_.fetch_add(1, std::memory_order_relaxed);

    try {
        // Validate request
        if (!request->has_container()) {
            response->set_success(false);
            response->set_error_message("Request missing container");
            error_count_.fetch_add(1, std::memory_order_relaxed);
            return grpc::Status::OK;
        }

        // Convert to native container
        auto native_container = container_adapter::from_grpc(request->container());

        // Process if processor is set
        {
            std::lock_guard<std::mutex> lock(processor_mutex_);
            if (processor_) {
                native_container = processor_(native_container);
            }
        }

        // Convert back to proto
        *response->mutable_result() = container_adapter::to_grpc(*native_container);
        response->set_success(true);
        messages_sent_.fetch_add(1, std::memory_order_relaxed);

        return grpc::Status::OK;

    } catch (const std::exception& e) {
        response->set_success(false);
        response->set_error_message(e.what());
        error_count_.fetch_add(1, std::memory_order_relaxed);
        return grpc::Status::OK;
    }
}

grpc::Status container_service_impl::ProcessContainer(
    grpc::ServerContext* context,
    const GrpcContainer* request,
    GrpcContainer* response) {

    request_count_.fetch_add(1, std::memory_order_relaxed);
    messages_received_.fetch_add(1, std::memory_order_relaxed);

    try {
        // Convert to native container
        auto native_container = container_adapter::from_grpc(*request);

        // Process if processor is set
        {
            std::lock_guard<std::mutex> lock(processor_mutex_);
            if (processor_) {
                native_container = processor_(native_container);
            }
        }

        // Convert back to proto
        *response = container_adapter::to_grpc(*native_container);
        messages_sent_.fetch_add(1, std::memory_order_relaxed);

        return grpc::Status::OK;

    } catch (const std::exception& e) {
        error_count_.fetch_add(1, std::memory_order_relaxed);
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

// =============================================================================
// Streaming RPCs
// =============================================================================

grpc::Status container_service_impl::StreamContainers(
    grpc::ServerContext* context,
    const SendContainerRequest* request,
    grpc::ServerWriter<GrpcContainer>* writer) {

    request_count_.fetch_add(1, std::memory_order_relaxed);
    messages_received_.fetch_add(1, std::memory_order_relaxed);

    try {
        // For now, just echo the container back
        // In a real implementation, this would be connected to a subscription system
        if (request->has_container()) {
            auto native_container = container_adapter::from_grpc(request->container());

            // Process and send
            {
                std::lock_guard<std::mutex> lock(processor_mutex_);
                if (processor_) {
                    native_container = processor_(native_container);
                }
            }

            GrpcContainer response = container_adapter::to_grpc(*native_container);
            writer->Write(response);
            messages_sent_.fetch_add(1, std::memory_order_relaxed);
        }

        return grpc::Status::OK;

    } catch (const std::exception& e) {
        error_count_.fetch_add(1, std::memory_order_relaxed);
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status container_service_impl::CollectContainers(
    grpc::ServerContext* context,
    grpc::ServerReader<GrpcContainer>* reader,
    BatchContainerResponse* response) {

    request_count_.fetch_add(1, std::memory_order_relaxed);

    try {
        GrpcContainer grpc_container;
        int processed_count = 0;
        int failed_count = 0;

        while (reader->Read(&grpc_container)) {
            messages_received_.fetch_add(1, std::memory_order_relaxed);

            try {
                auto native_container = container_adapter::from_grpc(grpc_container);

                // Process if processor is set
                {
                    std::lock_guard<std::mutex> lock(processor_mutex_);
                    if (processor_) {
                        native_container = processor_(native_container);
                    }
                }

                // Add to results
                *response->add_results() = container_adapter::to_grpc(*native_container);
                processed_count++;

            } catch (const std::exception&) {
                failed_count++;
            }
        }

        response->set_success(failed_count == 0);
        response->set_processed_count(processed_count);
        response->set_failed_count(failed_count);

        if (failed_count > 0) {
            response->set_error_message(
                "Failed to process " + std::to_string(failed_count) + " containers");
            error_count_.fetch_add(failed_count, std::memory_order_relaxed);
        }

        return grpc::Status::OK;

    } catch (const std::exception& e) {
        error_count_.fetch_add(1, std::memory_order_relaxed);
        response->set_success(false);
        response->set_error_message(e.what());
        return grpc::Status::OK;
    }
}

grpc::Status container_service_impl::ProcessStream(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<GrpcContainer, GrpcContainer>* stream) {

    request_count_.fetch_add(1, std::memory_order_relaxed);

    try {
        GrpcContainer grpc_container;

        while (stream->Read(&grpc_container)) {
            messages_received_.fetch_add(1, std::memory_order_relaxed);

            try {
                auto native_container = container_adapter::from_grpc(grpc_container);

                // Process if processor is set
                {
                    std::lock_guard<std::mutex> lock(processor_mutex_);
                    if (processor_) {
                        native_container = processor_(native_container);
                    }
                }

                // Send response
                GrpcContainer response = container_adapter::to_grpc(*native_container);
                stream->Write(response);
                messages_sent_.fetch_add(1, std::memory_order_relaxed);

            } catch (const std::exception& e) {
                error_count_.fetch_add(1, std::memory_order_relaxed);
                // Continue processing other messages
            }
        }

        return grpc::Status::OK;

    } catch (const std::exception& e) {
        error_count_.fetch_add(1, std::memory_order_relaxed);
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status container_service_impl::GetStreamStatus(
    grpc::ServerContext* context,
    const SendContainerRequest* request,
    StreamStatus* response) {

    request_count_.fetch_add(1, std::memory_order_relaxed);

    response->set_connected(true);
    response->set_messages_sent(messages_sent_.load(std::memory_order_relaxed));
    response->set_messages_received(messages_received_.load(std::memory_order_relaxed));

    // Extract client ID from metadata if available
    auto metadata = context->client_metadata();
    auto it = metadata.find("client-id");
    if (it != metadata.end()) {
        response->set_client_id(std::string(it->second.begin(), it->second.end()));
    }

    return grpc::Status::OK;
}

} // namespace kcenon::container_grpc

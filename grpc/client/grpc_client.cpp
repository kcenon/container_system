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

#include "grpc_client.h"
#include "../adapters/container_adapter.h"
#include "container_service.grpc.pb.h"

#include <grpcpp/grpcpp.h>

namespace container_grpc {

// =============================================================================
// Implementation Class
// =============================================================================

class grpc_client::impl {
public:
    explicit impl(const client_config& config)
        : config_(config)
        , timeout_(config.timeout) {

        // Create channel
        if (config.use_ssl) {
            channel_ = grpc::CreateChannel(
                config.target_address,
                grpc::SslCredentials(grpc::SslCredentialsOptions()));
        } else {
            channel_ = grpc::CreateChannel(
                config.target_address,
                grpc::InsecureChannelCredentials());
        }

        stub_ = ContainerService::NewStub(channel_);
    }

    bool is_connected() const noexcept {
        if (!channel_) return false;
        auto state = channel_->GetState(false);
        return state == GRPC_CHANNEL_READY ||
               state == GRPC_CHANNEL_IDLE;
    }

    std::string target() const noexcept {
        return config_.target_address;
    }

    void set_timeout(std::chrono::milliseconds timeout) {
        timeout_ = timeout;
    }

    std::chrono::milliseconds timeout() const noexcept {
        return timeout_;
    }

    client_result<std::shared_ptr<container_module::value_container>>
    send(std::shared_ptr<container_module::value_container> container) {
        client_result<std::shared_ptr<container_module::value_container>> result;

        if (!container) {
            result.error_message = "Container is null";
            return result;
        }

        try {
            SendContainerRequest request;
            *request.mutable_container() = container_adapter::to_grpc(*container);

            SendContainerResponse response;
            grpc::ClientContext context;
            set_context_options(context);

            grpc::Status status = stub_->SendContainer(&context, request, &response);

            if (!status.ok()) {
                result.error_message = status.error_message();
                return result;
            }

            if (!response.success()) {
                result.error_message = response.error_message();
                return result;
            }

            result.value = container_adapter::from_grpc(response.result());
            result.success = true;
            return result;

        } catch (const std::exception& e) {
            result.error_message = e.what();
            return result;
        }
    }

    client_result<std::shared_ptr<container_module::value_container>>
    process(std::shared_ptr<container_module::value_container> container) {
        client_result<std::shared_ptr<container_module::value_container>> result;

        if (!container) {
            result.error_message = "Container is null";
            return result;
        }

        try {
            GrpcContainer request = container_adapter::to_grpc(*container);
            GrpcContainer response;
            grpc::ClientContext context;
            set_context_options(context);

            grpc::Status status = stub_->ProcessContainer(&context, request, &response);

            if (!status.ok()) {
                result.error_message = status.error_message();
                return result;
            }

            result.value = container_adapter::from_grpc(response);
            result.success = true;
            return result;

        } catch (const std::exception& e) {
            result.error_message = e.what();
            return result;
        }
    }

    bool stream(
        std::shared_ptr<container_module::value_container> request,
        stream_callback callback) {

        if (!request || !callback) {
            return false;
        }

        try {
            SendContainerRequest grpc_request;
            *grpc_request.mutable_container() = container_adapter::to_grpc(*request);

            grpc::ClientContext context;
            set_context_options(context);

            auto reader = stub_->StreamContainers(&context, grpc_request);

            GrpcContainer grpc_response;
            while (reader->Read(&grpc_response)) {
                auto container = container_adapter::from_grpc(grpc_response);
                callback(container);
            }

            grpc::Status status = reader->Finish();
            return status.ok();

        } catch (const std::exception&) {
            return false;
        }
    }

    client_result<std::vector<std::shared_ptr<container_module::value_container>>>
    send_batch(
        const std::vector<std::shared_ptr<container_module::value_container>>& containers) {

        client_result<std::vector<std::shared_ptr<container_module::value_container>>> result;

        try {
            grpc::ClientContext context;
            set_context_options(context);

            BatchContainerResponse response;
            auto writer = stub_->CollectContainers(&context, &response);

            for (const auto& container : containers) {
                if (container) {
                    GrpcContainer grpc_container = container_adapter::to_grpc(*container);
                    if (!writer->Write(grpc_container)) {
                        break;
                    }
                }
            }

            writer->WritesDone();
            grpc::Status status = writer->Finish();

            if (!status.ok()) {
                result.error_message = status.error_message();
                return result;
            }

            if (!response.success()) {
                result.error_message = response.error_message();
                return result;
            }

            // Convert results
            for (const auto& grpc_result : response.results()) {
                result.value.push_back(container_adapter::from_grpc(grpc_result));
            }

            result.success = true;
            return result;

        } catch (const std::exception& e) {
            result.error_message = e.what();
            return result;
        }
    }

    std::optional<std::pair<int64_t, int64_t>> get_status() {
        try {
            SendContainerRequest request;
            StreamStatus response;
            grpc::ClientContext context;
            set_context_options(context);

            // Add client ID to metadata
            if (!config_.client_id.empty()) {
                context.AddMetadata("client-id", config_.client_id);
            }

            grpc::Status status = stub_->GetStreamStatus(&context, request, &response);

            if (!status.ok()) {
                return std::nullopt;
            }

            return std::make_pair(response.messages_sent(),
                                  response.messages_received());

        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    bool ping() {
        // Use GetStreamStatus as health check
        return get_status().has_value();
    }

private:
    void set_context_options(grpc::ClientContext& context) {
        auto deadline = std::chrono::system_clock::now() + timeout_;
        context.set_deadline(deadline);

        if (!config_.client_id.empty()) {
            context.AddMetadata("client-id", config_.client_id);
        }
    }

    client_config config_;
    std::chrono::milliseconds timeout_;
    std::shared_ptr<grpc::Channel> channel_;
    std::unique_ptr<ContainerService::Stub> stub_;
};

// =============================================================================
// grpc_client Implementation
// =============================================================================

grpc_client::grpc_client(const std::string& target)
    : impl_(std::make_unique<impl>(client_config{target})) {}

grpc_client::grpc_client(const client_config& config)
    : impl_(std::make_unique<impl>(config)) {}

grpc_client::~grpc_client() = default;

grpc_client::grpc_client(grpc_client&&) noexcept = default;
grpc_client& grpc_client::operator=(grpc_client&&) noexcept = default;

bool grpc_client::is_connected() const noexcept {
    return impl_->is_connected();
}

std::string grpc_client::target() const noexcept {
    return impl_->target();
}

void grpc_client::set_timeout(std::chrono::milliseconds timeout) {
    impl_->set_timeout(timeout);
}

std::chrono::milliseconds grpc_client::timeout() const noexcept {
    return impl_->timeout();
}

client_result<std::shared_ptr<container_module::value_container>>
grpc_client::send(std::shared_ptr<container_module::value_container> container) {
    return impl_->send(std::move(container));
}

client_result<std::shared_ptr<container_module::value_container>>
grpc_client::process(std::shared_ptr<container_module::value_container> container) {
    return impl_->process(std::move(container));
}

bool grpc_client::stream(
    std::shared_ptr<container_module::value_container> request,
    stream_callback callback) {
    return impl_->stream(std::move(request), std::move(callback));
}

client_result<std::vector<std::shared_ptr<container_module::value_container>>>
grpc_client::send_batch(
    const std::vector<std::shared_ptr<container_module::value_container>>& containers) {
    return impl_->send_batch(containers);
}

std::optional<std::pair<int64_t, int64_t>> grpc_client::get_status() {
    return impl_->get_status();
}

bool grpc_client::ping() {
    return impl_->ping();
}

} // namespace container_grpc

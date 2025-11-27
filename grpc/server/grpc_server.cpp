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

#include "grpc_server.h"
#include "service_impl.h"

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <chrono>

namespace container_grpc {

// =============================================================================
// Implementation Class
// =============================================================================

class grpc_server::impl {
public:
    explicit impl(const server_config& config)
        : config_(config)
        , running_(false) {}

    ~impl() {
        stop(0);
    }

    bool start() {
        if (running_.load(std::memory_order_acquire)) {
            return false;  // Already running
        }

        grpc::EnableDefaultHealthCheckService(true);

        grpc::ServerBuilder builder;

        // Configure server
        builder.AddListeningPort(config_.address,
                                  grpc::InsecureServerCredentials());
        builder.SetMaxReceiveMessageSize(config_.max_receive_message_size);
        builder.SetMaxSendMessageSize(config_.max_send_message_size);

        // Register service
        builder.RegisterService(&service_);

        // Build and start server
        server_ = builder.BuildAndStart();

        if (!server_) {
            return false;
        }

        running_.store(true, std::memory_order_release);
        return true;
    }

    void stop(int deadline_ms) {
        if (!running_.load(std::memory_order_acquire)) {
            return;
        }

        if (server_) {
            if (deadline_ms > 0) {
                auto deadline = std::chrono::system_clock::now() +
                               std::chrono::milliseconds(deadline_ms);
                server_->Shutdown(deadline);
            } else {
                server_->Shutdown();
            }
            server_.reset();
        }

        running_.store(false, std::memory_order_release);
    }

    void wait() {
        if (server_) {
            server_->Wait();
        }
    }

    bool is_running() const noexcept {
        return running_.load(std::memory_order_acquire);
    }

    std::string address() const noexcept {
        return config_.address;
    }

    void set_processor(container_processor processor) {
        service_.set_processor(std::move(processor));
    }

    size_t request_count() const noexcept {
        return service_.request_count();
    }

    size_t error_count() const noexcept {
        return service_.error_count();
    }

private:
    server_config config_;
    container_service_impl service_;
    std::unique_ptr<grpc::Server> server_;
    std::atomic<bool> running_;
};

// =============================================================================
// grpc_server Implementation
// =============================================================================

grpc_server::grpc_server(const std::string& address)
    : impl_(std::make_unique<impl>(server_config{address})) {}

grpc_server::grpc_server(const server_config& config)
    : impl_(std::make_unique<impl>(config)) {}

grpc_server::~grpc_server() = default;

bool grpc_server::start() {
    return impl_->start();
}

void grpc_server::stop(int deadline_ms) {
    impl_->stop(deadline_ms);
}

void grpc_server::wait() {
    impl_->wait();
}

bool grpc_server::is_running() const noexcept {
    return impl_->is_running();
}

std::string grpc_server::address() const noexcept {
    return impl_->address();
}

void grpc_server::set_processor(container_processor processor) {
    impl_->set_processor(std::move(processor));
}

size_t grpc_server::request_count() const noexcept {
    return impl_->request_count();
}

size_t grpc_server::error_count() const noexcept {
    return impl_->error_count();
}

} // namespace container_grpc

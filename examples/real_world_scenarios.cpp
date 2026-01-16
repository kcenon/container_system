// BSD 3-Clause License
//
// Copyright (c) 2021-2025, 🍀☀🌕🌥 🌊
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "container.h"

using namespace container_module;

/**
 * @brief Real-world scenarios demonstrating practical usage of the container system
 *
 * This example shows how the container system can be used in actual applications:
 * 1. IoT Data Collection System
 * 2. Financial Transaction Processing
 * 3. Gaming Event System
 * 4. Content Management System
 *
 * All examples use the variant-based API (set_value/get_value).
 */

namespace scenarios {

/**
 * @brief IoT Data Collection System
 *
 * Simulates collecting sensor data from multiple IoT devices,
 * aggregating them, and sending to a central processing system.
 */
class IoTDataCollectionSystem {
private:
    struct SensorReading {
        std::string device_id;
        std::string sensor_type;
        double value;
        std::chrono::system_clock::time_point timestamp;
    };

    std::atomic<int> readings_collected_{0};
    std::atomic<int> batches_sent_{0};

public:
    void simulate_iot_scenario() {
        std::cout << "\n=== IoT Data Collection Scenario ===" << std::endl;

        const int num_devices = 5;
        const int readings_per_device = 20;
        const int batch_size = 10;

        std::vector<std::string> device_types = {"temperature", "humidity", "pressure"};
        std::vector<std::thread> device_threads;

        std::queue<SensorReading> sensor_queue;
        std::mutex queue_mutex;
        std::condition_variable queue_cv;
        std::atomic<bool> collection_active{true};

        // Data aggregator thread
        std::thread aggregator_thread([&]() {
            std::vector<SensorReading> batch;
            batch.reserve(batch_size);

            while (collection_active || !sensor_queue.empty()) {
                std::unique_lock<std::mutex> lock(queue_mutex);
                queue_cv.wait(lock, [&]() { return !sensor_queue.empty() || !collection_active; });

                while (!sensor_queue.empty() && batch.size() < static_cast<size_t>(batch_size)) {
                    batch.push_back(sensor_queue.front());
                    sensor_queue.pop();
                }

                if (!batch.empty()) {
                    lock.unlock();
                    send_iot_batch(batch);
                    batch.clear();
                    batches_sent_++;
                }
            }
        });

        // Device simulation threads
        std::random_device rd;
        for (int device_id = 0; device_id < num_devices; ++device_id) {
            device_threads.emplace_back([&, device_id]() {
                std::mt19937 gen(rd());
                std::uniform_real_distribution<> temp_dist(18.0, 35.0);
                std::uniform_real_distribution<> humidity_dist(30.0, 80.0);
                std::uniform_real_distribution<> pressure_dist(990.0, 1030.0);

                for (int reading = 0; reading < readings_per_device; ++reading) {
                    for (const auto& sensor_type : device_types) {
                        SensorReading sensor_reading;
                        sensor_reading.device_id = "device_" + std::to_string(device_id);
                        sensor_reading.sensor_type = sensor_type;
                        sensor_reading.timestamp = std::chrono::system_clock::now();

                        if (sensor_type == "temperature") {
                            sensor_reading.value = temp_dist(gen);
                        } else if (sensor_type == "humidity") {
                            sensor_reading.value = humidity_dist(gen);
                        } else {
                            sensor_reading.value = pressure_dist(gen);
                        }

                        {
                            std::lock_guard<std::mutex> lock(queue_mutex);
                            sensor_queue.push(sensor_reading);
                        }
                        queue_cv.notify_one();
                        readings_collected_++;

                        std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    }
                }
            });
        }

        for (auto& thread : device_threads) {
            thread.join();
        }

        collection_active = false;
        queue_cv.notify_all();
        aggregator_thread.join();

        std::cout << "IoT simulation completed:" << std::endl;
        std::cout << "  Readings collected: " << readings_collected_.load() << std::endl;
        std::cout << "  Batches sent: " << batches_sent_.load() << std::endl;
    }

private:
    void send_iot_batch(const std::vector<SensorReading>& batch) {
        auto container = std::make_shared<value_container>();
        container->set_source("iot_aggregator", "batch_processor");
        container->set_target("iot_analytics_service", "data_processor");
        container->set_message_type("sensor_data_batch");

        container->set("batch_size", static_cast<int32_t>(batch.size()));
        container->set("batch_timestamp", static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));

        for (size_t i = 0; i < batch.size(); ++i) {
            const auto& reading = batch[i];
            std::string prefix = "reading_" + std::to_string(i) + "_";

            container->set(prefix + "device_id", reading.device_id);
            container->set(prefix + "sensor_type", reading.sensor_type);
            container->set(prefix + "value", reading.value);
            container->set(prefix + "timestamp", static_cast<int64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    reading.timestamp.time_since_epoch()).count()));
        }

        std::string serialized = container->serialize_string(value_container::serialization_format::binary).value();
        std::cout << "  Sent IoT batch: " << batch.size() << " readings, "
                  << serialized.size() << " bytes" << std::endl;
    }
};

/**
 * @brief Financial Transaction Processing System
 */
class FinancialTransactionSystem {
private:
    struct Transaction {
        std::string transaction_id;
        std::string account_from;
        std::string account_to;
        double amount;
        std::string currency;
        std::string transaction_type;
        std::chrono::system_clock::time_point timestamp;
    };

    std::atomic<int> transactions_processed_{0};
    std::atomic<int> fraud_alerts_{0};
    double total_amount_{0.0};
    std::mutex amount_mutex_;

public:
    void simulate_financial_scenario() {
        std::cout << "\n=== Financial Transaction Processing Scenario ===" << std::endl;

        const int num_transactions = 100;

        std::queue<Transaction> transaction_queue;
        std::mutex transaction_mutex;
        std::condition_variable transaction_cv;
        std::atomic<bool> processing_active{true};

        std::thread fraud_detector([&]() {
            while (processing_active || !transaction_queue.empty()) {
                std::unique_lock<std::mutex> lock(transaction_mutex);
                transaction_cv.wait(lock, [&]() { return !transaction_queue.empty() || !processing_active; });

                if (!transaction_queue.empty()) {
                    Transaction transaction = transaction_queue.front();
                    transaction_queue.pop();
                    lock.unlock();

                    process_transaction(transaction);
                    transactions_processed_++;
                }
            }
        });

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> amount_dist(10.0, 10000.0);
        std::uniform_int_distribution<> account_dist(1000, 9999);
        std::vector<std::string> currencies = {"USD", "EUR", "GBP"};
        std::vector<std::string> types = {"transfer", "payment", "withdrawal"};

        for (int i = 0; i < num_transactions; ++i) {
            Transaction transaction;
            transaction.transaction_id = "TXN" + std::to_string(1000000 + i);
            transaction.account_from = "ACC" + std::to_string(account_dist(gen));
            transaction.account_to = "ACC" + std::to_string(account_dist(gen));
            transaction.amount = amount_dist(gen);
            transaction.currency = currencies[i % currencies.size()];
            transaction.transaction_type = types[i % types.size()];
            transaction.timestamp = std::chrono::system_clock::now();

            {
                std::lock_guard<std::mutex> lock(transaction_mutex);
                transaction_queue.push(transaction);
            }
            transaction_cv.notify_one();

            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        processing_active = false;
        transaction_cv.notify_all();
        fraud_detector.join();

        std::cout << "Financial processing completed:" << std::endl;
        std::cout << "  Transactions processed: " << transactions_processed_.load() << std::endl;
        std::cout << "  Fraud alerts generated: " << fraud_alerts_.load() << std::endl;
        double total;
        {
            std::lock_guard<std::mutex> lock(amount_mutex_);
            total = total_amount_;
        }
        std::cout << "  Total amount processed: $" << std::fixed << std::setprecision(2) << total << std::endl;
    }

private:
    void process_transaction(const Transaction& transaction) {
        bool is_suspicious = (transaction.amount > 5000.0) ||
                           (transaction.account_from == transaction.account_to);

        {
            std::lock_guard<std::mutex> lock(amount_mutex_);
            total_amount_ += transaction.amount;
        }

        auto container = std::make_shared<value_container>();
        container->set_source("transaction_processor", "fraud_detection");
        container->set_target("compliance_service", "transaction_monitor");
        container->set_message_type(is_suspicious ? "suspicious_transaction" : "normal_transaction");

        container->set("transaction_id", transaction.transaction_id);
        container->set("account_from", transaction.account_from);
        container->set("account_to", transaction.account_to);
        container->set("amount", transaction.amount);
        container->set("currency", transaction.currency);
        container->set("transaction_type", transaction.transaction_type);
        container->set("timestamp", static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                transaction.timestamp.time_since_epoch()).count()));
        container->set("risk_score", is_suspicious ? 85.0 : 15.0);

        if (is_suspicious) {
            fraud_alerts_++;
            container->set("alert_reason",
                std::string(transaction.amount > 5000.0 ? "high_amount" : "same_account"));
            container->set("requires_manual_review", true);

            std::cout << "  FRAUD ALERT: " << transaction.transaction_id
                      << " Amount: $" << transaction.amount << std::endl;
        }

        container->serialize_string(value_container::serialization_format::binary).value();
    }
};

/**
 * @brief Gaming Event System
 */
class GamingEventSystem {
private:
    struct GameEvent {
        std::string player_id;
        std::string event_type;
        std::map<std::string, std::string> event_data;
        std::chrono::system_clock::time_point timestamp;
    };

    std::atomic<int> events_processed_{0};
    std::atomic<int> achievements_unlocked_{0};
    std::map<std::string, int> player_scores_;
    std::mutex scores_mutex_;

public:
    void simulate_gaming_scenario() {
        std::cout << "\n=== Gaming Event System Scenario ===" << std::endl;

        const int num_players = 10;
        const int events_per_player = 50;
        std::vector<std::thread> player_threads;

        std::queue<GameEvent> event_queue;
        std::mutex event_mutex;
        std::condition_variable event_cv;
        std::atomic<bool> game_active{true};

        std::thread event_processor([&]() {
            while (game_active || !event_queue.empty()) {
                std::unique_lock<std::mutex> lock(event_mutex);
                event_cv.wait(lock, [&]() { return !event_queue.empty() || !game_active; });

                if (!event_queue.empty()) {
                    GameEvent event = event_queue.front();
                    event_queue.pop();
                    lock.unlock();

                    process_game_event(event);
                    events_processed_++;
                }
            }
        });

        std::random_device rd;
        for (int player_id = 0; player_id < num_players; ++player_id) {
            player_threads.emplace_back([&, player_id]() {
                std::mt19937 gen(rd());
                std::uniform_int_distribution<> action_dist(0, 4);
                std::uniform_int_distribution<> score_dist(10, 500);
                std::uniform_int_distribution<> level_dist(1, 50);

                std::vector<std::string> actions = {"kill", "death", "level_up", "item_collected", "quest_completed"};

                for (int event_count = 0; event_count < events_per_player; ++event_count) {
                    GameEvent event;
                    event.player_id = "player_" + std::to_string(player_id);
                    event.event_type = actions[action_dist(gen)];
                    event.timestamp = std::chrono::system_clock::now();

                    if (event.event_type == "kill") {
                        event.event_data["target"] = "player_" + std::to_string((player_id + 1) % num_players);
                        event.event_data["score"] = std::to_string(score_dist(gen));
                    } else if (event.event_type == "level_up") {
                        event.event_data["new_level"] = std::to_string(level_dist(gen));
                    }

                    {
                        std::lock_guard<std::mutex> lock(event_mutex);
                        event_queue.push(event);
                    }
                    event_cv.notify_one();

                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }
            });
        }

        for (auto& thread : player_threads) {
            thread.join();
        }

        game_active = false;
        event_cv.notify_all();
        event_processor.join();

        print_leaderboard();

        std::cout << "Gaming simulation completed:" << std::endl;
        std::cout << "  Events processed: " << events_processed_.load() << std::endl;
        std::cout << "  Achievements unlocked: " << achievements_unlocked_.load() << std::endl;
    }

private:
    void process_game_event(const GameEvent& event) {
        auto container = std::make_shared<value_container>();
        container->set_source("game_client", event.player_id);
        container->set_target("game_server", "event_processor");
        container->set_message_type("game_event");

        container->set("player_id", event.player_id);
        container->set("event_type", event.event_type);
        container->set("timestamp", static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                event.timestamp.time_since_epoch()).count()));

        for (const auto& data_pair : event.event_data) {
            container->set(data_pair.first, data_pair.second);
        }

        if (event.event_data.find("score") != event.event_data.end()) {
            int score = std::stoi(event.event_data.at("score"));
            {
                std::lock_guard<std::mutex> lock(scores_mutex_);
                player_scores_[event.player_id] += score;
            }
        }

        if (event.event_type == "level_up" && event.event_data.find("new_level") != event.event_data.end()) {
            int level = std::stoi(event.event_data.at("new_level"));
            if (level >= 25) {
                achievements_unlocked_++;
                send_achievement_notification(event.player_id, "High Level Achiever");
            }
        }

        container->serialize_string(value_container::serialization_format::binary).value();
    }

    void send_achievement_notification(const std::string& player_id, const std::string& achievement) {
        auto notification = std::make_shared<value_container>();
        notification->set_source("achievement_system", "unlock_processor");
        notification->set_target("notification_service", "player_notifier");
        notification->set_message_type("achievement_unlocked");

        notification->set("player_id", player_id);
        notification->set("achievement_name", achievement);
        notification->set("timestamp", static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));

        std::cout << "  ACHIEVEMENT: " << player_id << " unlocked '" << achievement << "'" << std::endl;
    }

    void print_leaderboard() {
        std::cout << "\n  === Leaderboard ===" << std::endl;

        std::vector<std::pair<std::string, int>> leaderboard;
        {
            std::lock_guard<std::mutex> lock(scores_mutex_);
            for (const auto& entry : player_scores_) {
                leaderboard.emplace_back(entry.first, entry.second);
            }
        }

        std::sort(leaderboard.begin(), leaderboard.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });

        for (size_t i = 0; i < std::min(size_t(5), leaderboard.size()); ++i) {
            std::cout << "  " << (i + 1) << ". " << leaderboard[i].first
                      << ": " << leaderboard[i].second << " points" << std::endl;
        }
        std::cout << "  ===================" << std::endl;
    }
};

/**
 * @brief Content Management System
 */
class ContentManagementSystem {
private:
    struct Document {
        std::string document_id;
        std::string title;
        std::string content;
        std::string author;
        std::string category;
        std::vector<std::string> tags;
        std::chrono::system_clock::time_point upload_time;
    };

    std::atomic<int> documents_processed_{0};
    std::atomic<int> documents_indexed_{0};

public:
    void simulate_cms_scenario() {
        std::cout << "\n=== Content Management System Scenario ===" << std::endl;

        const int num_documents = 50;

        std::queue<Document> document_queue;
        std::mutex document_mutex;
        std::condition_variable document_cv;
        std::atomic<bool> uploading_active{true};

        std::thread document_processor([&]() {
            while (uploading_active || !document_queue.empty()) {
                std::unique_lock<std::mutex> lock(document_mutex);
                document_cv.wait(lock, [&]() { return !document_queue.empty() || !uploading_active; });

                if (!document_queue.empty()) {
                    Document document = document_queue.front();
                    document_queue.pop();
                    lock.unlock();

                    process_document(document);
                    documents_processed_++;
                }
            }
        });

        std::vector<std::string> categories = {"article", "report", "manual"};
        std::vector<std::string> authors = {"john_smith", "jane_doe", "bob_wilson"};
        std::vector<std::vector<std::string>> tag_sets = {
            {"programming", "cpp", "tutorial"},
            {"business", "report", "analysis"},
            {"technical", "manual", "guide"}
        };

        for (int i = 0; i < num_documents; ++i) {
            Document document;
            document.document_id = "DOC" + std::to_string(10000 + i);
            document.title = "Document Title " + std::to_string(i);
            document.content = generate_sample_content(i);
            document.author = authors[i % authors.size()];
            document.category = categories[i % categories.size()];
            document.tags = tag_sets[i % tag_sets.size()];
            document.upload_time = std::chrono::system_clock::now();

            {
                std::lock_guard<std::mutex> lock(document_mutex);
                document_queue.push(document);
            }
            document_cv.notify_one();

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        uploading_active = false;
        document_cv.notify_all();
        document_processor.join();

        std::cout << "CMS simulation completed:" << std::endl;
        std::cout << "  Documents processed: " << documents_processed_.load() << std::endl;
        std::cout << "  Documents indexed: " << documents_indexed_.load() << std::endl;
    }

private:
    std::string generate_sample_content(int index) {
        std::stringstream content;
        content << "This is sample content for document " << index << ". ";
        content << "It contains important information for demonstration purposes.";
        return content.str();
    }

    void process_document(const Document& document) {
        auto container = std::make_shared<value_container>();
        container->set_source("cms_upload_service", "document_processor");
        container->set_target("search_indexer", "text_analyzer");
        container->set_message_type("document_processing");

        container->set("document_id", document.document_id);
        container->set("title", document.title);
        container->set("author", document.author);
        container->set("category", document.category);
        container->set("content_length", static_cast<int32_t>(document.content.length()));
        container->set("upload_timestamp", static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                document.upload_time.time_since_epoch()).count()));
        container->set("tag_count", static_cast<int32_t>(document.tags.size()));
        container->set("content", document.content);

        for (size_t i = 0; i < document.tags.size(); ++i) {
            container->set("tag_" + std::to_string(i), document.tags[i]);
        }

        container->serialize_string(value_container::serialization_format::binary).value();
        create_search_index_entry(document);
        documents_indexed_++;

        if (documents_processed_ % 20 == 0) {
            std::cout << "  Processed " << documents_processed_.load() << " documents..." << std::endl;
        }
    }

    void create_search_index_entry(const Document& document) {
        auto index_container = std::make_shared<value_container>();
        index_container->set_source("text_analyzer", "indexing_service");
        index_container->set_target("search_service", "index_updater");
        index_container->set_message_type("search_index_update");

        index_container->set("document_id", document.document_id);
        index_container->set("indexed_title", document.title);
        index_container->set("indexed_category", document.category);
        index_container->set("word_count", static_cast<int32_t>(count_words(document.content)));
        index_container->set("index_timestamp", static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()));

        index_container->serialize_string(value_container::serialization_format::binary).value();
    }

    size_t count_words(const std::string& text) {
        std::istringstream iss(text);
        std::string word;
        size_t count = 0;
        while (iss >> word) {
            count++;
        }
        return count;
    }
};

} // namespace scenarios

int main() {
    try {
        std::cout << "=== Real-World Container System Scenarios ===" << std::endl;
        std::cout << "Demonstrating practical applications using variant-based API" << std::endl;

        scenarios::IoTDataCollectionSystem iot_system;
        iot_system.simulate_iot_scenario();

        scenarios::FinancialTransactionSystem financial_system;
        financial_system.simulate_financial_scenario();

        scenarios::GamingEventSystem gaming_system;
        gaming_system.simulate_gaming_scenario();

        scenarios::ContentManagementSystem cms_system;
        cms_system.simulate_cms_scenario();

        std::cout << "\n=== All Real-World Scenarios Completed Successfully ===" << std::endl;
        std::cout << "The container system demonstrated versatility across:" << std::endl;
        std::cout << "  - IoT data aggregation and processing" << std::endl;
        std::cout << "  - Financial transaction processing with fraud detection" << std::endl;
        std::cout << "  - Gaming event systems with real-time processing" << std::endl;
        std::cout << "  - Content management with search indexing" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Fatal error in real-world scenarios: " << e.what() << std::endl;
        return 1;
    }
}

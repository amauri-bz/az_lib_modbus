#pragma once

#include <asio.hpp>
#include <thread>
#include <optional>
#include <future>

namespace modbus {

using asio::ip::tcp;

class ModbusContext {
private:
    asio::io_context io_context_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;
    std::thread io_thread_;

public:
    ModbusContext() {
        // Keep the io_context running.
        work_guard_.emplace(io_context_.get_executor());

        // Starts the background thread to run the event loop.
        io_thread_ = std::thread([this] {
            io_context_.run();
        });
    }

    ~ModbusContext() {
        // Release the work_guard, allowing io_context::run() to finish.
        work_guard_.reset();

        // Wait for the I/O thread.
        if (io_thread_.joinable()) {
            io_thread_.join();
        }
    }

    asio::io_context::executor_type get_executor() {
        return io_context_.get_executor();
    }

    asio::io_context& get_io_context() {
        return io_context_;
    }
};

} // namespace modbus
#pragma once

#include "az_modbus_context.hpp"
#include "az_modbus_transport_awaitable.hpp"
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>

namespace modbus {

using asio::awaitable;
using asio::co_spawn;
using asio::use_awaitable;

class AsioChannel : public IModbusChannel {
private:
    tcp::socket socket_;
    asio::any_io_executor executor_;

    // Internal curtain for connection.
    awaitable<void> do_connect(const std::string& host, const std::string& port) {
        tcp::resolver resolver(executor_);
        auto endpoints = co_await resolver.async_resolve(host, port, use_awaitable);
        co_await asio::async_connect(socket_, endpoints, use_awaitable);
    }

    // Internal writing curtain
    awaitable<size_t> do_write(const std::vector<std::uint8_t>& data) {
        co_return co_await asio::async_write(socket_, asio::buffer(data), use_awaitable);
    }

    // Internal curtain for reading
    awaitable<std::vector<std::uint8_t>> do_read(size_t bytes_to_read) {
        std::vector<std::uint8_t> buffer(bytes_to_read);
        co_await asio::async_read(socket_, asio::buffer(buffer), use_awaitable);
        co_return buffer;
    }

public:
    AsioChannel(ModbusContext& context)
        : socket_(context.get_executor()), executor_(context.get_executor()) {}

    AsioChannel(tcp::socket socket)
        : socket_(std::move(socket)), executor_(socket_.get_executor()) {}

    std::future<void> connect(const std::string& host, const std::string& port) override {
        return co_spawn(executor_, do_connect(host, port), asio::use_future);
    }

    std::future<size_t> write(const std::vector<std::uint8_t>& data) override {
        return co_spawn(executor_, do_write(data), asio::use_future);
    }

    std::future<std::vector<std::uint8_t>> read(size_t bytes_to_read) override {
        return co_spawn(executor_, do_read(bytes_to_read), asio::use_future);
    }

    void close() override {
        if (socket_.is_open()) {
            asio::error_code ec;
            socket_.shutdown(tcp::socket::shutdown_both, ec);
            socket_.close(ec);
        }
    }

    awaitable<std::vector<std::uint8_t>> co_read(size_t bytes_to_read) override {
        std::vector<std::uint8_t> buffer(bytes_to_read);
        co_await asio::async_read(socket_, asio::buffer(buffer), asio::use_awaitable);
        co_return buffer;
    }

    awaitable<size_t> co_write(const std::vector<std::uint8_t>& data) override {
        co_return co_await asio::async_write(socket_, asio::buffer(data), asio::use_awaitable);
    }

    asio::any_io_executor get_executor() override {
        return socket_.get_executor();
    }
};

} // namespace modbus
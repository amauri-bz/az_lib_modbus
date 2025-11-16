#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <future>
#include <asio.hpp>

namespace modbus {

using asio::awaitable;
using asio::ip::tcp;

class IModbusChannel {
public:
    virtual ~IModbusChannel() = default;

    virtual std::future<void> connect(const std::string& host, const std::string& port) = 0;

    virtual std::future<size_t> write(const std::vector<std::uint8_t>& data) = 0;

    virtual std::future<std::vector<std::uint8_t>> read(size_t bytes_to_read) = 0;

    virtual awaitable<std::vector<std::uint8_t>> co_read(size_t bytes_to_read) = 0;

    virtual awaitable<size_t> co_write(const std::vector<std::uint8_t>& data) = 0;

    virtual asio::any_io_executor get_executor() = 0;

    virtual void close() = 0;
};

} // namespace modbus
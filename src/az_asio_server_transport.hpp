#pragma once
#include "az_modbus_context.hpp"
#include "az_modbus_server_transport_awaitable.hpp"
#include "az_asio_channel.hpp"
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>

namespace modbus {

class AsioServerTransport : public IServerTransport {
private:
    tcp::acceptor acceptor_;
    asio::io_context::executor_type executor_;

    // Main curtain for accepting connections
    awaitable<void> do_accept(NewConnectionHandler handler) {
        acceptor_.listen();
        while (true) {
            tcp::socket new_socket = co_await acceptor_.async_accept(use_awaitable);
            handler(std::make_unique<AsioChannel>(std::move(new_socket)));
        }
    }

public:
    AsioServerTransport(ModbusContext& context)
        : executor_(context.get_executor()),
          acceptor_(context.get_io_context()) {}

    std::future<void> start_accepting(const std::string& ipv4, const std::string& port, NewConnectionHandler handler) override {
        tcp::resolver resolver(executor_);
        auto endpoints = resolver.resolve(ipv4, port);
        asio::ip::tcp::endpoint endpoint = *endpoints.begin();
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);

        // It starts the acceptance coroutine, using 'detached' because the loop runs indefinitely.
        return co_spawn(executor_, do_accept(handler), asio::use_future);
    }
};

} // namespace modbus
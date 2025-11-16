#pragma once
#include <future>
#include <functional>
#include <vector>
#include <memory>

#include "az_modbus_transport_awaitable.hpp"

namespace modbus {

class IServerTransport {
public:
    virtual ~IServerTransport() = default;

    // Type of function for handling new connections: Receives the newly created channel (socket).
    using NewConnectionHandler = std::function<void(std::unique_ptr<IModbusChannel>)>;

    // It starts listening on a port and begins accepting connections.
    virtual std::future<void> start_accepting(
        const std::string& ipv4,
        const std::string& port,
        NewConnectionHandler handler
    ) = 0;
};

} // namespace modbus
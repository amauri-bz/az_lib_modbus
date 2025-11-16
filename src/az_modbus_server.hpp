#pragma once

#include "az_modbus_server_transport_awaitable.hpp"
#include "az_modbus_protocol.hpp"
#include "az_database_interface.hpp"
#include "az_helper.hpp"
#include <stdexcept>
#include <memory>

namespace modbus {

class ModbusServer {
private:
    std::unique_ptr<IServerTransport> transport_;
    std::unique_ptr<db::DatabaseInterface> database_;
    modbus::UnitID unit_id_{0};

    asio::awaitable<void> do_modbus_loop(std::unique_ptr<IModbusChannel> channel) {
        try {
            while (true) {
                std::vector<std::uint8_t> adu_buffer;

                std::cout << "[SERVER] Waiting for connection" << std::endl;

                auto header_buffer = co_await channel->co_read(MBAP_HEADER_SIZE);

                helper::print_hex_buffer(header_buffer, "<<<< header: ");

                auto header = modbus::decode_header(header_buffer);
                if (header.unit_id != unit_id_.value) throw std::runtime_error("invalid UNIT_ID");

                size_t pdu_size = header.length - 1;
                auto pdu_data = co_await channel->co_read(pdu_size);

                helper::print_hex_buffer(pdu_data, "<<<< pdu_data: ");

                auto data = decode_request(pdu_data);

                if (std::holds_alternative<modbus::RequestData>(data)) {

                    auto request = std::get<modbus::RequestData>(data);

                    std::cout
                    << "PDU FC=0x"
                    << std::hex
                    << static_cast<int>(request.func_code)
                    << std::dec
                    << " Start Addr:"
                    << request.start_addr
                    << " Number:"
                    << request.number
                    << " Value:"
                    << request.value
                    << "\n";

                    std::vector<std::uint8_t> dados_bit;
                    std::vector<std::uint16_t> dados_reg;

                    auto start = request.start_addr - 1;
                    auto stop = start + request.number;
                    switch (request.func_code) {
                        case ReadCoils:
                            for(std::uint16_t id=start; id < stop; id++) {
                                auto ret = std::get<uint8_t>(database_->db_read(db::DbType::BITS, id));
                                dados_bit.push_back(ret);
                            }
                            adu_buffer = modbus::handle_read_bits(header, request, dados_bit);
                            break;
                        case ReadDiscreteInputs:
                            for(std::uint16_t id=start; id < stop; id++) {
                                database_->db_update(db::DbType::BITS_INPUT, id, static_cast<std::uint8_t>(id%2 ? 1: 0)); //input simulation
                                dados_bit.push_back(std::get<uint8_t>(database_->db_read(db::DbType::BITS_INPUT, id)));
                            }
                            adu_buffer = modbus::handle_read_bits(header, request, dados_bit);
                            break;
                        case HoldingRegisters:
                            for(std::uint16_t id=start; id < stop; id++) {
                                dados_reg.push_back(std::get<uint16_t>(database_->db_read(db::DbType::REGISTER, id)));
                            }
                            adu_buffer = modbus::handle_read_registers(header, request, dados_reg);
                            break;
                        case InputRegisters:
                            for(std::uint16_t id=start; id < stop; id++) {
                                database_->db_update(db::DbType::REGISTER_INPUT, id, static_cast<std::uint16_t>(id%2 ? 1: 0)); //input simulation
                                dados_reg.push_back(std::get<uint16_t>(database_->db_read(db::DbType::REGISTER_INPUT, id)));
                            }
                            adu_buffer = modbus::handle_read_registers(header, request, dados_reg);
                            break;
                        case WriteSingleCoil:
                            database_->db_update(db::DbType::BITS, start, static_cast<std::uint8_t>(request.value));
                            adu_buffer = modbus::create_write_adu(header.transaction_id, header.unit_id, request.start_addr, request.value, FunctionCode::WriteSingleCoil);
                            break;
                        case WriteSingleRegister:
                            database_->db_update(db::DbType::REGISTER, start, static_cast<std::uint16_t>(request.value));
                            adu_buffer = modbus::create_write_adu(header.transaction_id, header.unit_id, request.start_addr, request.value, FunctionCode::WriteSingleRegister);
                            break;
                        default:
                            throw std::runtime_error("Modbus Exception: " + std::to_string(request.func_code));
                            break;
                    }
                }
                else {
                    auto exception = std::get<modbus::Exceptiondata>(data);
                    std::cerr << "[SERVER] Exception response: [" << static_cast<int>(exception.code) << "] " << exception.name << std::endl;
                    adu_buffer = modbus::create_modbus_exception_adu(header, exception);
                }
                helper::print_hex_buffer(adu_buffer, ">>>> adu_buffer: ");
                co_await channel->co_write(adu_buffer);
            }
        } catch (const std::exception& e) {
            std::cerr << "[SERVER] Connection closed, error: " << e.what() << std::endl;
        }
    }

    void handle_new_connection(std::unique_ptr<IModbusChannel> channel) {
        std::cout << "[SERVER] New connection accepted. Starting the request loop." << std::endl;

        if (!channel) {
            std::cerr << "FATAL ERROR : Null channel. Accept failed" << std::endl;
            return;
        }
        auto ex = channel->get_executor();
        auto loop = do_modbus_loop(std::move(channel));
        asio::co_spawn(ex, std::move(loop), asio::detached);
    }

public:
    ModbusServer(std::unique_ptr<IServerTransport> transport,
        std::unique_ptr<db::DatabaseInterface> database,
        modbus::UnitID unit_id)
        : transport_(std::move(transport)),
        database_(std::move(database)),
        unit_id_(unit_id) {}

    void start(const modbus::Ipv4& ipv4, const modbus::Port& port) {
        transport_->start_accepting(ipv4.value, port.value, [this](std::unique_ptr<IModbusChannel> channel) {
            this->handle_new_connection(std::move(channel));
        });
        std::cout << "[SERVER] Modbus Server listening on port " << port.value << std::endl;
    }
};
} // namespace modbus
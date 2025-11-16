#pragma once

#include "az_modbus_transport_awaitable.hpp"
#include "az_modbus_protocol.hpp"
#include "az_helper.hpp"
#include <stdexcept>
#include <memory>

namespace modbus {

class ModbusClient {
private:
    std::unique_ptr<IModbusChannel> transport_;
    std::uint16_t next_tid_ = 0;

    std::variant<std::vector<uint8_t>, std::vector<uint16_t>> read_data(
        std::uint8_t unit_id,
        std::uint16_t start_address,
        std::uint16_t quantity,
        modbus::FunctionCode function_code) {

        std::uint16_t tid = next_tid_++;
        auto request = modbus::create_read_adu(tid, unit_id, start_address, quantity, function_code);

        helper::print_hex_buffer(request, ">>>> request: ");

        // Send the request message - Blocking operation
        transport_->write(request).get();

        auto header_buffer = transport_->read(MBAP_HEADER_SIZE).get();
        auto header = modbus::decode_header(header_buffer);

        if (header.transaction_id != tid) {
            throw std::runtime_error("TID de resposta incorreto.");
        }
        helper::print_hex_buffer(header_buffer, "<<<< header_buffer: ");

        std::cout
            << "unit_id:"
            << static_cast<int>(header.unit_id)
            << " transaction_id:"
            << header.transaction_id
            << " protocol_id:"
            << header.protocol_id
            << " length:"
            << header.length
            << "\n";

        size_t pdu_size = header.length - 1;
        auto pdu_data = transport_->read(pdu_size).get();

        modbus::check_exception(pdu_data);

        helper::print_hex_buffer(pdu_data, "<<<< pdu_data: ");

        switch (function_code) {
            case modbus::FunctionCode::ReadCoils:
            case modbus::FunctionCode::ReadDiscreteInputs:
            {
                auto data_response = modbus::decode_read_coils_response(pdu_data, quantity);
                return data_response;
            }
            case modbus::FunctionCode::HoldingRegisters:
            case modbus::FunctionCode::InputRegisters:
            {
                auto data_response = modbus::decode_read_register_response(pdu_data, quantity);
                return data_response;
            }
            default:
                throw std::runtime_error("Function Code " + std::to_string(function_code) + " nnot supported");
        }
    }

    void write_data(std::uint8_t unit_id, std::uint16_t address, std::uint16_t value, modbus::FunctionCode function_code)
    {
        // Build the message
        std::uint16_t tid = next_tid_++;
        auto request = modbus::create_write_adu(tid, unit_id, address, value, function_code);

        helper::print_hex_buffer(request, ">>>> request: ");

        // Send the request message - Blocking operation
        transport_->write(request).get();

        // Header processing - Blocking operation
        auto header_buffer = transport_->read(MBAP_HEADER_SIZE).get();

        helper::print_hex_buffer(header_buffer, "<<<< header_buffer: ");

        auto header = modbus::decode_header(header_buffer);

        // PDU processing - Blocking operation
        size_t pdu_size = header.length - 1;
        auto pdu_data = transport_->read(pdu_size).get();

        helper::print_hex_buffer(pdu_data, "<<<< pdu_data: ");

        auto data = decode_request(pdu_data);

        if (std::holds_alternative<modbus::RequestData>(data)) {

            auto response = std::get<modbus::RequestData>(data);

            std::cout
            << "PDU FC=0x"
            << std::hex
            << static_cast<int>(response.func_code)
            << std::dec
            << " Start Addr:"
            << response.start_addr
            << " Value:"
            << response.value
            << "\n";

            modbus::check_exception(pdu_data);

            if(header.transaction_id != tid) throw std::runtime_error("Invalid TID");
            if(header.unit_id != unit_id) throw std::runtime_error("Invalid UNIT_ID");
            if(response.start_addr != address) throw std::runtime_error("Invalid START_ADDR");
            if(response.value != value) throw std::runtime_error("Invalid VALUE");
        }
        else {
            auto exception = std::get<modbus::Exceptiondata>(data);
            std::cerr << "[SERVER] Exception response: [" << static_cast<int>(exception.code) << "] " << exception.name << std::endl;
        }
    }

public:
    ModbusClient(std::unique_ptr<IModbusChannel> transport)
        : transport_(std::move(transport)) {}

    void connect(const modbus::Ipv4& host, const modbus::Port& port) {
        transport_->connect(host.value, port.value).get();
    }

    void close() {
        transport_->close();
    }

    // FC 0x01: Read Coils
    std::vector<uint8_t> read_coil(std::uint8_t unit_id, std::uint16_t start_address, std::uint16_t quantity)
    {
        return std::get<std::vector<uint8_t>>(read_data(unit_id, start_address, quantity, modbus::FunctionCode::ReadCoils));
    }

    // FC 0x02: Read Discrete Inputs
    std::vector<uint8_t> read_discrete_input(std::uint8_t unit_id, std::uint16_t start_address, std::uint16_t quantity)
    {
        return std::get<std::vector<uint8_t>>(read_data(unit_id, start_address, quantity, modbus::FunctionCode::ReadDiscreteInputs));
    }

    // FC 0x03: Holding Registers
    std::vector<uint16_t> read_holding_registers(std::uint8_t unit_id, std::uint16_t start_address, std::uint16_t quantity)
    {
        return std::get<std::vector<uint16_t>>(read_data(unit_id, start_address, quantity, modbus::FunctionCode::HoldingRegisters));
    }

    // FC 0x04: Input Registers
    std::vector<uint16_t> read_input_registers(std::uint8_t unit_id, std::uint16_t start_address, std::uint16_t quantity)
    {
        return std::get<std::vector<uint16_t>>(read_data(unit_id, start_address, quantity, modbus::FunctionCode::InputRegisters));
    }

    // FC 0x05: Write Single Coil
    void write_single_coil(std::uint8_t unit_id, std::uint16_t address, std::uint16_t value)
    {
        write_data(unit_id, address, value, modbus::FunctionCode::WriteSingleCoil);
    }

    // FC 0x06: Write Holding Register
    void write_single_register(std::uint8_t unit_id, std::uint16_t address, std::uint16_t value)
    {
        write_data(unit_id, address, value, modbus::FunctionCode::WriteSingleRegister);
    }
};

} // namespace modbus
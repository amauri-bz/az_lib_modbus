#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <variant>

namespace modbus {

/**
 * @brief TCP MBAP header size
 */
constexpr size_t MBAP_HEADER_SIZE = 7;

/**
 * @brief Modbus exception code
 */
constexpr uint8_t EXC_ILLEGAL_FUNCTION = 0x01;
constexpr uint8_t EXC_ILLEGAL_DATA_ADDRESS = 0x02;
constexpr uint8_t EXC_ILLEGAL_DATA_VALUE = 0x03;
constexpr uint8_t EXC_SLAVE_DEVICE_FAILURE = 0x04;

/**
 * @brief Protocol Constants
 */
enum FunctionCode : std::uint8_t {
    ReadCoils = 0x01,
    ReadDiscreteInputs = 0x02,
    HoldingRegisters = 0x03,
    InputRegisters = 0x04,
    WriteSingleCoil = 0x05,
    WriteSingleRegister = 0x06
 };

/**
 * @brief TCP MBAP header structure:
 */
struct MbapHeader {
    std::uint16_t transaction_id;
    std::uint16_t protocol_id = 0x0000;
    std::uint16_t length; // Data length (PDU + Unit ID)
    std::uint8_t unit_id;
};

/**
 * @brief Message request structure:
 */
struct RequestData {
    std::uint8_t func_code;
    std::uint16_t start_addr;
    std::uint16_t number;
    std::uint16_t value;
};

/**
 * @brief Exeption wrapper structure:
 */
struct Exceptiondata {
    std::uint8_t code;
    std::string name;
};

/**
 * @brief IPV4 string format wrapper
 */
struct Ipv4 {
    std::string value;
    explicit Ipv4(std::string v) : value(v) {}
};

/**
 * @brief TCP port string format wrapper
 */
struct Port {
    std::string value;
    explicit Port(std::string v) : value(v) {}
};

/**
 * @brief Unit ID modbus wrapper
 */
struct UnitID {
    unsigned short value;
    explicit UnitID(unsigned short v) : value(v) {}
};

/**
 * @brief Convert uint16_t (Host Endian) to two bytes (Network Endian - Big Endian)
 *
 * @param value buffer to be converted
 * @return byte array converted
 */
inline std::array<std::uint8_t, 2> to_big_endian(std::uint16_t value) {
    return {
        static_cast<std::uint8_t>((value >> 8) & 0xFF),
        static_cast<std::uint8_t>(value & 0xFF)
    };
}

/**
 * @brief Converts two bytes (Big Endian) to uint16_t (Host Endian)
 *
 * @param high hight byte to be converted
 * @param low low byte to be converted.
 * @return double byte values converted
 */
inline std::uint16_t from_big_endian(std::uint8_t high, std::uint8_t low) {
    return (static_cast<std::uint16_t>(high) << 8) | static_cast<std::uint16_t>(low);
}

/**
 * @brief Exceptions handler
 *
 * @param expected_fc Function code
 * @param pdu Message PDU to be processed
 */
inline void check_exception(const std::vector<std::uint8_t>& pdu) {
    if (pdu[0] == (0x01 | 0x80)) {
        throw std::runtime_error("Modbus Exception: " + std::to_string(pdu[1]));
    }
}

/**
 * @brief Create the MBAP header
 *
 * @param adu message buffer to be filled
 * @param pdu_size message payload size
 * @param transaction_id transactionid nunmber
 * @param unit_id device unit number
 */
inline void create_mbap_header(
    std::vector<std::uint8_t> &adu,
    const std::uint16_t pdu_size,
    std::uint16_t transaction_id,
    std::uint8_t unit_id)
{
    // MBAP Header (7 bytes)
    auto tid_bytes = to_big_endian(transaction_id);
    auto len_bytes = to_big_endian(pdu_size + 1); // Length = PDU + 1B Unit ID

    // 0-1: Transaction ID
    std::copy(tid_bytes.begin(), tid_bytes.end(), adu.begin());
    // 2-3: Protocol ID (0x0000) - Já é 0 por padrão no vector
    // 4-5: Length
    std::copy(len_bytes.begin(), len_bytes.end(), adu.begin() + 4);
    // 6: Unit ID
    adu[6] = unit_id;
}

/**
 * @brief Create the frame for read function code
 *
 * @param transaction_id transactionid nunmber
 * @param unit_id device unit number
 * @param start_address start adress number
 * @param quantity quantity of addresses
 * @param function_code function code number
 * @return message buffer
 */
inline std::vector<std::uint8_t> create_read_adu(
    std::uint16_t transaction_id,
    std::uint8_t unit_id,
    std::uint16_t start_address,
    std::uint16_t quantity,
    FunctionCode function_code)
{
    // PDU Size (1B FC + 2B Address + 2B Quantity) = 5
    const std::uint16_t pdu_size = 5;

    // ADU size (MBAP Header 7B + PDU 5B) = 12
    std::vector<std::uint8_t> adu(MBAP_HEADER_SIZE + pdu_size);
    create_mbap_header(adu, pdu_size, transaction_id, unit_id);

    // 7: Function Code (0x01)
    adu[7] = function_code;

    // 8-9: Start Address
    auto addr_bytes = to_big_endian(start_address);
    std::copy(addr_bytes.begin(), addr_bytes.end(), adu.begin() + 8);

    // 10-11: Quantity
    auto qty_bytes = to_big_endian(quantity);
    std::copy(qty_bytes.begin(), qty_bytes.end(), adu.begin() + 10);

    return adu;
}

/**
 * @brief Create the frame for Write values
 *
 * @param transaction_id transactionid nunmber
 * @param unit_id device unit number
 * @param address adress number
 * @param value value to be applyed
 * @param function_code function code number
 * @return message buffer
 */
inline std::vector<std::uint8_t> create_write_adu(
    std::uint16_t transaction_id,
    std::uint8_t unit_id,
    std::uint16_t address,
    std::uint16_t value,
    FunctionCode function_code)
{
    const std::uint16_t pdu_size = 5;
    std::vector<std::uint8_t> adu(MBAP_HEADER_SIZE + pdu_size);
    create_mbap_header(adu, pdu_size, transaction_id, unit_id);

    // PDU
    adu[7] = function_code;

    // 8-9: Output Address
    auto addr_bytes = to_big_endian(address);
    std::copy(addr_bytes.begin(), addr_bytes.end(), adu.begin() + 8);

    if(function_code == modbus::FunctionCode::WriteSingleCoil) {
        // 10-11: Output Value (0xFF00 para ON, 0x0000 para OFF)
        if (value == 1) {
            adu[10] = 0xFF; // High
            adu[11] = 0x00; // Low
        } else {
            adu[10] = 0x00; // High
            adu[11] = 0x00; // Low
        }
    }
    else if(function_code == modbus::FunctionCode::WriteSingleRegister) {
        auto value_bytes = to_big_endian(value);
        std::copy(value_bytes.begin(), value_bytes.end(), adu.begin() + 10);
    }

    return adu;
}

/**
 * @brief BIT coil handler to process read messages
 *
 * @param header_data struct with header data
 * @param pdu_data struct with pdu data
 * @param pdu_response_buffer message buffer received
 * @return response message buffer
 */
inline std::vector<uint8_t> handle_read_bits(
    const MbapHeader header_data,
    const RequestData pdu_data,
    const std::vector<uint8_t>& pdu_response_buffer)
{
    uint8_t start_id = static_cast<uint8_t>(pdu_data.start_addr);
    size_t byte_count = (pdu_data.number + 7) / 8;
    std::vector<uint8_t> data_bytes(byte_count, 0);

    for (int i = 0; i < pdu_data.number; ++i) {
        uint8_t bit_value = pdu_response_buffer[i];

        if (bit_value != 0) {
            data_bytes[i / 8] |= (1 << (i % 8));
        }
    }

    std::vector<uint8_t> pdu_response;
    pdu_response.push_back(pdu_data.func_code);
    pdu_response.push_back(static_cast<uint8_t>(byte_count));
    pdu_response.insert(pdu_response.end(), data_bytes.begin(), data_bytes.end());

    std::vector<uint8_t> adu_response;
    std::vector<uint8_t> mbap_response_header(MBAP_HEADER_SIZE);
    create_mbap_header(mbap_response_header, (uint16_t)pdu_response.size(), header_data.transaction_id, header_data.unit_id);

    adu_response.insert(adu_response.end(), mbap_response_header.begin(), mbap_response_header.end());
    adu_response.insert(adu_response.end(), pdu_response.begin(), pdu_response.end());

    return adu_response;
}

/**
 * @brief Register handler to process read messages
 *
 * @param header_data struct with header data
 * @param pdu_data struct with pdu data
 * @param pdu_response_buffer message buffer received
 * @return
 */
inline std::vector<uint8_t> handle_read_registers(
    const MbapHeader header_data,
    const RequestData pdu_data,
    const std::vector<uint16_t>& pdu_response_buffer)
{
    uint8_t start_id = static_cast<uint8_t>(pdu_data.start_addr);
    size_t byte_count = pdu_data.number * 2;
    std::vector<uint8_t> data_bytes;
    data_bytes.reserve(byte_count);

    for (int i = 0; i < pdu_data.number; ++i) {
        uint16_t reg_value = pdu_response_buffer[i];
        data_bytes.push_back(static_cast<uint8_t>(reg_value >> 8));   // MSB
        data_bytes.push_back(static_cast<uint8_t>(reg_value & 0xFF)); // LSB
    }

    std::vector<uint8_t> pdu_response;
    pdu_response.push_back(pdu_data.func_code);
    pdu_response.push_back(static_cast<uint8_t>(byte_count));
    pdu_response.insert(pdu_response.end(), data_bytes.begin(), data_bytes.end());

    std::vector<uint8_t> adu_response;
    std::vector<uint8_t> mbap_response_header(MBAP_HEADER_SIZE);
    create_mbap_header(mbap_response_header, (uint16_t)pdu_response.size(), header_data.transaction_id, header_data.unit_id);

    adu_response.insert(adu_response.end(), mbap_response_header.begin(), mbap_response_header.end());
    adu_response.insert(adu_response.end(), pdu_response.begin(), pdu_response.end());

    return adu_response;
}

/**
 * @brief Message header decoder
 *
 * @param buffer message buffer received
 * @return struct with header data
 */
inline MbapHeader decode_header(const std::vector<std::uint8_t>& buffer) {
    if (buffer.size() < MBAP_HEADER_SIZE)
        throw std::runtime_error("MBAP header too short");

    MbapHeader header;
    header.transaction_id = from_big_endian(buffer[0], buffer[1]);
    header.protocol_id = from_big_endian(buffer[2], buffer[3]);
    header.length = from_big_endian(buffer[4], buffer[5]);
    header.unit_id = buffer[6];

    if (header.protocol_id != 0x0000) {
        throw std::runtime_error("invalid Protocol ID");
    }
    return header;
}

/**
 * @brief Request message decoder
 *
 * @param buffer message buffer received
 * @return struct with request data
 */
inline std::variant<modbus::RequestData, modbus::Exceptiondata> decode_request(const std::vector<std::uint8_t>& buffer) {
    if (buffer.size() < 5)
        throw std::runtime_error("EXC_ILLEGAL_BUFFER_SIZE");

    uint16_t tmp_val{0};

    RequestData request;
    request.func_code = buffer[0];
    request.start_addr = from_big_endian(buffer[1], buffer[2]);

    if (request.func_code < 0x01  && request.func_code > 0x06)
        return Exceptiondata{EXC_ILLEGAL_FUNCTION, "EXC_ILLEGAL_FUNCTION"};

    switch (request.func_code) {
        case WriteSingleCoil:
            request.number = 0;
            tmp_val = from_big_endian(buffer[3], buffer[4]);
            request.value = (tmp_val ==0xFF00)? true : false;
            break;
        case WriteSingleRegister:
            request.number = 0;
            request.value = from_big_endian(buffer[3], buffer[4]);
            break;
        default:
            request.number = from_big_endian(buffer[3], buffer[4]);
            request.value = 0;
            if (request.number == 0 || request.number > 2000)
                return Exceptiondata{EXC_ILLEGAL_DATA_VALUE, "EXC_ILLEGAL_DATA_VALUE"};
            break;
    }

    return request;
}

/**
 * @brief Read coils message decoder
 *
 * @param pdu_response message data buffer
 * @param quantity quantity of address to be processed
 * @return buffer with address data
 */
inline std::vector<uint8_t> decode_read_coils_response(
        const std::vector<uint8_t>& pdu_response,
        std::uint16_t quantity) {

    std::vector<uint8_t> response;

    uint8_t function_code = pdu_response[0];

    if (function_code & 0x80) {
        uint8_t exception_code = pdu_response[1];
        std::string error_msg = "Exception FC: " + std::to_string(function_code & 0x7F) +
                                ", exception_code: " + std::to_string(exception_code);
        throw std::runtime_error(error_msg);
    }

    if (function_code != modbus::FunctionCode::ReadCoils &&
        function_code != modbus::FunctionCode::ReadDiscreteInputs) {
        throw std::runtime_error("Exception invalid FC: " + std::to_string(function_code));
    }

    // Byte Count (Byte 8)
    uint8_t byte_count = pdu_response[1];

    // Check if the total length matches the expected length.
    if (pdu_response.size() != (1 + 1 + byte_count)) {
        throw std::runtime_error("Exception Invalid Byte Count:" + std::to_string(byte_count));
    }

    // Data decoder
    for (int i = 0; i < byte_count; ++i) {
        uint8_t current_byte = pdu_response[2 + i];

        // Each byte contains 8 coils (bits). The LSB (bit 0) is the first coil read.
        for (int bit = 0; bit < 8; ++bit) {
            // Extracts the bit value (true if it's 1, false if it's 0)
            bool coil_value = (current_byte >> bit) & 0x01;
            if(quantity>0) {
                response.push_back(coil_value);
                quantity--;
            }
        }
    }
    return response;
}

/**
 * @brief Read register message decoder
 *
 * @param pdu_response message data buffer
 * @param quantity quantity of address to be processed
 * @return buffer with address data
 */
inline std::vector<uint16_t> decode_read_register_response(
        const std::vector<uint8_t>& pdu_response,
        std::uint16_t quantity) {

    std::vector<uint16_t> response;

    uint8_t function_code = pdu_response[0];

    if (function_code & 0x80) {
        uint8_t exception_code = pdu_response[1];
        std::string error_msg = "Exception FC: " + std::to_string(function_code & 0x7F) +
                                ", exception_code: " + std::to_string(exception_code);
        throw std::runtime_error(error_msg);
    }

    if (function_code != modbus::FunctionCode::HoldingRegisters &&
        function_code != modbus::FunctionCode::InputRegisters) {
        throw std::runtime_error("Exception invalid FC: " + std::to_string(function_code));
    }

    // Byte Count (Byte 8)
    uint8_t byte_count = pdu_response[1];

    auto data_offset = 2;

    // Check if the total length matches the expected length.
    if (pdu_response.size() != (data_offset + byte_count)) {
        throw std::runtime_error("Exception Invalid Byte Count:" + std::to_string(byte_count));
    }

    // 2 bytes data size (uint16_t)
    size_t num_registers = byte_count / 2;

    for (size_t i = 0; i < num_registers; ++i) {
        const uint8_t* register_data = pdu_response.data() + data_offset + (i * 2);
        uint16_t reg_value = from_big_endian(register_data[0], register_data[1]);
        response.push_back(reg_value);
    }
    return response;
}

/**
 * @brief Build the Exception PDU
 *
 * @param header_data message header buffer
 * @param requested_fc function code number
 * @param exception_code exception code number
 * @return response message buffer with the exception number
 */
std::vector<uint8_t> create_modbus_exception_adu(
    const MbapHeader header_data,
    Exceptiondata exception_code)
{
    std::vector<uint8_t> pdu_exception;

    // Exception Function Code = Default FC + 0x80
    uint8_t exception_fc = 0x01 | 0x80;
    pdu_exception.push_back(exception_fc);

    // Exception Code
    pdu_exception.push_back(exception_code.code);

    // The exception PDU is always 2 bytes long.
    // The Modbus/TCP length is PDU size (2)
    uint16_t new_length = 2;

     std::vector<uint8_t> mbap_response_header(MBAP_HEADER_SIZE);
    create_mbap_header(mbap_response_header, (uint16_t)new_length, header_data.transaction_id, header_data.unit_id);

    std::vector<uint8_t> adu_response;
    adu_response.reserve(mbap_response_header.size() + pdu_exception.size());

    // ADU = MBAP Header (7 bytes) + Exception PDU (2 bytes) = 9 bytes
    adu_response.insert(adu_response.end(), mbap_response_header.begin(), mbap_response_header.end());
    adu_response.insert(adu_response.end(), pdu_exception.begin(), pdu_exception.end());

    return adu_response;
}

} // namespace modbus
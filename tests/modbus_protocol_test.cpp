#include <vector>
#include <algorithm>
#include <variant>

#include "../src/az_modbus_protocol.hpp"
#include "../src/az_helper.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("Create MBAP Header") {
    //Expected result:                          tid       prot_id      lenght   unit
    std::vector<std::uint8_t> expect_msg = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01};

    std::vector<std::uint8_t> header(modbus::MBAP_HEADER_SIZE);
    std::uint16_t transaction_id = 1u;
    std::uint16_t pdu_size = 5u;
    std::uint8_t unit_id = 1u;

    modbus::create_mbap_header(header, pdu_size, transaction_id, unit_id);

    helper::print_hex_buffer(header, "<<<< adu: ");

    REQUIRE(header.size() == expect_msg.size());
    REQUIRE((std::equal(header.begin(), header.end(), expect_msg.begin())) == true);
}

TEST_CASE("Decode MBAP Header") {
    //Expected result:                     tid       prot_id      lenght   unit
    std::vector<uint8_t> expect_msg = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01};

    modbus::MbapHeader header = modbus::decode_header(expect_msg);

    REQUIRE(header.transaction_id == 1u);
    REQUIRE(header.protocol_id == 0u);
    REQUIRE(header.length == 6u);
    REQUIRE(header.unit_id == 1u);
}

TEST_CASE("FC 0x01: Read Coils Request") {
    //Expected result:                     tid       prot_id      lenght   unit   fc
    std::vector<uint8_t> expect_msg = {0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x01, 0x01, 0x00, 0x05, 0x00, 0x02};

    std::uint8_t unit_id = 1u;
    std::uint16_t start_address = 5u;
    std::uint16_t quantity = 2u;
    modbus::FunctionCode function_code = modbus::FunctionCode::ReadCoils;
    std::uint16_t tid = 0u;
    auto request = modbus::create_read_adu(tid, unit_id, start_address, quantity, function_code);

    helper::print_hex_buffer(request, "<<<< request: ");

    REQUIRE(request.size() == expect_msg.size());
    REQUIRE((std::equal(request.begin(), request.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x01: Read Coils Process Request") {
    //Expected result:                  fc       addr       num
    std::vector<uint8_t> expect_msg = {0x01, 0x00, 0x05, 0x00, 0x02};

    auto request = modbus::decode_request(expect_msg);

    REQUIRE(std::holds_alternative<modbus::RequestData>(request) == true);

    if (std::holds_alternative<modbus::RequestData>(request)) {
        auto data = std::get<modbus::RequestData>(request);
        REQUIRE(data.func_code == 1u);
        REQUIRE(data.start_addr == 5u);
        REQUIRE(data.number == 2u);
        REQUIRE(data.value == 0u);
    }
}

TEST_CASE("FC 0x01: Read Coils Request Handler") {
    //Expected result:                     tid       prot_id      lenght   unit   fc   byte   val
    std::vector<uint8_t> expect_msg = {0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x01, 0x01, 0x01, 0x00 };

    modbus::MbapHeader header_data;
    header_data.transaction_id = 0u;
    header_data.protocol_id = 0u;
    header_data.length = 4u;
    header_data.unit_id =1u;

    modbus::RequestData pdu_data;
    pdu_data.func_code = modbus::FunctionCode::ReadCoils;
    pdu_data.start_addr = 5u;
    pdu_data.number = 2u;
    pdu_data.value = 0u;

    std::vector<uint8_t> pdu_response_buffer = {0, 0};

    std::vector<uint8_t> response =  handle_read_bits(header_data, pdu_data, pdu_response_buffer);

    helper::print_hex_buffer(response, "<<<< request: ");

    REQUIRE(response.size() == expect_msg.size());
    REQUIRE((std::equal(response.begin(), response.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x01: Read Coils Response") {
    std::vector<uint8_t> expect_pdu = { 0x01, 0x01, 0x00 };
    std::uint16_t quantity = 2u;
    std::vector<uint8_t> data_response = modbus::decode_read_coils_response(expect_pdu, quantity);

    REQUIRE(data_response[0] == 0);
    REQUIRE(data_response[1] == 0);
}

TEST_CASE("FC 0x02: Read Discrete Inputs Request") {
    //Expected result:                     tid       prot_id      lenght   unit   fc
    std::vector<uint8_t> expect_msg = {0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x01, 0x02, 0x00, 0x14, 0x00, 0x03};

    std::uint8_t unit_id = 1u;
    std::uint16_t start_address = 20u;
    std::uint16_t quantity = 3u;
    modbus::FunctionCode function_code = modbus::FunctionCode::ReadDiscreteInputs;
    std::uint16_t tid = 1u;
    auto request = modbus::create_read_adu(tid, unit_id, start_address, quantity, function_code);

    helper::print_hex_buffer(request, "<<<< request: ");

    REQUIRE(request.size() == expect_msg.size());
    REQUIRE((std::equal(request.begin(), request.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x02: Read Discrete Inputs Process Request") {
    //Expected result:                  fc       addr       num
    std::vector<uint8_t> expect_msg = {0x02, 0x00, 0x14, 0x00, 0x03};

    auto request = modbus::decode_request(expect_msg);

    REQUIRE(std::holds_alternative<modbus::RequestData>(request) == true);

    if (std::holds_alternative<modbus::RequestData>(request)) {
        auto data = std::get<modbus::RequestData>(request);
        REQUIRE(data.func_code == 2u);
        REQUIRE(data.start_addr == 20u);
        REQUIRE(data.number == 3u);
        REQUIRE(data.value == 0u);
    }
}

TEST_CASE("FC 0x02: Read Discrete Inputs Handler") {
    //Expected result:                     tid       prot_id      lenght   unit   fc   byte   val
    std::vector<uint8_t> expect_msg = {0x00, 0x01, 0x00, 0x00, 0x00, 0x04, 0x01, 0x02, 0x01, 0x05 };

    modbus::MbapHeader header_data;
    header_data.transaction_id = 1u;
    header_data.protocol_id = 0u;
    header_data.length = 4u;
    header_data.unit_id =1u;

    modbus::RequestData pdu_data;
    pdu_data.func_code = modbus::FunctionCode::ReadDiscreteInputs;
    pdu_data.start_addr = 20u;
    pdu_data.number = 3u;
    pdu_data.value = 0u;

    std::vector<uint8_t> pdu_response_buffer = {1, 0, 1};

    std::vector<uint8_t> response =  handle_read_bits(header_data, pdu_data, pdu_response_buffer);

    helper::print_hex_buffer(response, "<<<< request: ");

    REQUIRE(response.size() == expect_msg.size());
    REQUIRE((std::equal(response.begin(), response.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x02: Read Discrete Inputs Response") {
    std::vector<uint8_t> expect_pdu = { 0x02, 0x01, 0x05 };
    std::uint16_t quantity = 3u;
    std::vector<uint8_t> data_response = modbus::decode_read_coils_response(expect_pdu, quantity);

    REQUIRE(data_response[0] == 1);
    REQUIRE(data_response[1] == 0);
    REQUIRE(data_response[2] == 1);
}

TEST_CASE("FC 0x03: Holding Registers Request") {
    //Expected result:                     tid       prot_id      lenght   unit   fc
    std::vector<uint8_t> expect_msg = {0x00, 0x02, 0x00, 0x00, 0x00, 0x06, 0x01, 0x03, 0x00, 0x0B, 0x00, 0x04};

    std::uint8_t unit_id = 1u;
    std::uint16_t start_address = 11u;
    std::uint16_t quantity = 4u;
    modbus::FunctionCode function_code = modbus::FunctionCode::HoldingRegisters;
    std::uint16_t tid = 2u;
    auto request = modbus::create_read_adu(tid, unit_id, start_address, quantity, function_code);

    helper::print_hex_buffer(request, "<<<< request: ");

    REQUIRE(request.size() == expect_msg.size());
    REQUIRE((std::equal(request.begin(), request.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x03: Holding Registers Process Request") {
    //Expected result:                  fc       addr       num
    std::vector<uint8_t> expect_msg = {0x03, 0x00, 0x0B, 0x00, 0x04};

    auto request = modbus::decode_request(expect_msg);

    REQUIRE(std::holds_alternative<modbus::RequestData>(request) == true);

    if (std::holds_alternative<modbus::RequestData>(request)) {
        auto data = std::get<modbus::RequestData>(request);
        REQUIRE(data.func_code == 3u);
        REQUIRE(data.start_addr == 11u);
        REQUIRE(data.number == 4u);
        REQUIRE(data.value == 0u);
    }
}

TEST_CASE("FC 0x03: Holding Registers Handler") {
    //Expected result:                     tid       prot_id      lenght   unit   fc   byte     val1         val2       val3        val4
    std::vector<uint8_t> expect_msg = {0x00, 0x02, 0x00, 0x00, 0x00, 0x0B, 0x01, 0x03, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    modbus::MbapHeader header_data;
    header_data.transaction_id = 2u;
    header_data.protocol_id = 0u;
    header_data.length = 11u;
    header_data.unit_id = 1u;

    modbus::RequestData pdu_data;
    pdu_data.func_code = modbus::FunctionCode::HoldingRegisters;
    pdu_data.start_addr = 11u;
    pdu_data.number = 4u;
    pdu_data.value = 0u;

    std::vector<uint16_t> pdu_response_buffer = {0, 0, 0, 0};

    std::vector<uint8_t> response =  handle_read_registers(header_data, pdu_data, pdu_response_buffer);

    helper::print_hex_buffer(response, "<<<< request: ");

    REQUIRE(response.size() == expect_msg.size());
    REQUIRE((std::equal(response.begin(), response.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x03: Holding Registers Response") {
    std::vector<uint8_t> expect_pdu = {0x03, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    std::uint16_t quantity = 4u;
    std::vector<uint16_t> data_response = modbus::decode_read_register_response(expect_pdu, quantity);

    REQUIRE(data_response[0] == 0);
    REQUIRE(data_response[1] == 0);
    REQUIRE(data_response[2] == 0);
    REQUIRE(data_response[3] == 0);
}

TEST_CASE("FC 0x04: Input Registers Request") {
    //Expected result:                     tid       prot_id      lenght   unit   fc
    std::vector<uint8_t> expect_msg = {0x00, 0x03, 0x00, 0x00, 0x00, 0x06, 0x01, 0x04, 0x00, 0x06, 0x00, 0x01};

    std::uint8_t unit_id = 1u;
    std::uint16_t start_address = 6u;
    std::uint16_t quantity = 1u;
    modbus::FunctionCode function_code = modbus::FunctionCode::InputRegisters;
    std::uint16_t tid = 3u;
    auto request = modbus::create_read_adu(tid, unit_id, start_address, quantity, function_code);

    helper::print_hex_buffer(request, "<<<< request: ");

    REQUIRE(request.size() == expect_msg.size());
    REQUIRE((std::equal(request.begin(), request.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x04: Input Registers Process Request") {
    //Expected result:                  fc       addr       num
    std::vector<uint8_t> expect_msg = {0x04, 0x00, 0x06, 0x00, 0x01};

    auto request = modbus::decode_request(expect_msg);

    REQUIRE(std::holds_alternative<modbus::RequestData>(request) == true);

    if (std::holds_alternative<modbus::RequestData>(request)) {
        auto data = std::get<modbus::RequestData>(request);
        REQUIRE(data.func_code == 4u);
        REQUIRE(data.start_addr == 6u);
        REQUIRE(data.number == 1u);
        REQUIRE(data.value == 0u);
    }
}

TEST_CASE("FC 0x04: Input Registers Handler") {
    //Expected result:                     tid       prot_id      lenght   unit   fc   byte     val
    std::vector<uint8_t> expect_msg = {0x00, 0x03, 0x00, 0x00, 0x00, 0x05, 0x01, 0x04, 0x02, 0x00, 0x01};

    modbus::MbapHeader header_data;
    header_data.transaction_id = 3u;
    header_data.protocol_id = 0u;
    header_data.length = 5u;
    header_data.unit_id = 1u;

    modbus::RequestData pdu_data;
    pdu_data.func_code = modbus::FunctionCode::InputRegisters;
    pdu_data.start_addr = 6u;
    pdu_data.number = 1u;
    pdu_data.value = 0u;

    std::vector<uint16_t> pdu_response_buffer = {1};

    std::vector<uint8_t> response =  handle_read_registers(header_data, pdu_data, pdu_response_buffer);

    helper::print_hex_buffer(response, "<<<< request: ");

    REQUIRE(response.size() == expect_msg.size());
    REQUIRE((std::equal(response.begin(), response.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x04: Input Registers Response") {
    std::vector<uint8_t> expect_pdu = { 0x04, 0x02, 0x00, 0x01 };
    std::uint16_t quantity = 1u;
    std::vector<uint16_t> data_response = modbus::decode_read_register_response(expect_pdu, quantity);

    REQUIRE(data_response[0] == 1);
}

TEST_CASE("FC 0x05: Write Single Coil") {
    //Expected result:                     tid       prot_id      lenght   unit   fc
    std::vector<uint8_t> expect_msg = {0x00, 0x04, 0x00, 0x00, 0x00, 0x06, 0x01, 0x05, 0x00, 0x08, 0xFF, 0x00};

    std::uint8_t unit_id = 1u;
    std::uint16_t start_address = 8u;
    std::uint16_t value = 1u;
    modbus::FunctionCode function_code = modbus::FunctionCode::WriteSingleCoil;
    std::uint16_t tid = 4u;
    auto request = modbus::create_write_adu(tid, unit_id, start_address, value, function_code);

    helper::print_hex_buffer(request, "<<<< request: ");

    REQUIRE(request.size() == expect_msg.size());
    REQUIRE((std::equal(request.begin(), request.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x05: Write Single Coil Process Request") {
    //Expected result:                  fc       addr       val
    std::vector<uint8_t> expect_msg = {0x05, 0x00, 0x08, 0xFF, 0x00};

    auto request = modbus::decode_request(expect_msg);

    REQUIRE(std::holds_alternative<modbus::RequestData>(request) == true);

    if (std::holds_alternative<modbus::RequestData>(request)) {
        auto data = std::get<modbus::RequestData>(request);
        REQUIRE(data.func_code == 5u);
        REQUIRE(data.start_addr == 8u);
        REQUIRE(data.number == 0u);
        REQUIRE(data.value == 1u);
    }
}

TEST_CASE("FC 0x06: Write Holding Register") {
    //Expected result:                     tid       prot_id      lenght   unit   fc
    std::vector<uint8_t> expect_msg = {0x00, 0x05, 0x00, 0x00, 0x00, 0x06, 0x01, 0x06, 0x00, 0x07, 0x00, 0xC8};

    std::uint8_t unit_id = 1u;
    std::uint16_t start_address = 7u;
    std::uint16_t value = 200u;
    modbus::FunctionCode function_code = modbus::FunctionCode::WriteSingleRegister;
    std::uint16_t tid = 5u;
    auto request = modbus::create_write_adu(tid, unit_id, start_address, value, function_code);

    helper::print_hex_buffer(request, "<<<< request: ");

    REQUIRE(request.size() == expect_msg.size());
    REQUIRE((std::equal(request.begin(), request.end(), expect_msg.begin())) == true);
}

TEST_CASE("FC 0x06: Write Holding Register Process Request") {
    //Expected result:                  fc       addr       val
    std::vector<uint8_t> expect_msg = {0x06, 0x00, 0x07, 0x00, 0xC8};

    auto request = modbus::decode_request(expect_msg);

    REQUIRE(std::holds_alternative<modbus::RequestData>(request) == true);

    if (std::holds_alternative<modbus::RequestData>(request)) {
        auto data = std::get<modbus::RequestData>(request);
        REQUIRE(data.func_code == 6u);
        REQUIRE(data.start_addr == 7u);
        REQUIRE(data.number == 0u);
        REQUIRE(data.value == 200u);
    }
}

TEST_CASE("FC 0x06: Write Holding Register Process exception") {
    //Expected result:                  fc       addr       val
    std::vector<uint8_t> expect_msg = {0x03, 0x00, 0x01, 0x0B, 0xB8};

    auto request = modbus::decode_request(expect_msg);

    REQUIRE(std::holds_alternative<modbus::Exceptiondata>(request) == true);

    if (std::holds_alternative<modbus::Exceptiondata>(request)) {
        auto data = std::get<modbus::Exceptiondata>(request);
        REQUIRE(data.code == 3u);
        REQUIRE(data.name == "EXC_ILLEGAL_DATA_VALUE");
    }
}


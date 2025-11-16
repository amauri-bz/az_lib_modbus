
set(DOCTEST_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/)

add_executable(az_modbus_tests modbus_protocol_test.cpp)

target_include_directories(az_modbus_tests PUBLIC ${ASIO_INCLUDE_DIR} PUBLIC ${DOCTEST_INCLUDE_DIR})

enable_testing()
add_test(NAME run_modbus_tests COMMAND az_modbus_tests)

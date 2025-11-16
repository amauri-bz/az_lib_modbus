cmake_minimum_required(VERSION 3.10)
project(az_lib_modbus CXX)

# O caminho para a pasta 'include' do Asio
set(ASIO_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/asio/asio/include)

# Define o padrão C++ moderno
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g")

add_subdirectory(examples)

add_subdirectory(tests)

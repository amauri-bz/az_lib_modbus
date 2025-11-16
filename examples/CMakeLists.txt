
add_executable(az_server az_server.cpp)

add_executable(az_client az_client.cpp)

target_include_directories(az_server PUBLIC ${ASIO_INCLUDE_DIR})

target_include_directories(az_client PUBLIC ${ASIO_INCLUDE_DIR})

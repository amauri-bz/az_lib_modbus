#pragma once

#include <iostream>
#include <string>
#include <iomanip>

namespace helper {
/**
 * @brief Prints the contents of any byte container (vector or array) in hexadecimal format.
 * @param buffer The byte container (vector or array).
 * @param separator The separator to be used between the bytes (e.g., " " or "").
 */
template <typename Container>
void print_hex_buffer(const Container& buffer, const std::string& message = " ", const std::string& separator = " ") {
    std::ios state(nullptr);
    state.copyfmt(std::cout);

    std::cout << message << "Buffer Hex [" << buffer.size() << " bytes]: ";

    std::cout << std::hex << std::uppercase << std::setfill('0');

    for (const auto& byte : buffer) {
        std::cout << std::setw(2) << static_cast<int>(byte) << separator;
    }

    std::cout << "\n";

    std::cout.copyfmt(state);
}
}
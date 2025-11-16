#include "../src/az_modbus_context.hpp"
#include "../src/az_asio_channel.hpp"
#include "../src/az_modbus_client.hpp"

#include <iostream>
#include <string>
#include <sstream>

int main() {
    try {
        modbus::ModbusContext context;
        modbus::ModbusClient client(
            std::make_unique<modbus::AsioChannel>(context)
        );

        std::cout << "--- Modbus Client ---" << std::endl;
        client.connect(modbus::Ipv4("127.0.0.1"), modbus::Port("1502"));
        std::cout << "Connecting to Modbus Slave. 127.0.0.1:1502" << std::endl;

        std::string command_oper;
        std::string command_sub_oper;
        std::string command_id_str;
        std::string command_val_str;
        while (true) {
            std::cout << "Operation['read', 'write', 'exit']> ";
            std::getline(std::cin, command_oper);

            if (command_oper == "exit") {
                break;
            }
            else if (command_oper == "read") {

                std::cout << "SubOperation['coil', 'cinput', 'register', 'rinput']> ";
                std::getline(std::cin, command_sub_oper);
                std::cout << "ADDR[1-100]> ";
                std::getline(std::cin, command_id_str);
                std::cout << "QUANT[1-100]> ";
                std::getline(std::cin, command_val_str);

                std::uint16_t command_id = static_cast<std::uint16_t>(std::stoul(command_id_str));
                std::uint16_t command_val = static_cast<std::uint16_t>(std::stoul(command_val_str));

                std::cout << "Reading (" << command_sub_oper << ", " << command_id_str << "," << command_val_str << ")\n";

                try {
                    if (command_sub_oper == "coil") {
                            std::vector<uint8_t> coils = client.read_coil(1, command_id, command_val);
                            std::cout << "SUCCESS." << std::endl;
                            for (size_t i = 0; i < coils.size(); ++i) {
                                std::cout << "  Coil " << (command_id + i) << ": " << (coils[i] ? "ON" : "OFF") << std::endl;
                            }
                    }
                    else if (command_sub_oper == "cinput") {
                            std::vector<uint8_t> coils = client.read_discrete_input(1, command_id, command_val);
                            std::cout << "SUCCESS." << std::endl;
                            for (size_t i = 0; i < coils.size(); ++i) {
                                std::cout << "  Coil Input " << (command_id + i) << ": " << (coils[i] ? "ON" : "OFF") << std::endl;
                            }
                    }
                    else if (command_sub_oper == "register") {
                            std::vector<uint16_t> coils = client.read_holding_registers(1, command_id, command_val);
                            std::cout << "SUCCESS." << std::endl;
                            for (size_t i = 0; i < coils.size(); ++i) {
                                std::cout << "  holding register " << (command_id + i) << ": " << (coils[i]) << std::endl;
                            }
                    }
                    else if (command_sub_oper == "rinput") {
                            std::vector<uint16_t> coils = client.read_input_registers(1, command_id, command_val);
                            std::cout << "SUCCESS." << std::endl;
                            for (size_t i = 0; i < coils.size(); ++i) {
                                std::cout << "  Input Register " << (command_id + i) << ": " << (coils[i]) << std::endl;
                            }
                    }
                } catch (const std::exception& e) {
                        std::cerr << "ERROR: " << e.what() << std::endl;
                }
            }
            else if (command_oper == "write") {

                std::cout << "SubOperation['coil', 'register']> ";
                std::getline(std::cin, command_sub_oper);
                std::cout << "ADDR[1-100]> ";
                std::getline(std::cin, command_id_str);
                std::cout << "VALUE[0-2000]> ";
                std::getline(std::cin, command_val_str);

                std::uint16_t command_id = static_cast<std::uint16_t>(std::stoul(command_id_str));
                std::uint16_t command_val = static_cast<std::uint16_t>(std::stoul(command_val_str));

                if (command_sub_oper == "coil") {
                    std::cout << "Writing Coil " << command_id_str << " to " << command_val_str << "\n";
                    try {
                        client.write_single_coil(1, command_id, command_val);
                        std::cout << "SUCCESS." << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "ERROR: " << e.what() << std::endl;
                    }
                } else if (command_sub_oper == "register") {
                    std::cout << "Writing Register " << command_id_str << " to " << command_val_str << "\n";
                    try {
                        client.write_single_register(1, command_id, command_val);
                        std::cout << "SUCCESS." << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "ERROR: " << e.what() << std::endl;
                    }
                }
            }
            else if (!command_oper.empty()) {
                std::cout << "Invalid command" << std::endl;
            }
        }

        client.close();

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        std::cerr << "Make sure the Modbus Slave is active on port 1502" << std::endl;
        return 1;
    }

    return 0;
}
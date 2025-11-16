#include "../src/az_modbus_context.hpp"
#include "../src/az_asio_server_transport.hpp"
#include "../src/az_modbus_server.hpp"
#include "../src/az_database_interface.hpp"

#include <vector>
#include <iostream>
#include <variant>

class Database : public db::DatabaseInterface {
private:
    std::vector<uint8_t> db_bits;
    std::vector<uint8_t> db_input_bits;
    std::vector<uint16_t> db_input_registers;
    std::vector<uint16_t> db_registers;
    uint8_t db_size = 0;
    std::mutex mtx_;

public:
    bool connect() override {return true;}
    bool release() override {return true;}
    bool db_delete(db::DbType type, std::uint16_t id) override {return true;}

    bool db_create(std::uint16_t num_itens) override {
        std::lock_guard<std::mutex> lock(mtx_);
        db_bits.resize(num_itens);
        db_input_bits.resize(num_itens);
        db_registers.resize(num_itens);
        db_input_registers.resize(num_itens);
        db_size = num_itens;
        return true;
    }

    std::variant<uint8_t, uint16_t> db_read(db::DbType type, std::uint16_t id) override {
        std::lock_guard<std::mutex> lock(mtx_);
        std::variant<uint8_t, uint16_t> value;

        switch (type) {
            case db::DbType::BITS: value = db_bits.at(id); break;
            case db::DbType::BITS_INPUT:value = db_input_bits.at(id); break;
            case db::DbType::REGISTER: value = db_registers.at(id); break;
            case db::DbType::REGISTER_INPUT: value = db_input_registers.at(id); break;
            default: std::cout << "db_read invalid type\n";
        }
        return value;
    }

    bool db_update(db::DbType type, std::uint16_t id, std::variant<uint8_t, uint16_t> value) override {
        std::lock_guard<std::mutex> lock(mtx_);

        if(std::holds_alternative<uint8_t>(value))
            std::cout << "db_update id:" << id << " value:" << static_cast<int>(std::get<uint8_t>(value)) << "\n";
        else if(std::holds_alternative<uint16_t>(value))
            std::cout << "db_update id:" << id << " value:" << static_cast<int>(std::get<uint16_t>(value)) << "\n";

        switch (type) {
            case db::DbType::BITS: db_bits.at(id) = std::get<uint8_t>(value); break;
            case db::DbType::BITS_INPUT: db_input_bits.at(id) = std::get<uint8_t>(value); break;
            case db::DbType::REGISTER: db_registers.at(id) = std::get<uint16_t>(value); break;
            case db::DbType::REGISTER_INPUT: db_input_registers.at(id) = std::get<uint16_t>(value); break;
            default: std::cout << "db_update invalid type\n";
        }
        return true;
    }
};

int main() {
    try {
        modbus::ModbusContext context;
        auto database_ptr = std::make_unique<Database>();
        database_ptr->db_create(100);

        modbus::ModbusServer server(
            std::make_unique<modbus::AsioServerTransport>(context),
            std::unique_ptr<db::DatabaseInterface>(std::move(database_ptr)),
            modbus::UnitID(1)
        );

        server.start(modbus::Ipv4("0.0.0.0"), modbus::Port("1502"));

        std::cout << "Servidor Modbus rodando. Pressione ENTER para parar." << std::endl;
        std::cin.get();

    } catch (const std::exception& e) {
        std::cerr << "Erro Fatal do Servidor: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
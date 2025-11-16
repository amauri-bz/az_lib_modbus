
#pragma once

#include <variant>

namespace db{

enum class DbType {
    BITS,
    BITS_INPUT,
    REGISTER,
    REGISTER_INPUT
};

class DatabaseInterface {
public:
    virtual ~DatabaseInterface() = default;
    virtual bool connect() = 0;
    virtual bool release() = 0;
    virtual bool db_create(std::uint16_t values) = 0;
    virtual std::variant<std::uint8_t, std::uint16_t> db_read(db::DbType type, std::uint16_t id) = 0;
    virtual bool db_update(db::DbType type, std::uint16_t id, std::variant<std::uint8_t, std::uint16_t> value) = 0;
    virtual bool db_delete(DbType type, std::uint16_t id) = 0;
};
}
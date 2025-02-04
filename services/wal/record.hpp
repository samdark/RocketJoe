#pragma once

#include <components/protocol/protocol.hpp>
#include "dto.hpp"

namespace services::wal {

    struct record_t final {
        size_tt size;
        crc32_t crc32;
        crc32_t last_crc32;
        id_t id;
        statement_type type;
        components::protocol::variant_statement_t data;

        bool is_valid() const;
        void set_data(msgpack::object& object);
    };

} // namespace services::wal

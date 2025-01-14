#pragma once

#include <vector>
#include <list>
#include <actor-zeta.hpp>
#include <components/protocol/base.hpp>
#include <components/document/document.hpp>
#include <services/wal/base.hpp>

namespace services::disk {

    struct result_collection_t {
        collection_name_t name;
        std::list<components::document::document_ptr> documents;
    };

    struct result_database_t {
        database_name_t name;
        std::vector<result_collection_t> collections;

        std::vector<collection_name_t> name_collections() const;
        void set_collection(const std::vector<collection_name_t> &names);
    };

    class result_load_t {
        using result_t = std::vector<result_database_t>;

    public:
        result_load_t() = default;
        result_load_t(const std::vector<database_name_t> &databases, wal::id_t wal_id);
        const result_t& operator*() const;
        result_t& operator*();
        std::vector<database_name_t> name_databases() const;
        std::size_t count_collections() const;
        void clear();

        wal::id_t wal_id() const;

    private:
        result_t databases_;
        wal::id_t wal_id_;
    };

} // namespace services::disk

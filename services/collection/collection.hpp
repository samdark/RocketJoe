#pragma once
#include <memory>
#include <unordered_map>

#include <absl/container/btree_map.h>
#include <pybind11/pytypes.h>

#include <components/protocol/base.hpp>
#include <components/protocol/insert_many.hpp>
#include <components/log/log.hpp>
#include <components/cursor/cursor.hpp>
#include <components/document/document.hpp>
#include <components/document/document_view.hpp>
#include <components/parser/conditional_expression.hpp>
#include <components/session/session.hpp>

#include <services/database/database.hpp>

#include "forward.hpp"
#include "result.hpp"
#include "route.hpp"

namespace services::collection {

    using document_id_t = components::document::document_id_t;
    using document_ptr = components::document::document_ptr;
    using storage_t = absl::btree_map<document_id_t, document_ptr>;
    using document_view_t = components::document::document_view_t;
    using components::parser::find_condition_ptr;

    class collection_t final : public actor_zeta::basic_async_actor {
    public:
        collection_t(database::database_t*, const std::string& name, log_t& log, actor_zeta::address_t mdisk);
        auto create_documents(session_id_t& session, std::list<document_ptr> &documents) -> void;
        auto size(session_id_t& session) -> void;
        void insert_one(session_id_t& session_t, document_ptr& document);
        void insert_many(session_id_t& session, std::list<document_ptr> &documents);
        auto find(const session_id_t& session, const find_condition_ptr& cond) -> void;
        auto find_one(const session_id_t& session, const find_condition_ptr& cond) -> void;
        auto delete_one(const session_id_t& session, const find_condition_ptr& cond) -> void;
        auto delete_many(const session_id_t& session, const find_condition_ptr& cond) -> void;
        auto update_one(const session_id_t& session, const find_condition_ptr& cond, const document_ptr& update, bool upsert) -> void;
        auto update_many(const session_id_t& session, const find_condition_ptr& cond, const document_ptr& update, bool upsert) -> void;
        void drop(const session_id_t& session);
        void close_cursor(session_id_t& session);

    private:
        document_id_t insert_(const document_ptr&document);
        document_view_t get_(const document_id_t& id) const;
        std::size_t size_() const;
        bool drop_();
        result_find search_(const find_condition_ptr& cond);
        result_find_one search_one_(const find_condition_ptr& cond);
        result_delete delete_one_(const find_condition_ptr& cond);
        result_delete delete_many_(const find_condition_ptr& cond);
        result_update update_one_(const find_condition_ptr& cond, const document_ptr& update, bool upsert);
        result_update update_many_(const find_condition_ptr& cond, const document_ptr& update, bool upsert);
        void remove_(const document_id_t& id);
        bool update_(const document_id_t& id, const document_ptr& update, bool is_commit);
        void send_update_to_disk_(const session_id_t& session, const result_update &result);
        void send_delete_to_disk_(const session_id_t& session, const result_delete &result);

        const std::string name_;
        const std::string database_name_;
        log_t log_;
        actor_zeta::address_t database_;
        actor_zeta::address_t mdisk_;
        storage_t storage_;
        std::unordered_map<session_id_t, std::unique_ptr<components::cursor::sub_cursor_t>> cursor_storage_;
        bool dropped_{false};

#ifdef DEV_MODE
    public:
        void insert_test(const document_ptr &doc);
        result_find find_test(find_condition_ptr cond);
        std::size_t size_test() const;
        document_view_t get_test(const std::string& id) const;
        result_delete delete_one_test(find_condition_ptr cond);
        result_delete delete_many_test(find_condition_ptr cond);
        result_update update_one_test(find_condition_ptr cond, const document_ptr& update, bool upsert);
        result_update update_many_test(find_condition_ptr cond, const document_ptr& update, bool upsert);
#endif
    };

} // namespace services::storage

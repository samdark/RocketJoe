#include "wal.hpp"
#include <unistd.h>
#include <utility>
#include <crc32c/crc32c.h>

#include "dto.hpp"
#include "route.hpp"

namespace services::wal {

    constexpr static auto wal_name = ".wal";

    bool file_exist_(const boost::filesystem::path &path) {
        boost::filesystem::file_status s = boost::filesystem::file_status{};
        return boost::filesystem::status_known(s)
                   ? boost::filesystem::exists(s)
                   : boost::filesystem::exists(path);
    }

    std::size_t next_index(std::size_t index, size_tt size) {
        return index + size + sizeof(size_tt) + sizeof(crc32_t);
    }


    wal_replicate_t::wal_replicate_t(manager_wal_replicate_t*manager, log_t& log, boost::filesystem::path path)
        : actor_zeta::basic_async_actor(manager, "wal")
        , log_(log.clone())
        , path_(std::move(path)) {
        add_handler(handler_id(route::load), &wal_replicate_t::load);
        add_handler(handler_id(route::create_database), &wal_replicate_t::create_database);
        add_handler(handler_id(route::drop_database), &wal_replicate_t::drop_database);
        add_handler(handler_id(route::create_collection), &wal_replicate_t::create_collection);
        add_handler(handler_id(route::drop_collection), &wal_replicate_t::drop_collection);
        add_handler(handler_id(route::insert_one), &wal_replicate_t::insert_one);
        add_handler(handler_id(route::insert_many), &wal_replicate_t::insert_many);
        add_handler(handler_id(route::delete_one), &wal_replicate_t::delete_one);
        add_handler(handler_id(route::delete_many), &wal_replicate_t::delete_many);
        add_handler(handler_id(route::update_one), &wal_replicate_t::update_one);
        add_handler(handler_id(route::update_many), &wal_replicate_t::update_many);
        if (!file_exist_(path_)) {
            boost::filesystem::create_directory(path_);
        }
        file_ = std::make_unique<core::file::file_t>(path_ / wal_name);
        file_->seek_eof();
        init_id();
    }

    void wal_replicate_t::send_success(session_id_t& session, address_t& sender) {
        if (sender) {
            actor_zeta::send(sender, address(), handler_id(route::success), session, services::wal::id_t(id_));
        }
    }

    wal_replicate_t::~wal_replicate_t() {
    }

    size_tt read_size_impl(buffer_t& input, int index_start) {
        size_tt size_tmp = 0;
        size_tmp = 0xff00 & size_tt(input[index_start] << 8);
        size_tmp |= 0x00ff & size_tt(input[index_start + 1]);
        return size_tmp;
    }

    static size_tt read_size_impl(const char* input, int index_start) {
        size_tt size_tmp = 0;
        size_tmp = 0xff00 & (size_tt(input[index_start] << 8));
        size_tmp |= 0x00ff & (size_tt(input[index_start + 1]));
        return size_tmp;
    }

    size_tt wal_replicate_t::read_size(size_t start_index) const {
        auto size_read = sizeof(size_tt);
        auto buffer = file_->read(size_read, off64_t(start_index));
        auto size_blob = read_size_impl(buffer.data(), 0);
        return size_blob;
    }

    buffer_t wal_replicate_t::read(size_t start_index, size_t finish_index) const {
        auto size_read = finish_index - start_index;
        buffer_t buffer;
        file_->read(buffer, size_read, off64_t(start_index));
        return buffer;
    }

    void wal_replicate_t::load(session_id_t& session, address_t& sender, services::wal::id_t wal_id) {
        trace(log_, "wal_replicate_t::load, id: {}", wal_id);
        std::size_t start_index = 0;
        next_id(wal_id);
        std::vector<record_t> records;
        if (find_start_record(wal_id, start_index)) {
            std::size_t size = 0;
            do {
                records.emplace_back(read_record(start_index));
                start_index = next_index(start_index, records[size].size);
            } while (records[size++].is_valid());
            records.erase(records.end() - 1);
        }
        actor_zeta::send(sender, address(), handler_id(route::load_finish), session, std::move(records));
    }

    void wal_replicate_t::create_database(session_id_t& session, address_t& sender, components::protocol::create_database_t& data) {
        trace(log_, "wal_replicate_t::create_database {}", data.database_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::drop_database(session_id_t& session, address_t& sender, components::protocol::drop_database_t& data) {
        trace(log_, "wal_replicate_t::drop_database {}", data.database_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::create_collection(session_id_t& session, address_t& sender, components::protocol::create_collection_t& data) {
        trace(log_, "wal_replicate_t::create_collection {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::drop_collection(session_id_t& session, address_t& sender, components::protocol::drop_collection_t& data) {
        trace(log_, "wal_replicate_t::drop_collection {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::insert_one(session_id_t& session, address_t& sender, insert_one_t& data) {
        trace(log_, "wal_replicate_t::insert_one {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::insert_many(session_id_t& session, address_t& sender, insert_many_t& data) {
        trace(log_, "wal_replicate_t::insert_many {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::delete_one(session_id_t& session, address_t& sender, delete_one_t& data) {
        trace(log_, "wal_replicate_t::delete_one {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::delete_many(session_id_t& session, address_t& sender, delete_many_t& data) {
        trace(log_, "wal_replicate_t::delete_many {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::update_one(session_id_t& session, address_t& sender, update_one_t& data) {
        trace(log_, "wal_replicate_t::update_one {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    void wal_replicate_t::update_many(session_id_t& session, address_t& sender, update_many_t& data) {
        trace(log_, "wal_replicate_t::update_many {}::{}", data.database_, data.collection_);
        write_data_(data);
        send_success(session, sender);
    }

    template<class T>
    void wal_replicate_t::write_data_(T &data) {
        next_id(id_);
        buffer_.clear();
        last_crc32_ = pack(buffer_, last_crc32_, id_, data);
        file_->append(buffer_.data(), buffer_.size());
    }

    void wal_replicate_t::init_id() {
        std::size_t start_index = 0;
        auto id = read_id(start_index);
        while (id > 0) {
            id_ = id;
            start_index = next_index(start_index, read_size(start_index));
            id = read_id(start_index);
        }
    }

    bool wal_replicate_t::find_start_record(services::wal::id_t wal_id, std::size_t &start_index) const {
        start_index = 0;
        auto first_id = read_id(start_index);
        if (first_id > 0) {
            for (auto n = first_id; n < wal_id; ++n) {
                auto size = read_size(start_index);
                if (size > 0) {
                    start_index = next_index(start_index, size);
                } else {
                    return false;
                }
            }
            return wal_id == read_id(start_index);
        }
        return false;
    }

    services::wal::id_t wal_replicate_t::read_id(std::size_t start_index) const {
        auto size = read_size(start_index);
        if (size > 0) {
            auto start = start_index + sizeof(size_tt);
            auto finish = start + size;
            auto output = read(start, finish);
            return unpack_wal_id(output);
        }
        return 0;
    }

    record_t wal_replicate_t::read_record(std::size_t start_index) const {
        record_t record;
        record.size = read_size(start_index);
        if (record.size > 0) {
            auto start = start_index + sizeof(size_tt);
            auto finish = start + record.size + sizeof(crc32_t);
            auto output = read(start, finish);
            record.crc32 = read_crc32(output, record.size);
            if (record.crc32 == crc32c::Crc32c(output.data(), record.size)) {
                msgpack::unpacked msg;
                msgpack::unpack(msg, output.data(), record.size);
                const auto& o = msg.get();
                record.last_crc32 = o.via.array.ptr[0].as<crc32_t>();
                record.id = o.via.array.ptr[1].as<services::wal::id_t>();
                record.type = static_cast<statement_type>(o.via.array.ptr[2].as<char>());
                record.set_data(o.via.array.ptr[3]);
            } else {
                record.type = statement_type::unused;
                //todo: error wal content
            }
        } else {
            record.type = statement_type::unused;
        }
        return record;
    }

#ifdef DEV_MODE
    bool wal_replicate_t::test_find_start_record(services::wal::id_t wal_id, std::size_t &start_index) const {
        return find_start_record(wal_id, start_index);
    }

    services::wal::id_t wal_replicate_t::test_read_id(std::size_t start_index) const {
        return read_id(start_index);
    }

    std::size_t wal_replicate_t::test_next_record(std::size_t start_index) const {
        return next_index(start_index, read_size(start_index));
    }

    record_t wal_replicate_t::test_read_record(std::size_t start_index) const {
        return read_record(start_index);
    }

    size_tt wal_replicate_t::test_read_size(size_t start_index) const {
        return read_size(start_index);
    }

    buffer_t wal_replicate_t::test_read(size_t start_index, size_t finish_index) const {
        return read(start_index, finish_index);
    }
#endif

} //namespace services::wal

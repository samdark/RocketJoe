#pragma once

#include <components/document/document.hpp>
#include <components/document/mutable/mutable_array.h>
#include <components/document/mutable/mutable_dict.h>
#include <msgpack.hpp>

using ::document::impl::mutable_array_t;
using ::document::impl::mutable_dict_t;
using ::document::impl::value_t;
using ::document::impl::value_type;

template<typename Stream>
void to_bin(msgpack::packer<Stream>& o, const components::document::document_ptr& doc) {
    auto data = std::make_unique<char[]>(doc->data.size());
    std::memcpy(data.get(), doc->data.data(), doc->data.size());
    o.pack_bin(doc->data.size());
    o.pack_bin_body(data.release(), doc->data.size());
}

msgpack::object to_bin(const components::document::document_ptr& doc,msgpack::zone& zone);

template<typename Stream>
void to_msgpack_(msgpack::packer<Stream>& o, const value_t* structure) {
    if (structure->type() == value_type::dict) {
        auto* dict = structure->as_dict();
        o.pack_map(dict->count());
        int i = 0;
        for (auto it = dict->begin(); it; ++it) {
            //todo kick memory leak
            //msg_dict.ptr[i].key = msgpack::object(std::string_view(it.key()->to_string()));
            auto* s = new std::string(it.key()->to_string());
            o.pack(s->data());
            to_msgpack_(o, it.value());
            ++i;
        }
        return;
    } else if (structure->type() == value_type::array) {
        auto* array = structure->as_array();
        o.pack_array(array->count());
        int i = 0;
        for (auto it = array->begin(); it; ++it) {
            to_msgpack_(o, it.value());
            ++i;
        }
        return;
    } else if (structure->is_unsigned()) {
        o.pack(structure->as_unsigned());
        return;
    } else if (structure->is_int()) {
        o.pack(structure->as_int());
        return;
    }
    return;
}

const value_t *to_structure(const msgpack::object &msg_object);
msgpack::object to_msgpack_(const value_t *structure,msgpack::zone& zone);

// User defined class template specialization
namespace msgpack {
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
        namespace adaptor {

            template<>
            struct convert<components::document::document_ptr> final {
                msgpack::object const& operator()(msgpack::object const& o, components::document::document_ptr& v) const {
                    if (o.type != msgpack::type::ARRAY) {
                        throw msgpack::type_error();
                    }

                    msgpack::sbuffer tmp;
                    tmp.write(o.via.array.ptr[1].via.bin.ptr, o.via.array.ptr[1].via.bin.size);
                    v = components::document::make_document(to_structure(o.via.array.ptr[0])->as_dict()->as_mutable(), tmp);
                    return o;
                }
            };

            template<>
            struct pack<components::document::document_ptr> final {
                template<typename Stream>
                packer<Stream>& operator()(msgpack::packer<Stream>& o, components::document::document_ptr const& v) const {
                    o.pack_array(2);
                    to_msgpack_(o, v->structure);
                    to_bin(o, v);
                    return o;
                }
            };

            template<>
            struct object_with_zone<components::document::document_ptr> final {
                void operator()(msgpack::object::with_zone& o, components::document::document_ptr const& v) const {
                    o.type = type::ARRAY;
                    o.via.array.size = 2;
                    o.via.array.ptr = static_cast<msgpack::object*>(o.zone.allocate_align(sizeof(msgpack::object) * o.via.array.size, MSGPACK_ZONE_ALIGNOF(msgpack::object)));
                    o.via.array.ptr[0] = to_msgpack_(v->structure, o.zone);
                    o.via.array.ptr[1] = to_bin(v, o.zone);
                }
            };

        } // namespace adaptor
    }     // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack
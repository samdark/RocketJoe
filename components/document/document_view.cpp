#include "document_view.hpp"
#include <components/document/core/dict.hpp>
#include <components/document/core/array.hpp>
#include <components/document/core/doc.hpp>
#include <components/document/mutable/mutable_dict.h>
#include <components/document/structure.hpp>
#include <iostream>

using ::document::impl::value_type;

namespace components::document {

document_view_t::document_view_t()
    : index_(nullptr)
    , array_(nullptr)
    , storage_(nullptr) {
}

document_view_t::document_view_t(document_view_t::index_t index, document_view_t::storage_t storage)
    : index_(index)
    , array_(nullptr)
    , storage_(storage) {
}

document_view_t::document_view_t(const document_t &src)
    : index_(src.structure)
    , array_(nullptr)
    , storage_(&src.data) {
}

document_view_t::document_view_t(const document_ptr &src)
    : index_(src->structure)
    , array_(nullptr)
    , storage_(&src->data) {
}

document_view_t::document_view_t(const document_view_t &src)
    : index_(src.index_)
    , array_(src.array_)
    , storage_(src.storage_) {
}

document_view_t &document_view_t::operator=(const document_view_t &src) {
    index_ = src.index_;
    array_ = src.array_;
    storage_ = src.storage_;
    return *this;
}

bool document_view_t::is_valid() const {
    return index_ != nullptr;
}

bool document_view_t::is_dict() const {
    return index_ != nullptr;
}

bool document_view_t::is_array() const {
    return array_ != nullptr;
}

std::size_t document_view_t::count() const {
    return is_dict() ? index_->count() : array_->count();
}

bool document_view_t::is_exists(std::string &&key) const {
    return index_->get(std::move(key)) != nullptr;
}

bool document_view_t::is_exists(const std::string &key) const {
    return index_->get(key) != nullptr;
}

bool document_view_t::is_exists(uint32_t index) const {
    return array_->get(index) != nullptr;
}

bool document_view_t::is_null(std::string &&key) const {
    return get_type(std::move(key)) == object_type::NIL;
}

bool document_view_t::is_null(const std::string &key) const {
    return get_type(key) == object_type::NIL;
}

bool document_view_t::is_null(uint32_t index) const {
    return get_type(index) == object_type::NIL;
}

bool document_view_t::is_bool(std::string &&key) const {
    return get_type(std::move(key)) == object_type::BOOLEAN;
}

bool document_view_t::is_bool(const std::string &key) const {
    return get_type(key) == object_type::BOOLEAN;
}

bool document_view_t::is_bool(uint32_t index) const {
    return get_type(index) == object_type::BOOLEAN;
}

bool document_view_t::is_ulong(std::string &&key) const {
    return get_type(std::move(key)) == object_type::POSITIVE_INTEGER;
}

bool document_view_t::is_ulong(const std::string &key) const {
    return get_type(key) == object_type::POSITIVE_INTEGER;
}

bool document_view_t::is_ulong(uint32_t index) const {
    return get_type(index) == object_type::POSITIVE_INTEGER;
}

bool document_view_t::is_long(std::string &&key) const {
    return get_type(std::move(key)) == object_type::NEGATIVE_INTEGER;
}

bool document_view_t::is_long(const std::string &key) const {
    return get_type(key) == object_type::NEGATIVE_INTEGER;
}

bool document_view_t::is_long(uint32_t index) const {
    return get_type(index) == object_type::NEGATIVE_INTEGER;
}

bool document_view_t::is_float(std::string &&key) const {
    return get_type(std::move(key)) == object_type::FLOAT;
}

bool document_view_t::is_float(const std::string &key) const {
    return get_type(key) == object_type::FLOAT;
}

bool document_view_t::is_float(uint32_t index) const {
    return get_type(index) == object_type::FLOAT;
}

bool document_view_t::is_double(std::string &&key) const {
    return get_type(std::move(key)) == object_type::FLOAT64;
}

bool document_view_t::is_double(const std::string &key) const {
    return get_type(key) == object_type::FLOAT64;
}

bool document_view_t::is_double(uint32_t index) const {
    return get_type(index) == object_type::FLOAT64;
}

bool document_view_t::is_string(std::string &&key) const {
    return get_type(std::move(key)) == object_type::STR;
}

bool document_view_t::is_string(const std::string &key) const {
    return get_type(key) == object_type::STR;
}

bool document_view_t::is_string(uint32_t index) const {
    return get_type(index) == object_type::STR;
}

bool document_view_t::is_array(std::string &&key) const {
    return get_type(std::move(key)) == object_type::ARRAY;
}

bool document_view_t::is_array(const std::string &key) const {
    return get_type(key) == object_type::ARRAY;
}

bool document_view_t::is_array(uint32_t index) const {
    return get_type(index) == object_type::ARRAY;
}

bool document_view_t::is_dict(std::string &&key) const {
    return get_type(std::move(key)) == object_type::MAP;
}

bool document_view_t::is_dict(const std::string &key) const {
    return get_type(key) == object_type::MAP;
}

bool document_view_t::is_dict(uint32_t index) const {
    return get_type(index) == object_type::MAP;
}

object_handle document_view_t::get(std::string &&key) const {
    auto field = index_ ? index_->get(std::move(key)) : array_->get(static_cast<uint32_t>(std::atol(key.c_str())));
    if (field && field->type() == value_type::array) {
        auto field_array = field->as_array();
        return get_value(structure::get_attribute(field_array, structure::attribute::offset)->as_unsigned(),
                         structure::get_attribute(field_array, structure::attribute::size)->as_unsigned());
    }
    return object_handle();
}

object_handle document_view_t::get(const std::string &key) const {
    auto field = index_ ? index_->get(key) : array_->get(static_cast<uint32_t>(std::atol(key.c_str())));
    if (field && field->type() == value_type::array) {
        auto field_array = field->as_array();
        return get_value(structure::get_attribute(field_array, structure::attribute::offset)->as_unsigned(),
                         structure::get_attribute(field_array, structure::attribute::size)->as_unsigned());
    }
    return object_handle();
}

object_handle document_view_t::get(uint32_t index) const {
    auto field = array_->get(index);
    if (field && field->type() == value_type::array) {
        auto field_array = field->as_array();
        return get_value(structure::get_attribute(field_array, structure::attribute::offset)->as_unsigned(),
                         structure::get_attribute(field_array, structure::attribute::size)->as_unsigned());
    }
    return object_handle();
}

bool document_view_t::get_bool(std::string &&key) const {
    return get_as<bool>(std::move(key));
}

ulong document_view_t::get_ulong(std::string &&key) const {
    return get_as<ulong>(std::move(key));
}

long document_view_t::get_long(std::string &&key) const {
    return get_as<long>(std::move(key));
}

float document_view_t::get_float(std::string &&key) const {
    return get_as<float>(std::move(key));
}

double document_view_t::get_double(std::string &&key) const {
    return get_as<double>(std::move(key));
}

std::string document_view_t::get_string(std::string &&key) const {
    return get_as<std::string>(std::move(key));
}

document_view_t document_view_t::get_array(std::string &&key) const {
    if (index_) return document_view_t(index_->get(std::move(key))->as_array(), storage_);
    return document_view_t(array_->get(static_cast<uint32_t>(std::atol(key.c_str())))->as_array(), storage_);
}

document_view_t document_view_t::get_array(const std::string &key) const {
    if (index_) return document_view_t(index_->get(key)->as_array(), storage_);
    return document_view_t(array_->get(static_cast<uint32_t>(std::atol(key.c_str())))->as_array(), storage_);
}

document_view_t document_view_t::get_array(uint32_t index) const {
    return document_view_t(array_->get(index)->as_array(), storage_);
}

document_view_t document_view_t::get_dict(std::string &&key) const {
    if (index_) return document_view_t(index_->get(std::move(key))->as_dict(), storage_);
    return document_view_t(array_->get(static_cast<uint32_t>(std::atol(key.c_str())))->as_dict(), storage_);
}

document_view_t document_view_t::get_dict(const std::string &key) const {
    if (index_) return document_view_t(index_->get(key)->as_dict(), storage_);
    return document_view_t(array_->get(static_cast<uint32_t>(std::atol(key.c_str())))->as_dict(), storage_);
}

document_view_t document_view_t::get_dict(uint32_t index) const {
    return document_view_t(array_->get(index)->as_dict(), storage_);
}

document_view_t::iterator_t document_view_t::begin() const {
    return index_->begin();
}

template <class T>
compare_t equals_(const document_view_t &doc1, const document_view_t &doc2, const std::string &key) {
    T v1 = doc1.get_as<T>(key);
    T v2 = doc2.get_as<T>(key);
    if (v1 < v2) return compare_t::less;
    if (v1 > v2) return compare_t::more;
    return compare_t::equals;
}

compare_t document_view_t::compare(const document_view_t &other, const std::string &key) const {
    if (is_exists(key) && !other.is_exists(key)) return compare_t::less;
    if (!is_exists(key) && other.is_exists(key))  return compare_t::more;
    if (!is_exists(key) && !other.is_exists(key)) return compare_t::equals;
    if (is_bool(key) && other.is_bool(key)) return equals_<bool>(*this, other, key);
    if (is_ulong(key) && other.is_ulong(key)) return equals_<ulong>(*this, other, key);
    if (is_long(key) && other.is_long(key)) return equals_<long>(*this, other, key);
    if (is_float(key) && other.is_float(key)) return equals_<float>(*this, other, key);
    if (is_double(key) && other.is_double(key)) return equals_<double>(*this, other, key);
    if (is_string(key) && other.is_string(key)) return equals_<std::string>(*this, other, key);
    return compare_t::equals;
}

std::string document_view_t::to_json() const {
    if (index_) return to_json_dict();
    if (array_) return to_json_array();
    return std::string();
}

::document::retained_t<::document::impl::dict_t> document_view_t::to_dict() const {
    //todo erase serialize to json
    auto doc = ::document::impl::doc_t::from_json(to_json());
    return ::document::impl::mutable_dict_t::new_dict(doc->root()->as_dict());
}

document_view_t::document_view_t(document_view_t::array_t array, document_view_t::storage_t storage)
    : index_(nullptr)
    , array_(array)
    , storage_(storage) {
}

object_type document_view_t::get_type(const ::document::impl::value_t *field) const {
    if (field != nullptr) {
        if (field->type() == value_type::dict) {
            return object_type::MAP;
        } else if (field->type() == value_type::array) {
            auto array = field->as_array();
            if (array->count() > 0 && (array->get(0)->type() == value_type::array || array->get(0)->type() == value_type::dict)) {
                return object_type::ARRAY;
            } else {
                return static_cast<object_type>(structure::get_attribute(field, structure::attribute::type)->as_int());
            }
        }
    }
    return object_type::NIL;
}

object_type document_view_t::get_type(std::string &&key) const {
    if (index_) return get_type(index_->get(std::move(key)));
    return get_type(array_->get(static_cast<uint32_t>(std::atol(key.c_str()))));
}

object_type document_view_t::get_type(const std::string &key) const {
    if (index_) return get_type(index_->get(key));
    return get_type(array_->get(static_cast<uint32_t>(std::atol(key.c_str()))));
}

object_type document_view_t::get_type(uint32_t index) const {
    return get_type(array_->get(index));
}

object_handle document_view_t::get_value(offset_t offset, std::size_t size) const {
    assert(offset + size <= storage_->size());
    auto data = storage_->data() + offset;
    return msgpack::unpack(data, size);
}

std::string document_view_t::to_json_dict() const {
    std::stringstream res;
    for (auto it = index_->begin(); it; ++it) {
        if (!res.str().empty()) res << ",";
        auto key = static_cast<std::string>(it.key()->as_string());
        if (is_dict(key)) {
            res << "\"" << key << "\"" << ":" << get_dict(std::move(key)).to_json();
        } else if (is_array(key)) {
            res << "\"" << key << "\"" << ":" << get_array(std::move(key)).to_json();
        } else {
            res << "\"" << key << "\"" << ":" << *get(std::move(key));
        }
    }
    return "{" + res.str() + "}";
}

std::string document_view_t::to_json_array() const {
    std::stringstream res;
    for (uint32_t index = 0; index < array_->count(); ++index) {
        if (!res.str().empty()) res << ",";
        if (is_dict(index)) {
            res << get_dict(index).to_json();
        } else if (is_array(index)) {
            res << get_array(index).to_json();
        } else {
            res << *get(index);
        }
    }
    return "[" + res.str() + "]";
}

}

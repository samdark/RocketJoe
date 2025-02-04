#pragma once
#include "conditional_expression.hpp"
#include <components/document/core/array.hpp>
#include <cmath>
#include <regex>
#include <utility>

namespace components::parser {

using ::document::impl::value_type;
using ::document::impl::value_t;
using ::document::impl::array_t;

/// SIMPLES VALUES ///

enum class condition_type : uint8_t {
    novalid,
    eq,
    ne,
    gt,
    lt,
    gte,
    lte,
    regex,
    any,
    all,
    union_and,
    union_or,
    union_not
};

template <class T>
class find_condition_simple : public conditional_expression {
public:
    find_condition_simple() = delete;
    explicit find_condition_simple(const std::string &key)
        : conditional_expression(key) {
    }

    find_condition_simple(const std::string &key, const T &value)
        : conditional_expression(key)
        , value_(value) {
    }

    void set_value(const value_t *value) override {
        value_ = value->as<T>();
    }

    bool check_document(const document_view_t &doc) const final {
        return check_value(doc.get_as<T>(this->key_));
    }

protected:
    T value_;

    virtual bool check_value(const T& value) const = 0;
};


template<class T> inline bool condition_eq(T v1, T v2) {
    return v1 == v2;
}
template<> inline bool condition_eq<double>(double v1, double v2) {
    return std::fabs(v1 - v2) < std::numeric_limits<double>::epsilon();
}


template <class T>
class find_condition_eq : public find_condition_simple<T> {
public:
    using find_condition_simple<T>::find_condition_simple;

private:
    bool check_value(const T& value) const final override {
        return condition_eq<T>(value, this->value_);
    }
};


template <class T>
class find_condition_ne : public find_condition_simple<T> {
public:
    using find_condition_simple<T>::find_condition_simple;

private:
    bool check_value(const T& value) const final override {
        return !condition_eq<T>(value, this->value_);
    }
};


template <class T>
class find_condition_gt : public find_condition_simple<T> {
public:
    using find_condition_simple<T>::find_condition_simple;

private:
    bool check_value(const T& value) const final override {
        return value > this->value_;
    }
};


template <class T>
class find_condition_lt : public find_condition_simple<T> {
public:
    using find_condition_simple<T>::find_condition_simple;

private:
    bool check_value(const T& value) const final override {
        return value < this->value_;
    }
};


template <class T>
class find_condition_gte : public find_condition_simple<T> {
public:
    using find_condition_simple<T>::find_condition_simple;

private:
    bool check_value(const T& value) const final override {
        return value >= this->value_;
    }
};


template <class T>
class find_condition_lte : public find_condition_simple<T> {
public:
    using find_condition_simple<T>::find_condition_simple;

private:
    bool check_value(const T& value) const final override {
        return value <= this->value_;
    }
};


template <class T>
class find_condition_between : public find_condition_simple<T> {
public:
    find_condition_between(const std::string &key, const T &value1, const T &value2)
        : find_condition_simple<T>(key, value1)
        , value2_(value2) {}

private:
    T value2_;

    bool check_value(const T& value) const final override {
        return this->value_ <= value && value <= this->value2_;
    }
};


class find_condition_regex : public find_condition_simple<std::string> {
public:
    using find_condition_simple<std::string>::find_condition_simple;

private:
    bool check_value(const std::string& value) const final override {
        return std::regex_match(value, std::regex(".*" + this->value_ + ".*"));
    }
};


/// ARRAY VALUES ///

template <class T>
class find_condition_array : public conditional_expression {
public:
    find_condition_array() = delete;
    explicit find_condition_array(std::string key)
        : key_(std::move(key)) {}

    find_condition_array(std::string key, const std::vector<T> &values)
        : key_(std::move(key))
        , values_(values) {}

    find_condition_array(std::string key, const value_t *value)
        : key_(std::move(key)) {
        set_value(value);
    }

    void set_value(const value_t *value) override {
        values_.clear();
        if (value->type() == value_type::array) {
            auto values = value->as_array();
            values_.reserve(values->count());
            for (auto it = values->begin(); it; ++it) {
                values_.emplace_back(it.value()->as<T>());
            }
        } else {
            values_.emplace_back(value->as<T>());
        }
    }

    bool check_document(const document_view_t &doc) const final {
        if (doc.is_array(this->key_)) {
            return check_array(doc.get_array(this->key_));
        }
        return check_value(doc.get_as<T>(this->key_));
    }

protected:
    std::string key_;
    std::vector<T> values_;

    bool check_value(const T& value) const {
        return std::find(values_.cbegin(), values_.cend(), value) != values_.cend();
    };

    virtual bool check_array(const document_view_t& array) const = 0;
    virtual bool check_array(const array_t *array) const = 0;
};


template <class T>
class find_condition_any : public find_condition_array<T> {
public:
    using find_condition_array<T>::find_condition_array;

private:
    bool check_array(const document_view_t& array) const final {
        for (uint32_t i = 0; i < array.count(); ++i) {
            if (array.is_array(i)) {
                if (check_array(array.get_array(i))) {
                    return true;
                }
            } else if (this->check_value(array.get_as<T>(i))) {
                return true;
            }
        }
        return false;
    }

    bool check_array(const array_t *array) const final {
        for (auto it = array->begin(); it; ++it) {
            if (it.value()->type() == value_type::array) {
                if (check_array(it.value()->as_array())) {
                    return true;
                }
            } else if (this->check_value(it.value()->as<T>())) {
                return true;
            }
        }
        return false;
    }
};


template <class T>
class find_condition_all : public find_condition_array<T> {
public:
    using find_condition_array<T>::find_condition_array;

private:
    bool check_array(const document_view_t& array) const final {
        for (uint32_t i = 0; i < array.count(); ++i) {
            if (array.is_array(i)) {
                if (!check_array(array.get_array(i))) {
                    return false;
                }
            } else if (!this->check_value(array.get_as<T>(i))) {
                return false;
            }
        }
        return true;
    }

    bool check_array(const array_t *array) const final {
        for (auto it = array->begin(); it; ++it) {
            if (it.value()->type() == value_type::array) {
                if (!check_array(it.value()->as_array())) {
                    return false;
                }
            } else if (!this->check_value(it.value()->as<T>())) {
                return false;
            }
        }
        return true;
    }
};


/// COMPLEX VALUES ///

class find_condition_and : public find_condition_t {
public:
    using find_condition_t::find_condition_t;
    bool check_document(const document_view_t &doc) const final;
private:
    template <class T> bool check_document_(const T &doc) const;
};


class find_condition_or : public find_condition_t {
public:
    using find_condition_t::find_condition_t;
    bool check_document(const document_view_t &doc) const final;
private:
    template <class T> bool check_document_(const T &doc) const;
};


class find_condition_not : public find_condition_t {
public:
    using find_condition_t::find_condition_t;
    bool check_document(const document_view_t &doc) const final;
private:
    template <class T> bool check_document_(const T &doc) const;
};


conditional_expression_ptr make_find_condition(condition_type type, const std::string &key, const value_t *value);

}

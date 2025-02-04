#pragma once

#include <components/document/support/exception.hpp>
#include <components/document/support/num_conversion.hpp>
#include <components/document/core/value.hpp>
#include <components/document/core/encoder.hpp>
#include <components/document/core/slice.hpp>
#include <stdio.h>

namespace boost { namespace json {
class value;
} }

namespace document { namespace impl {

class json_coder {
public:
    static alloc_slice_t from_json(encoder_t &enc, slice_t json);
    static alloc_slice_t from_json(slice_t json, shared_keys_t *sk = nullptr);
};


class json_encoder_t {
public:
    json_encoder_t(size_t reserve_output_size = 256);

    void set_canonical(bool canonical);

    bool empty() const;
    size_t bytes_written_size() const;

    alloc_slice_t finish();

    void reset();

    void write_null();
    void write_bool(bool b);

    void write_int(int64_t i);
    void write_uint(uint64_t i);
    void write_float(float f);
    void write_double(double d);

    void write_string(const std::string &s);
    void write_string(slice_t s);
    void write_date_string(int64_t timestamp, bool utc);

    void write_data(slice_t d);
    void write_value(const value_t *v);

    void write_json(slice_t json);
    void write_raw(slice_t raw);

    void begin_array();
    void end_array();

    void begin_dict();
    void end_dict();

    void write_key(slice_t s);
    void write_key(const std::string &s);
    void write_key(const value_t *v);

    json_encoder_t& operator<< (long long i);
    json_encoder_t& operator<< (unsigned long long i);
    json_encoder_t& operator<< (long i);
    json_encoder_t& operator<< (unsigned long i);
    json_encoder_t& operator<< (int i);
    json_encoder_t& operator<< (unsigned int i);
    json_encoder_t& operator<< (double d);
    json_encoder_t& operator<< (float f);
    json_encoder_t& operator<< (const std::string &str);
    json_encoder_t& operator<< (slice_t s);
    json_encoder_t& operator<< (const value_t *v);

    void begin_array(size_t);
    void begin_dict(size_t);
    void write_undefined();

private:
    void write_dict(const dict_t *dict);
    template <class T> void _write_int(const char *fmt, T t);
    template <class T> void _write_float(T t);
    void comma();

    writer_t _out;
    bool _canonical {false};
    bool _first {true};
};


} }

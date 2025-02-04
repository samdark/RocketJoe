#pragma once

#include <components/document/mutable/value_slot.hpp>
#include <components/document/core/dict.hpp>
#include <components/document/core/shared_keys.hpp>
#include <deque>
#include <map>

namespace document { namespace impl {
class encoder_t;
class shared_keys_t;
} }

namespace document { namespace impl { namespace internal {

class heap_array_t;

class heap_dict_t : public heap_collection_t {
public:
    using key_map_t = std::map<key_t, value_slot_t>;

    class iterator {
    public:
        iterator(const heap_dict_t *dict NONNULL) noexcept;
        iterator(const mutable_dict_t *dict NONNULL) noexcept;

        uint32_t count() const noexcept;
        slice_t key_string() const noexcept;
        const value_t* value() const noexcept;

        explicit operator bool() const noexcept;
        iterator& operator ++();

    private:
        void get_source();
        void get_new();
        void decode_key(key_t key);

        slice_t _key;
        const value_t* _value;
        dict_t::iterator _source_iter;
        key_map_t::const_iterator _new_iter, _new_end;
        bool _source_active, _new_active;
        key_t _source_key;
        uint32_t _count;
        shared_keys_t* _shared_keys;
    };


    heap_dict_t(const dict_t *dict = nullptr);

    static mutable_dict_t* as_mutable_dict(heap_dict_t *a);
    mutable_dict_t* as_mutable_dict() const;

    const dict_t* source() const;
    shared_keys_t* shared_keys() const;
    uint32_t count() const;
    bool empty() const;

    const value_t* get(slice_t key_to_find) const noexcept;
    const value_t* get(int key_to_find) const noexcept;
    const value_t* get(dict_t::key_t &key_to_find) const noexcept;
    const value_t* get(const key_t &key_to_find) const noexcept;

    template <typename T>
    void set(slice_t key, T value)  { setting(key).set(value); }

    value_slot_t& setting(slice_t str_key);

    void remove(slice_t str_key);
    void remove_all();

    mutable_array_t* get_mutable_array(slice_t key);
    mutable_dict_t* get_mutable_dict(slice_t key);

    void disconnect_from_source();
    void copy_children(copy_flags flags);

    void write_to(encoder_t &enc);

protected:
    friend class document::impl::array_t;
    friend class document::impl::mutable_dict_t;

    ~heap_dict_t() = default;
    heap_array_t* array_key_value();

private:
    key_t encode_key(slice_t) const noexcept;
    void mark_changed();
    key_t _allocate_key(key_t key);
    value_slot_t* _find_value_for(slice_t key_to_find) const noexcept;
    value_slot_t* _find_value_for(key_t key_to_find) const noexcept;
    value_slot_t& _make_value_for(key_t key);
    heap_collection_t* get_mutable(slice_t str_key, tags if_type);
    bool too_many_ancestors() const;

    uint32_t _count {0};
    retained_const_t<dict_t> _source;
    retained_t<shared_keys_t> _shared_keys;
    key_map_t _map;
    std::deque<alloc_slice_t> _backing_slices;
    retained_t<heap_array_t> _iterable;
};

} } }

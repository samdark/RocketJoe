#pragma once
#include <document/document_view.hpp>
#include <functional>
#include <memory>

using ::components::document::document_view_t;

namespace services::storage::sort {

    using components::document::compare_t;

    enum class order {
        descending = -1,
        ascending = 1
    };

    class sorter_t {
        using function_t = std::function<compare_t(const document_view_t*, const document_view_t*)>;

    public:
        explicit sorter_t() = default;
        explicit sorter_t(const std::string& key, order order_ = order::ascending);

        void add(const std::string& key, order order_ = order::ascending);
        bool operator()(const document_view_t* doc1, const document_view_t* doc2) const {
            for (auto f : functions_) {
                auto res = f(doc1, doc2);
                if (res < compare_t::equals) {
                    return true;
                } else if (res > compare_t::equals) {
                    return false;
                }
            }
            return true;
        }

    private:
        std::vector<function_t> functions_;
    };

} // namespace services::storage::sort

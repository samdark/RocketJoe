#pragma once
#include "conditional_expression.hpp"
#include "document/document.hpp"

namespace components::parser {

find_condition_ptr parse_find_condition(const document_view_t &condition);
find_condition_ptr parse_find_condition(const document_ptr &condition);

}

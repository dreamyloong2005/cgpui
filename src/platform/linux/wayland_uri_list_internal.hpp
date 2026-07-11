#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

[[nodiscard]] std::vector<std::string> parse_uri_list(std::string_view payload);

} // namespace cgpui

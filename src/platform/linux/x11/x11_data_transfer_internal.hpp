#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui {

[[nodiscard]] std::string x11_encode_uri_list(
    std::span<const std::string> paths);
[[nodiscard]] std::vector<std::string> x11_parse_uri_list(
    std::string_view payload);

}  // namespace cgpui

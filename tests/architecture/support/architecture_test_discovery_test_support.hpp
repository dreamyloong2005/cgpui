#pragma once

#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace cgpui_test {
// Deliberately narrow suite grammar: literal root tests, one-level owner
// *_structure_test.cpp groups, and explicitly listed support sources. Unknown
// syntax is rejected, never silently interpreted differently from mcpp.
std::unordered_set<std::string> architecture_test_discovery(
    std::string_view manifest, const std::vector<std::string>& cpp_sources);
std::unordered_set<std::string> architecture_registered_sources(
    const std::filesystem::path& root);
} // namespace cgpui_test

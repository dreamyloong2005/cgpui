#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace architecture_test {

// Read ordinary/raw string tokens and join adjacent literals, skipping comments
// and character constants. Iterative scanning keeps long source strings bounded.
std::vector<std::string> cpp_string_literals(std::string_view source);

} // namespace architecture_test

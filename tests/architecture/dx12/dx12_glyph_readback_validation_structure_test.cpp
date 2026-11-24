#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::string& path) {
  std::ifstream file(root / path);
  if (!file) throw std::runtime_error("missing glyph validation leaf: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& s, const char* token) { return s.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
bool before(const std::string& s, const char* a, const char* b) {
  const auto x = s.find(a), y = s.find(b);
  return x != std::string::npos && y != std::string::npos && x < y;
}
}
int main() {
  try {
    const auto helper = read("tests/renderer-dx12/dx12_glyph_readback_test_support.cpp");
    require(before(helper, "if (!completed)", "job->buffer->Map(") &&
        before(helper, "job->buffer->Unmap(", "dx12_check_debug_errors(state)") &&
        before(helper, "dx12_check_debug_errors(state)", "return result;"),
        "whole-page bytes cannot escape before actual completion, Unmap and debug validation");
    require(has(helper, "(void)job.release()"), "unknown-GPU readback owners must not be force-freed");
    const auto fault = read("tests/renderer-dx12/dx12_glyph_readback_debug_gate_test.cpp");
    require(has(fault, "AddApplicationMessage(D3D12_MESSAGE_SEVERITY_ERROR") &&
        has(fault, "only_expected_error(") && has(fault, "const auto denied = glyph_readback(") &&
        !has(fault, "dx12_fail(40,"), "debug gate control must use real info queue and ordinary failure codes");
    require(has(read("tests/renderer-dx12/mcpp.toml"), "dx12_glyph_readback_debug_gate_test.cpp") &&
        has(read("scripts/ci/architecture-header-targets.txt"), "dx12_glyph_readback_validation_structure_test|"),
        "new debug acceptance/structure controls must be registered");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

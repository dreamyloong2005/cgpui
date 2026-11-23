#include "support/architecture_test_discovery_test_support.hpp"
#include <iostream>
#include <stdexcept>

namespace {
const std::string manifest = R"manifest([build]
sources = [
    "support/discovery_test_support.cpp",
]
[test]
discover = [
    "root_structure_test.cpp",
    "dx12/*_structure_test.cpp",
]
)manifest";
const std::vector<std::string> inventory{
    "root_structure_test.cpp", "dx12/dx12_a_structure_test.cpp",
    "dx12/dx12_b_structure_test.cpp", "support/discovery_test_support.cpp"};
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
std::string changed(const std::string& from, const std::string& to) {
  auto value = manifest;
  value.replace(value.find(from), from.size(), to);
  return value;
}
void rejects(const std::string& source, const std::vector<std::string>& files) {
  bool rejected = false;
  try { (void)cgpui_test::architecture_test_discovery(source, files); }
  catch (const std::runtime_error&) { rejected = true; }
  require(rejected, "invalid registration/inventory unexpectedly accepted");
}
} // namespace

int main() {
  try {
    const auto found = cgpui_test::architecture_test_discovery(manifest, inventory);
    require(found == std::unordered_set<std::string>{"root_structure_test.cpp",
                "dx12/dx12_a_structure_test.cpp", "dx12/dx12_b_structure_test.cpp"},
            "owner group must discover each program, not its support implementation");
    rejects(changed("    \"root_structure_test.cpp\",", "    # \"root_structure_test.cpp\","), inventory);
    rejects(changed("    \"root_structure_test.cpp\",", "    \"root_structure_test.cpp\",\n    \"root_structure_test.cpp\","), inventory);
    rejects(changed("dx12/*_structure_test.cpp", "../*_structure_test.cpp"), inventory);
    rejects(changed("dx12/*_structure_test.cpp", "**/*.cpp"), inventory);
    rejects(changed("dx12/*_structure_test.cpp", "empty/*_structure_test.cpp"), inventory);
    rejects(changed("root_structure_test.cpp", "missing_structure_test.cpp"), inventory);
    rejects(manifest + "programs = \"directory\"\n", inventory);
    rejects(manifest + "[target.'cfg(os = \"windows\")'.test]\ndiscover = []\n", inventory);
    rejects(manifest.substr(0, manifest.rfind(']')), inventory);
    auto extra = inventory;
    extra.push_back("unregistered_structure_test.cpp");
    rejects(manifest, extra);
    extra = inventory;
    extra.push_back("support/unregistered_test_support.cpp");
    rejects(manifest, extra);
    extra = inventory;
    extra.push_back("dx12/root_structure_test.cpp");
    rejects(manifest, extra);
    auto missing = inventory;
    missing.pop_back();
    rejects(manifest, missing);
    rejects(changed("sources = [\n    \"support/discovery_test_support.cpp\",\n]", "sources = []"), inventory);
    require(cgpui_test::architecture_test_discovery(
                "[build]\nsources = []\n[test]\ndiscover = [\n    \"one_test.cpp\", # real entry\n]\n",
                {"one_test.cpp"}).contains("one_test.cpp"), "empty support list and trailing comments remain valid");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

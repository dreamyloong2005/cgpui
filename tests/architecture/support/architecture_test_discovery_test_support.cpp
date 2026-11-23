#include "architecture_test_discovery_test_support.hpp"
#include <fstream>
#include <iterator>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace cgpui_test {
namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
std::string trim(std::string value) {
  const auto first = value.find_first_not_of(" \t\r");
  return first == std::string::npos ? std::string{} :
      value.substr(first, value.find_last_not_of(" \t\r") - first + 1);
}
std::vector<std::string> array(std::string_view manifest, const char* section, const char* key) {
  std::istringstream lines{std::string(manifest)};
  const std::regex opening(std::string("^") + key + R"(\s*=\s*\[(.*)$)");
  const std::regex entry(R"entry(^"([a-z0-9_./*]+\.cpp)"\s*,?$)entry");
  std::string line;
  std::vector<std::string> values;
  bool selected = false, found = false, in_array = false;
  while (std::getline(lines, line)) {
    // The accepted source-path alphabet cannot contain '#'. Comment text must
    // never manufacture a discovery entry or satisfy a missing registration.
    line = trim(line.substr(0, line.find('#')));
    if (line.empty()) continue;
    if (in_array) {
      if (line == "]") { in_array = false; continue; }
      std::smatch match;
      require(std::regex_match(line, match, entry), "invalid suite array entry; keep one source per line");
      values.push_back(match[1].str());
    } else if (line.front() == '[') {
      if (std::string_view(section) == "test")
        require(!line.ends_with(".test]"), "conditional discovery is outside this suite's explicit grammar");
      selected = line == std::string("[") + section + "]";
    } else if (selected) {
      std::smatch match;
      if (!std::regex_match(line, match, opening)) {
        require(std::string_view(section) != "test", "unsupported suite test option");
        continue;
      }
      require(!found, "duplicate suite array declaration");
      found = true;
      const auto tail = trim(match[1].str());
      require(tail.empty() || tail == "]", "suite arrays must be empty or one source per line");
      in_array = tail.empty();
    }
  }
  require(found && !in_array, "missing or unterminated suite array declaration");
  return values;
}
std::string read(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("missing suite manifest: " + path.generic_string());
  return {std::istreambuf_iterator<char>(input), {}};
}
} // namespace

std::unordered_set<std::string> architecture_test_discovery(
    std::string_view manifest, const std::vector<std::string>& cpp_sources) {
  const std::regex literal(R"([a-z][a-z0-9_]*\.cpp)");
  const std::regex owner_group(R"(([a-z][a-z0-9_]*)/\*_structure_test\.cpp)");
  const std::regex support(R"(support/[a-z][a-z0-9_]*_test_support\.cpp)");
  std::unordered_set<std::string> inventory(cpp_sources.begin(), cpp_sources.end());
  require(inventory.size() == cpp_sources.size(), "duplicate source inventory entry");
  std::unordered_set<std::string> tests, support_sources, names;
  for (const auto& pattern : array(manifest, "test", "discover")) {
    std::vector<std::string> matches;
    std::smatch group;
    if (std::regex_match(pattern, literal)) {
      require(inventory.contains(pattern), "explicitly registered test file is missing");
      matches.push_back(pattern);
    } else {
      require(std::regex_match(pattern, group, owner_group), "unsupported suite test-discovery pattern");
      const auto directory = group[1].str() + "/";
      for (const auto& source : cpp_sources) {
        if (source.starts_with(directory) && source.find('/', directory.size()) == std::string::npos &&
            source.ends_with("_structure_test.cpp")) matches.push_back(source);
      }
      require(!matches.empty(), "declared architecture owner group is empty");
    }
    for (const auto& source : matches) {
      require(tests.insert(source).second, "test is registered by more than one discovery entry");
      const auto name = std::filesystem::path(source).stem().string();
      require(names.insert(name).second, "discovered test program names collide");
    }
  }
  for (const auto& source : array(manifest, "build", "sources")) {
    require(std::regex_match(source, support) && inventory.contains(source), "invalid or missing explicit test support");
    require(!tests.contains(source) && support_sources.insert(source).second, "duplicate or discovered support source");
  }
  require(tests.size() + support_sources.size() == inventory.size(), "unregistered C++ test/support source in suite");
  return tests;
}

std::unordered_set<std::string> architecture_registered_sources(const std::filesystem::path& root) {
  namespace fs = std::filesystem;
  std::unordered_set<std::string> registered, names;
  for (const auto* suite : {"tests/architecture", "tests/header_cleanliness"}) {
    std::vector<std::string> inventory;
    for (const auto& entry : fs::recursive_directory_iterator(root / suite)) {
      if (entry.is_regular_file() && entry.path().extension() == ".cpp")
        inventory.push_back(entry.path().lexically_relative(root / suite).generic_string());
    }
    for (const auto& file : architecture_test_discovery(read(root / suite / "mcpp.toml"), inventory)) {
      require(names.insert(fs::path(file).stem().string()).second, "test program name collides across suites");
      registered.insert(std::string(suite) + "/" + file);
    }
  }
  return registered;
}
} // namespace cgpui_test

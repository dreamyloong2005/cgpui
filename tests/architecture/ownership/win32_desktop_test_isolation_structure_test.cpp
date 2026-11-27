#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
std::string read(const char* path, std::size_t cap = 3000) {
  std::ifstream stream(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!stream) throw std::runtime_error(std::string("missing desktop-test owner: ") + path);
  std::string source{std::istreambuf_iterator<char>(stream), {}};
  require(static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) <= cap,
          "desktop-test owner exceeds its focused line budget");
  return source;
}
// Ignore comments and literal contents: explanatory prose cannot satisfy code checks.
std::string code(std::string source) {
  for (const auto splice : {std::string{"\\\r\n"}, std::string{"\\\n"}})
    for (auto at = source.find(splice); at != source.npos; at = source.find(splice, at))
      source.erase(at, splice.size());
  std::string result;
  for (std::size_t at = 0; at < source.size();) {
    const std::string_view tail(source.data() + at, source.size() - at);
    if (tail.starts_with("//") || tail.starts_with("/*")) {
      const bool line = tail.starts_with("//");
      const auto end = source.find(line ? "\n" : "*/", at + 2);
      at = end == source.npos ? source.size() : end + (line ? 1 : 2);
    } else if (tail.starts_with("R\"")) {
      const auto open = source.find('(', at + 2);
      require(open != source.npos, "unterminated raw literal in inspected source");
      const auto close = ")" + source.substr(at + 2, open - at - 2) + "\"";
      const auto end = source.find(close, open + 1);
      require(end != source.npos, "unterminated raw literal in inspected source");
      at = end + close.size();
    } else if (source[at] == '"' || source[at] == '\'') {
      const char quote = source[at++];
      while (at < source.size()) {
        const char value = source[at++];
        if (value == '\\' && at < source.size()) ++at;
        else if (value == quote) break;
      }
    } else {
      const char value = source[at++];
      if (!std::isspace(static_cast<unsigned char>(value))) result += value;
    }
  }
  return result;
}
bool has(std::string_view source, std::string_view token) { return source.find(token) != source.npos; }
void precedes(std::string_view source, std::string_view first, std::string_view second) {
  const auto a = source.find(first), b = source.find(second);
  require(a != source.npos && b != source.npos && a < b,
          "desktop ownership/readiness must precede the activity being bounded");
}
void windows_only(const char* manifest, const char* owner) {
  std::istringstream lines(read(manifest));
  std::string section, line;
  bool sources = false, found = false;
  while (std::getline(lines, line)) {
    line.erase(line.find('#') == line.npos ? line.size() : line.find('#'));
    line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); }), line.end());
    if (line.empty()) continue;
    if (line.front() == '[') { section = line; sources = false; }
    else if (line == "sources=[") sources = true;
    else if (line == "]") sources = false;
    else if (line == "\"" + std::string(owner) + "\"," || line == "\"" + std::string(owner) + "\"") {
      require(sources && section == "[target.'cfg(os=\"windows\")'.build]" && !found,
              "native desktop-test implementation must be registered once, only in Windows sources");
      found = true;
    }
  }
  require(found, "native desktop-test implementation is missing from its support library");
}
} // namespace

int main() {
  try {
    const auto header = code(read("tests/support/win32_desktop_test_support.hpp", 40));
    const auto owner = code(read("tests/support/win32_desktop_test_support.cpp", 60));
    require(has(header, "~Win32DesktopTestLease();") && has(header, "boolready()constnoexcept;") &&
                has(header, "explicitWin32DesktopTestLease(DesktopTestAccessaccess=DesktopTestAccess::exclusive);") &&
                !has(header, "CreateMutexW(") && !has(header, "WaitForSingleObject("),
            "shared lease is a declaration leaf, not an inline native implementation");
    require(has(owner, "Win32DesktopTestLease::Win32DesktopTestLease(") &&
                has(owner, "Win32DesktopTestLease::~Win32DesktopTestLease()") &&
                has(owner, "Win32DesktopTestLease::ready()") && has(owner, "ReleaseMutex("),
            "native desktop ownership belongs to one focused implementation");
    const auto wait_header = code(read("tests/renderer-dx12/dx12_loader_desktop_test_support.hpp", 20));
    const auto wait = code(read("tests/renderer-dx12/dx12_loader_desktop_test_support.cpp", 40));
    require(has(wait_header, "unsignedlongloader_wait_for_child(void*process,void*desktop_ready);") &&
                !has(wait_header, "WaitFor"), "loader waiting has a declaration-only leaf");
    for (const auto* manifest : {"tests/renderer/mcpp.toml", "tests/renderer-dx12/mcpp.toml"})
      windows_only(manifest, "../support/win32_desktop_test_support.cpp");
    windows_only("tests/renderer-dx12/mcpp.toml", "dx12_loader_desktop_test_support.cpp");
    const auto dx12 = code(read("tests/renderer-dx12/dx12_recovery_test_support.hpp"));
    const auto vulkan_header = code(read("tests/renderer/vulkan_device_recovery_scene_test_support.hpp"));
    const auto vulkan = code(read("tests/renderer/vulkan_device_recovery_scene_test_support.cpp"));
    require(has(dx12, "Win32DesktopTestLeasedesktop_;") && has(dx12, ":desktop_(access){") &&
                has(vulkan_header, "Win32DesktopTestLeasedesktop_;"), "watchdogs own their acquisition lifetime");
    for (const auto& watchdog : {dx12, vulkan}) {
      precedes(watchdog, "if(!desktop_.ready())return;", "CreateEventW(");
      precedes(watchdog, "if(!desktop_.ready())return;", "std::thread(");
    }
    const auto parent = code(read("tests/renderer-dx12/dx12_loader_process_support.hpp"));
    const auto launcher = code(read("tests/renderer-dx12/dx12_loader_isolation_test.cpp"));
    for (const auto& coordinator : {parent, launcher, wait})
      require(!has(coordinator, "Win32DesktopTestLease") && !has(coordinator, "Dx12RecoveryDeadline"),
              "loader parent/coordinator must not hold a lease while awaiting its child");
    require(has(parent, "loader_wait_for_child(child.value,desktop_ready.value)"),
            "loader process orchestration delegates queue/activity waiting to its focused owner");
    const auto child = code(read("tests/renderer-dx12/dx12_initialization_fallback_test_support.cpp"));
    precedes(child, "Dx12RecoveryDeadlinedeadline{DesktopTestAccess::exclusive};", "SetEvent(desktop_ready)");
    precedes(child, "if(!deadline.done)return1;", "SetEvent(desktop_ready)");
    precedes(wait, "WaitForMultipleObjects(", "WaitForSingleObject(process,20000)");
    require(has(wait, "constHANDLEsignals[]{desktop_ready,process};"),
            "queue waiting must observe both readiness and an early child exit");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

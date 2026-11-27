#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, std::string_view value) {
  return text.find(value) != std::string::npos;
}

std::string section_between(const std::string& text, std::string_view begin,
                            std::string_view end) {
  const std::size_t start = text.find(begin);
  const std::size_t finish = text.find(end, start);
  if (start == std::string::npos || finish == std::string::npos) return {};
  return text.substr(start, finish - start);
}

std::string line_containing(const std::string& text, std::string_view anchor) {
  const std::size_t index = text.find(anchor);
  if (index == std::string::npos) return {};
  const std::size_t begin = text.rfind('\n', index);
  const std::size_t end = text.find('\n', index);
  return text.substr(begin == std::string::npos ? 0 : begin + 1,
                     end == std::string::npos ? std::string::npos
                                              : end - (begin + 1));
}

std::string json_row(const std::string& text, std::string_view name) {
  const std::string anchor = "\"upstream_gpui\": \"" + std::string{name} + "\"";
  const std::size_t index = text.find(anchor);
  if (index == std::string::npos) return {};
  const std::size_t begin = text.rfind("    {", index);
  const std::size_t end = text.find("\n    }", index);
  if (begin == std::string::npos || end == std::string::npos) return {};
  return text.substr(begin, end - begin);
}

}  // namespace

int main() {
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string api_parity =
      read_source("docs/gpui-core-api-parity.md");
  const std::string mac_readiness =
      read_source("docs/platform-mac-readiness.md");
  const std::string* required[]{
      &ledger_md,
      &ledger_json,
      &api_parity,
      &mac_readiness};
  for (const auto* source : required) {
    if (source->empty()) return 1;
  }

  const std::string x11_md = line_containing(
      ledger_md, "| gpui_platform x11 feature | X11 + Vulkan |");
  const std::string x11_json =
      json_row(ledger_json, "gpui_platform x11 feature");
  const std::string platform_md =
      section_between(ledger_md, "## Platform Targets", "## Scope Guard");
  const std::string platform_json = section_between(
      ledger_json, "\"platform_targets\": {", "\"status_counts\": {");
  if (!contains(x11_md, "| Adapted |") ||
      !contains(x11_json, "\"status\": \"adapted\"") ||
      !contains(ledger_json, "\"required\": 0") ||
      !contains(ledger_json, "\"adapted\": 31") ||
      !contains(ledger_json, "\"deferred\": 0") ||
      !contains(ledger_json, "\"non_goal\": 1") ||
      !contains(platform_md, "| gpui_platform x11 feature | Required |") ||
      !contains(platform_json,
                "\"x11\": {\n      \"backend\": \"x11\",\n      "
                "\"renderer\": \"vulkan\",\n      \"status\": \"required\"")) {
    return 30;
  }

  if (!contains(ledger_json, "\"phase_i_status\"") ||
      !contains(ledger_json, "\"phase_i_required_x11_gaps\": 0")) {
    return 50;
  }

  if (!contains(api_parity,
                "Linux provides Wayland and X11 platform backends")) {
    return 52;
  }
  return 0;
}

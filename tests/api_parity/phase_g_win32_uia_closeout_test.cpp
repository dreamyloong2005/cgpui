#include <fstream>
#include <iterator>
#include <string>

namespace {

std::string read_source(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream source((root == nullptr ? std::string{"."} : root) + "/" + path);
  if (!source) source.open(path);
  return source ? std::string{std::istreambuf_iterator<char>(source), {}}
                : std::string{};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

} // namespace

int main() {
  const std::string provider = read_source(
      "tests/architecture/phase_g_win32_uia_provider_object_structure_test.cpp");
  const std::string navigation = read_source(
      "tests/architecture/phase_g_win32_uia_tree_navigation_structure_test.cpp");
  const std::string patterns = read_source(
      "tests/architecture/phase_g_win32_uia_pattern_provider_structure_test.cpp");
  const std::string events = read_source(
      "tests/architecture/phase_g_win32_uia_live_event_structure_test.cpp");
  const std::string integration = read_source(
      "tests/architecture/phase_g_win32_uia_change_integration_structure_test.cpp");
  const std::string lifetime = read_source(
      "tests/architecture/phase_g_win32_uia_provider_lifetime_structure_test.cpp");
  const std::string stress = read_source(
      "tests/architecture/phase_g_win32_uia_lifecycle_stress_structure_test.cpp");
  const std::string source_guard = read_source(
      "tests/architecture/win32_window_source_test.cpp");
  const std::string manifests = read_source("tests/api_parity/mcpp.toml") +
      read_source("tests/architecture/mcpp.toml") +
      read_source("tests/platform/mcpp.toml");
  const std::string* required[]{
      &provider,
      &navigation,
      &patterns,
      &events,
      &integration,
      &lifetime,
      &stress,
      &source_guard,
      &manifests};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(provider, "IRawElementProviderSimple") ||
      !contains(navigation, "IRawElementProviderFragmentRoot") ||
      !contains(patterns, "IRangeValueProvider") ||
      !contains(events, "UIA_Text_TextChangedEventId") ||
      !contains(integration, "request_keyboard_focus(integration_input_id)") ||
      !contains(lifetime, "UIA_E_ELEMENTNOTAVAILABLE") ||
      !contains(stress, "revision <= 128") ||
      !contains(stress, "cycle < 64")) return 2;
  const char* targets[]{
      "win32_uia_provider_object_test.cpp", "win32_uia_tree_navigation_test.cpp",
      "win32_uia_get_object_message_test.cpp", "win32_uia_pattern_provider_test.cpp",
      "win32_uia_live_event_test.cpp", "win32_uia_runtime_change_integration_test.cpp",
      "win32_uia_provider_lifetime_test.cpp", "win32_uia_lifecycle_stress_test.cpp",
      "phase_g_win32_uia_provider_object_structure_test.cpp",
      "phase_g_win32_uia_tree_navigation_structure_test.cpp",
      "phase_g_win32_uia_pattern_provider_structure_test.cpp",
      "phase_g_win32_uia_live_event_structure_test.cpp",
      "phase_g_win32_uia_change_integration_structure_test.cpp",
      "phase_g_win32_uia_provider_lifetime_structure_test.cpp",
      "phase_g_win32_uia_lifecycle_stress_structure_test.cpp"};
  for (const char* target : targets) {
    if (!contains(manifests, (std::string{"\""} + target + "\"").c_str())) {
      return 3;
    }
  }
  if (!contains(source_guard, "win32_uia_provider_state.cpp") ||
      !contains(source_guard, "win32_uia_range_pattern.cpp") ||
      !contains(source_guard, "win32_uia_tree_state.cpp")) return 4;
  if (!contains(manifests, "\"phase_g_win32_uia_closeout_test.cpp\"") ||
      !contains(manifests,
                "\"phase_g_win32_uia_closeout_test.cpp\"")) {
    return 5;
  }

  return 0;
}

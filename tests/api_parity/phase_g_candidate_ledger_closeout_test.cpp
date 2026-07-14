#include <array>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string read_source(const char* path) {
  const char* root = std::getenv("CGPUI_SOURCE_ROOT");
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
                     end == std::string::npos ? text.size() - index
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

std::size_t count(const std::string& text, std::string_view value) {
  std::size_t result = 0;
  for (std::size_t index = 0; (index = text.find(value, index)) != std::string::npos;
       index += value.size()) {
    ++result;
  }
  return result;
}

template <std::size_t Size>
bool rows_have_status(const std::string& markdown, const std::string& json,
                      const std::array<const char*, Size>& names,
                      std::string_view markdown_status,
                      std::string_view json_status) {
  for (const char* name : names) {
    const std::string markdown_row =
        line_containing(markdown, "| " + std::string{name} + " |");
    const std::string structured_row = json_row(json, name);
    if (markdown_row.empty() || structured_row.empty() ||
        !contains(markdown_row, markdown_status) ||
        !contains(structured_row, json_status)) {
      return false;
    }
  }
  return true;
}

} // namespace

int main() {
  const std::string self = read_source(
      "tests/api_parity/phase_g_candidate_ledger_closeout_test.cpp");
  const std::string predecessor =
      read_source("tests/architecture/phase_g_action_macro_structure_test.cpp");
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string* required[]{&self,       &predecessor, &xmake,    &roadmap,
                                &ledger_md,  &ledger_json, &task_plan, &findings};
  for (const auto* source : required) if (source->empty()) return 1;

  if (!contains(xmake, "target(\"phase_g_candidate_ledger_closeout_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_g_candidate_ledger_closeout_test.cpp") ||
      !contains(predecessor, "Step 676 candidate-ledger closeout audit")) {
    return 2;
  }

  const std::string platform_md =
      section_between(ledger_md, "## Platform Targets", "## Scope Guard");
  const std::string platform_json = section_between(
      ledger_json, "\"platform_targets\": {", "\"status_counts\": {");
  if (!contains(platform_md, "| gpui Windows backend | Required |") ||
      !contains(platform_md, "| gpui_platform wayland feature | Required |") ||
      !contains(platform_md, "| gpui macOS backend | Deferred |") ||
      !contains(platform_md, "| gpui_platform x11 feature | Deferred |") ||
      count(platform_json, "\"status\": \"required\"") != 2 ||
      count(platform_json, "\"status\": \"deferred\"") != 2 ||
      count(platform_json, "\"status\": \"non_goal\"") != 1) {
    return 3;
  }

  constexpr std::array adapted{
      "gpui_platform::application", "gpui::Application", "gpui::App",
      "gpui::Window", "gpui::WindowOptions", "gpui::Context<T>",
      "gpui::View<T>", "gpui::AsyncApp", "gpui::AsyncWindowContext",
      "gpui::TestAppContext", "gpui::Entity<T>", "gpui::Render",
      "gpui::IntoElement", "gpui::div", "gpui::prelude",
      "gpui::actions! / action macro", "gpui key_context",
      "gpui keymap dispatch", "gpui element styling", "gpui uniform_list",
      "gpui text system", "gpui image assets", "gpui SVG", "gpui animation",
      "gpui::test", "gpui accessibility", "gpui platform services",
      "gpui Windows backend", "gpui_platform wayland feature"};
  constexpr std::array deferred{"gpui_platform x11 feature",
                                "gpui macOS backend"};
  constexpr std::array non_goal{"gpui wasm backend"};
  const std::string candidates = section_between(
      ledger_md, "## Required Surface Summary", "## Phase D Text Evidence Addendum");
  if (!rows_have_status(candidates, ledger_json, adapted, "| Adapted |",
                        "\"status\": \"adapted\"") ||
      !rows_have_status(candidates, ledger_json, deferred, "| Deferred |",
                        "\"status\": \"deferred\"") ||
      !rows_have_status(candidates, ledger_json, non_goal, "| Non-goal |",
                        "\"status\": \"non_goal\"")) {
    return 4;
  }

  if (count(candidates, "| Required |") != 0 ||
      count(candidates, "| Adapted |") != adapted.size() ||
      count(candidates, "| Deferred |") != deferred.size() ||
      count(candidates, "| Non-goal |") != non_goal.size() ||
      count(ledger_json, "\"upstream_gpui\":") != 32 ||
      !contains(ledger_json, "\"required\": 0") ||
      !contains(ledger_json, "\"adapted\": 29")) {
    return 5;
  }

  constexpr const char* completion =
      "Phase G Step 676 closes the candidate ledger with zero required "
      "Windows/Linux candidate gaps, 29 adapted rows, deferred macOS and "
      "optional X11, and wasm as a non-goal, while active Windows/Linux "
      "platform targets remain required scope rather than unresolved rows. "
      "Step 677 final Windows/WSL verification is next.";
  const std::string* documents[]{&roadmap, &ledger_md, &ledger_json, &task_plan,
                                 &findings};
  for (const auto* document : documents) {
    if (!contains(*document, completion)) return 6;
  }
  if (!contains(ledger_json, "\"phase_g_step_676_sources\"") ||
      !contains(ledger_json,
                "\"phase_g_step_676_remaining_gap\": \"Step 677 final "
                "Windows/WSL verification\"") ||
      !contains(ledger_json,
                "\"phase_f_current_handoff\": \"Step 677 final "
                "Windows/WSL verification\"")) {
    return 7;
  }
  return 0;
}

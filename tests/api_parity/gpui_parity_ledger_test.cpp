#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::string source_root() {
  if (const char* root = std::getenv("CGPUI_SOURCE_ROOT"); root != nullptr) {
    return root;
  }
  return ".";
}

std::string read_source(const char* path) {
  const std::string root = source_root();
  std::ifstream source(root + "/" + path);
  if (!source) {
    source.open(path);
  }
  if (!source) {
    return {};
  }

  return std::string{
      std::istreambuf_iterator<char>(source),
      std::istreambuf_iterator<char>()};
}

bool contains(const std::string& text, const char* value) {
  return text.find(value) != std::string::npos;
}

int require_contains_all(
    const std::string& text,
    const std::vector<const char*>& values,
    int exit_code) {
  for (const char* value : values) {
    if (!contains(text, value)) {
      return exit_code;
    }
  }
  return 0;
}

} // namespace

int main() {
  const std::string pinned =
      read_source("docs/gpui-upstream-pinned-revision.md");
  if (pinned.empty()) {
    return 1;
  }
  if (const int result = require_contains_all(
          pinned,
          {
              "5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0",
              "gpui = 0.2.2",
              "gpui_platform = 0.1.0",
              "https://github.com/zed-industries/zed/tree/5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0/crates/gpui",
              "README.md",
              "src/gpui.rs",
              "docs/contexts.md",
              "docs/key_dispatch.md",
              "examples/hello_world.rs",
          },
          2);
      result != 0) {
    return result;
  }

  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  if (ledger.empty()) {
    return 3;
  }
  if (const int result = require_contains_all(
          ledger,
          {
              "# Complete GPUI Parity Ledger",
              "Pinned upstream revision",
              "Required",
              "Adapted",
              "Deferred",
              "Non-goal",
              "Application and app context",
              "Entities and state",
              "Views and Render",
              "Elements and styling",
              "Actions and key dispatch",
              "Platform services",
              "Async executor",
              "Test support",
              "Examples",
              "Windows: Win32 + Vulkan",
              "Linux: Wayland + Vulkan",
              "macOS: Cocoa + Metal",
              "X11",
          },
          4);
      result != 0) {
    return result;
  }
  if (!contains(ledger, "| upstream_gpui | cgpui_target | status |") ||
      !contains(ledger, "| gpui::Application |") ||
      !contains(ledger, "| gpui::App |") ||
      !contains(ledger, "| gpui::Window |") ||
      !contains(ledger, "| gpui::Context<T> |") ||
      !contains(ledger, "| gpui::View<T> |") ||
      !contains(ledger, "| gpui::Render |") ||
      !contains(ledger, "| gpui::div |") ||
      !contains(ledger, "| gpui::test |") ||
      !contains(ledger, "| gpui_platform::application |")) {
    return 5;
  }
  if (!contains(ledger, "`App` facade from `AppContext::app()`") ||
      !contains(ledger,
                "`Window` facade from `WindowRuntimeContext::window()`") ||
      !contains(ledger, "tests/api_parity/app_window_context_test.cpp")) {
    return 15;
  }
  if (!contains(ledger, "`Context<T>` alias with app/window/entity helpers") ||
      !contains(ledger, "`EntityHandle<T>`") ||
      !contains(ledger, "`ViewHandle<T>` and `WeakViewHandle<T>`") ||
      !contains(ledger, "`Render<T>` concept over") ||
      !contains(ledger, "`IntoElement` alias plus `into_element`") ||
      !contains(ledger, "include/cgpui/ui/render.hpp") ||
      !contains(ledger, "include/cgpui/ui/view_handle.hpp") ||
      !contains(ledger,
                "tests/api_parity/context_capabilities_test.cpp") ||
      !contains(ledger,
                "tests/api_parity/view_handle_spelling_test.cpp")) {
    return 16;
  }
  if (!contains(ledger, "| gpui_platform x11 feature | Deferred |") ||
      !contains(ledger, "| gpui_platform wayland feature | Required |") ||
      !contains(ledger, "| gpui Windows backend | Required |")) {
    return 6;
  }

  const std::string status_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  if (status_json.empty()) {
    return 7;
  }
  if (const int result = require_contains_all(
          status_json,
          {
              "\"upstream_revision\"",
              "\"5a823cf70ebb1d7a158c6a7ca455860cd9f6aed0\"",
              "\"required\"",
              "\"adapted\"",
              "\"deferred\"",
              "\"non_goal\"",
              "\"examples\"",
              "\"hello_world\"",
              "\"uniform_list\"",
              "\"window_shadow\"",
              "\"x11\"",
          },
          8);
      result != 0) {
    return result;
  }

  const std::string tool_readme = read_source("tools/gpui_parity/README.md");
  const std::string extractor =
      read_source("tools/gpui_parity/extract_upstream_symbols.py");
  if (tool_readme.empty() || extractor.empty()) {
    return 9;
  }
  if (!contains(tool_readme, "extract_upstream_symbols.py") ||
      !contains(tool_readme,
                "docs/gpui-complete-parity-ledger.json") ||
      !contains(extractor, "GPUI_UPSTREAM_REVISION") ||
      !contains(extractor, "extract_public_reexports") ||
      !contains(extractor, "extract_examples")) {
    return 10;
  }

  const std::string plan = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-upstream-parity-ledger-plan.md");
  if (plan.empty() ||
      !contains(plan, "Steps 219-258") ||
      !contains(plan, "Run RED") ||
      !contains(plan, "Run GREEN") ||
      !contains(plan, "Phase A complete")) {
    return 11;
  }

  const std::string example =
      read_source("examples/api_parity/hello_world/main.cpp");
  if (example.empty()) {
    return 12;
  }
  if (!contains(example, "class HelloWorldView") ||
      !contains(example, "cgpui::Context<HelloWorldView>&") ||
      !contains(example, "cgpui::IntoElement render") ||
      !contains(example, "cgpui::Render<HelloWorldView>") ||
      !contains(example, "cgpui::div()") ||
      !contains(example, "cgpui::Application::create") ||
      !contains(example, "app->run") ||
      !contains(example, "GPUI upstream hello_world.rs parity")) {
    return 13;
  }

  const std::string xmake = read_source("xmake.lua");
  if (!contains(xmake, "target(\"gpui_parity_ledger_test\")") ||
      !contains(xmake, "target(\"context_render_spelling_test\")") ||
      !contains(xmake, "target(\"context_capabilities_test\")") ||
      !contains(xmake, "target(\"view_handle_spelling_test\")") ||
      !contains(xmake, "target(\"api_parity_hello_world\")") ||
      !contains(xmake, "tests/api_parity/gpui_parity_ledger_test.cpp") ||
      !contains(xmake, "tests/api_parity/context_render_spelling_test.cpp") ||
      !contains(xmake, "tests/api_parity/context_capabilities_test.cpp") ||
      !contains(xmake, "tests/api_parity/view_handle_spelling_test.cpp") ||
      !contains(xmake, "examples/api_parity/hello_world/main.cpp")) {
    return 14;
  }

  return 0;
}

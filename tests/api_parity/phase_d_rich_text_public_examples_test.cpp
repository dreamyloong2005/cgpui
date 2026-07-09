#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <string>

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

} // namespace

int main() {
  const std::string example =
      read_source("examples/api_parity/public_rich_text_examples/main.cpp");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger_md =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string xmake = read_source("xmake.lua");

  if (example.empty() || roadmap.empty() || ledger_md.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty() ||
      xmake.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"api_parity_public_rich_text_examples\")") ||
      !contains(xmake,
                "examples/api_parity/public_rich_text_examples/main.cpp") ||
      !contains(xmake,
                "target(\"phase_d_rich_text_public_examples_test\")") ||
      !contains(
          xmake,
          "tests/api_parity/phase_d_rich_text_public_examples_test.cpp")) {
    return 2;
  }

  constexpr std::array required_example_fragments{
      "#include \"cgpui/prelude.hpp\"",
      "class PublicRichTextExamplesView",
      "cgpui::IntoElement render(",
      "cgpui::Context<PublicRichTextExamplesView>&",
      "static_assert(cgpui::Render<PublicRichTextExamplesView>)",
      "cgpui::RichTextSpan",
      "cgpui::RichTextAttributes",
      "cgpui::RichTextDecoration::underline",
      "cgpui::RichTextLinkId",
      "cgpui::build_rich_text_runs(",
      "cgpui::rich_text_run_at_byte_offset(",
      "cgpui::rich_text_link_at_byte_offset(",
      "cgpui::rich_text_run_at_point(",
      "cgpui::rich_text_link_activation_at_point(",
      "cgpui::rich_text_pointer_button_can_activate_link(",
      "cgpui::RichTextInlineImageSpan",
      "cgpui::build_rich_text_inline_image_runs(",
      "cgpui::RichTextSyntaxTheme",
      "cgpui::build_rich_text_syntax_spans(",
      "cgpui::rich_text_syntax_attributes_for_role(",
      "cgpui::PaintList",
      "fill_rich_text(",
      "commands()",
      "CGPUI_RUN_PUBLIC_RICH_TEXT_EXAMPLES",
  };
  for (std::size_t index = 0; index < required_example_fragments.size();
       ++index) {
    if (!contains(example, required_example_fragments[index])) {
      return 10 + static_cast<int>(index);
    }
  }

  constexpr std::array required_docs{
      "Phase D Step 446 rich text public example",
      "examples/api_parity/public_rich_text_examples/main.cpp",
      "api_parity_public_rich_text_examples",
      "phase_d_rich_text_public_examples_test.cpp",
      "Step 446 extends the text-input examples band",
      "rich-text run, syntax, inline-image, hit, activation, and paint metadata",
      "renderer glyph coloring and inline image drawing remain later work",
  };
  for (std::size_t index = 0; index < required_docs.size(); ++index) {
    if (!contains(roadmap, required_docs[index]) ||
        !contains(ledger_md, required_docs[index]) ||
        !contains(ledger_json, required_docs[index]) ||
        !contains(task_plan, required_docs[index]) ||
        !contains(findings, required_docs[index])) {
      return 50 + static_cast<int>(index);
    }
  }

  if (!contains(ledger_json,
                "\"step_446\": \"Phase D Step 446 rich text public example\"")) {
    return 80;
  }

  constexpr std::array forbidden_example_fragments{
      "#include \"cgpui/ui/",
      "#include \"cgpui/platform/",
      "#include \"cgpui/renderer/",
      "#include \"src/",
      "#include \"../",
      "WindowRuntime",
      "TextModel::begin_composition_history_group",
      "TextModel::record_edit_history_transaction",
      "ClipboardItem",
      "gpui::test",
      "TaskPriority",
      "StructuredTaskGroup",
      ".runtime",
      "runtime.",
      "window_runtime",
      "std::unordered_map",
  };
  for (std::size_t index = 0; index < forbidden_example_fragments.size();
       ++index) {
    if (contains(example, forbidden_example_fragments[index])) {
      return 100 + static_cast<int>(index);
    }
  }

  return 0;
}

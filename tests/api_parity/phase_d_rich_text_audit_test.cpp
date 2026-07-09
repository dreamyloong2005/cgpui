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

std::string line_containing(const std::string& text, const char* anchor) {
  const std::size_t anchor_index = text.find(anchor);
  if (anchor_index == std::string::npos) {
    return {};
  }
  const std::size_t line_start = text.rfind('\n', anchor_index);
  const std::size_t line_end = text.find('\n', anchor_index);
  const std::size_t start =
      line_start == std::string::npos ? 0 : line_start + 1U;
  const std::size_t end =
      line_end == std::string::npos ? text.size() : line_end;
  return text.substr(start, end - start);
}

} // namespace

int main() {
  const std::string xmake = read_source("xmake.lua");
  const std::string roadmap = read_source(
      "docs/superpowers/plans/2026-07-04-gpui-complete-replication-roadmap.md");
  const std::string ledger =
      read_source("docs/gpui-complete-parity-ledger.md");
  const std::string ledger_json =
      read_source("docs/gpui-complete-parity-ledger.json");
  const std::string task_plan = read_source("task_plan.md");
  const std::string findings = read_source("findings.md");
  const std::string text_aggregate = read_source("include/cgpui/ui/text.hpp");
  const std::string text_rich_text_header =
      read_source("include/cgpui/ui/text_rich_text.hpp");
  const std::string text_rich_text_source =
      read_source("src/ui/text_rich_text.cpp");
  const std::string text_rich_text_hit_testing_header =
      read_source("include/cgpui/ui/text_rich_text_hit_testing.hpp");
  const std::string text_rich_text_hit_testing_source =
      read_source("src/ui/text_rich_text_hit_testing.cpp");
  const std::string text_rich_text_inline_image_header =
      read_source("include/cgpui/ui/text_rich_text_inline_image.hpp");
  const std::string text_rich_text_inline_image_source =
      read_source("src/ui/text_rich_text_inline_image.cpp");
  const std::string text_rich_text_syntax_header =
      read_source("include/cgpui/ui/text_rich_text_syntax.hpp");
  const std::string text_rich_text_syntax_source =
      read_source("src/ui/text_rich_text_syntax.cpp");
  const std::string text_rich_text_activation_header =
      read_source("include/cgpui/ui/text_rich_text_activation.hpp");
  const std::string text_rich_text_activation_source =
      read_source("src/ui/text_rich_text_activation.cpp");
  const std::string paint_header = read_source("include/cgpui/ui/paint.hpp");
  const std::string paint_rich_text_source =
      read_source("src/ui/paint_rich_text.cpp");
  const std::string renderer_commands_header =
      read_source("include/cgpui/renderer/renderer_commands.hpp");
  const std::string render_view_commands_source =
      read_source("src/ui/render_view_commands.cpp");
  const std::string rich_text_run_test =
      read_source("tests/ui/rich_text_run_test.cpp");
  const std::string render_view_test =
      read_source("tests/ui/render_view_test.cpp");
  const std::string structure_test =
      read_source("tests/architecture/ui_source_structure_test.cpp");
  const std::string header_cleanliness =
      read_source("tests/header_cleanliness/ui_header_cleanliness.cpp");

  if (xmake.empty() || roadmap.empty() || ledger.empty() ||
      ledger_json.empty() || task_plan.empty() || findings.empty() ||
      text_aggregate.empty() || text_rich_text_header.empty() ||
      text_rich_text_source.empty() ||
      text_rich_text_hit_testing_header.empty() ||
      text_rich_text_hit_testing_source.empty() ||
      text_rich_text_inline_image_header.empty() ||
      text_rich_text_inline_image_source.empty() ||
      text_rich_text_syntax_header.empty() ||
      text_rich_text_syntax_source.empty() ||
      text_rich_text_activation_header.empty() ||
      text_rich_text_activation_source.empty() || paint_header.empty() ||
      paint_rich_text_source.empty() || renderer_commands_header.empty() ||
      render_view_commands_source.empty() || rich_text_run_test.empty() ||
      render_view_test.empty() || structure_test.empty() ||
      header_cleanliness.empty()) {
    return 1;
  }

  if (!contains(xmake, "target(\"phase_d_rich_text_audit_test\")") ||
      !contains(xmake,
                "tests/api_parity/phase_d_rich_text_audit_test.cpp")) {
    return 2;
  }

  if (!contains(roadmap,
                "- [x] Steps 435-442: Add rich text runs") ||
      contains(roadmap, "- [ ] Steps 435-442") ||
      !contains(roadmap, "Step 442 closes the rich-text metadata band") ||
      !contains(roadmap,
                "tests/api_parity/phase_d_rich_text_audit_test.cpp") ||
      !contains(roadmap,
                "- [x] Steps 443-450: Add text input parity examples")) {
    return 3;
  }

  if (!contains(task_plan, "Step 442 closes the rich-text metadata band") ||
      !contains(findings, "Step 442 is an audit-only closeout")) {
    return 4;
  }

  const std::string text_row = line_containing(ledger, "| gpui text system |");
  if (text_row.empty() ||
      !contains(text_row, "Step 442 rich-text metadata band audit") ||
      !contains(text_row, "RichTextSpan") ||
      !contains(text_row, "RichTextRun") ||
      !contains(text_row, "RichTextRunHit") ||
      !contains(text_row, "RichTextRunPointHit") ||
      !contains(text_row, "RichTextInlineImageRun") ||
      !contains(text_row, "RichTextSyntaxTheme") ||
      !contains(text_row, "PaintList::fill_rich_text") ||
      !contains(text_row, "TextDraw::rich_text_runs") ||
      !contains(text_row, "RichTextLinkActivation") ||
      !contains(text_row, "phase_d_rich_text_audit_test") ||
      !contains(text_row, "runtime rich-text element dispatch") ||
      !contains(text_row, "actual link command handling") ||
      !contains(text_row, "inline image drawing/loading") ||
      !contains(text_row, "syntax parsing/editor token source integration")) {
    return 5;
  }
  if (contains(text_row,
               "candidate placement, rich text runs, and final Phase D")) {
    return 6;
  }

  if (!contains(ledger_json, "\"step_442\"") ||
      !contains(ledger_json, "\"rich_text_metadata_closeout_surface\"") ||
      !contains(ledger_json, "phase_d_rich_text_audit_test.cpp") ||
      !contains(ledger_json,
                "\"rich_text_metadata_closeout_remaining_gap\"")) {
    return 7;
  }

  constexpr std::array aggregate_evidence{
      "#include \"cgpui/ui/text_rich_text.hpp\"",
      "#include \"cgpui/ui/text_rich_text_hit_testing.hpp\"",
      "#include \"cgpui/ui/text_rich_text_inline_image.hpp\"",
      "#include \"cgpui/ui/text_rich_text_syntax.hpp\"",
      "#include \"cgpui/ui/text_rich_text_activation.hpp\"",
  };
  for (std::size_t index = 0; index < aggregate_evidence.size(); ++index) {
    if (!contains(text_aggregate, aggregate_evidence[index])) {
      return 20 + static_cast<int>(index);
    }
  }

  constexpr std::array public_evidence{
      "struct RichTextSpan",
      "struct RichTextRun",
      "struct RichTextRunBuildScratch",
      "struct RichTextRunHit",
      "struct RichTextLinkHit",
      "struct RichTextRunPointHit",
      "struct RichTextLinkPointHit",
      "struct RichTextInlineImageSpan",
      "struct RichTextInlineImageRun",
      "enum class RichTextSyntaxRole",
      "struct RichTextSyntaxTheme",
      "struct RichTextLinkActivation",
      "void fill_rich_text(",
      "std::vector<RichTextRun> rich_text_runs",
      "std::vector<RichTextInlineImageRun> rich_text_inline_images",
  };
  const std::array public_sources{
      text_rich_text_header,
      text_rich_text_header,
      text_rich_text_header,
      text_rich_text_header,
      text_rich_text_header,
      text_rich_text_hit_testing_header,
      text_rich_text_hit_testing_header,
      text_rich_text_inline_image_header,
      text_rich_text_inline_image_header,
      text_rich_text_syntax_header,
      text_rich_text_syntax_header,
      text_rich_text_activation_header,
      paint_header,
      renderer_commands_header,
      renderer_commands_header,
  };
  for (std::size_t index = 0; index < public_evidence.size(); ++index) {
    if (!contains(public_sources[index], public_evidence[index])) {
      return 40 + static_cast<int>(index);
    }
  }

  constexpr std::array source_evidence{
      "build_rich_text_runs(",
      "rich_text_run_at_byte_offset(",
      "rich_text_link_at_byte_offset(",
      "rich_text_run_at_point(",
      "rich_text_link_at_point(",
      "build_rich_text_inline_image_runs(",
      "build_rich_text_syntax_spans(",
      "PaintList::fill_rich_text(",
      "rich_text_pointer_button_can_activate_link(",
      "rich_text_link_activation_at_point(",
  };
  const std::array source_files{
      text_rich_text_source,
      text_rich_text_source,
      text_rich_text_source,
      text_rich_text_hit_testing_source,
      text_rich_text_hit_testing_source,
      text_rich_text_inline_image_source,
      text_rich_text_syntax_source,
      paint_rich_text_source,
      text_rich_text_activation_source,
      text_rich_text_activation_source,
  };
  for (std::size_t index = 0; index < source_evidence.size(); ++index) {
    if (!contains(source_files[index], source_evidence[index]) ||
        !contains(structure_test, source_evidence[index])) {
      return 70 + static_cast<int>(index);
    }
  }

  if (!contains(render_view_commands_source, ".rich_text_runs =") ||
      !contains(render_view_commands_source, ".rich_text_inline_images =") ||
      contains(text_rich_text_hit_testing_source, "WindowRuntime::") ||
      contains(text_rich_text_activation_source, "WindowRuntime::") ||
      contains(text_rich_text_activation_source, "std::string") ||
      !contains(header_cleanliness, "text_rich_text_activation.hpp")) {
    return 90;
  }

  constexpr std::array behavior_evidence{
      "test_plain_text_builds_single_default_run",
      "test_spans_clip_and_merge_attributes",
      "test_run_and_link_hits_use_half_open_byte_ranges",
      "test_point_hits_reuse_text_hit_testing_offsets",
      "test_inline_images_are_clipped_filtered_and_sorted",
      "test_syntax_tokens_build_rich_text_spans",
      "test_link_activation_uses_primary_single_release_hits",
      "fill_rich_text(",
      "rich_text_frame.last_text.rich_text_runs.size()",
      "rich_text_frame.last_text.rich_text_inline_images.size()",
  };
  const std::array behavior_sources{
      rich_text_run_test,
      rich_text_run_test,
      rich_text_run_test,
      rich_text_run_test,
      rich_text_run_test,
      rich_text_run_test,
      rich_text_run_test,
      render_view_test,
      render_view_test,
      render_view_test,
  };
  for (std::size_t index = 0; index < behavior_evidence.size(); ++index) {
    if (!contains(behavior_sources[index], behavior_evidence[index])) {
      return 110 + static_cast<int>(index);
    }
  }

  return 0;
}

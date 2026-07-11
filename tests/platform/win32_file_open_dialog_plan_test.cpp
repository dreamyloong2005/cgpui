#include "win32_file_dialog_internal.hpp"

int main() {
  const cgpui::NativeFileDialogOptions single{
      .kind = cgpui::NativeFileDialogKind::open_file,
      .title = "Open project",
      .default_directory = "C:/Projects",
      .filters = {{.name = "Sources", .extensions = {"cpp", ".hpp", "*.cxx"}},
                  {.name = "All", .extensions = {"*"}}},
  };
  const auto single_plan = cgpui::win32_file_dialog_plan(single);
  if (!single_plan.has_value() ||
      (single_plan->options & FOS_FILEMUSTEXIST) == 0 ||
      (single_plan->options & FOS_ALLOWMULTISELECT) != 0 ||
      single_plan->title != L"Open project" ||
      single_plan->default_directory != L"C:/Projects" ||
      single_plan->filters.size() != 2 ||
      single_plan->filters[0].pattern != L"*.cpp;*.hpp;*.cxx" ||
      single_plan->filters[1].pattern != L"*.*") return 1;

  const auto multiple = cgpui::win32_file_dialog_plan(
      cgpui::NativeFileDialogOptions{.kind = cgpui::NativeFileDialogKind::open_files});
  if (!multiple.has_value() ||
      (multiple->options & FOS_ALLOWMULTISELECT) == 0) return 2;
  return 0;
}

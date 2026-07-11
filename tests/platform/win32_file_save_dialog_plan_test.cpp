#include "win32_file_dialog_internal.hpp"

int main() {
  const auto save = cgpui::win32_file_dialog_plan(cgpui::NativeFileDialogOptions{
      .kind = cgpui::NativeFileDialogKind::save_file,
      .title = "Save project",
      .default_directory = "C:/Projects",
      .suggested_name = "project.cgpui",
      .filters = {{.name = "CGPUI", .extensions = {"cgpui"}}},
  });
  if (!save.has_value() || (save->options & FOS_FORCEFILESYSTEM) == 0 ||
      (save->options & FOS_OVERWRITEPROMPT) == 0 ||
      (save->options & FOS_FILEMUSTEXIST) != 0 ||
      (save->options & FOS_ALLOWMULTISELECT) != 0 ||
      save->title != L"Save project" ||
      save->default_directory != L"C:/Projects" ||
      save->suggested_name != L"project.cgpui" ||
      save->filters.size() != 1 || save->filters[0].pattern != L"*.cgpui") return 1;
  return 0;
}

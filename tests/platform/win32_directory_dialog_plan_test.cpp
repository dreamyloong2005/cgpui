#include "win32_file_dialog_internal.hpp"

int main() {
  const auto directory = cgpui::win32_file_dialog_plan(
      cgpui::NativeFileDialogOptions{
          .kind = cgpui::NativeFileDialogKind::pick_directory,
          .title = "Select workspace",
          .default_directory = "C:/Projects",
          .filters = {{.name = "Ignored", .extensions = {"txt"}}},
      });
  if (!directory.has_value() ||
      (directory->options & FOS_FORCEFILESYSTEM) == 0 ||
      (directory->options & FOS_PICKFOLDERS) == 0 ||
      (directory->options & FOS_PATHMUSTEXIST) == 0 ||
      (directory->options & FOS_FILEMUSTEXIST) != 0 ||
      (directory->options & FOS_ALLOWMULTISELECT) != 0 ||
      directory->title != L"Select workspace" ||
      directory->default_directory != L"C:/Projects" ||
      !directory->filters.empty()) return 1;
  return 0;
}

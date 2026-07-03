#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace cgpui {

enum class NativeFileDialogKind {
  open_file,
  open_files,
  save_file,
};

struct NativeFileDialogFilter {
  std::string name;
  std::vector<std::string> extensions;
};

struct NativeFileDialogOptions {
  NativeFileDialogKind kind = NativeFileDialogKind::open_file;
  std::string title;
  std::string default_directory;
  std::string suggested_name;
  std::vector<NativeFileDialogFilter> filters;
};

struct NativeFileDialogResult {
  bool supported = false;
  bool accepted = false;
  std::string backend;
  NativeFileDialogKind kind = NativeFileDialogKind::open_file;
  std::vector<std::string> paths;
  std::string error_message;
  std::size_t filter_count = 0;
};

} // namespace cgpui

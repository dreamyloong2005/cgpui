#include "win32_file_dialog_internal.hpp"

#include "win32_internal.hpp"

#include <shobjidl.h>

#include <limits>
#include <utility>

namespace cgpui {
namespace {

template <typename T>
class ComPtr {
 public:
  ~ComPtr() { if (value_ != nullptr) value_->Release(); }
  T** put() { return &value_; }
  T* operator->() const { return value_; }
  explicit operator bool() const { return value_ != nullptr; }

 private:
  T* value_ = nullptr;
};

std::wstring extension_pattern(std::string extension) {
  if (extension.empty()) return {};
  if (extension == "*") return L"*.*";
  if (extension.starts_with("*.")) return widen(extension);
  if (extension.front() == '.') extension.erase(extension.begin());
  return L"*." + widen(extension);
}

std::string hresult_message(const char* operation, HRESULT result) {
  return std::string(operation) + " failed with HRESULT " +
      std::to_string(static_cast<unsigned long>(result));
}

bool append_shell_item_path(IShellItem& item, NativeFileDialogResult& result) {
  PWSTR path = nullptr;
  const HRESULT status = item.GetDisplayName(SIGDN_FILESYSPATH, &path);
  if (FAILED(status) || path == nullptr) return false;
  result.paths.push_back(utf8_from_utf16(path));
  CoTaskMemFree(path);
  return !result.paths.back().empty();
}

} // namespace

std::optional<Win32FileDialogPlan> win32_file_dialog_plan(
    const NativeFileDialogOptions& options) {
  if (options.kind == NativeFileDialogKind::save_file) return std::nullopt;
  Win32FileDialogPlan plan{
      .options = FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST |
          (options.kind == NativeFileDialogKind::open_files
              ? FOS_ALLOWMULTISELECT : 0U),
      .title = widen(options.title),
      .default_directory = widen(options.default_directory),
  };
  for (const auto& filter : options.filters) {
    std::wstring pattern;
    for (const auto& extension : filter.extensions) {
      const std::wstring item = extension_pattern(extension);
      if (item.empty()) continue;
      if (!pattern.empty()) pattern += L';';
      pattern += item;
    }
    if (!pattern.empty()) plan.filters.push_back({widen(filter.name), std::move(pattern)});
  }
  return plan;
}

NativeFileDialogResult show_win32_native_file_dialog(
    const NativeFileDialogOptions& options) {
  NativeFileDialogResult result{.backend = "win32", .kind = options.kind,
                                .filter_count = options.filters.size()};
  auto plan = win32_file_dialog_plan(options);
  if (!plan.has_value()) return result;
  result.supported = true;

  const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  const bool uninitialize = SUCCEEDED(initialized);
  if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE) {
    result.supported = false;
    result.error_message = hresult_message("CoInitializeEx", initialized);
    return result;
  }

  ComPtr<IFileOpenDialog> dialog;
  HRESULT status = CoCreateInstance(
      CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(dialog.put()));
  if (SUCCEEDED(status)) status = dialog->SetOptions(plan->options);
  if (SUCCEEDED(status) && !plan->title.empty()) status = dialog->SetTitle(plan->title.c_str());

  std::vector<COMDLG_FILTERSPEC> specs;
  specs.reserve(plan->filters.size());
  for (const auto& filter : plan->filters) {
    specs.push_back({filter.name.c_str(), filter.pattern.c_str()});
  }
  if (SUCCEEDED(status) && !specs.empty() &&
      specs.size() <= (std::numeric_limits<UINT>::max)()) {
    status = dialog->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
  }

  ComPtr<IShellItem> directory;
  if (SUCCEEDED(status) && !plan->default_directory.empty() &&
      SUCCEEDED(SHCreateItemFromParsingName(
          plan->default_directory.c_str(), nullptr, IID_PPV_ARGS(directory.put())))) {
    status = dialog->SetDefaultFolder(directory.operator->());
  }
  if (SUCCEEDED(status)) status = dialog->Show(nullptr);
  if (status == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
    if (uninitialize) CoUninitialize();
    return result;
  }
  if (FAILED(status)) {
    result.error_message = hresult_message("IFileOpenDialog", status);
    if (uninitialize) CoUninitialize();
    return result;
  }

  if (options.kind == NativeFileDialogKind::open_files) {
    ComPtr<IShellItemArray> items;
    status = dialog->GetResults(items.put());
    DWORD count = 0;
    if (SUCCEEDED(status)) status = items->GetCount(&count);
    for (DWORD index = 0; SUCCEEDED(status) && index < count; ++index) {
      ComPtr<IShellItem> item;
      status = items->GetItemAt(index, item.put());
      if (SUCCEEDED(status) && !append_shell_item_path(*item.operator->(), result)) status = E_FAIL;
    }
  } else {
    ComPtr<IShellItem> item;
    status = dialog->GetResult(item.put());
    if (SUCCEEDED(status) && !append_shell_item_path(*item.operator->(), result)) status = E_FAIL;
  }
  result.accepted = SUCCEEDED(status) && !result.paths.empty();
  if (FAILED(status)) result.error_message = hresult_message("IFileOpenDialog result", status);
  if (uninitialize) CoUninitialize();
  return result;
}

} // namespace cgpui

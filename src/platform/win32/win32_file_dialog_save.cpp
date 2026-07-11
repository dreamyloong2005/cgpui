#include "win32_file_dialog_internal.hpp"

#include <limits>

namespace cgpui {

NativeFileDialogResult show_win32_native_save_file_dialog(
    const NativeFileDialogOptions& options,
    const Win32FileDialogPlan& plan) {
  NativeFileDialogResult result{.supported = true, .backend = "win32",
                                .kind = options.kind,
                                .filter_count = options.filters.size()};
  const HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
  const bool uninitialize = SUCCEEDED(initialized);
  if (FAILED(initialized) && initialized != RPC_E_CHANGED_MODE) {
    result.supported = false;
    result.error_message = win32_file_dialog_hresult_message("CoInitializeEx", initialized);
    return result;
  }

  Win32FileDialogComPtr<IFileSaveDialog> dialog;
  HRESULT status = CoCreateInstance(
      CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER,
      IID_PPV_ARGS(dialog.put()));
  if (SUCCEEDED(status)) status = dialog->SetOptions(plan.options);
  if (SUCCEEDED(status) && !plan.title.empty()) status = dialog->SetTitle(plan.title.c_str());
  if (SUCCEEDED(status) && !plan.suggested_name.empty()) {
    status = dialog->SetFileName(plan.suggested_name.c_str());
  }

  std::vector<COMDLG_FILTERSPEC> specs;
  specs.reserve(plan.filters.size());
  for (const auto& filter : plan.filters) {
    specs.push_back({filter.name.c_str(), filter.pattern.c_str()});
  }
  if (SUCCEEDED(status) && !specs.empty() &&
      specs.size() <= (std::numeric_limits<UINT>::max)()) {
    status = dialog->SetFileTypes(static_cast<UINT>(specs.size()), specs.data());
  }

  Win32FileDialogComPtr<IShellItem> directory;
  if (SUCCEEDED(status) && !plan.default_directory.empty() &&
      SUCCEEDED(SHCreateItemFromParsingName(
          plan.default_directory.c_str(), nullptr, IID_PPV_ARGS(directory.put())))) {
    status = dialog->SetDefaultFolder(directory.get());
  }
  if (SUCCEEDED(status)) status = dialog->Show(nullptr);
  if (status == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
    if (uninitialize) CoUninitialize();
    return result;
  }
  if (SUCCEEDED(status)) {
    Win32FileDialogComPtr<IShellItem> item;
    status = dialog->GetResult(item.put());
    if (SUCCEEDED(status) &&
        !win32_file_dialog_append_shell_item_path(*item.get(), result)) status = E_FAIL;
  }
  result.accepted = SUCCEEDED(status) && result.paths.size() == 1;
  if (FAILED(status)) {
    result.error_message = win32_file_dialog_hresult_message("IFileSaveDialog", status);
  }
  if (uninitialize) CoUninitialize();
  return result;
}

} // namespace cgpui

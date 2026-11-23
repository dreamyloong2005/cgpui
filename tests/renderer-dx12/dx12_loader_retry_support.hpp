#pragma once

#include "dx12_loader_process_support.hpp"
#include "dx12_loader_cases_support.hpp"

namespace cgpui_test {
inline bool loader_remove_owned_invalid_dll() {
  const auto executable = loader_self_executable().lexically_normal();
  wchar_t temporary[32768]{};
  const DWORD length = GetTempPathW(32768, temporary);
  if (!length || length >= 32768 || !executable.is_absolute() || executable.filename() != L"loader-child.exe")
    return false;
  auto parent = loader_fs::absolute(temporary).lexically_normal();
  if (parent.filename().empty()) parent = parent.parent_path();
  const auto directory = executable.parent_path();
  const std::wstring name = directory.filename().wstring();
  constexpr std::wstring_view prefix = L"cgpui-loader-";
  if (directory.parent_path() != parent || !name.starts_with(prefix)) return false;
  unsigned separators = 0, digits = 0;
  for (const wchar_t character : std::wstring_view(name).substr(prefix.size())) {
    if (character >= L'0' && character <= L'9') { ++digits; continue; }
    if (character != L'-' || !digits || ++separators > 2) return false;
    digits = 0;
  }
  if (separators != 2 || !digits) return false;
  const auto invalid = directory / L"vulkan-1.dll";
  const DWORD folder_attributes = GetFileAttributesW(directory.c_str());
  const DWORD file_attributes = GetFileAttributesW(invalid.c_str());
  const DWORD executable_attributes = GetFileAttributesW(executable.c_str());
  if (folder_attributes == INVALID_FILE_ATTRIBUTES || file_attributes == INVALID_FILE_ATTRIBUTES ||
      executable_attributes == INVALID_FILE_ATTRIBUTES || !(folder_attributes & FILE_ATTRIBUTE_DIRECTORY) ||
      (folder_attributes & FILE_ATTRIBUTE_REPARSE_POINT) ||
      (file_attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
      (executable_attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) ||
      invalid.parent_path() != directory || !invalid.is_absolute() || GetModuleHandleW(L"vulkan-1.dll")) return false;
  {
    std::ifstream stream(invalid, std::ios::binary | std::ios::ate);
    if (!stream || stream.tellg() != static_cast<std::streamoff>(loader_invalid_marker.size())) return false;
    stream.seekg(0);
    std::string contents(loader_invalid_marker.size(), '\0');
    stream.read(contents.data(), static_cast<std::streamsize>(contents.size()));
    if (!stream || contents != loader_invalid_marker) return false;
  }
  // Delete exactly the verified fixture marker beside our owned temporary copy.
  // No system path, recursive removal, arbitrary child argument or DLL search.
  return DeleteFileW(invalid.c_str()) != FALSE;
}

inline int loader_retry_case() {
  using namespace cgpui;
  if (GetModuleHandleW(L"vulkan-1.dll")) return dx12_fail(30, "retry child imported Vulkan before main");
  if (_putenv_s("CGPUI_RENDERER", "vulkan") || _putenv_s("CGPUI_DX12_ADAPTER", "warp"))
    return dx12_fail(31, "cannot select explicit Vulkan retry");
  {
    Dx12Window window;
    if (!window.hwnd) return dx12_fail(32, "retry child native window unavailable");
    auto renderer = create_renderer(window.descriptor());
    if (renderer || renderer.error().code != ErrorCode::renderer_initialization_failed ||
        renderer.error().message.find("vulkan-1.dll") == std::string::npos || GetModuleHandleW(L"vulkan-1.dll"))
      return dx12_fail(33, "retry did not begin with an unloaded, explicitly failed Vulkan factory");
    if (!loader_remove_owned_invalid_dll()) return dx12_fail(34, "refused to remove an unverified fixture DLL");
    renderer = create_renderer(window.descriptor());
    if (!renderer) return dx12_fail(35, renderer.error().message.c_str());
    const auto info = (*renderer)->backend_info();
    if (info.backend != RendererBackend::vulkan || info.selection != RendererBackendSelection::vulkan ||
        info.adapter_name.empty() || !info.device_generation || !info.fallback_reason.empty() ||
        !GetModuleHandleW(L"vulkan-1.dll") || !loader_case_pixels(**renderer))
      return dx12_fail(36, "retry failed to load real Vulkan and present correct captured pixels");
    std::cout << "LOADER_CASE mode=retry-vulkan failed-then-retried=1 pixels=64x64 adapter="
              << info.adapter_name << '\n';
  }
  if (!GetModuleHandleW(L"vulkan-1.dll")) return dx12_fail(37, "retried Vulkan loader lost its process-lifetime pin");
  return 0;
}
} // namespace cgpui_test

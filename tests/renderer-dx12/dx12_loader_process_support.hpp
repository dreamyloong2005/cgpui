#pragma once

#include "dx12_test_support.hpp"
#include "dx12_loader_desktop_test_support.hpp"
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace cgpui_test {
namespace loader_fs = std::filesystem;
inline constexpr std::string_view loader_invalid_marker =
    "CGPUI isolated invalid PE, not a Vulkan implementation.\n";

struct LoaderHandle {
  HANDLE value = nullptr;
  ~LoaderHandle() { if (value && value != INVALID_HANDLE_VALUE) CloseHandle(value); }
  bool valid() const { return value && value != INVALID_HANDLE_VALUE; }
};

inline loader_fs::path loader_self_executable() {
  std::wstring path(32768, L'\0');
  const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  if (!length || length >= path.size()) return {};
  path.resize(length);
  return loader_fs::path(path);
}

class LoaderIsolationDirectory {
 public:
  explicit LoaderIsolationDirectory(const loader_fs::path& executable) {
    wchar_t temporary[32768]{};
    const DWORD count = GetTempPathW(32768, temporary);
    if (!count || count >= 32768 || executable.empty()) return;
    auto parent = loader_fs::absolute(temporary).lexically_normal();
    if (parent.filename().empty()) parent = parent.parent_path();
    for (unsigned attempt = 0; attempt != 32; ++attempt) {
      const auto candidate = parent / (L"cgpui-loader-" + std::to_wstring(GetCurrentProcessId()) +
          L"-" + std::to_wstring(GetTickCount64()) + L"-" + std::to_wstring(attempt));
      if (candidate.parent_path() != parent || !candidate.is_absolute()) return;
      if (CreateDirectoryW(candidate.c_str(), nullptr)) { directory_ = candidate; break; }
      if (GetLastError() != ERROR_ALREADY_EXISTS) return;
    }
    if (directory_.empty()) return;
    executable_ = owned_file(L"loader-child.exe");
    const auto invalid = owned_file(L"vulkan-1.dll");
    if (!CopyFileW(executable.c_str(), executable_.c_str(), TRUE)) return;
    std::ofstream stream(invalid, std::ios::binary);
    stream << loader_invalid_marker;
    stream.close();
    valid_ = stream.good();
  }
  ~LoaderIsolationDirectory() { (void)cleanup(); }
  LoaderIsolationDirectory(const LoaderIsolationDirectory&) = delete;
  LoaderIsolationDirectory& operator=(const LoaderIsolationDirectory&) = delete;
  bool valid() const { return valid_; }
  const loader_fs::path& executable() const { return executable_; }
  const loader_fs::path& directory() const { return directory_; }
  loader_fs::path owned_file(const wchar_t* name) {
    const auto file = directory_ / name;
    if (directory_.empty() || file.parent_path() != directory_ || !file.is_absolute()) return {};
    files_.push_back(file);
    return file;
  }
  bool cleanup() {
    if (directory_.empty()) return true;
    bool ok = true;
    // Only exact files created by this fixture, never recursive removal or
    // system DLL manipulation. A surviving child keeps its files in place.
    for (const auto& file : files_) {
      if (file.parent_path() != directory_ || !file.is_absolute()) { ok = false; continue; }
      if (!DeleteFileW(file.c_str()) && GetLastError() != ERROR_FILE_NOT_FOUND) ok = false;
    }
    if (!RemoveDirectoryW(directory_.c_str()) && GetLastError() != ERROR_PATH_NOT_FOUND) ok = false;
    if (ok) { directory_.clear(); files_.clear(); }
    return ok;
  }
 private:
  loader_fs::path directory_, executable_;
  std::vector<loader_fs::path> files_;
  bool valid_ = false;
};

inline bool loader_run_child(const loader_fs::path& executable, const wchar_t* mode,
                             const loader_fs::path& log, const loader_fs::path& working_directory) {
  SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
  LoaderHandle output{CreateFileW(log.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
      &security, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr)};
  LoaderHandle input{CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
      &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)};
  if (!output.valid() || !input.valid()) return false;
  const bool visible_child = std::wstring_view(mode) == L"checked-fallback";
  LoaderHandle desktop_ready{visible_child ? CreateEventW(&security, TRUE, FALSE, nullptr) : nullptr};
  if (visible_child && !desktop_ready.valid()) return false;
  SIZE_T bytes = 0;
  InitializeProcThreadAttributeList(nullptr, 1, 0, &bytes);
  std::vector<unsigned char> storage(bytes);
  auto* attributes = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(storage.data());
  if (!InitializeProcThreadAttributeList(attributes, 1, 0, &bytes)) return false;
  struct AttributeOwner {
    LPPROC_THREAD_ATTRIBUTE_LIST value;
    ~AttributeOwner() { DeleteProcThreadAttributeList(value); }
  } attributes_owner{attributes};
  HANDLE inherited[]{output.value, input.value, desktop_ready.value};
  if (!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
      inherited, sizeof(HANDLE) * (visible_child ? 3 : 2), nullptr, nullptr)) return false;
  STARTUPINFOEXW startup{};
  startup.StartupInfo.cb = sizeof(startup);
  startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
  startup.StartupInfo.hStdOutput = startup.StartupInfo.hStdError = output.value;
  startup.StartupInfo.hStdInput = input.value;
  startup.lpAttributeList = attributes;
  PROCESS_INFORMATION process{};
  std::wstring command = L"\"" + executable.wstring() + L"\" --loader-case " + mode;
  if (visible_child) command += L" " + std::to_wstring(reinterpret_cast<std::uintptr_t>(desktop_ready.value));
  if (!CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE,
      CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT, nullptr, working_directory.c_str(),
      &startup.StartupInfo, &process)) {
    std::cerr << "loader child CreateProcess failed: " << GetLastError() << '\n';
    return false;
  }
  LoaderHandle child{process.hProcess}, thread{process.hThread};
  const DWORD waited = loader_wait_for_child(child.value, desktop_ready.value);
  if (waited != WAIT_OBJECT_0) {
    // Only the process handle returned by our own successful CreateProcess.
    TerminateProcess(child.value, 90);
    WaitForSingleObject(child.value, 5000);
  }
  DWORD code = 91;
  const bool exited = GetExitCodeProcess(child.value, &code) != FALSE && code != STILL_ACTIVE;
  FlushFileBuffers(output.value);
  std::ifstream transcript(log, std::ios::binary);
  std::string text(8192, '\0');
  transcript.read(text.data(), static_cast<std::streamsize>(text.size()));
  text.resize(static_cast<std::size_t>(transcript.gcount()));
  std::cout << text;
  std::cout << "LOADER_CHILD mode=" << loader_fs::path(mode).string()
            << " pid=" << process.dwProcessId << " exit=" << code
            << " timeout=" << (waited != WAIT_OBJECT_0) << '\n';
  return waited == WAIT_OBJECT_0 && exited && code == 0;
}
} // namespace cgpui_test

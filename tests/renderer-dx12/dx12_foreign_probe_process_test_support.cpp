#include "dx12_foreign_probe_control_test_support.hpp"
#include <array>
#include <string>
#include <vector>

namespace cgpui_test {
namespace {
struct Attributes {
  std::vector<unsigned char> storage;
  LPPROC_THREAD_ATTRIBUTE_LIST list = nullptr;
  HANDLE job = nullptr; // Attribute value storage lives through DeleteProcThreadAttributeList.
  ~Attributes() { if (list) DeleteProcThreadAttributeList(list); }
  bool initialize(HANDLE* handles, std::size_t count, HANDLE owned_job) {
    job = owned_job;
    SIZE_T bytes = 0;
    (void)InitializeProcThreadAttributeList(nullptr, 2, 0, &bytes);
    if (!bytes) return false;
    storage.resize(bytes);
    auto* candidate = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(storage.data());
    if (!InitializeProcThreadAttributeList(candidate, 2, 0, &bytes)) return false;
    list = candidate;
    return UpdateProcThreadAttribute(list, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST,
        handles, count * sizeof(HANDLE), nullptr, nullptr) != FALSE &&
        UpdateProcThreadAttribute(list, 0, PROC_THREAD_ATTRIBUTE_JOB_LIST,
            &job, sizeof(job), nullptr, nullptr) != FALSE;
  }
};
bool inherit_standard(DWORD which, ForeignProbeHandle& copy) {
  const HANDLE original = GetStdHandle(which);
  return original && original != INVALID_HANDLE_VALUE && DuplicateHandle(GetCurrentProcess(), original,
      GetCurrentProcess(), copy.put(), 0, TRUE, DUPLICATE_SAME_ACCESS);
}
} // namespace
int foreign_probe_process(ForeignProbeMode mode) {
  SECURITY_ATTRIBUTES security{sizeof(security), nullptr, TRUE};
  ForeignProbeHandle cancel(CreateEventW(&security, TRUE, FALSE, nullptr));
  ForeignProbeHandle input(CreateFileW(L"NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
      &security, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
  ForeignProbeHandle output, error, job(CreateJobObjectW(nullptr, nullptr));
  if (!cancel.get() || !job.get() || input.get() == INVALID_HANDLE_VALUE || !inherit_standard(STD_OUTPUT_HANDLE, output) ||
      !inherit_standard(STD_ERROR_HANDLE, error)) return 81;
  JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits{};
  limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
  if (!SetInformationJobObject(job.get(), JobObjectExtendedLimitInformation, &limits, sizeof(limits))) return 82;
  HANDLE inherited[] = {cancel.get(), input.get(), output.get(), error.get()};
  Attributes attributes;
  if (!attributes.initialize(inherited, std::size(inherited), job.get())) return 83;
  std::array<wchar_t, 32768> path{};
  const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  if (!length || length >= path.size()) return 84;
  std::wstring command = L"\"" + std::wstring(path.data(), length) + L"\" --probe-child ";
  command += mode == ForeignProbeMode::pumping ? L"pumping " : L"parked ";
  command += std::to_wstring(reinterpret_cast<std::uintptr_t>(cancel.get()));
  STARTUPINFOEXW startup{};
  startup.StartupInfo.cb = sizeof(startup);
  startup.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
  startup.StartupInfo.hStdInput = input.get();
  startup.StartupInfo.hStdOutput = output.get();
  startup.StartupInfo.hStdError = error.get();
  startup.lpAttributeList = attributes.list;
  PROCESS_INFORMATION created{};
  if (!CreateProcessW(path.data(), command.data(), nullptr, nullptr, TRUE,
      CREATE_SUSPENDED | CREATE_NO_WINDOW | EXTENDED_STARTUPINFO_PRESENT,
      nullptr, nullptr, &startup.StartupInfo, &created)) return 85;
  ForeignProbeHandle process(created.hProcess), thread(created.hThread);
  // Job association was part of creation; even the suspended interval is contained.
  if (ResumeThread(thread.get()) == MAXDWORD) {
    (void)TerminateProcess(process.get(), 86);
    (void)WaitForSingleObject(process.get(), 3000);
    return 86;
  }
  // This supervisor never writes stdout/stderr. Child backpressure cannot block enforcement.
  const DWORD waited = WaitForSingleObject(process.get(), 45000);
  if (waited != WAIT_OBJECT_0) {
    SetEvent(cancel.get());
    if (WaitForSingleObject(process.get(), 3000) != WAIT_OBJECT_0) {
      (void)TerminateProcess(process.get(), 90); // Owned isolated process only, never a native thread.
      (void)WaitForSingleObject(process.get(), 3000);
    }
    return waited == WAIT_TIMEOUT ? 90 : 91; // Cooperative exit after expiry is still FAIL.
  }
  DWORD result = 92;
  if (!GetExitCodeProcess(process.get(), &result)) return 92;
  return result == 0 ? 0 : 94; // Child stages carry detail when available; nonzero never becomes PASS.
}
} // namespace cgpui_test

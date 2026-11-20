#include "cgpui/platform/clipboard.hpp"
#include "win32_clipboard_test_lock.hpp"
#include "win32_clipboard_test_open.hpp"

#include <shellapi.h>
#include <timeapi.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace {

using Clock = std::chrono::steady_clock;

// Emulates a clipboard listener: a child process (this binary run with
// `--hold <ms> <event>`) opens the clipboard, signals `event`, holds it for
// <ms>, then closes it. Like real listeners it opens with a window:
// OpenClipboard(nullptr) does not block another OpenClipboard(nullptr).
int hold_clipboard(const wchar_t* hold_ms, const wchar_t* event_name) {
  const HANDLE opened = OpenEventW(EVENT_MODIFY_STATE, FALSE, event_name);
  const HWND window = CreateWindowExW(0, L"STATIC", L"", 0, 0, 0, 0, 0,
                                      HWND_MESSAGE, nullptr, nullptr, nullptr);
  if (opened == nullptr || window == nullptr) return 1;
  bool open = false;
  for (int attempt = 0; attempt < 100 && !open; ++attempt) {
    open = OpenClipboard(window) != FALSE;
    if (!open) Sleep(10);
  }
  if (!open) return 1;
  SetEvent(opened);
  Sleep(static_cast<DWORD>(_wtoi(hold_ms)));
  CloseClipboard();
  CloseHandle(opened);
  return 0;
}

class ClipboardHolder {
 public:
  explicit ClipboardHolder(std::chrono::milliseconds hold) {
    const std::wstring event_name = L"Local\\cgpui-clipboard-holder-" +
        std::to_wstring(GetCurrentProcessId());
    opened_ = CreateEventW(nullptr, TRUE, FALSE, event_name.c_str());
    wchar_t self[MAX_PATH]{};
    GetModuleFileNameW(nullptr, self, MAX_PATH);
    std::wstring command = L"\"" + std::wstring{self} + L"\" --hold " +
        std::to_wstring(hold.count()) + L" " + event_name;
    STARTUPINFOW startup{.cb = sizeof(STARTUPINFOW)};
    PROCESS_INFORMATION info{};
    if (opened_ == nullptr ||
        CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, 0,
                       nullptr, nullptr, &startup, &info) == FALSE) return;
    CloseHandle(info.hThread);
    process_ = info.hProcess;
    const HANDLE waits[]{opened_, process_};
    held_ = WaitForMultipleObjects(2, waits, FALSE, 5000U) == WAIT_OBJECT_0;
  }
  ~ClipboardHolder() {
    if (process_ != nullptr) WaitForSingleObject(process_, 10000U);
    if (process_ != nullptr) CloseHandle(process_);
    if (opened_ != nullptr) CloseHandle(opened_);
  }
  ClipboardHolder(const ClipboardHolder&) = delete;
  ClipboardHolder& operator=(const ClipboardHolder&) = delete;
  [[nodiscard]] bool held() const { return held_; }

 private:
  HANDLE opened_ = nullptr;
  HANDLE process_ = nullptr;
  bool held_ = false;
};

long long elapsed_ms(Clock::time_point start) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             Clock::now() - start).count();
}

} // namespace

// The Win32 clipboard retries OpenClipboard for 500 ms (clipboard_win32_open
// .cpp). A 250 ms hold must be waited out; a hold past the budget must fail
// within it instead of blocking the caller.
// main, not wmain: the Windows ASan build renames main to the entry of
// scripts/dev/asan_entry_windows.cpp, which has no wide form.
int main() {
  int argc = 0;
  wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  if (argv != nullptr && argc == 4 && std::wstring_view{argv[1]} == L"--hold") {
    const int held = hold_clipboard(argv[2], argv[3]);
    LocalFree(argv);
    return held;
  }
  if (argv != nullptr) LocalFree(argv);
  const cgpui::test::Win32ClipboardTestLock clipboard_lock;
  if (!clipboard_lock) return 10;
  // 1 ms timer resolution makes a Sleep-count retry budget its shortest.
  timeBeginPeriod(1);
  const std::unique_ptr<cgpui::Clipboard> clipboard =
      cgpui::create_platform_clipboard();
  if (clipboard == nullptr) return 1;
  const std::optional<std::string> original = clipboard->read_text();
  constexpr std::string_view payload{"contention \xE4\xB8\xAD"};
  constexpr std::chrono::milliseconds short_hold{250};
  int result = 0;
  {
    const ClipboardHolder holder(short_hold);
    if (!holder.held()) return 2;
    const auto start = Clock::now();
    if (!clipboard->write_text(payload)) result = 3;
    else if (elapsed_ms(start) > 700) result = 4;
  }
  if (result == 0) {
    const ClipboardHolder holder(short_hold);
    if (!holder.held()) return 2;
    const auto text = clipboard->read_text();
    if (!text || *text != payload) result = 5;
  }
  if (result == 0) {
    const ClipboardHolder holder(std::chrono::milliseconds{1500});
    if (!holder.held()) return 2;
    const auto start = Clock::now();
    const bool wrote = clipboard->write_text("past budget");
    const long long waited = elapsed_ms(start);
    if (wrote) result = 6;
    else if (waited < 450 || waited > 800) result = 7;
  }
  if (original) (void)clipboard->write_text(*original);
  timeEndPeriod(1);
  return result;
}

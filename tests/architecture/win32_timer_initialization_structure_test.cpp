#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error("missing timer owner: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* text) { return source.find(text) != std::string::npos; }
void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
void cap(const std::string& path, std::size_t maximum) {
  const auto source = read(path);
  require(std::count(source.begin(), source.end(), '\n') <= maximum, "timer module exceeded its unchanged cap");
}
}

int main() {
  try {
    const std::string base = "src/platform/win32/";
    const auto header = read(base + "win32_timer_wakeup_internal.hpp");
    const auto timer = read(base + "win32_timer_wakeup.cpp");
    const auto application = read(base + "win32_application.cpp");
    const auto factory_header = read(base + "win32_application_initialization_internal.hpp");
    const auto factory = read(base + "win32_application_initialization.cpp");
    cap(base + "win32_timer_wakeup_internal.hpp", 50);
    cap(base + "win32_timer_wakeup.cpp", 80);
    cap(base + "win32_application.cpp", 170);
    cap(base + "win32_application_initialization_internal.hpp", 20);
    cap(base + "win32_application_initialization.cpp", 20);
    require(has(header, "using CreateTimer = decltype(&CreateThreadpoolTimer)") &&
                has(header, "DWORD creation_error_") && has(header, "initialization_result() const"),
            "creation-only seam and captured status belong to the private timer owner");
    require(has(timer, "create_timer(&timer_callback, this, nullptr)") &&
                has(timer, "creation_error_(timer_ ? ERROR_SUCCESS : GetLastError())") &&
                has(timer, "ErrorCode::platform_initialization_failed") &&
                has(timer, "std::to_string(creation_error_)"),
            "native timer allocation error must be captured once and reported without a public API change");
    require(has(factory, "create_platform_application()") &&
                has(factory, "create_win32_application(&CreateThreadpoolTimer)") &&
                has(factory_header, "Win32TimerWakeup::CreateTimer create_timer") &&
                !has(factory_header, "create_win32_application(\n    Win32TimerWakeup::CreateTimer create_timer) {"),
            "public entry must bind real native creation through a declaration-only private seam");
    for (const auto* source : {&header, &timer, &application, &factory_header, &factory})
      require(!has(*source, "getenv(") && !has(*source, "SetLastError(") &&
                  !has(*source, "fault_injection") && !has(*source, "static CreateTimer"),
              "no process-wide fault switch, fake native success, or injected last-error reader");
    const auto disarm = timer.find("SetThreadpoolTimer(timer_, nullptr, 0, 0)");
    const auto drain = timer.find("WaitForThreadpoolTimerCallbacks(timer_, TRUE)");
    const auto close = timer.find("CloseThreadpoolTimer(timer_)");
    require(disarm < drain && drain < close && close != std::string::npos &&
                has(timer, "static_cast<Win32TimerWakeup*>(context)->post_wakeup()") &&
                has(application, "Win32TimerWakeup timer_wakeup_"),
            "real nonmoving timer owner must disarm, drain native callbacks and close in order");
    const auto request_start = timer.find("void Win32TimerWakeup::request(std::uint64_t delay_ms)");
    const auto request_end = timer.find("void Win32TimerWakeup::cancel()", request_start);
    require(request_start != std::string::npos && request_end != std::string::npos &&
                !has(timer.substr(request_start, request_end - request_start), "post_wakeup()") &&
                has(application, "if (auto ready = application->initialization_result(); !ready)") &&
                has(application, "return std::unexpected(ready.error())"),
            "failed delayed creation must neither post immediately nor expose a usable application");
    const auto public_api = read("include/cgpui/platform/platform_application.hpp");
    require(!has(public_api, "CreateTimer") && !has(public_api, "initialization_result") &&
                has(public_api, "virtual void request_wakeup_after(std::uint64_t delay_ms)"),
            "timer creation failure must not expand the public platform or renderer contract");
    require(has(read("packages/platform-win32/mcpp.toml"), "../../src/platform/win32/*.cpp"),
            "focused factory cpp must remain owned by platform-win32's source glob");
    for (const auto* guard : {"platform_source_structure_test.cpp", "win32_window_source_test.cpp"}) {
      const auto source = read(std::string("tests/architecture/") + guard);
      require(has(source, "win32_application_initialization.cpp") &&
                  has(source, "win32_application_initialization_internal.hpp"),
              "native source inventories must retain the new focused factory owners");
    }
    const auto manifest = read("tests/platform/mcpp.toml");
    for (const auto* test : {"win32_timer_creation_error_test.cpp", "win32_timer_allocation_failure_test.cpp",
                            "win32_application_timer_failure_test.cpp", "win32_timer_native_lifetime_test.cpp"}) {
      require(has(manifest, test), "timer failure/native behavior requires Windows test registration");
      cap(std::string("tests/platform/") + test, 90);
    }
    cap("tests/platform/win32_timer_test_support.hpp", 30);
    require(has(read("tests/architecture/mcpp.toml"), "win32_timer_initialization_structure_test.cpp") &&
                has(read("scripts/ci/architecture-header-targets.txt"),
                    "win32_timer_initialization_structure_test|tests/architecture/win32_timer_initialization_structure_test.cpp"),
            "timer initialization structure guard must be registered in mcpp and CI");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

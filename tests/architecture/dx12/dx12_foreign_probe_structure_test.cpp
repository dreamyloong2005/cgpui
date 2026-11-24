#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& name) {
  std::ifstream source(std::filesystem::path(CGPUI_SOURCE_ROOT) / name);
  if (!source) throw std::runtime_error("missing foreign-probe owner: " + name);
  return {std::istreambuf_iterator<char>(source), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* why) { if (!value) throw std::runtime_error(why); }
} // namespace
int main() {
  try {
    const std::string root = "tests/renderer-dx12/";
    const auto manifest = read(root + "mcpp.toml");
    for (const auto* leaf : {"control", "process", "fixture", "frame", "retirement", "peer", "case"}) {
      const std::string file = "dx12_foreign_probe_" + std::string(leaf) + "_test_support.cpp";
      const auto body = read(root + file);
      require(std::count(body.begin(), body.end(), '\n') <= 150, "probe implementations stay focused");
      require(manifest.find('"' + file + '"') != std::string::npos, "probe support must be explicitly registered");
    }
    for (const auto* leaf : {"control", "native", "peer"}) {
      const auto header = read(root + "dx12_foreign_probe_" + std::string(leaf) + "_test_support.hpp");
      require(std::count(header.begin(), header.end(), '\n') <= 100, "declaration-only support headers stay focused");
    }
    const auto main = read(root + "dx12_idle_foreign_native_probe_test.cpp");
    require(std::count(main.begin(), main.end(), '\n') <= 70 &&
        has(manifest, "\"dx12_idle_foreign_native_probe_test.cpp\""), "small executable is in Windows test discovery");
    const auto process = read(root + "dx12_foreign_probe_process_test_support.cpp");
    require(has(process, "PROC_THREAD_ATTRIBUTE_HANDLE_LIST") && has(process, "CREATE_SUSPENDED") &&
        has(process, "JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE") && has(process, "WaitForSingleObject(process.get(), 45000)") &&
        has(process, "SetEvent(cancel.get())") && has(process, "TerminateProcess(process.get(), 90)") &&
        !has(process, "TerminateThread"), "deadline must supervise only an explicitly owned child process");
    require(has(process, "PROC_THREAD_ATTRIBUTE_JOB_LIST") &&
        has(process, "InitializeProcThreadAttributeList(nullptr, 2") &&
        has(process, "InitializeProcThreadAttributeList(candidate, 2") &&
        !has(process, "AssignProcessToJobObject"), "job containment must be part of creation, with no late-assign fallback");
    require(!has(process, "foreign_probe_log(") && !has(process, "WriteFile(") &&
        !has(main, "foreign_probe_log(") && !has(main, "WriteFile("),
        "supervisor and parent entry must not perform output I/O on their control paths");
    const auto control = read(root + "dx12_foreign_probe_control_test_support.cpp");
    require(has(control, "MsgWaitForMultipleObjectsEx") && has(control, "WaitForMultipleObjects(count, handles, FALSE") &&
        has(control, "remaining") && !has(control, "Sleep("), "separate event-driven pumping/parked waits, no busy spin");
    const auto retire = read(root + "dx12_foreign_probe_retirement_test_support.cpp");
    require(has(retire, "std::try_to_lock") && has(retire, "peer.operation_depth != 0") &&
        has(retire, "GetDeviceRemovedReason()") && has(retire, "dx12_release_native_objects(peer)") &&
        !has(retire, "dx12_create_device"), "probe uses existing native-only teardown after genuine removal");
    const auto cohort = read("src/renderer/dx12/dx12_device_cohort.cpp");
    require(has(cohort, "origin.window_thread_id != GetCurrentThreadId()") &&
        has(cohort, "entry.window_thread == origin.window_thread_id"), "tests-only probe cannot expand production thread eligibility");
    const auto test_case = read(root + "dx12_foreign_probe_case_test_support.cpp");
    require(has(test_case, "dx12_remove_native_device(before.info)") &&
        has(test_case, "foreign_probe_filter_excludes") && has(test_case, "pin.reset()") &&
        has(test_case, "peer.finish(result == 0)"), "real removal/filter control and owner-thread final destruction required");
    const auto frame = read(root + "dx12_foreign_probe_frame_test_support.cpp");
    require(has(frame, "dx12_admit_frame") && has(frame, "frame->request_pixel_capture()") &&
        has(frame, "frame->present()") && has(frame, "bytes(clear), 0") && has(frame, "bytes(rect), 0"),
        "probe must render/capture exact native pixels, not synthetic completion");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

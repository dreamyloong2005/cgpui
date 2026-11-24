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
  if (!file) throw std::runtime_error("missing steady-state owner: " + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
std::size_t occurrences(const std::string& text, const std::string& token) {
  std::size_t count = 0, at = 0;
  while ((at = text.find(token, at)) != std::string::npos) { ++count; at += token.size(); }
  return count;
}
std::string owner(const char* name, unsigned cap) {
  const auto text = read(std::string("tests/renderer-steady-state/") + name);
  require(std::count(text.begin(), text.end(), '\n') <= cap, "steady-state module exceeded focused cap");
  return text;
}
}
int main() {
  try {
    const auto run = owner("probe_run.cpp", 140);
    const auto replay = owner("probe_scene.cpp", 75);
    const auto acquisition = owner("probe_acquisition.cpp", 45);
    const auto verify = owner("probe_verification.cpp", 30);
    for (const auto* text : {&run, &replay, &acquisition})
      for (const auto* forbidden : {"request_pixel_capture", "std::cout", "std::cerr", "std::ofstream",
          "SetMaximumFrameLatency", "SetEventOnCompletion", "WaitForSingleObject", "sleep_for", "this_thread::yield"})
        require(!has(*text, forbidden), "measured replay must not capture, report, override pacing or hot-poll");
    require(occurrences(run, "->present()") == 1 && occurrences(verify, "request_pixel_capture()") == 1 &&
        has(run, "// Exactly once, no retry or capture.") && has(acquisition, "ErrorCode::frame_not_ready"),
        "only acquisition readiness may retry; actual capture belongs to verification");
    require(run.find("report.phase = \"measured_no_capture\"") < run.find("report.phase = \"postflight_capture\"") &&
        run.find("const auto completed = Clock::now();") < run.find("if (auto stable = window.pump();") &&
        has(run, "report.samples.reserve(options.samples)") && has(run, "window.lock_display(report)"),
        "timing, sample storage and display validation must retain their boundaries");
    const auto factory_start = run.find("const auto factory_started = Clock::now();");
    const auto factory_end = run.find("if (!renderer) return");
    require(factory_start != std::string::npos && factory_end > factory_start &&
        run.find("const auto descriptor = window.descriptor();") < factory_start &&
        occurrences(run, "&report.measured_creation") == 1 && has(run, "CreationOutcome::exception") &&
        has(run, "CreationOutcome::factory_error") && has(run, "CreationOutcome::null_renderer"),
        "observe only measured factory duration, including errors/throws/null results");
    const auto factory = run.substr(factory_start, factory_end - factory_start);
    for (const auto* forbidden : {"window.", "backend_info()", "verify_scene(", "sample("})
      require(!has(factory, forbidden), "factory timing excludes fixture/identity/first-frame/postflight work");
    const auto window = owner("probe_window.cpp", 110);
    const auto display = owner("probe_display.cpp", 70);
    for (const auto* event : {"WM_MOVE", "WM_SIZE", "WM_DPICHANGED", "WM_DISPLAYCHANGE"})
      require(has(window, event), "window/display notifications need a permanent observation epoch");
    require(has(window, "++self->display_event_epoch_") && has(window, "return pump(false);") &&
        has(display, "result.event_epoch = display_event_epoch_") &&
        has(display, "report_->display_before != report_->display_after"),
        "facts and event epochs must reject switch-and-return without full queries inside pending timing");
    const auto header = owner("probe_internal.hpp", 110);
    require(!has(header, "return ") && !has(owner("probe_report_support.hpp", 40), "return "),
        "private report headers remain declarations/data");
    owner("probe_window.hpp", 40); owner("probe_options.cpp", 95);
    owner("renderer_steady_state_probe.cpp", 65); owner("probe_report_io.cpp", 35);
    const auto report = owner("probe_report.cpp", 90);
    const auto format = owner("probe_report_format.cpp", 75);
    const auto stats = owner("probe_statistics.cpp", 65);
    for (const auto* text : {&report, &format, &stats})
      for (const auto* forbidden : {"windows.h", "create_dx12_renderer", "create_vulkan_renderer", "request_pixel_capture"})
        require(!has(*text, forbidden), "pure statistics/schema owners must remain non-native");
    require(has(report, "report_valid(options, report)") && has(report, "display_last_observed") &&
        has(report, "unnotified between-sample changes remain unproven") &&
        has(stats, "sorted.size() - sorted.size() / 20 - 1") &&
        has(report, "write_creation(out, report.measured_creation)") &&
        has(stats, "report.measured_creation->outcome != CreationOutcome::created"),
        "schema must classify partial failures and disclose display/statistics evidence limits");
    const auto manifest = owner("mcpp.toml", 55);
    const auto begin = manifest.find("\n[test]");
    const auto end = manifest.find("\n[target.", begin);
    require(begin != std::string::npos && end != std::string::npos &&
        !has(manifest.substr(begin, end - begin), "renderer_steady_state_probe.cpp"),
        "manual native benchmark must not enter regression discovery");
    for (const auto* test : {"probe_statistics_test.cpp", "probe_report_schema_test.cpp", "probe_display_facts_test.cpp",
        "probe_creation_schema_test.cpp"}) {
      owner(test, 100);
      require(occurrences(manifest.substr(begin, end - begin), test) == 1, "register each pure regression exactly once");
    }
    require(occurrences(read("mcpp.toml"), "\"tests/renderer-steady-state\"") == 1 &&
        occurrences(read("tests/architecture/mcpp.toml"), "\"renderer_steady_state_structure_test.cpp\"") == 1 &&
        occurrences(read("scripts/ci/architecture-header-targets.txt"),
            "renderer_steady_state_structure_test|tests/architecture/renderer_steady_state_structure_test.cpp") == 1,
        "workspace, structure discovery and CI exact inventory must all register the owner once");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

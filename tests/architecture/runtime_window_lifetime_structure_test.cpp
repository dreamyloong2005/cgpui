#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  const char* root = CGPUI_SOURCE_ROOT;
  std::ifstream input(std::filesystem::path(root ? root : ".") / path);
  if (!input) throw std::runtime_error(std::string("missing window lifetime module: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
void require(bool condition, const char* reason) {
  if (!condition) throw std::runtime_error(reason);
}
bool has(const std::string& source, std::string_view token) {
  return source.find(token) != std::string::npos;
}
void limit(const char* path, std::size_t maximum) {
  const auto source = read(path);
  const auto lines = static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) +
      (!source.empty() && source.back() != '\n' ? 1U : 0U);
  require(lines <= maximum, path);
}
bool before(const std::string& source, std::string_view first, std::string_view second) {
  const auto left = source.find(first);
  const auto right = source.find(second);
  return left != std::string::npos && right != std::string::npos && left < right;
}
} // namespace

int main() {
  try {
    limit("src/ui/runtime_window_store_internal.hpp", 40);
    limit("src/ui/runtime_window_store.cpp", 60);
    limit("src/ui/runtime_window_lifetime_internal.hpp", 65);
    limit("src/ui/runtime_window_lifetime.cpp", 110);
    limit("include/cgpui/ui/detail/window_runtime_window_lifetime_declarations.hpp", 12);
    limit("src/ui/runtime_window_resource_cleanup.cpp", 60);
    limit("src/ui/runtime_window_shutdown.cpp", 60);
    limit("src/ui/runtime_window_close.cpp", 35);
    limit("src/ui/runtime_window_frame_results.cpp", 120);
    limit("include/cgpui/ui/detail/window_runtime_window_scheduling_declarations.hpp", 12);
    limit("src/ui/runtime_window_scheduling.cpp", 90);
    limit("src/ui/runtime_window_activation.cpp", 70);
    limit("src/ui/runtime_run.cpp", 141);
    limit("src/ui/runtime_window_records.cpp", 150);
    limit("src/ui/runtime_scheduling.cpp", 140);
    limit("include/cgpui/ui/detail/window_runtime_declarations.hpp", 260);
    limit("include/cgpui/ui/window_runtime.hpp", 263);

    const auto store_header = read("src/ui/runtime_window_store_internal.hpp");
    const auto store = read("src/ui/runtime_window_store.cpp");
    const auto lifetime_header = read("src/ui/runtime_window_lifetime_internal.hpp");
    const auto lifetime = read("src/ui/runtime_window_lifetime.cpp");
    const auto scheduling_header = read("include/cgpui/ui/detail/window_runtime_window_scheduling_declarations.hpp");
    const auto scheduling = read("src/ui/runtime_window_scheduling.cpp");
    const auto internal = read("include/cgpui/ui/detail/window_runtime_declarations.hpp");
    const auto public_header = read("include/cgpui/ui/window_runtime.hpp");
    const auto records = read("src/ui/runtime_window_records.cpp");
    const auto ownership = read("src/ui/runtime_window_ownership.cpp");
    const auto activation = read("src/ui/runtime_window_activation.cpp");
    const auto run = read("src/ui/runtime_run.cpp");
    const auto cleanup = read("src/ui/runtime_window_resource_cleanup.cpp");
    const auto shutdown = read("src/ui/runtime_window_shutdown.cpp");

    for (const auto* header : {&store_header, &lifetime_header, &scheduling_header}) {
      require(!has(*header, "return ") && !has(*header, ".push_back(") &&
                  !has(*header, ".extract(") &&
                  !std::regex_search(*header, std::regex(R"(\)\s*(?:const\s*)?\{)")),
              "window storage/lifetime/scheduling headers declare, focused sources implement");
    }
    require(has(store_header, "std::list<WindowRuntimeRecord> records_") &&
                has(store_header, "mutable std::vector<WindowRuntimeRecord> snapshot_") &&
                has(store, "RuntimeWindowStore::snapshot()") &&
                has(store, "snapshot_.assign(records_.begin(), records_.end())") &&
                !has(internal, "std::vector<WindowRuntimeRecord>"),
            "live callback records stay address-stable; only the public snapshot is contiguous");
    require(std::regex_search(public_header, std::regex(
                R"(std::span<const WindowRuntimeRecord>\s+window_runtime_records\(\)\s+const;)")) &&
                has(records, "window_runtime_records_->snapshot()") &&
                !has(public_header, "std::list<WindowRuntimeRecord>") &&
                !has(public_header, "std::unordered_map<Renderer*"),
            "keep the public record span while hiding storage and ownership mechanisms");
    require(has(lifetime_header, "std::unordered_map<Renderer*, std::unique_ptr<Renderer>>") &&
                has(lifetime_header, "retired_renderers_") && has(lifetime_header, "retired_views_") &&
                has(lifetime, "RuntimeWindowLifetime::owns(") &&
                has(lifetime, "renderers_.contains(renderer)") &&
                has(lifetime, "renderers_.extract(renderer)"),
            "renderer ownership is an actual unique-owner ledger, not a record flag");
    require(has(activation, "window_lifetime_->owns(") && has(run, "window_lifetime_->owns(") &&
                !has(read("src/ui/runtime_windows.cpp"), ".owns_renderer = true") &&
                !has(read("src/ui/runtime_window_results.cpp"), ".owns_renderer = true") &&
                !has(read("src/ui/app_runner.cpp"), "std::vector<std::unique_ptr<Renderer>>"),
            "the runtime reports actual renderer ownership and app runner cannot retain closed renderers");
    require(has(lifetime, "callback_depth_ != 0") && has(lifetime, "collecting_") &&
                has(lifetime, "RuntimeWindowCallbackScope::~RuntimeWindowCallbackScope()") &&
                has(read("src/ui/runtime_event_windows.cpp"), "RuntimeWindowCallbackScope") &&
                before(ownership, "begin_collection()", "release_retired(state_->windows.retired_native_windows_)") &&
                before(ownership, "release_retired(state_->windows.retired_native_windows_)", "reclaim_closed_window_records()") &&
                before(lifetime, "auto windows = std::move(native_windows)", "renderers.clear()") &&
                before(lifetime, "renderers.clear()", "windows.clear()") &&
                before(lifetime, "auto views = std::move(retired_views_)", "trees.clear()") &&
                before(lifetime, "trees.clear()", "views.clear()"),
            "callback-safe reclamation releases owned rendering resources before native windows");
    require(has(shutdown, "RuntimeWindowCallbackScope shutdown_scope") &&
                before(shutdown, "record.active = false", "window_lifetime_->retire(renderer)") &&
                before(shutdown, "window_lifetime_->retire(renderer)", "collect_retired_native_windows()") &&
                !has(shutdown, "window_lifetime_->release(renderer)"),
            "deactivation retires all child surfaces before invoking renderer destructors");
    require(has(cleanup, "WindowRuntime::reset_root_window_resources()") &&
                has(cleanup, "window_ = nullptr") && has(cleanup, "renderer_ = nullptr") &&
                has(cleanup, "window_lifetime_->release(renderer)") &&
                has(cleanup, "WindowRuntime::retire_window_view(") &&
                has(lifetime, "runtime.reset_root_window_resources()") &&
                before(run, "std::unique_ptr<PlatformWindow> window", "RuntimeRootWindowScope") &&
                before(run, "RuntimeRootWindowScope", "auto renderer_result = try_create_renderer("),
            "root initialization failures and window views have a focused lifetime cleanup boundary");
    require(has(scheduling, "WindowRuntime::request_paint_for_window(") &&
                has(scheduling, "find_window_runtime_record(runtime_id)") &&
                has(scheduling, "record->window->request_redraw()") &&
                has(read("src/ui/window.cpp"), "request_render_for_window(runtime_id_)") &&
                has(read("src/ui/runtime_context_scheduling.cpp"),
                    "request_render_for_window(runtime.normalize_window_context_id(window_runtime_id))") &&
                has(read("src/ui/runtime_window_callback_binding.cpp"), "id.value == 0 ? state_->windows.root_window_runtime_id_ : id") &&
                has(read("src/ui/runtime_window_timers.cpp"), "id = normalize_window_context_id(id)"),
            "window and context invalidation must retain the addressed window id");
  } catch (const std::exception& error) {
    std::cerr << "window lifetime structure: " << error.what() << '\n';
    return 1;
  }
  return 0;
}

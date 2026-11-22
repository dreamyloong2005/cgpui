#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {
std::string read(const char* path) {
  std::ifstream input(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!input) throw std::runtime_error(std::string("missing task group module: ") + path);
  return {std::istreambuf_iterator<char>(input), {}};
}
void require(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}
bool has(const std::string& source, std::string_view value) {
  return source.find(value) != std::string::npos;
}
bool before(const std::string& source, std::string_view first, std::string_view second) {
  const auto left = source.find(first);
  const auto right = source.find(second);
  return left != std::string::npos && right != std::string::npos && left < right;
}
void limit(const std::string& source, std::size_t maximum) {
  require(static_cast<std::size_t>(std::count(source.begin(), source.end(), '\n')) +
              (!source.empty() && source.back() != '\n' ? 1U : 0U) <= maximum,
          "task group lifetime module exceeded its line budget");
}
} // namespace

int main() {
  try {
    const auto header = read("include/cgpui/ui/task_group.hpp");
    const auto facade = read("src/ui/task_group.cpp");
    const auto context = read("src/ui/task_group_window_context.cpp");
    const auto lifetime = read("src/ui/task_group_lifetime.cpp");
    const auto store = read("src/ui/runtime_task_group_internal.hpp");
    const auto record = read("src/ui/runtime_task_group_record_internal.hpp");
    const auto shutdown = read("src/ui/runtime_shutdown.cpp");
    const auto resources = read("src/ui/runtime_shutdown_resources.cpp");
    limit(header, 80);
    limit(facade, 160);
    limit(context, 25);
    limit(lifetime, 65);
    limit(store, 70);
    limit(record, 25);
    limit(shutdown, 70);
    limit(resources, 60);
    require(has(header, "std::weak_ptr<detail::TaskGroupLifetime>") &&
                !has(header, "struct TaskGroupLifetime {") &&
                !has(header, "RuntimeTaskGroupStore"),
            "TaskGroup carries an opaque weak lifetime, not private store state");
    require(has(store, "std::shared_ptr<detail::TaskGroupLifetime>") &&
                has(store, "using Record = detail::RuntimeTaskGroupRecord") &&
                has(lifetime, "RuntimeTaskGroupStore::detach_handles()") &&
                has(lifetime, "lifetime_.reset()") &&
                has(lifetime, "runtime_lifetime_.expired() ? nullptr : runtime_") &&
                has(shutdown, "task_group_store_->detach_handles()"),
            "the runtime detaches facade handles before task metadata destruction");
    require(before(shutdown, "task_pool_->shutdown()", "retire_runtime_user_resources()") &&
                before(shutdown, "retired_callbacks.clear()", "retire_runtime_user_resources()") &&
                before(shutdown, "retire_remaining_renderers()", "retire_runtime_user_resources()") &&
                before(shutdown, "retire_runtime_user_resources()", "detach_handles()") &&
                has(shutdown, "while (retire_runtime_user_resources())") &&
                has(resources, "std::exchange(state_->entities.globals_, {})") &&
                has(resources, "std::exchange(state_->entities.entity_stores_, {})") &&
                has(resources, "std::exchange(state_->entities.entity_observers_, {})") &&
                has(resources, "std::exchange(state_->actions.action_handlers_, {})") &&
                has(resources, "std::exchange(state_->animations.animations_, {})") &&
                has(resources, "std::exchange(state_->views.view_registry_, {})") &&
                has(resources, "std::exchange(state_->elements.window_element_hosts_, {})") &&
                before(resources, "auto views =", "tree.reset()") &&
                before(resources, "tree.reset()", "views.clear()"),
            "drain coherent user-resource batches while task handles are still attached");
    require(!has(facade, "runtime_->") && !has(facade, "*runtime_") &&
                !has(context, "runtime_->") && !has(context, "*runtime_") &&
                has(facade, "attached_runtime()") && has(context, "attached_runtime()") &&
                has(facade, "runtime_lifetime_(std::move(other.runtime_lifetime_))") &&
                has(facade, "runtime_lifetime_ = std::move(other.runtime_lifetime_)"),
            "all facade dereferences require an attached runtime, including move destinations");
  } catch (const std::exception& error) {
    std::cerr << "task group lifetime structure: " << error.what() << '\n';
    return 1;
  }
  return 0;
}

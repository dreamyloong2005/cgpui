#include "runtime_state_internal.hpp"
#include "runtime_window_lifetime_internal.hpp"

namespace cgpui {

Result<Renderer*> WindowRuntime::RuntimeWindowLifetime::create(const RenderSurfaceDescriptor& descriptor) {
  auto result = factory(descriptor);
  if (!result) return std::unexpected(result.error());
  if (*result == nullptr) return static_cast<Renderer*>(nullptr);
  Renderer* pointer = result->get();
  renderers_.emplace(pointer, std::move(*result));
  return pointer;
}

bool WindowRuntime::RuntimeWindowLifetime::owns(Renderer* renderer) const {
  return renderers_.contains(renderer);
}

void WindowRuntime::RuntimeWindowLifetime::release(Renderer* renderer) {
  auto owner = renderers_.extract(renderer);
}

void WindowRuntime::RuntimeWindowLifetime::retire(Renderer* renderer) {
  auto owner = renderers_.extract(renderer);
  if (!owner.empty()) retired_renderers_.push_back(std::move(owner.mapped()));
}

void WindowRuntime::RuntimeWindowLifetime::retire_remaining_renderers() {
  for (auto& [pointer, owner] : renderers_) retired_renderers_.push_back(std::move(owner));
  renderers_.clear();
}

void WindowRuntime::RuntimeWindowLifetime::retire_view(std::unique_ptr<View> view) {
  if (view) retired_views_.push_back(std::move(view));
}

void WindowRuntime::RuntimeWindowLifetime::retire_tree(std::unique_ptr<ElementTree> tree) {
  if (tree) retired_trees_.push_back(std::move(tree));
}

bool WindowRuntime::RuntimeWindowLifetime::begin_collection() {
  if (collecting_ || callback_depth_ != 0) return false;
  collecting_ = true;
  return true;
}

void WindowRuntime::RuntimeWindowLifetime::release_retired(
    std::vector<std::unique_ptr<PlatformWindow>>& native_windows) {
  do {
    release_retired_views();
    // Detach both queues before any destructor can retire another window.
    auto renderers = std::move(retired_renderers_);
    auto windows = std::move(native_windows);
    renderers.clear();
    windows.clear();
  } while (!retired_renderers_.empty() || !native_windows.empty() ||
           !retired_trees_.empty() || !retired_views_.empty());
}

void WindowRuntime::RuntimeWindowLifetime::release_retired_views() {
  while (!retired_trees_.empty() || !retired_views_.empty()) {
    // A tree destructor may close another window. Keep its view in the next
    // cohort alongside its tree, rather than destroying that new view early.
    auto trees = std::move(retired_trees_);
    auto views = std::move(retired_views_);
    trees.clear();
    views.clear();
  }
}

void WindowRuntime::RuntimeWindowLifetime::end_collection() { collecting_ = false; }
void WindowRuntime::RuntimeWindowLifetime::enter_callback() { ++callback_depth_; }
void WindowRuntime::RuntimeWindowLifetime::leave_callback() {
  if (callback_depth_ == 1) release_retired_views();
  --callback_depth_;
}
bool WindowRuntime::RuntimeWindowLifetime::in_callback() const { return callback_depth_ != 0; }

WindowRuntime::RuntimeWindowCallbackScope::RuntimeWindowCallbackScope(WindowRuntime& value, bool enabled)
    : runtime(value), enabled(enabled) {
  if (enabled) runtime.state_->windows.window_lifetime_->enter_callback();
}
WindowRuntime::RuntimeWindowCallbackScope::~RuntimeWindowCallbackScope() {
  if (enabled) runtime.state_->windows.window_lifetime_->leave_callback();
}

WindowRuntime::RuntimeRootWindowScope::RuntimeRootWindowScope(WindowRuntime& value)
    : runtime(value) {}
WindowRuntime::RuntimeRootWindowScope::~RuntimeRootWindowScope() {
  runtime.reset_root_window_resources();
}

} // namespace cgpui

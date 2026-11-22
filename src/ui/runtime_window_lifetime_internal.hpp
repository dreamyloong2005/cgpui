#pragma once

#include "cgpui/ui/window_runtime.hpp"

namespace cgpui {

// The raw factory remains borrowed. Only the owning factory enters this ledger.
class WindowRuntime::RuntimeWindowLifetime {
 public:
  AppRendererFactory factory;
  Result<Renderer*> create(const RenderSurfaceDescriptor& descriptor);
  bool owns(Renderer* renderer) const;
  void release(Renderer* renderer);
  void retire(Renderer* renderer);
  void retire_remaining_renderers();
  void retire_view(std::unique_ptr<View> view);
  void retire_tree(std::unique_ptr<ElementTree> tree);
  bool begin_collection();
  void release_retired(std::vector<std::unique_ptr<PlatformWindow>>& native_windows);
  void release_retired_views();
  void end_collection();
  void enter_callback();
  void leave_callback();
  bool in_callback() const;

 private:
  std::unordered_map<Renderer*, std::unique_ptr<Renderer>> renderers_;
  std::vector<std::unique_ptr<Renderer>> retired_renderers_;
  std::vector<std::unique_ptr<View>> retired_views_;
  std::vector<std::unique_ptr<ElementTree>> retired_trees_;
  unsigned callback_depth_ = 0;
  bool collecting_ = false;
};

struct WindowRuntime::RuntimeWindowCallbackScope {
  explicit RuntimeWindowCallbackScope(WindowRuntime& runtime, bool enabled = true);
  ~RuntimeWindowCallbackScope();
  RuntimeWindowCallbackScope(const RuntimeWindowCallbackScope&) = delete;
  RuntimeWindowCallbackScope& operator=(const RuntimeWindowCallbackScope&) = delete;
  WindowRuntime& runtime;
  bool enabled;
};

struct WindowRuntime::RuntimeRootWindowScope {
  explicit RuntimeRootWindowScope(WindowRuntime& runtime);
  ~RuntimeRootWindowScope();
  RuntimeRootWindowScope(const RuntimeRootWindowScope&) = delete;
  RuntimeRootWindowScope& operator=(const RuntimeRootWindowScope&) = delete;
  WindowRuntime& runtime;
};

} // namespace cgpui

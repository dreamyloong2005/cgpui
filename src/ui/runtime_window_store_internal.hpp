#pragma once

#include "cgpui/ui/window_runtime.hpp"

#include <list>

namespace cgpui {

// Live records have stable addresses. The public contiguous range is a snapshot,
// not the storage traversed by event and rendering callbacks.
class WindowRuntime::RuntimeWindowStore {
 public:
  WindowRuntimeRecord& insert(WindowRuntimeRecord record);
  WindowRuntimeRecord* find(WindowRuntimeId id);
  const WindowRuntimeRecord* find(WindowRuntimeId id) const;
  std::list<WindowRuntimeRecord>& records();
  const std::list<WindowRuntimeRecord>& records() const;
  std::span<const WindowRuntimeRecord> snapshot() const;
  std::size_t size() const;

 private:
  std::list<WindowRuntimeRecord> records_;
  mutable std::vector<WindowRuntimeRecord> snapshot_;
};

} // namespace cgpui

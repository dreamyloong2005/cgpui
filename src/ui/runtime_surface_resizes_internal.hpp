#pragma once

#include "cgpui/renderer/renderer_frame.hpp"
#include "cgpui/ui/runtime_ids.hpp"
#include <memory>
#include <unordered_map>

namespace cgpui {
// Owns desired extents only. No renderer/window pointer survives a call.
// A pending request prevents rendering with desired layout on an old surface.
class RuntimeSurfaceResizes {
 public:
  RuntimeSurfaceResizes();
  ~RuntimeSurfaceResizes();
  Result<void> request(WindowRuntimeId, Renderer&, Size, DpiScale);
  Result<void> retry(WindowRuntimeId, Renderer&);
  bool has_pending(WindowRuntimeId) const;
  void clear(WindowRuntimeId);
  void clear_all();
 private:
  struct Pending;
  Result<void> apply(WindowRuntimeId, Renderer&, const std::shared_ptr<Pending>&);
  std::unordered_map<std::uint64_t, std::shared_ptr<Pending>> pending_;
};
} // namespace cgpui

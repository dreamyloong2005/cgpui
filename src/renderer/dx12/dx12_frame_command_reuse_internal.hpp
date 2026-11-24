#pragma once
#include "dx12_draw_commands_internal.hpp"
#include <memory>

namespace cgpui {
struct Dx12CommandReuseSurfaceKey;
class Dx12FrameCommandReuse {
 public:
  Dx12FrameCommandReuse();
  ~Dx12FrameCommandReuse();
  Result<bool> record(Dx12State&, Color, std::span<const Dx12DrawCommand>, bool capture, bool mutations);
  void parent_reset(Dx12State&) noexcept;
  bool prearm(Dx12State&) noexcept;
  void completed(Dx12State&) noexcept;
  void removed(Dx12State&) noexcept;
  void parent_released(Dx12State&) noexcept;
  void publish_success(Dx12State&) noexcept;
  void accept_surface(const Dx12CommandReuseSurfaceKey&) noexcept;
 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
  void discard_retired() noexcept;
};
} // namespace cgpui

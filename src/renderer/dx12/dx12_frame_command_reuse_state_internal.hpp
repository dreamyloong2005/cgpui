#pragma once
#include "dx12_frame_command_reuse_internal.hpp"
#include "dx12_command_reuse_lifetime_internal.hpp"
#include "dx12_command_reuse_surface_epoch_internal.hpp"
#include "dx12_command_bundle_internal.hpp"

namespace cgpui {
struct Dx12FrameCommandReuse::Impl {
  Dx12CommandReuseLifetime lifetime;
  Dx12CommandReuseSurfaceEpoch surface;
  std::unique_ptr<Dx12CommandBundle> entry;
  std::uint64_t parent = 0, submission = 0;
  bool attached = false, exhausted = false;
};
} // namespace cgpui

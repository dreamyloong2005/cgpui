#pragma once

namespace cgpui {
struct Dx12State;

// Only for confirmed removal, or a partial initialization with no submission.
// CPU caches, immutable snapshots, window identity and recovery evidence remain.
void dx12_release_native_objects(Dx12State& state) noexcept;
} // namespace cgpui

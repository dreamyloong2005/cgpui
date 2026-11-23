#pragma once

namespace cgpui {
struct Dx12State;

// May block indefinitely if a live device never accepts a completion ticket.
// A timeout is not evidence that releasing submitted GPU ownership is safe.
void dx12_retire_submitted_work(Dx12State& state) noexcept;
} // namespace cgpui

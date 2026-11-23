#pragma once

#include <memory>
#include <vector>

namespace cgpui {
struct Dx12State;

// Call after complete initialization while holding the state's operation gate.
// Registration stores only immutable identity metadata and weak ownership.
void dx12_register_device_cohort(const std::shared_ptr<Dx12State>& state);
std::vector<std::weak_ptr<Dx12State>> dx12_device_cohort_peers(const Dx12State& origin);

// Origin's native device must actually report removal and its gate must be held.
// Only idle operation gates on the current HWND-owner thread are acquired; no
// registry lock remains held while inspecting or releasing native objects.
void dx12_retire_removed_cohort_peers(Dx12State& origin);
} // namespace cgpui

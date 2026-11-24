#include "dx12_command_reuse_supplement_test_support.hpp"
#include "../../src/renderer/dx12/dx12_geometry_upload_frame_internal.hpp"
#include <array>
#include <algorithm>

int main() {
  using namespace cgpui; using namespace cgpui_test;
  Dx12RecoveryDeadline deadline; Dx12Window window; auto state = srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1, "reuse overwrite native setup");
  state->command_observation = std::make_unique<Dx12CommandObservation>(32768);
  auto tail = std::make_unique<ReuseQueueTail>(); auto* verifier = tail.get();
  state->command_observation->verifier = std::move(tail);
  if (!reuse_scene(state, true, true)) return dx12_fail(2, "reuse overwrite DIRECT warm capture");
  const auto expected = state->pixels->rgba8;
  std::vector<std::pair<UINT64, UINT64>> transient_ranges;
  for (const auto& event : state->command_observation->events())
    if (event.type == D3D12_COMMAND_LIST_TYPE_DIRECT &&
        (event.kind == Dx12CommandEventKind::vertex || event.kind == Dx12CommandEventKind::index))
      transient_ranges.push_back({event.a, event.b});
  const auto pages = state->geometry_uploads->stats().page_count;
  if (transient_ranges.empty() || !pages || pages > 4 || !reuse_scene(state, false, false) ||
      !reuse_trace_pixels_ready(*state, *verifier, expected)) return dx12_fail(3, "reuse overwrite native fresh prerequisite");
  std::array<std::uint64_t, 3> parents{verifier->parent, 0, 0};
  if (!dx12_begin_geometry_upload_frame(*state)) return dx12_fail(4, "idle transient rewind rejected");
  std::vector<std::uint8_t> poison(256 * 1024, 0xa5);
  std::vector<std::pair<UINT64, UINT64>> rewritten;
  for (std::size_t page = 0; page < pages; ++page) {
    auto slice = dx12_upload_geometry(*state, poison.data(), poison.size());
    if (!slice) return dx12_fail(5, "actual transient poison allocation");
    void* mapped = nullptr; const D3D12_RANGE read{slice->offset, slice->offset + slice->size};
    if (FAILED(slice->resource->Map(0, &read, &mapped))) return dx12_fail(6, "actual transient poison map");
    const auto* bytes = static_cast<const std::uint8_t*>(mapped) + slice->offset;
    const bool correct = std::equal(poison.begin(), poison.end(), bytes);
    const D3D12_RANGE no_write{0, 0}; slice->resource->Unmap(0, &no_write);
    if (!correct) return dx12_fail(7, "actual old transient bytes were not overwritten");
    rewritten.push_back({slice->address, slice->size});
  }
  for (const auto [address, size] : transient_ranges) {
    const bool covered = std::ranges::any_of(rewritten, [&](const auto& range) {
      return address >= range.first && address - range.first <= range.second && size <= range.second - (address - range.first);
    });
    if (!covered) return dx12_fail(8, "poison did not cover exact warm native VB/IB addresses");
  }
  for (std::size_t i = 1; i != parents.size(); ++i) {
    if (!reuse_scene(state, false, false) || !reuse_trace_pixels_ready(*state, *verifier, expected) ||
        !dx12_check_debug_errors(*state)) return dx12_fail(9, "frozen bundle pixels changed after actual transient overwrite");
    parents[i] = verifier->parent;
  }
  if (!reuse_native_history(state->command_observation->events(), parents) || state->command_observation->overflowed())
    return dx12_fail(10, "post-poison pixels came from re-recording/fallback instead of same native immutable bundle");
  return 0;
}

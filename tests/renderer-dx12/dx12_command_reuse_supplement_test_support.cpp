#include "dx12_command_reuse_supplement_test_support.hpp"
#include "../../src/renderer/dx12/dx12_image_resolve_internal.hpp"
#include <algorithm>

namespace cgpui_test {
using namespace cgpui;
std::size_t reuse_event_count(const Dx12State& state, Dx12CommandEventKind kind) {
  return std::ranges::count(state.command_observation->events(), kind, &Dx12CommandEvent::kind);
}
ID3D12GraphicsCommandList* reuse_last_bundle(const Dx12State& state) {
  ID3D12GraphicsCommandList* result = nullptr;
  for (const auto& event : state.command_observation->events())
    if (event.kind == Dx12CommandEventKind::bundle_created && SUCCEEDED(event.result)) result = event.list;
  return result; // Borrowed observation; no COM owner escapes into retirement.
}
bool reuse_parent_before_bundle_release(const Dx12State& state) {
  bool parent = false, bundle = false;
  for (const auto& event : state.command_observation->events()) {
    if (event.kind == Dx12CommandEventKind::parent_released) parent = true;
    if (event.kind == Dx12CommandEventKind::bundle_released) { if (!parent) return false; bundle = true; }
  }
  return parent && bundle;
}
bool reuse_native_sentinel(ID3D12Object& object, std::atomic_bool& released) {
  released = false; Dx12Ptr<IUnknown> sentinel;
  sentinel.Attach(new (std::nothrow) Dx12ResourceReleaseSentinel(released));
  if (!sentinel) return false;
  constexpr GUID key{0x305b4c92, 0x3213, 0x410e, {0x8b, 0x95, 0xfe, 0x12, 0x61, 0x94, 0x84, 0x31}};
  const auto result = object.SetPrivateDataInterface(key, sentinel.Get());
  sentinel.Reset(); return SUCCEEDED(result) && !released;
}
Result<std::vector<Dx12DrawCommand>> reuse_raw_scene(Dx12State& state) {
  if (state.scale.value != 1) return std::unexpected(dx12_error(E_INVALIDARG, "supplement raw scene scale1 only"));
  std::vector<Dx12DrawCommand> result;
  const SolidRect rect{.rect = {{1, 1}, {27, 27}}, .color = {0.1F, 0.1F, 0.8F, 1}};
  const RoundedRectDraw rounded{.rect = {{10, 9}, {16, 15}}, .color = {1, 1, 0, 1}, .radius = BorderRadii::all(3)};
  const ImageAsset patterned{.id = {9901}, .logical_size = {2, 2},
      .bitmap = {.width = 2, .height = 2, .stride = 8,
          .pixels = {0,255,0,255,0,0,255,255,255,255,255,255,0,0,0,255}}};
  auto image = [&](Rect bounds, const ImageAsset& source, ImageSamplingMode sampling, std::size_t index) -> Result<void> {
    auto resolved = dx12_resolve_image_draw(state, Dx12PendingImageDraw{ImageDraw{
        .bounds = bounds, .asset = describe_image_asset(source), .sampling = sampling}});
    if (!resolved) return std::unexpected(resolved.error());
    auto* draw = std::get_if<Dx12TexturedDraw>(&*resolved);
    if (!draw) return std::unexpected(dx12_error(E_UNEXPECTED, "supplement requires installed immutable images"));
    result.push_back({std::move(*draw), RendererPrimitiveKind::image, index}); return {};
  };
  result.push_back({build_solid_rect_geometry(std::span{&rect, 1}), RendererPrimitiveKind::solid_rect, 0});
  if (auto added = image({{4,4},{18,17}}, srv_tile_image(0), ImageSamplingMode::nearest, 1); !added)
    return std::unexpected(added.error());
  result.push_back({build_rounded_rect_geometry(std::span{&rounded, 1}), RendererPrimitiveKind::rounded_rect, 2});
  if (auto added = image({{15,14},{13,13}}, patterned, ImageSamplingMode::linear, 3); !added)
    return std::unexpected(added.error());
  return result;
}
Result<void> reuse_raw_submit(Dx12State& state, std::span<const Dx12DrawCommand> draws) {
  return dx12_submit_frame(state, {0.02F,0.04F,0.07F,1}, draws, false, draws.size());
}
HRESULT ReuseFailListReset::reset_allocator(ID3D12CommandAllocator& allocator) noexcept {
  ++allocator_calls;
  allocator_result = dx12_native_glyph_command_api().reset_allocator(allocator);
  return allocator_result;
}
HRESULT ReuseFailListReset::reset_list(ID3D12GraphicsCommandList&, ID3D12CommandAllocator&, ID3D12PipelineState*) noexcept {
  ++list_calls; list_result = E_ACCESSDENIED;
  return list_result;
}
HRESULT ReuseFailListReset::close(ID3D12GraphicsCommandList& list) noexcept {
  ++close_calls;
  close_result = dx12_native_glyph_command_api().close(list);
  return close_result;
}
} // namespace cgpui_test

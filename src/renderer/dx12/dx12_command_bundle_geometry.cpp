#include "dx12_command_bundle_geometry_internal.hpp"
#include <cassert>

namespace cgpui {
Dx12CommandBundleGeometry::Dx12CommandBundleGeometry() = default;
Dx12CommandBundleGeometry::~Dx12CommandBundleGeometry() = default;
Result<void> Dx12CommandBundleGeometry::prepare(ID3D12Device& device, std::span<const Dx12DrawCommand> draws) {
  assert(!frozen_ && groups_.empty());
  groups_.reserve(draws.size());
  for (const auto& command : draws) {
    Dx12PreparedDraw prepared;
    if (const auto* geometry = std::get_if<RectGeometry>(&command.payload)) {
      auto vertices = storage_.upload(device, geometry->vertices.data(), geometry->vertices.size() * sizeof(RectVertex));
      if (!vertices) return std::unexpected(vertices.error());
      auto indices = storage_.upload(device, geometry->indices.data(), geometry->indices.size() * sizeof(std::uint32_t));
      if (!indices) return std::unexpected(indices.error());
      prepared.vertex = {vertices->address, vertices->size, sizeof(RectVertex)};
      prepared.index = {indices->address, indices->size, DXGI_FORMAT_R32_UINT}; prepared.indexed = true;
    } else {
      const auto& image = std::get<Dx12TexturedDraw>(command.payload);
      auto vertices = storage_.upload(device, image.vertices.data(), image.vertices.size() * sizeof(Dx12TextureVertex));
      if (!vertices) return std::unexpected(vertices.error());
      prepared.vertex = {vertices->address, vertices->size, sizeof(Dx12TextureVertex)};
      prepared.count = static_cast<UINT>(image.vertices.size());
    }
    groups_.push_back(prepared);
  }
  if (storage_.stats().page_count > 4 || storage_.stats().capacity_bytes > 4 * 256 * 1024)
    return std::unexpected(dx12_error(E_UNEXPECTED, "reuse geometry budget disagrees with real allocator"));
  frozen_ = true; return {};
}
const Dx12PreparedDraw& Dx12CommandBundleGeometry::views(std::size_t group) const noexcept {
  assert(frozen_); return groups_[group];
}
Dx12GeometryUploadStats Dx12CommandBundleGeometry::stats() const noexcept { return storage_.stats(); }
} // namespace cgpui

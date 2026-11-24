#pragma once
#include "dx12_geometry_upload_internal.hpp"
#include "dx12_prepared_draw_internal.hpp"
#include "dx12_draw_commands_internal.hpp"
#include <memory>

namespace cgpui {
class Dx12CommandBundleGeometry {
 public:
  Dx12CommandBundleGeometry();
  ~Dx12CommandBundleGeometry();
  Result<void> prepare(ID3D12Device&, std::span<const Dx12DrawCommand>);
  [[nodiscard]] const Dx12PreparedDraw& views(std::size_t group) const noexcept;
  [[nodiscard]] Dx12GeometryUploadStats stats() const noexcept;
 private:
  Dx12GeometryUploads storage_; // Dedicated owner; never begin_frame() after packing.
  std::vector<Dx12PreparedDraw> groups_;
  bool frozen_ = false;
};
} // namespace cgpui

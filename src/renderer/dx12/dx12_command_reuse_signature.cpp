#include "dx12_command_reuse_signature_internal.hpp"
#include <bit>

namespace cgpui {
namespace {
void scalar(std::vector<std::uint64_t>& key, float value) { key.push_back(std::bit_cast<std::uint32_t>(value)); }
void clip(std::vector<std::uint64_t>& key, const std::optional<Rect>& value) {
  key.push_back(bool(value));
  if (value) { scalar(key, value->origin.x); scalar(key, value->origin.y);
    scalar(key, value->size.width); scalar(key, value->size.height); }
}
void pointer(std::vector<std::uint64_t>& key, const void* value) {
  key.push_back(reinterpret_cast<std::uintptr_t>(value));
}
}
bool dx12_reuse_signature_equal(const Dx12ReuseScene& a, const Dx12ReuseScene& b) noexcept {
  return a.signature == b.signature; // Owned values, never struct padding or hash-only equality.
}
void dx12_append_reuse_signature(Dx12ReuseScene& scene, std::span<const Dx12DrawCommand> draws) {
  auto& key = scene.signature;
  key.push_back(draws.size());
  for (std::size_t group = 0; group < draws.size(); ++group) {
    const auto& command = draws[group];
    key.insert(key.end(), {std::uint64_t(command.kind), command.command_index, command.resource_offset, command.payload.index()});
    if (const auto* geometry = std::get_if<RectGeometry>(&command.payload)) {
      key.insert(key.end(), {geometry->vertices.size(), geometry->indices.size(), geometry->draws.size()});
      for (const auto& vertex : geometry->vertices) {
        for (auto value : vertex.position) scalar(key, value);
        for (auto value : vertex.color) scalar(key, value);
        scalar(key, vertex.coverage);
      }
      for (auto value : geometry->indices) key.push_back(value);
      for (const auto& range : geometry->draws) {
        key.insert(key.end(), {range.source_index, range.first_vertex, range.vertex_count, range.first_index, range.index_count});
        clip(key, range.clip_rect);
      }
    } else {
      const auto& image = std::get<Dx12TexturedDraw>(command.payload);
      pointer(key, image.texture.get()); key.push_back(image.vertices.size());
      key.push_back(std::uint64_t(image.sampling)); clip(key, image.clip_rect);
      for (const auto& vertex : image.vertices) {
        for (auto value : vertex.position) scalar(key, value);
        for (auto value : vertex.uv) scalar(key, value);
        for (auto value : vertex.color) scalar(key, value);
      }
      const auto& b = scene.bindings[group];
      for (const void* value : {static_cast<void*>(b.resource), static_cast<void*>(b.pipeline),
            static_cast<void*>(b.root), static_cast<void*>(b.heap), static_cast<void*>(b.samplers)}) pointer(key, value);
      key.insert(key.end(), {b.srv.ptr, b.sampler.ptr});
    }
  }
}
} // namespace cgpui

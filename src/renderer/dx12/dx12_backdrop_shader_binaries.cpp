#include "dx12_shader_binaries_internal.hpp"
#include <array>
#include <cstdint>

namespace cgpui {
namespace {
#include "generated/backdrop_blur_dxil.inc"
}
D3D12_SHADER_BYTECODE dx12_backdrop_fullscreen_vertex_shader() {
  return {backdrop_fullscreen_vertex_dxil.data(), backdrop_fullscreen_vertex_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_backdrop_downsample_fragment_shader() {
  return {backdrop_downsample_fragment_dxil.data(), backdrop_downsample_fragment_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_backdrop_blur_fragment_shader() {
  return {backdrop_blur_fragment_dxil.data(), backdrop_blur_fragment_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_backdrop_composite_vertex_shader() {
  return {backdrop_composite_vertex_dxil.data(), backdrop_composite_vertex_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_backdrop_composite_fragment_shader() {
  return {backdrop_composite_fragment_dxil.data(), backdrop_composite_fragment_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_backdrop_composite_alpha_fragment_shader() {
  return {backdrop_composite_alpha_fragment_dxil.data(), backdrop_composite_alpha_fragment_dxil.size()};
}
} // namespace cgpui

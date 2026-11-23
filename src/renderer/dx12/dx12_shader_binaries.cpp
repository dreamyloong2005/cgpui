#include "dx12_shader_binaries_internal.hpp"
#include <array>
#include <cstdint>

namespace cgpui {
namespace {
#include "generated/rounded_rect_dxil.inc"
#include "generated/text_dxil.inc"
#include "generated/image_dxil.inc"
#include "generated/box_shadow_dxil.inc"
}
D3D12_SHADER_BYTECODE dx12_rect_vertex_shader() {
  return {rect_vertex_dxil.data(), rect_vertex_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_rect_fragment_shader() {
  return {rect_fragment_dxil.data(), rect_fragment_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_text_vertex_shader() {
  return {text_vertex_dxil.data(), text_vertex_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_text_fragment_shader() {
  return {text_fragment_dxil.data(), text_fragment_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_image_vertex_shader() {
  return {image_vertex_dxil.data(), image_vertex_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_image_fragment_shader() {
  return {image_fragment_dxil.data(), image_fragment_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_shadow_vertex_shader() {
  return {shadow_vertex_dxil.data(), shadow_vertex_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_shadow_fragment_shader() {
  return {shadow_fragment_dxil.data(), shadow_fragment_dxil.size()};
}
}

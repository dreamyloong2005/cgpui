#include "dx12_shader_binaries_internal.hpp"
#include <array>
#include <cstdint>

namespace cgpui {
namespace {
#include "generated/rounded_rect_dxil.inc"
}
D3D12_SHADER_BYTECODE dx12_rect_vertex_shader() {
  return {rect_vertex_dxil.data(), rect_vertex_dxil.size()};
}
D3D12_SHADER_BYTECODE dx12_rect_fragment_shader() {
  return {rect_fragment_dxil.data(), rect_fragment_dxil.size()};
}
}

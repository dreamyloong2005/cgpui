#pragma once
#include "dx12_native_internal.hpp"

namespace cgpui {
D3D12_SHADER_BYTECODE dx12_rect_vertex_shader();
D3D12_SHADER_BYTECODE dx12_rect_fragment_shader();
D3D12_SHADER_BYTECODE dx12_text_vertex_shader();
D3D12_SHADER_BYTECODE dx12_text_fragment_shader();
D3D12_SHADER_BYTECODE dx12_image_vertex_shader();
D3D12_SHADER_BYTECODE dx12_image_fragment_shader();
D3D12_SHADER_BYTECODE dx12_shadow_vertex_shader();
D3D12_SHADER_BYTECODE dx12_shadow_fragment_shader();
}

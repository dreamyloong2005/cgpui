#pragma once
#include "dx12_command_reuse_supplement_test_support.hpp"
namespace cgpui_test {
std::weak_ptr<cgpui::Dx12State> reuse_public_state(const cgpui::RendererBackendInfo&, HWND);
void reuse_public_scene(cgpui::RenderFrame&, bool upload);
} // namespace cgpui_test

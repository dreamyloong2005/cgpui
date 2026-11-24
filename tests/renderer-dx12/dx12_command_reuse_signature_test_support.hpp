#pragma once
#include "../../src/renderer/dx12/dx12_command_reuse_signature_internal.hpp"
#include <functional>
#include <string>
#include <utility>

namespace cgpui_test {
struct SignatureFixture {
  std::vector<cgpui::Dx12DrawCommand> draws;
  cgpui::Dx12ReuseScene scene;
};
using SignatureMutation = std::pair<std::string, std::function<void(SignatureFixture&)>>;
SignatureFixture signature_fixture();
cgpui::RectGeometry& signature_rect(SignatureFixture&);
cgpui::Dx12TexturedDraw& signature_image(SignatureFixture&);
cgpui::Dx12ReuseScene signature_serialized(const SignatureFixture&);
void* signature_token(unsigned index); // Identity bytes only: never native dereference/AddRef.
std::vector<SignatureMutation> signature_geometry_mutations();
std::vector<SignatureMutation> signature_image_mutations();
void signature_require(bool, const std::string&);
} // namespace cgpui_test

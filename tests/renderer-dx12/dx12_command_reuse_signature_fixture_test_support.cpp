#include "dx12_command_reuse_signature_test_support.hpp"
#include <array>
#include <stdexcept>

namespace cgpui_test {
using namespace cgpui;
void signature_require(bool value, const std::string& message) {
  if (!value) throw std::runtime_error(message);
}
void* signature_token(unsigned index) {
  static std::array<std::uint64_t, 16> identities{};
  return &identities.at(index);
}
RectGeometry& signature_rect(SignatureFixture& x) { return std::get<RectGeometry>(x.draws[0].payload); }
Dx12TexturedDraw& signature_image(SignatureFixture& x) { return std::get<Dx12TexturedDraw>(x.draws[1].payload); }
SignatureFixture signature_fixture() {
  SignatureFixture x;
  RectGeometry rect;
  rect.vertices = {{{0,0},{1,0,0,1},1}, {{8,0},{0,1,0,1},1}, {{0,8},{0,0,1,1},1}};
  rect.indices = {0,1,2};
  rect.draws = {{.source_index=0,.first_vertex=0,.vertex_count=3,.first_index=0,.index_count=3,
      .clip_rect=Rect{{0,0},{64,64}}}};
  auto texture = std::make_shared<Dx12Texture>(Dx12Texture{.width=1,.height=1,.stride=4,
      .format=DXGI_FORMAT_R8G8B8A8_UNORM,.pixels={255,0,0,255}});
  Dx12TexturedDraw image{.texture=texture,
      .vertices={{{0,0},{0,0},{1,1,1,1}},{{8,0},{1,0},{1,1,1,1}},{{0,8},{0,1},{1,1,1,1}}},
      .clip_rect=Rect{{0,0},{64,64}},.sampling=ImageSamplingMode::nearest};
  x.draws = {{std::move(rect),RendererPrimitiveKind::solid_rect,0},
             {std::move(image),RendererPrimitiveKind::image,1}};
  x.scene.bindings.resize(2);
  x.scene.bindings[1] = {reinterpret_cast<ID3D12Resource*>(signature_token(0)),
      reinterpret_cast<ID3D12PipelineState*>(signature_token(1)),
      reinterpret_cast<ID3D12RootSignature*>(signature_token(2)),
      reinterpret_cast<ID3D12DescriptorHeap*>(signature_token(3)),
      reinterpret_cast<ID3D12DescriptorHeap*>(signature_token(4)),{100},{200}};
  return x;
}
Dx12ReuseScene signature_serialized(const SignatureFixture& x) {
  auto scene = x.scene;
  scene.signature.clear(); // CPU serializer fields only; scene header is tested through actual selector.
  dx12_append_reuse_signature(scene, x.draws);
  return scene;
}
} // namespace cgpui_test

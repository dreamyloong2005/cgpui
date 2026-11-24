#include "dx12_command_reuse_signature_test_support.hpp"

namespace cgpui_test {
using namespace cgpui;
std::vector<SignatureMutation> signature_image_mutations() {
  std::vector<SignatureMutation> cases;
  auto add = [&](std::string name, auto mutate) { cases.emplace_back(std::move(name), mutate); };
  add("image.cpu.identity",[](auto& x){signature_image(x).texture=std::make_shared<Dx12Texture>(*signature_image(x).texture);});
  add("image.vertex.count",[](auto& x){signature_image(x).vertices.push_back(signature_image(x).vertices[0]);});
  add("image.sampling",[](auto& x){signature_image(x).sampling=ImageSamplingMode::linear;});
  for (std::size_t vertex=0; vertex!=3; ++vertex) {
    for (std::size_t axis=0; axis!=2; ++axis) {
      add("image.position."+std::to_string(vertex)+"."+std::to_string(axis),[=](auto& x){signature_image(x).vertices[vertex].position[axis]+=1;});
      add("image.uv."+std::to_string(vertex)+"."+std::to_string(axis),[=](auto& x){signature_image(x).vertices[vertex].uv[axis]+=0.25F;});
    }
    for (std::size_t channel=0; channel!=4; ++channel)
      add("image.color."+std::to_string(vertex)+"."+std::to_string(channel),[=](auto& x){signature_image(x).vertices[vertex].color[channel]-=0.25F;});
  }
  add("image.clip.present",[](auto& x){signature_image(x).clip_rect.reset();});
  add("image.clip.x",[](auto& x){signature_image(x).clip_rect->origin.x+=1;});
  add("image.clip.y",[](auto& x){signature_image(x).clip_rect->origin.y+=1;});
  add("image.clip.width",[](auto& x){signature_image(x).clip_rect->size.width-=1;});
  add("image.clip.height",[](auto& x){signature_image(x).clip_rect->size.height-=1;});
  add("native.resource",[](auto& x){x.scene.bindings[1].resource=reinterpret_cast<ID3D12Resource*>(signature_token(8));});
  add("native.pso",[](auto& x){x.scene.bindings[1].pipeline=reinterpret_cast<ID3D12PipelineState*>(signature_token(8));});
  add("native.root",[](auto& x){x.scene.bindings[1].root=reinterpret_cast<ID3D12RootSignature*>(signature_token(8));});
  add("native.srv.heap",[](auto& x){x.scene.bindings[1].heap=reinterpret_cast<ID3D12DescriptorHeap*>(signature_token(8));});
  add("native.sampler.heap",[](auto& x){x.scene.bindings[1].samplers=reinterpret_cast<ID3D12DescriptorHeap*>(signature_token(8));});
  add("native.srv.handle",[](auto& x){++x.scene.bindings[1].srv.ptr;});
  add("native.sampler.handle",[](auto& x){++x.scene.bindings[1].sampler.ptr;});
  return cases;
}
} // namespace cgpui_test

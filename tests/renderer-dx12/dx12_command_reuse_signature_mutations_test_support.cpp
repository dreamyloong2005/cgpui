#include "dx12_command_reuse_signature_test_support.hpp"
#include <cmath>

namespace cgpui_test {
using namespace cgpui;
std::vector<SignatureMutation> signature_geometry_mutations() {
  std::vector<SignatureMutation> cases;
  auto add = [&](std::string name, auto mutate) { cases.emplace_back(std::move(name), mutate); };
  for (std::size_t group=0; group!=2; ++group) {
    const auto name = std::to_string(group);
    add("command.kind."+name,[=](auto& x){x.draws[group].kind=RendererPrimitiveKind::text_caret;});
    add("command.index."+name,[=](auto& x){++x.draws[group].command_index;});
    add("command.offset."+name,[=](auto& x){++x.draws[group].resource_offset;});
  }
  add("group.count",[](auto& x){x.draws.push_back(x.draws[0]);x.scene.bindings.push_back({});});
  add("group.order",[](auto& x){std::swap(x.draws[0],x.draws[1]);std::swap(x.scene.bindings[0],x.scene.bindings[1]);});
  add("payload.kind",[](auto& x){x.draws[0].payload=x.draws[1].payload;x.scene.bindings[0]=x.scene.bindings[1];});
  add("rect.vertex.count",[](auto& x){signature_rect(x).vertices.push_back(signature_rect(x).vertices[0]);});
  add("rect.index.count",[](auto& x){signature_rect(x).indices.push_back(0);});
  add("rect.range.count",[](auto& x){signature_rect(x).draws.push_back(signature_rect(x).draws[0]);});
  for (std::size_t vertex=0; vertex!=3; ++vertex) {
    for (std::size_t axis=0; axis!=2; ++axis)
      add("rect.position."+std::to_string(vertex)+"."+std::to_string(axis),[=](auto& x){signature_rect(x).vertices[vertex].position[axis]+=1;});
    for (std::size_t channel=0; channel!=4; ++channel)
      add("rect.color."+std::to_string(vertex)+"."+std::to_string(channel),[=](auto& x){signature_rect(x).vertices[vertex].color[channel]+=0.25F;});
    add("rect.coverage."+std::to_string(vertex),[=](auto& x){signature_rect(x).vertices[vertex].coverage=0.5F;});
    add("rect.index."+std::to_string(vertex),[=](auto& x){signature_rect(x).indices[vertex]=(signature_rect(x).indices[vertex]+1)%3;});
  }
  constexpr std::size_t RectDrawRange::* fields[] = {&RectDrawRange::source_index,&RectDrawRange::first_vertex,
      &RectDrawRange::vertex_count,&RectDrawRange::first_index,&RectDrawRange::index_count};
  for (std::size_t index=0; index!=5; ++index) {
    const auto field=fields[index];
    add("rect.range.field."+std::to_string(index),[=](auto& x){++(signature_rect(x).draws[0].*field);});
  }
  add("rect.clip.present",[](auto& x){signature_rect(x).draws[0].clip_rect.reset();});
  add("rect.clip.x",[](auto& x){signature_rect(x).draws[0].clip_rect->origin.x+=1;});
  add("rect.clip.y",[](auto& x){signature_rect(x).draws[0].clip_rect->origin.y+=1;});
  add("rect.clip.width",[](auto& x){signature_rect(x).draws[0].clip_rect->size.width-=1;});
  add("rect.clip.height",[](auto& x){signature_rect(x).draws[0].clip_rect->size.height-=1;});
  add("exact.signed.zero",[](auto& x){signature_rect(x).vertices[0].position[0]=-0.0F;});
  add("exact.one.ulp",[](auto& x){signature_rect(x).vertices[0].coverage=std::nextafter(1.0F,2.0F);});
  return cases;
}
} // namespace cgpui_test

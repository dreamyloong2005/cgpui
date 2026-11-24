#include "dx12_command_reuse_selection_test_support.hpp"
#include "../../src/renderer/dx12/dx12_texture_upload_internal.hpp"
#include <algorithm>

namespace cgpui_test {
using namespace cgpui;
SelectionObservation selection_observation(const Dx12State& state) {
  SelectionObservation out;
  for (const auto& [key,native] : state.texture_renderer->native_textures)
    out.native.emplace_back(key,native.resource.Get(),native.srv.heap(),native.srv.gpu().ptr,native.uploaded);
  out.events=state.command_observation->events().size();
  out.slots=state.srv_pool->stats().live_slots; out.pages=state.srv_pool->stats().page_count;
  out.pending=state.texture_renderer->pending_uploads.size(); out.retained=state.texture_renderer->retained.size();
  out.uploads=state.texture_uploads->stats().total_allocations;
  return out; // No CPU pins or native COM references escape the snapshot.
}
std::vector<SelectionCase> selection_policy_cases(const std::vector<Dx12DrawCommand>& base) {
  std::vector<SelectionCase> out;
  auto add=[&](const char* name,auto change,bool eligible=false){auto draws=base;change(draws);out.push_back({name,std::move(draws),eligible});};
  add("unchanged",[](auto&){},true);
  add("empty",[](auto& x){x.clear();});
  add("missing",[](auto& x){x[1].payload=Dx12MissingImageDraw{};});
  add("pending",[](auto& x){x[1].payload=Dx12PendingImageDraw{};});
  add("shadow",[](auto& x){x[0].payload=BoxShadowGeometry{};});
  add("backdrop",[](auto& x){x[0].payload=BackdropBlurPlan{};});
  add("glyph",[](auto& x){std::get<Dx12TexturedDraw>(x[1].payload).glyph=true;});
  add("image.kind",[](auto& x){x[1].kind=RendererPrimitiveKind::solid_rect;});
  add("rect.kind",[](auto& x){x[0].kind=RendererPrimitiveKind::image;});
  add("cold.snapshot",[](auto& x){auto& d=std::get<Dx12TexturedDraw>(x[1].payload);d.texture=std::make_shared<Dx12Texture>(*d.texture);});
  add("absent.texture",[](auto& x){std::get<Dx12TexturedDraw>(x[1].payload).texture.reset();});
  add("wrong.format",[](auto& x){auto& d=std::get<Dx12TexturedDraw>(x[1].payload);auto t=std::make_shared<Dx12Texture>(*d.texture);t->format=DXGI_FORMAT_R8_UNORM;d.texture=std::move(t);});
  add("empty.image",[](auto& x){std::get<Dx12TexturedDraw>(x[1].payload).vertices.clear();});
  add("empty.rect.vertices",[](auto& x){std::get<RectGeometry>(x[0].payload).vertices.clear();});
  add("empty.rect.indices",[](auto& x){std::get<RectGeometry>(x[0].payload).indices.clear();});
  add("empty.rect.ranges",[](auto& x){std::get<RectGeometry>(x[0].payload).draws.clear();});
  add("empty.range",[](auto& x){std::get<RectGeometry>(x[0].payload).draws[0].index_count=0;});
  add("culled",[](auto& x){std::get<RectGeometry>(x[0].payload).draws[0].clip_rect=Rect{{100,100},{1,1}};});
  add("different.scissor",[](auto& x){std::get<RectGeometry>(x[0].payload).draws[0].clip_rect=Rect{{0,0},{63,64}};});
  add("opaque.paint.alpha",[](auto& x){std::get<RectGeometry>(x[0].payload).vertices[0].color[3]=0.5F;
      std::get<Dx12TexturedDraw>(x[1].payload).vertices[0].color[3]=0.5F;},true);
  for (const auto groups : {256U,257U}) {
    auto draws=std::vector<Dx12DrawCommand>(groups,base[0]);
    out.push_back({"group.boundary."+std::to_string(groups),std::move(draws),groups==256});
  }
  for (const auto ranges : {512U,513U}) {
    auto command=base[0];auto& geometry=std::get<RectGeometry>(command.payload);
    geometry.draws.resize(ranges,geometry.draws[0]);
    out.push_back({"draw.boundary."+std::to_string(ranges),{std::move(command)},ranges==512});
  }
  add("oversize.vertex.slice",[](auto& x){std::get<RectGeometry>(x[0].payload).vertices.resize(256*1024/sizeof(RectVertex)+1);});
  add("oversize.index.slice",[](auto& x){std::get<RectGeometry>(x[0].payload).indices.resize(256*1024/sizeof(std::uint32_t)+1);});
  for (const auto groups : {2U,3U}) {
    auto command=base[0];auto& geometry=std::get<RectGeometry>(command.payload);
    geometry.vertices.resize(256*1024/sizeof(RectVertex)); // Each vertices+indices pair needs two ordinary pages.
    out.push_back({"page.boundary."+std::to_string(groups*2),std::vector<Dx12DrawCommand>(groups,command),groups==2});
  }
  return out;
}
} // namespace cgpui_test

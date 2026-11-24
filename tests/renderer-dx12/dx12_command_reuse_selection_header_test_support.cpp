#include "dx12_command_reuse_selection_test_support.hpp"
#include <bit>

namespace cgpui_test {
using namespace cgpui;
namespace {
struct RestoreHeader {
  Dx12State& state;
  UINT width, height;
  DpiScale scale;
  std::uint64_t generation;
  bool composition, transparent;
  explicit RestoreHeader(Dx12State& s) : state(s),width(s.width),height(s.height),scale(s.scale),
      generation(s.info.device_generation),composition(s.composition_surface),transparent(s.transparent_background) {}
  ~RestoreHeader() {state.width=width;state.height=height;state.scale=scale;state.info.device_generation=generation;
      state.composition_surface=composition;state.transparent_background=transparent;}
};
}
void selection_header_controls(Dx12State& state,std::span<const Dx12DrawCommand> draws) {
  const Color clear{0.02F,0.04F,0.07F,1};
  auto selected=dx12_select_reuse_scene(state,clear,draws,false,false,1);
  signature_require(selected && bool(*selected),"warm real selector prerequisite");
  const auto expected=(**selected).signature;
  signature_require(expected.at(9)==reinterpret_cast<std::uintptr_t>(state.pipeline.Get()) &&
      expected.at(10)==reinterpret_cast<std::uintptr_t>(state.root_signature.Get()) &&
      expected.at(11)==DXGI_FORMAT_R8G8B8A8_UNORM && expected.at(12)==1,"actual selector fixed/native header mapping");
  selected->reset(); // No extra selected-scene pins across fixture transitions.
  auto changed=[&](std::size_t field,std::uint64_t value,Color color,std::uint64_t surface=1){
    auto actual=dx12_select_reuse_scene(state,color,draws,false,false,surface);
    signature_require(actual && bool(*actual),"header query unexpectedly ineligible");
    const auto& signature=(**actual).signature;
    signature_require(signature!=expected && signature.at(field)==value,"actual selector omitted header field "+std::to_string(field));
  };
  {RestoreHeader restore(state);++state.info.device_generation;changed(0,state.info.device_generation,clear);}
  changed(1,2,clear,2);
  {RestoreHeader restore(state);++state.width;changed(2,state.width,clear);}
  {RestoreHeader restore(state);++state.height;changed(3,state.height,clear);}
  {RestoreHeader restore(state);state.scale={2};changed(4,std::bit_cast<std::uint32_t>(2.0F),clear);}
  float Color::* const channels[]={&Color::r,&Color::g,&Color::b,&Color::a};
  for (std::size_t channel=0;channel!=4;++channel) {
    auto color=clear;color.*channels[channel]=0.5F;
    changed(5+channel,std::bit_cast<std::uint32_t>(0.5F),color);
  }
  for (const bool composition : {false,true}) {
    RestoreHeader restore(state);
    if (composition) state.composition_surface=true; else state.transparent_background=true;
    auto actual=dx12_select_reuse_scene(state,clear,draws,false,false,1);
    signature_require(actual && !*actual,"surface alpha must reject while paint alpha remains eligible");
  }
  auto equal=dx12_select_reuse_scene(state,clear,draws,false,false,1);
  signature_require(equal && bool(*equal) && (**equal).signature==expected,"header restoration changed native query identity");
}
} // namespace cgpui_test

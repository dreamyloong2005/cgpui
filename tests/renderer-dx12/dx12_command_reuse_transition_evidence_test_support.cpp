#include "dx12_command_reuse_transition_test_support.hpp"
#include "../../src/renderer/dx12/dx12_frame_diagnostics_internal.hpp"

namespace cgpui_test {
using namespace cgpui;
TransitionFrame transition_frame(TransitionFixture& fixture,TransitionScene scene,TransitionUpload upload,bool capture,
    TransitionRoute route,ID3D12GraphicsCommandList* previous,std::size_t images) {
  auto& state=*fixture.state;auto& tail=*fixture.tail;const auto preceding=tail.parent;
  transition_require(transition_present(fixture.state,scene,upload,capture),"transition production Present failed");
  transition_require(!tail.failed && tail.parent>preceding && tail.completed_parent==tail.parent && tail.ticket &&
      !state.submitted && state.info.backend==RendererBackend::dx12 && state.info.adapter_luid && state.info.device_generation &&
      state.diagnostics && state.diagnostics->submitted_draws.image_count==images &&
      !state.command_observation->overflowed() && bool(dx12_check_debug_errors(state)),"transition tail/backend/debug/diagnostics failed");
  unsigned creates=0,closes=0,executes=0,direct_draws=0,bundle_draws=0,producer=0,verifier=0,complete=0;
  unsigned resets=0,parent_closes=0,targets=0;
  ID3D12GraphicsCommandList* created=nullptr;ID3D12CommandAllocator* allocator=nullptr;
  ID3D12GraphicsCommandList* executed=nullptr;ID3D12GraphicsCommandList* parent_list=nullptr;
  void* target=nullptr;void* queue=nullptr;
  for(const auto& e:state.command_observation->events()) if(e.parent==tail.parent) {
    using K=Dx12CommandEventKind;
    if(e.kind==K::parent_reset) {
      transition_require(e.list && e.allocator && e.type==D3D12_COMMAND_LIST_TYPE_DIRECT,"wrong actual parent reset");
      parent_list=e.list;++resets;
    }
    if(e.kind==K::parent_target) {transition_require(resets==1 && e.list==parent_list && e.a!=0,"missing actual RTV/parent");target=e.object;++targets;}
    if(e.kind==K::bundle_created) {
      transition_require(e.list && e.allocator && e.type==D3D12_COMMAND_LIST_TYPE_BUNDLE && SUCCEEDED(e.result),"native BUNDLE creation failed");
      ++creates;created=e.list;allocator=e.allocator;
    }
    if(e.kind==K::bundle_closed) {
      transition_require(e.list==created && e.allocator==allocator && e.type==D3D12_COMMAND_LIST_TYPE_BUNDLE && SUCCEEDED(e.result),"wrong native Close identity/result");++closes;
    }
    if(e.kind==K::execute_bundle) {transition_require(e.list==parent_list && e.type==D3D12_COMMAND_LIST_TYPE_DIRECT &&
        !parent_closes && (!created || closes==1),"wrong bundle parent/close order");
      executed=static_cast<ID3D12GraphicsCommandList*>(e.object);++executes;}
    if(e.kind==K::draw || e.kind==K::indexed_draw) {
      transition_require(!parent_closes,"draw after parent Close");
      if(e.type==D3D12_COMMAND_LIST_TYPE_DIRECT) {transition_require(e.list==parent_list,"wrong direct draw list");++direct_draws;}
      else {transition_require(e.type==D3D12_COMMAND_LIST_TYPE_BUNDLE && e.list==created && !closes,"draw from wrong/closed BUNDLE");++bundle_draws;}
    }
    if(e.kind==K::parent_closed) {transition_require(e.list==parent_list && SUCCEEDED(e.result),"wrong parent Close identity/result");++parent_closes;}
    if(e.kind==K::producer_execute) {
      transition_require(targets==1 && parent_closes==1 && target && target==tail.target_identity && e.object2==target &&
          e.list==parent_list && !verifier && !complete,"producer target/parent/order");
      queue=e.object;++producer;
    }
    if(e.kind==K::tail_execute) {transition_require(producer==1 && queue && e.object && e.object2==queue && !complete,"tail queue/order");++verifier;}
    if(e.kind==K::completed) {transition_require(producer==1 && verifier==1 && e.a==tail.ticket,"completed ticket/order");++complete;}
  }
  transition_require(resets==1 && parent_closes==1 && producer==1 && verifier==1 && complete==1,"one actual producer+verifier+completion required");
  const auto draw_count=images+2; // Rect and rounded rect, plus the actually available images.
  if(route==TransitionRoute::direct)
    transition_require(!creates && !closes && !executes && !bundle_draws && direct_draws==draw_count,"ineligible/public-mutation frame did not use exact DIRECT route");
  else if(route==TransitionRoute::fresh)
    transition_require(creates==1 && closes==1 && executes==1 && created==executed && !direct_draws && bundle_draws==draw_count,"eligible transition did not create/close/execute fresh actual BUNDLE");
  else transition_require(previous && !creates && !closes && executes==1 && executed==previous && !direct_draws && !bundle_draws,
      "unchanged transition did not execute same immutable native BUNDLE without recording");
  transition_require(tail.pixels.valid() && !tail.pixels.rgba8.empty(),"missing same-frame tail pixels");
  if(capture) transition_require(state.pixels && state.pixels->rgba8==tail.pixels.rgba8,"independent explicit DIRECT capture disagrees with same-frame tail");
  if(upload==TransitionUpload::initial) transition_require(dx12_pixel(&tail.pixels,6,6,{255,0,0,255},0) &&
      dx12_pixel(&tail.pixels,16,15,{0,255,0,255},0) && dx12_pixel(&tail.pixels,21,20,{64,128,128,255},2),"initial distinguishable red/pattern pixels wrong");
  const std::size_t expected_upload=upload==TransitionUpload::initial?20:
      (upload==TransitionUpload::replace || upload==TransitionUpload::resupply?4:0);
  transition_require(state.diagnostics->upload_bytes.image_byte_count==expected_upload,"actual native image upload bytes differ from public request semantics");
  return {executed,tail.pixels.rgba8,state.diagnostics->upload_bytes.image_byte_count};
}
} // namespace cgpui_test

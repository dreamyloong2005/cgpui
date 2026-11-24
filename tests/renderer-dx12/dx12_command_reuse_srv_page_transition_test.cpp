#include "dx12_command_reuse_transition_test_support.hpp"
#include <exception>

int main() {
  using namespace cgpui;
  using namespace cgpui_test;
  try {
    Dx12RecoveryDeadline deadline;transition_require(deadline.done!=nullptr,"missing bounded deadline");
    TransitionFixture fixture;
    fixture.state->srv_pool=std::make_unique<Dx12SrvDescriptorPool>(3);
    // Real unused leases consume two slots before either real image is realized.
    // Both local leases die before the state; no fabricated native binding/readiness.
    auto first=fixture.state->srv_pool->allocate(*fixture.state->device.Get(),1);
    auto second=fixture.state->srv_pool->allocate(*fixture.state->device.Get(),1);
    transition_require(bool(first) && bool(second),"real SRV pool prefill failed");
    const auto full=transition_frame(fixture,{},TransitionUpload::initial,true,TransitionRoute::direct);
    transition_require(srv_topology(*fixture.state,2,2),"real images did not cross distinct shader-visible heaps");
    const auto stats=fixture.state->srv_pool->stats();
    transition_require(stats.live_slots==4 && stats.page_count==2,"unexpected real SRV slot/page topology");
    const TransitionScene one_image{TransitionChange::none,false};
    const auto one=transition_frame(fixture,one_image,TransitionUpload::none,true,TransitionRoute::direct,nullptr,1);
    transition_pixels(one,full,false,"one-image page control must differ visibly");
    auto fresh=transition_frame(fixture,one_image,TransitionUpload::none,false,TransitionRoute::fresh,nullptr,1);
    transition_pixels(fresh,one,true,"one real heap eligible fresh pixels");
    transition_pixels(transition_frame(fixture,one_image,TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle,1),one,true,"one-heap hit pixels");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::direct),full,true,"two actual SRV heaps must force DIRECT with same full pixels");
    fresh=transition_frame(fixture,one_image,TransitionUpload::none,false,TransitionRoute::fresh,nullptr,1);
    transition_pixels(fresh,one,true,"page-boundary fallback must return to fresh eligibility");
    transition_pixels(transition_frame(fixture,one_image,TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle,1),one,true,"page-boundary recovery hit pixels");
    transition_require(srv_topology(*fixture.state,2,2) && fixture.state->srv_pool->stats().live_slots==4,"page controls changed native resource topology");
    std::cout<<"one actual SRV heap fresh/hit, two distinct heaps DIRECT, one-heap fresh/hit restoration passed\n";
    return 0;
  } catch(const std::exception& e) {return dx12_fail(1,e.what());}
}

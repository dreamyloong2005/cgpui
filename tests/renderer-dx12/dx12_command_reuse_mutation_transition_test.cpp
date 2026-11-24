#include "dx12_command_reuse_transition_test_support.hpp"
#include <exception>

int main() {
  using namespace cgpui_test;
  try {
    Dx12RecoveryDeadline deadline;transition_require(deadline.done!=nullptr,"missing bounded deadline");
    TransitionFixture fixture;
    const auto initial=transition_frame(fixture,{},TransitionUpload::initial,true,TransitionRoute::direct);
    auto fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),initial,true,"initial immutable hit pixels");
    const auto replaced=transition_frame(fixture,{},TransitionUpload::replace,false,TransitionRoute::direct);
    transition_pixels(replaced,initial,false,"public same-ID replacement did not change pixels");
    fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(fresh,replaced,true,"replacement fresh pixels");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),replaced,true,"replacement hit pixels");
    const auto missing=transition_frame(fixture,{},TransitionUpload::invalidate,false,TransitionRoute::direct,nullptr,1);
    transition_pixels(missing,replaced,false,"public invalidation must remove image pixels");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::direct,nullptr,1),missing,true,"missing image remains DIRECT without rejecting frame");
    const auto restored=transition_frame(fixture,{},TransitionUpload::resupply,false,TransitionRoute::direct);
    transition_pixels(restored,initial,true,"public resupply must restore original pixels");
    fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(fresh,initial,true,"resupply first eligible pixels");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),initial,true,"resupply immutable hit pixels");
    const auto undrawn=transition_frame(fixture,{},TransitionUpload::undrawn,false,TransitionRoute::direct);
    transition_pixels(undrawn,initial,true,"undrawn public upload must not change scene pixels");
    fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(fresh,initial,true,"undrawn mutation resets first eligible frame");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),initial,true,"undrawn mutation recovery hit pixels");
    const auto undrawn_invalidated=transition_frame(fixture,{},TransitionUpload::undrawn_invalidate,false,TransitionRoute::direct);
    transition_pixels(undrawn_invalidated,initial,true,"undrawn public invalidation must not change scene pixels");
    fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(fresh,initial,true,"undrawn invalidation resets first eligible frame");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),initial,true,"undrawn invalidation recovery hit pixels");
    std::cout<<"public replacement/invalidate/resupply/undrawn mutation wiring: exact native routes and upload diagnostics passed\n";
    return 0;
  } catch(const std::exception& e) {return dx12_fail(1,e.what());}
}

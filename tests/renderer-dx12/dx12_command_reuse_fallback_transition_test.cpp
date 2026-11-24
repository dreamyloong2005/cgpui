#include "dx12_command_reuse_transition_test_support.hpp"
#include <exception>

int main() {
  using namespace cgpui_test;
  try {
    Dx12RecoveryDeadline deadline;transition_require(deadline.done!=nullptr,"missing bounded deadline");
    TransitionFixture fixture;
    const auto baseline=transition_frame(fixture,{},TransitionUpload::initial,true,TransitionRoute::direct);
    auto fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),baseline,true,"initial hit pixels");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,true,TransitionRoute::direct),baseline,true,"public capture must force DIRECT and preserve pixels");
    fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(fresh,baseline,true,"capture recovery fresh pixels");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),baseline,true,"capture recovery hit pixels");
    const TransitionScene mixed{TransitionChange::mixed_clip};
    const auto clipped=transition_frame(fixture,mixed,TransitionUpload::none,false,TransitionRoute::direct);
    transition_pixels(clipped,baseline,false,"mixed scissors must visibly clip the first image");
    transition_pixels(transition_frame(fixture,mixed,TransitionUpload::none,true,TransitionRoute::direct),clipped,true,"mixed-scissor DIRECT pixels disagree with explicit capture");
    fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(fresh,baseline,true,"mixed-scissor fallback must not make eligibility sticky");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),baseline,true,"mixed-scissor recovery hit pixels");
    std::cout<<"capture and mixed-scissor DIRECT fallback followed by actual fresh BUNDLE and immutable hit passed\n";
    return 0;
  } catch(const std::exception& e) {return dx12_fail(1,e.what());}
}

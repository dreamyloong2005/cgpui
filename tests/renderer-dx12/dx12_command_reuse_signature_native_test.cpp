#include "dx12_command_reuse_transition_test_support.hpp"
#include <array>
#include <exception>

int main() {
  using namespace cgpui_test;
  try {
    Dx12RecoveryDeadline deadline;transition_require(deadline.done!=nullptr,"missing bounded deadline");
    TransitionFixture fixture;
    const auto baseline=transition_frame(fixture,{},TransitionUpload::initial,true,TransitionRoute::direct);
    auto fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
    transition_pixels(fresh,baseline,true,"initial eligible frame pixels");
    transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),baseline,true,"initial hit pixels");
    constexpr std::array changes{TransitionChange::clear,TransitionChange::geometry,TransitionChange::order,
        TransitionChange::common_clip,TransitionChange::sampling};
    for(const auto change:changes) {
      const TransitionScene changed{change};
      const auto different=transition_frame(fixture,changed,TransitionUpload::none,false,TransitionRoute::fresh);
      transition_pixels(different,baseline,false,"eligible changed scene did not visibly change");
      transition_pixels(transition_frame(fixture,changed,TransitionUpload::none,false,TransitionRoute::hit,different.bundle),different,true,"changed hit pixels");
      const auto direct=transition_frame(fixture,changed,TransitionUpload::none,true,TransitionRoute::direct);
      transition_pixels(direct,different,true,"changed BUNDLE differs from independent captured DIRECT control");
      fresh=transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::fresh);
      transition_pixels(fresh,baseline,true,"restored signature must restore pixels");
      transition_pixels(transition_frame(fixture,{},TransitionUpload::none,false,TransitionRoute::hit,fresh.bundle),baseline,true,"restored hit pixels");
    }
    std::cout<<"five eligible signature changes: actual fresh BUNDLE, immutable hit, independent DIRECT pixels; no speedup claim\n";
    return 0;
  } catch(const std::exception& e) {return dx12_fail(1,e.what());}
}

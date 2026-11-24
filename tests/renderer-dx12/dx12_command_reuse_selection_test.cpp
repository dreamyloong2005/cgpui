#include "dx12_command_reuse_selection_test_support.hpp"
#include <iostream>

int main() {
  using namespace cgpui;using namespace cgpui_test;
  Dx12RecoveryDeadline deadline;Dx12Window window;auto state=srv_native_state(&window);
  if (!deadline.done || !state) return dx12_fail(1,"selector controls native setup");
  try {
    state->command_observation=std::make_unique<Dx12CommandObservation>(8192);
    signature_require(reuse_scene(state,true,true) && !state->submitted && !state->failed,"real warm idle selector prerequisite");
    auto draws=reuse_raw_scene(*state);signature_require(bool(draws),"selector owned raw scene");
    const auto before=selection_observation(*state);
    const auto cases=selection_policy_cases(*draws);
    for (const auto& item : cases) {
      auto selected=dx12_select_reuse_scene(*state,{0.02F,0.04F,0.07F,1},item.draws,false,false,1);
      signature_require(selected && bool(*selected)==item.eligible,"selector policy: "+item.name);
      selected->reset();
      signature_require(selection_observation(*state)==before,"selector mutated native resource/event state: "+item.name);
    }
    for (unsigned gate=0;gate!=3;++gate) {
      auto selected=dx12_select_reuse_scene(*state,{0,0,0,1},*draws,gate==0,gate==1,gate==2?0:1);
      signature_require(selected && !*selected,"capture/mutation/absent surface must reject without renderer error");
    }
    selection_header_controls(*state,*draws);
    signature_require(selection_observation(*state)==before && !state->command_observation->overflowed() &&
        bool(dx12_check_debug_errors(*state)),"read-only query changed uploads/slots/native commands/debug state");
    std::cout << "actual selector field/policy controls: " << cases.size() << "; query coverage, not native reuse proof\n";
    return 0;
  } catch (const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

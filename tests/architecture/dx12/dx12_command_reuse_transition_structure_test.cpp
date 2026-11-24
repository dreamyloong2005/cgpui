#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT)/path);
  if(!file) throw std::runtime_error(path);
  return {std::istreambuf_iterator<char>(file),{}};
}
void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
bool has(const std::string& value,const char* token) {return value.find(token)!=std::string::npos;}
}
int main() {
  try {
    const auto manifest=read("tests/renderer-dx12/mcpp.toml");
    for(const auto* name:{"dx12_command_reuse_signature_native_test.cpp","dx12_command_reuse_mutation_transition_test.cpp",
        "dx12_command_reuse_fallback_transition_test.cpp","dx12_command_reuse_srv_page_transition_test.cpp",
        "dx12_command_reuse_transition_fixture_test_support.cpp","dx12_command_reuse_transition_scene_test_support.cpp",
        "dx12_command_reuse_transition_evidence_test_support.cpp"}) {
      auto source=read(std::string("tests/renderer-dx12/")+name);
      require(std::count(source.begin(),source.end(),'\n')<=150,"native transition owner grew broad");
      require(has(manifest,name),"native transition registration must be additive in owning suite");
      std::erase_if(source,[](unsigned char c){return std::isspace(c)!=0;});
      for(const auto* forbidden:{"state.submitted=false","state->submitted=false","native.uploaded=",
          "publish_success(","parent_reset_succeeded(","reserve_submission(","state.failed=false","state->failed=false"})
        require(!has(source,forbidden),"transition tests must not manufacture lifetime/readiness proof");
    }
    const auto header=read("tests/renderer-dx12/dx12_command_reuse_transition_test_support.hpp");
    require(std::count(header.begin(),header.end(),'\n')<=50 && !has(header,"return ") && !has(header,"CreateCommittedResource("),
        "transition header must stay declaration/data-only");
    const auto scene=read("tests/renderer-dx12/dx12_command_reuse_transition_scene_test_support.cpp");
    for(const auto* call:{"frame.upload_image(","frame.invalidate_image(","frame.request_pixel_capture()","frame.present()"})
      require(has(scene,call),"transition input must use actual public frame request wiring");
    const auto evidence=read("tests/renderer-dx12/dx12_command_reuse_transition_evidence_test_support.cpp");
    for(const auto* call:{"command_observation->events()","tail.completed_parent==tail.parent","bundle_created","bundle_closed",
        "execute_bundle","producer_execute","tail_execute","e.a==tail.ticket","tail.pixels.rgba8",
        "state.diagnostics->upload_bytes.image_byte_count==expected_upload","dx12_check_debug_errors(state)"})
      require(has(evidence,call),"transition route requires actual native commands, same-frame tail and upload diagnostics");
    const auto page=read("tests/renderer-dx12/dx12_command_reuse_srv_page_transition_test.cpp");
    require(has(page,"Dx12SrvDescriptorPool>(3)") && has(page,"srv_topology(*fixture.state,2,2)") && has(page,"stats.live_slots==4"),
        "SRV-page refusal must use real distinct descriptor heaps and live slots");
    require(has(read("scripts/ci/architecture-header-targets.txt"),"dx12_command_reuse_transition_structure_test|"),"native transition structure CI entry missing");
    return 0;
  } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

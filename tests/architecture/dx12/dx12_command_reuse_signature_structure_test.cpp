#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const std::string& path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT)/path);
  if (!file) throw std::runtime_error(path);
  return {std::istreambuf_iterator<char>(file),{}};
}
void require(bool value,const char* reason) {if(!value) throw std::runtime_error(reason);}
bool has(const std::string& value,const char* token) {return value.find(token)!=std::string::npos;}
}
int main() {
  try {
    const auto manifest=read("tests/renderer-dx12/mcpp.toml");
    for (const auto* name : {"dx12_command_reuse_signature_test.cpp","dx12_command_reuse_selection_test.cpp",
        "dx12_command_reuse_signature_fixture_test_support.cpp","dx12_command_reuse_signature_mutations_test_support.cpp",
        "dx12_command_reuse_image_signature_test_support.cpp","dx12_command_reuse_selection_cases_test_support.cpp",
        "dx12_command_reuse_selection_header_test_support.cpp"}) {
      const auto source=read(std::string("tests/renderer-dx12/")+name);
      require(std::count(source.begin(),source.end(),'\n')<=150,"signature/selector test owner grew broad");
      require(has(manifest,name),"signature controls require additive owning registration");
      for (const auto* forbidden : {"reserve_submission(","parent_reset_succeeded(","publish_success(","state.submitted =", "native.uploaded ="})
        require(!has(source,forbidden),"query controls cannot manufacture native lifetime/readiness proof");
    }
    for (const auto* name : {"dx12_command_reuse_signature_test_support.hpp","dx12_command_reuse_selection_test_support.hpp"}) {
      const auto source=read(std::string("tests/renderer-dx12/")+name);
      require(std::count(source.begin(),source.end(),'\n')<=50 && !has(source,"CreateCommittedResource(") && !has(source,"return "),
          "focused support headers remain declaration/data-only");
    }
    const auto cpu=read("tests/renderer-dx12/dx12_command_reuse_signature_test.cpp");
    require(has(cpu,"signature_serialized(changed)") && has(cpu,"dx12_reuse_signature_equal(") && !has(cpu,"srv_native_state("),
        "field mutations must hit actual serializer, without masquerading as native execution");
    const auto header=read("tests/renderer-dx12/dx12_command_reuse_selection_header_test_support.cpp");
    require(has(header,"dx12_select_reuse_scene(") && has(header,"RestoreHeader") && has(header,"std::bit_cast<std::uint32_t>") &&
        !has(header,"signature.push_back") && !has(header,"signature.insert"),"header mapping must come from actual selector, not a hand-edited key");
    const auto policy=read("tests/renderer-dx12/dx12_command_reuse_selection_test.cpp");
    require(has(policy,"selection_observation(*state)==before") && has(policy,"selected->reset()") &&
        has(policy,"reuse_scene(state,true,true)"),"selection must remain read-only over real warm readiness");
    require(has(read("scripts/ci/architecture-header-targets.txt"),"dx12_command_reuse_signature_structure_test|"),"structure CI discovery missing");
    return 0;
  } catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
}

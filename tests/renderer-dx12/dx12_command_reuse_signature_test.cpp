#include "dx12_command_reuse_signature_test_support.hpp"
#include <iostream>

int main() {
  using namespace cgpui; using namespace cgpui_test;
  try {
    const auto fixture=signature_fixture();
    const auto expected=signature_serialized(fixture);
    signature_require(dx12_reuse_signature_equal(expected,signature_serialized(fixture)),"owned copy must compare equal");
    auto cases=signature_geometry_mutations();
    auto images=signature_image_mutations();
    cases.insert(cases.end(),images.begin(),images.end());
    for (const auto& [name,mutate] : cases) {
      auto changed=fixture;
      mutate(changed);
      signature_require(!dx12_reuse_signature_equal(expected,signature_serialized(changed)),"serializer omitted "+name);
      signature_require(dx12_reuse_signature_equal(expected,signature_serialized(fixture)),"mutation escaped owned fixture: "+name);
    }
    std::cout << "actual serializer single-field controls: " << cases.size() << "; CPU-only, not native proof\n";
    return 0;
  } catch (const std::exception& e) {std::cerr << e.what() << '\n';return 1;}
}

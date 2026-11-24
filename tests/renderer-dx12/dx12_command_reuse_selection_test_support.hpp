#pragma once
#include "dx12_command_reuse_signature_test_support.hpp"
#include "dx12_command_reuse_supplement_test_support.hpp"
#include <tuple>

namespace cgpui_test {
struct SelectionCase {
  std::string name;
  std::vector<cgpui::Dx12DrawCommand> draws;
  bool eligible = false;
};
struct SelectionObservation {
  std::vector<std::tuple<const void*,const void*,const void*,std::uint64_t,bool>> native;
  std::size_t events=0, slots=0, pages=0, pending=0, retained=0, uploads=0;
  bool operator==(const SelectionObservation&) const = default;
};
SelectionObservation selection_observation(const cgpui::Dx12State&);
std::vector<SelectionCase> selection_policy_cases(const std::vector<cgpui::Dx12DrawCommand>&);
void selection_header_controls(cgpui::Dx12State&, std::span<const cgpui::Dx12DrawCommand>);
} // namespace cgpui_test

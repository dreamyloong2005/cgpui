#include "dx12_dred_breadcrumb_test_support.hpp"
#include <limits>
using namespace cgpui;
using namespace cgpui_test;
int main() {
  DredMemoryScript script;
  std::vector<std::uint32_t> ring(dx12_dred_history_capacity);
  for (std::uint32_t i = 0; i != ring.size(); ++i) ring[i] = i;
  script.put(dred_history_address, std::span<const std::uint32_t>(ring));
  for (const auto [recorded, completed] : {std::pair{65540u, 65535u}, std::pair{65540u, 2u},
        std::pair{std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max()}}) {
    script.calls = script.bytes = 0;
    script.put(dred_node_address, dred_memory_node(recorded)); script.put(dred_counter_address, completed);
    const auto out = dx12_sample_dred_breadcrumbs(dred_node_address, script);
    if (out.summary.status != Dx12DredBreadcrumbStatus::sampled || out.summary.nodes != 1 ||
        !out.summary.history_truncated || !out.nodes[0].history_truncated ||
        out.nodes[0].operation_count > 32 || script.calls > 6 || script.bytes > 512)
      return dred_breadcrumb_fail(1, "huge counts must use bounded ring samples without count-sized reads");
    const auto& node = out.nodes[0];
    for (unsigned i = 0; i != node.operation_count; ++i)
      if (node.operations[i] != (node.first_operation + i) % dx12_dred_history_capacity)
        return dred_breadcrumb_fail(2, "ring-wrapped operation contents differ");
    if (recorded == 65540 && completed == 65535 &&
        (node.first_operation != 65519 || node.operation_count != 21 || node.completed_overwritten))
      return dred_breadcrumb_fail(3, "window must include before-progress and next retained operations");
    if (completed == 2 && (!node.completed_overwritten || node.first_operation != 4))
      return dred_breadcrumb_fail(4, "overwritten progress must be reported rather than indexed into stale slots");
  }
  return 0;
}

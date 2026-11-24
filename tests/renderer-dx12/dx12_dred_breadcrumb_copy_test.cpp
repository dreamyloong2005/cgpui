#include "dx12_dred_breadcrumb_test_support.hpp"
using namespace cgpui;
using namespace cgpui_test;
int main() {
  DredMemoryScript empty;
  if (dx12_sample_dred_breadcrumbs(0, empty).summary.status != Dx12DredBreadcrumbStatus::observed_empty || empty.calls)
    return dred_breadcrumb_fail(1, "null output is an observed empty list without memory reads");
  DredMemoryScript script; dred_memory_fixture(script);
  const auto report = dx12_sample_dred_breadcrumbs(dred_node_address, script);
  if (report.summary.status != Dx12DredBreadcrumbStatus::sampled || report.summary.nodes != 1 ||
      report.summary.operations != 4 || report.summary.history_truncated || script.calls != 5 ||
      report.nodes[0].recorded != 4 || report.nodes[0].completed != 2 ||
      report.nodes[0].first_operation || report.nodes[0].operations[0] != 11 || report.nodes[0].operations[3] != 44)
    return dred_breadcrumb_fail(2, "real copier lost progress/ops or followed ignored native/name pointers");
  script.calls = script.bytes = 0;
  script.put(dred_node_address, dred_memory_node(4, dred_node_address + 512));
  script.put(dred_node_address + 512, dred_memory_node(4, dred_node_address));
  const auto cycle = dx12_sample_dred_breadcrumbs(dred_node_address, script);
  if (cycle.summary.status != Dx12DredBreadcrumbStatus::cycle || cycle.summary.nodes != 2 || script.calls != 10)
    return dred_breadcrumb_fail(3, "cycle must retain earlier owned nodes and stop");
  DredMemoryScript many;
  for (unsigned i = 0; i != 65; ++i) {
    D3D12_AUTO_BREADCRUMB_NODE node{};
    node.pNext = reinterpret_cast<const D3D12_AUTO_BREADCRUMB_NODE*>(dred_node_address + (i + 1) * 512);
    many.put(dred_node_address + i * 512, node);
  }
  const auto truncated = dx12_sample_dred_breadcrumbs(dred_node_address, many);
  if (truncated.summary.status != Dx12DredBreadcrumbStatus::truncated || truncated.summary.nodes != 64 ||
      many.calls != 128 || many.bytes > 65536)
    return dred_breadcrumb_fail(4, "node and native-read limits were not bounded");
  return 0;
}

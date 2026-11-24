#include "dx12_dred_breadcrumb_test_support.hpp"
#include <limits>
using namespace cgpui;
using namespace cgpui_test;
namespace {
void change_header(DredMemoryScript& script, std::uintptr_t) {
  if (script.calls == 4) script.put(dred_node_address, dred_memory_node(3));
}
void change_counter(DredMemoryScript& script, std::uintptr_t) {
  if (script.calls == 5) script.put(dred_counter_address, std::uint32_t{3});
}
}
int main() {
  for (std::size_t call = 1; call <= 5; ++call) {
    for (bool short_read : {false, true}) {
      DredMemoryScript script; dred_memory_fixture(script);
      (short_read ? script.short_call : script.fail_call) = call;
      const auto out = dx12_sample_dred_breadcrumbs(dred_node_address, script);
      if (out.summary.status != (short_read ? Dx12DredBreadcrumbStatus::short_read : Dx12DredBreadcrumbStatus::read_failed) ||
          out.summary.nodes || out.summary.operations || script.calls != call)
        return dred_breadcrumb_fail(1, "failed/short node,counter,history or re-read was published");
    }
  }
  for (auto mutate : {change_header, change_counter}) {
    DredMemoryScript script; dred_memory_fixture(script); script.before_read = mutate;
    const auto out = dx12_sample_dred_breadcrumbs(dred_node_address, script);
    if (out.summary.status != Dx12DredBreadcrumbStatus::inconsistent || out.summary.nodes)
      return dred_breadcrumb_fail(2, "changed header/progress must discard the in-progress sample");
  }
  DredMemoryScript script; dred_memory_fixture(script);
  script.put(dred_counter_address, std::uint32_t{5});
  if (dx12_sample_dred_breadcrumbs(dred_node_address, script).summary.status != Dx12DredBreadcrumbStatus::inconsistent)
    return dred_breadcrumb_fail(3, "completed beyond recorded was not rejected");
  DredMemoryScript overflow;
  const auto result = dx12_sample_dred_breadcrumbs(std::numeric_limits<std::uintptr_t>::max() - 1, overflow);
  if (result.summary.status != Dx12DredBreadcrumbStatus::invalid_address || overflow.calls)
    return dred_breadcrumb_fail(4, "wrapping node range reached external reader");
  DredMemoryScript null_history; dred_memory_fixture(null_history);
  auto node = dred_memory_node(); node.pCommandHistory = nullptr;
  null_history.put(dred_node_address, node);
  if (dx12_sample_dred_breadcrumbs(dred_node_address, null_history).summary.status != Dx12DredBreadcrumbStatus::invalid_address)
    return dred_breadcrumb_fail(5, "null history accepted");
  DredMemoryScript budget; budget.put(dred_counter_address, std::uint32_t{});
  Dx12DredBreadcrumbSummary status; Dx12DredCopy copy(budget, status);
  std::uint32_t value = 0;
  for (unsigned i = 0; i != 512; ++i) if (!copy.counter(dred_counter_address, value)) return 6;
  if (copy.counter(dred_counter_address, value) || status.status != Dx12DredBreadcrumbStatus::budget_exhausted || budget.calls != 512)
    return dred_breadcrumb_fail(7, "external-call budget did not stop before invoking reader");
  DredMemoryScript bytes; std::array<std::byte, 512> storage{};
  bytes.put(dred_history_address, storage);
  Dx12DredBreadcrumbSummary byte_status; Dx12DredCopy byte_copy(bytes, byte_status);
  for (unsigned i = 0; i != 128; ++i)
    if (!byte_copy.exact(dred_history_address, storage)) return 8;
  if (byte_copy.exact(dred_history_address, storage) ||
      byte_status.status != Dx12DredBreadcrumbStatus::budget_exhausted || bytes.calls != 128 || bytes.bytes != 65536)
    return dred_breadcrumb_fail(9, "byte budget did not stop before invoking reader");
  DredMemoryScript history_overflow; dred_memory_fixture(history_overflow);
  auto wrapping = dred_memory_node(100);
  wrapping.pCommandHistory = reinterpret_cast<const D3D12_AUTO_BREADCRUMB_OP*>(
      std::numeric_limits<std::uintptr_t>::max() - 128);
  history_overflow.put(dred_node_address, wrapping);
  history_overflow.put(dred_counter_address, std::uint32_t{80});
  const auto wrapped = dx12_sample_dred_breadcrumbs(dred_node_address, history_overflow);
  if (wrapped.summary.status != Dx12DredBreadcrumbStatus::invalid_address || history_overflow.calls != 2)
    return dred_breadcrumb_fail(10, "wrapping history offset reached external reader");
  return 0;
}

#include "../../src/renderer/dx12/dx12_dred_breadcrumb_format_internal.hpp"
#include <limits>
#include <string_view>
using namespace cgpui;
int main() {
  Dx12DredBreadcrumbSummary summary{.status = Dx12DredBreadcrumbStatus::short_read,
      .output_hresult = -1, .output_requested = true, .history_truncated = true, .nodes = 2, .operations = 32, .read_error = 299};
  const auto text = dx12_format_dred_breadcrumb_summary(summary);
  const std::wstring_view view(text.text.data(), text.used);
  if (text.truncated || view.find(L"atomic=false coherent=false status=short_read") == view.npos ||
      view.find(L"output_hresult=0xFFFFFFFF") == view.npos || view.find(L"history_truncated=1") == view.npos ||
      view.find(L"page_fault=not_requested") == view.npos) return 1;
  Dx12DredBreadcrumbNode node; node.recorded = node.completed = std::numeric_limits<std::uint32_t>::max();
  node.operation_count = 32; node.first_operation = node.recorded - 32;
  node.operations.fill(std::numeric_limits<std::uint32_t>::max());
  const auto line = dx12_format_dred_breadcrumb_node(63, node);
  const std::wstring_view nodes(line.text.data(), line.used);
  if (line.truncated || line.used >= line.text.size() || line.text[line.used] != L'\0' ||
      nodes.find(L"4294967295,4294967295") == nodes.npos || nodes.find(L"node=63") == nodes.npos) return 2;
  return 0;
}

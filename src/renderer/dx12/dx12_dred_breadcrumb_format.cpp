#include "dx12_dred_breadcrumb_format_internal.hpp"
#include <cwchar>

namespace cgpui {
const wchar_t* dx12_dred_breadcrumb_status_name(Dx12DredBreadcrumbStatus status) noexcept {
  switch (status) {
    case Dx12DredBreadcrumbStatus::not_attempted: return L"not_attempted";
    case Dx12DredBreadcrumbStatus::query_failed: return L"query_failed";
    case Dx12DredBreadcrumbStatus::observed_empty: return L"observed_empty";
    case Dx12DredBreadcrumbStatus::sampled: return L"sampled";
    case Dx12DredBreadcrumbStatus::read_failed: return L"read_failed";
    case Dx12DredBreadcrumbStatus::short_read: return L"short_read";
    case Dx12DredBreadcrumbStatus::invalid_address: return L"invalid_address";
    case Dx12DredBreadcrumbStatus::inconsistent: return L"inconsistent";
    case Dx12DredBreadcrumbStatus::cycle: return L"cycle";
    case Dx12DredBreadcrumbStatus::truncated: return L"truncated";
    case Dx12DredBreadcrumbStatus::budget_exhausted: return L"budget_exhausted";
  }
  return L"unknown";
}
namespace {
void length(Dx12DredStatusMessage& text, int written) noexcept {
  text.truncated = written < 0 || static_cast<std::size_t>(written) >= text.text.size();
  text.used = text.truncated ? text.text.size() - 1 : static_cast<std::size_t>(written);
  text.text[text.used] = L'\0';
}
}
Dx12DredStatusMessage dx12_format_dred_breadcrumb_summary(const Dx12DredBreadcrumbSummary& value) noexcept {
  Dx12DredStatusMessage text;
  const int written = std::swprintf(text.text.data(), text.text.size(),
      L"CGPUI DX12 breadcrumbs best_effort=true atomic=false coherent=false status=%ls "
      L"output_requested=%u output_hresult=0x%08X nodes=%u sampled_ops=%u read_error=%u history_truncated=%u "
      L"native_epoch=unknown participant_coverage=unknown page_fault=not_requested\n",
      dx12_dred_breadcrumb_status_name(value.status), unsigned(value.output_requested),
      static_cast<unsigned>(value.output_hresult), value.nodes, value.operations, value.read_error, unsigned(value.history_truncated));
  length(text, written);
  return text;
}
Dx12DredStatusMessage dx12_format_dred_breadcrumb_node(std::uint32_t index, const Dx12DredBreadcrumbNode& node) noexcept {
  Dx12DredStatusMessage text;
  int written = std::swprintf(text.text.data(), text.text.size(),
      L"CGPUI DX12 breadcrumb_sample node=%u recorded=%u completed=%u first_op=%u "
      L"completed_overwritten=%u ops=", index, node.recorded, node.completed,
      node.first_operation, unsigned(node.completed_overwritten));
  length(text, written);
  for (std::uint32_t operation = 0; operation < node.operation_count && !text.truncated; ++operation) {
    const auto remaining = text.text.size() - text.used;
    written = std::swprintf(text.text.data() + text.used, remaining, L"%ls%u",
        operation ? L"," : L"", node.operations[operation]);
    if (written < 0 || static_cast<std::size_t>(written) >= remaining) { text.truncated = true; break; }
    text.used += written;
  }
  if (text.used + 1 < text.text.size()) text.text[text.used++] = L'\n';
  else text.truncated = true;
  text.text[text.used] = L'\0';
  return text;
}
} // namespace cgpui

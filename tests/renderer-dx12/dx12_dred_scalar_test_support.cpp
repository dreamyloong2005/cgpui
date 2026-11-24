#include "dx12_dred_scalar_test_support.hpp"
#include <iostream>

namespace cgpui_test {
cgpui::Dx12DredInterfaceResult DredSetupScript::query_settings() noexcept {
  ++queries;
  return query;
}
void DredSetupScript::enable_breadcrumbs() noexcept { ++setters; }
std::int32_t DredStatusScript::removed_reason() noexcept {
  ++reasons;
  if (during_reason) during_reason(*this);
  return reason;
}
cgpui::Dx12DredInterfaceResult DredStatusScript::query_interface() noexcept {
  ++queries;
  if (during_query) during_query(*this);
  return query;
}
cgpui::Dx12DredBreadcrumbSummary DredStatusScript::breadcrumb_summary() const noexcept { return breadcrumbs; }
int dred_fail(int code, const char* message) {
  std::cerr << "DRED scalar CPU/API seam: " << message << '\n';
  return code;
}
} // namespace cgpui_test

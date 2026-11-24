#include "dx12_dred_scalar_test_support.hpp"
#include "../../src/renderer/dx12/dx12_dred_status_sink_internal.hpp"
#include <string_view>
#include <type_traits>

using namespace cgpui;
using namespace cgpui_test;
static_assert(std::is_same_v<decltype(Dx12DredStatus::breadcrumb_count), Dx12DredUnknown>);
static_assert(std::is_same_v<decltype(Dx12DredStatus::native_epoch), Dx12DredUnknown>);
namespace {
bool includes(std::wstring_view text, std::wstring_view piece) {
  return text.find(piece) != std::wstring_view::npos;
}
bool bounded(const Dx12DredStatusMessage& value) {
  return !value.truncated && value.used < value.text.size() && value.text[value.used] == L'\0';
}
}
int main() {
  Dx12DredStatus off;
  off.actual_removed_reason = dred_removed;
  const auto first = dx12_format_dred_status(off);
  constexpr std::wstring_view expected =
      L"CGPUI DX12 removal scope=site_local native_epoch=unknown removed_reason=0x887A0005 "
      L"setup=not_requested interface=not_attempted(debug_not_requested) "
      L"output=not_requested_boundary_unproven breadcrumb_count=unknown "
      L"participant_coverage=unknown native_device_setup_coverage=unknown\n";
  if (!bounded(first) || std::wstring_view(first.text.data(), first.used) != expected)
    return dred_fail(1, "default private status must distinguish unknown/skipped from empty/success");
  Dx12DredStatus requested;
  requested.actual_removed_reason = dred_reset;
  requested.setup = Dx12DredSetupRequestReturned{{0, true}};
  requested.interface_query = Dx12DredInterfaceResult{dred_no_interface, false};
  const auto second = dx12_format_dred_status(requested);
  const std::wstring_view text(second.text.data(), second.used);
  if (!bounded(second) || !includes(text, L"removed_reason=0x887A0007") ||
      !includes(text, L"setup=request_returned(settings_hresult=0x00000000,interface_received=true,void_setter_returned=true)") ||
      !includes(text, L"interface=attempted(hresult=0x80004002,received=false)") ||
      !includes(text, L"breadcrumb_count=unknown") || includes(text, L"setter_hresult"))
    return dred_fail(2, "formatter invented setter result or lost actual HRESULT bits");
  requested.setup = Dx12DredSetupUnavailable{{0, false}};
  requested.interface_query = Dx12DredInterfaceResult{0, false};
  const auto third = dx12_format_dred_status(requested);
  const std::wstring_view absent(third.text.data(), third.used);
  if (!bounded(third) ||
      !includes(absent, L"setup=request_not_called(settings_hresult=0x00000000,interface_received=false)") ||
      !includes(absent, L"interface=attempted(hresult=0x00000000,received=false)") ||
      includes(absent, L"void_setter_returned=true") || includes(absent, L"breadcrumb_count=0"))
    return dred_fail(3, "success/null is neither a void request return nor observed-empty breadcrumbs");
  return 0;
}

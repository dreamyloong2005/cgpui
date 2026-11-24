#include "dx12_dred_status_sink_internal.hpp"
#include <bit>

namespace cgpui {
namespace {
class Writer {
 public:
  explicit Writer(Dx12DredStatusMessage& message) noexcept : message_(message) {}
  void append(const wchar_t* text) noexcept {
    for (; *text; ++text) character(*text);
  }
  void boolean(bool value) noexcept { append(value ? L"true" : L"false"); }
  void hresult(std::int32_t value) noexcept {
    constexpr wchar_t digits[] = L"0123456789ABCDEF";
    const auto bits = std::bit_cast<std::uint32_t>(value);
    append(L"0x");
    for (int shift = 28; shift >= 0; shift -= 4) character(digits[(bits >> shift) & 15]);
  }
 private:
  void character(wchar_t value) noexcept {
    if (message_.used + 1 < message_.text.size()) message_.text[message_.used++] = value;
    else message_.truncated = true;
    message_.text[message_.used] = L'\0';
  }
  Dx12DredStatusMessage& message_;
};
void settings_result(Writer& out, Dx12DredInterfaceResult query) noexcept {
  out.append(L"settings_hresult="); out.hresult(query.hresult);
  out.append(L",interface_received="); out.boolean(query.received);
}
void setup(Writer& out, const Dx12DredSetupObservation& observation) noexcept {
  if (const auto* returned = std::get_if<Dx12DredSetupRequestReturned>(&observation)) {
    out.append(L"request_returned("); settings_result(out, returned->query);
    out.append(L",void_setter_returned=true)");
  } else if (const auto* unavailable = std::get_if<Dx12DredSetupUnavailable>(&observation)) {
    out.append(L"request_not_called("); settings_result(out, unavailable->query); out.append(L")");
  } else out.append(L"not_requested");
}
}
Dx12DredStatusMessage dx12_format_dred_status(const Dx12DredStatus& status) noexcept {
  Dx12DredStatusMessage message;
  Writer out(message);
  out.append(L"CGPUI DX12 removal scope=site_local native_epoch=unknown removed_reason=");
  out.hresult(status.actual_removed_reason);
  out.append(L" setup="); setup(out, status.setup);
  out.append(L" interface=");
  if (const auto* query = std::get_if<Dx12DredInterfaceResult>(&status.interface_query)) {
    out.append(L"attempted(hresult="); out.hresult(query->hresult);
    out.append(L",received="); out.boolean(query->received); out.append(L")");
  } else out.append(L"not_attempted(debug_not_requested)");
  out.append(status.output == Dx12DredOutput::best_effort_non_atomic
      ? L" output=best_effort_non_atomic breadcrumb_count=unknown"
      : L" output=not_requested_boundary_unproven breadcrumb_count=unknown");
  out.append(L" participant_coverage=unknown native_device_setup_coverage=unknown\n");
  return message;
}
} // namespace cgpui

#pragma once
#include "../../src/renderer/dx12/dx12_dred_api_internal.hpp"
#include "../../src/renderer/dx12/dx12_dred_site_internal.hpp"
#include <bit>

namespace cgpui_test {
inline constexpr auto dred_removed = std::bit_cast<std::int32_t>(std::uint32_t{0x887A0005});
inline constexpr auto dred_reset = std::bit_cast<std::int32_t>(std::uint32_t{0x887A0007});
inline constexpr auto dred_no_interface = std::bit_cast<std::int32_t>(std::uint32_t{0x80004002});
// CPU scripts only. No result below is actual native removal or availability.
struct DredSetupScript final : cgpui::Dx12DredSetupApi {
  cgpui::Dx12DredInterfaceResult query{};
  unsigned queries = 0, setters = 0;
  cgpui::Dx12DredInterfaceResult query_settings() noexcept override;
  void enable_breadcrumbs() noexcept override;
};
struct DredStatusScript final : cgpui::Dx12DredStatusApi {
  std::int32_t reason = 0;
  cgpui::Dx12DredInterfaceResult query{};
  unsigned reasons = 0, queries = 0;
  void (*during_reason)(DredStatusScript&) noexcept = nullptr;
  void (*during_query)(DredStatusScript&) noexcept = nullptr;
  cgpui::Dx12DredSite* site = nullptr;
  bool expect_previous = false, bad_phase = false, bad_previous = false, nested_published = false;
  std::int32_t previous_reason = 0;
  std::int32_t removed_reason() noexcept override;
  cgpui::Dx12DredInterfaceResult query_interface() noexcept override;
  cgpui::Dx12DredBreadcrumbSummary breadcrumb_summary() const noexcept override;
  cgpui::Dx12DredBreadcrumbSummary breadcrumbs;
};
int dred_fail(int code, const char* message);
} // namespace cgpui_test

#pragma once
#include "probe_internal.hpp"
#include <iosfwd>
#include <string_view>

namespace renderer_probe {
struct Statistics {
  std::size_t count = 0;
  std::uint64_t minimum = 0, p50 = 0, p95 = 0, maximum = 0;
  long double mean = 0;
};
[[nodiscard]] std::optional<Statistics> statistics(std::span<const std::uint64_t> values);
[[nodiscard]] bool report_valid(const Options& options, const Report& report);
void write_statistic(std::ostream& out, std::span<const Sample> samples,
    std::uint64_t Sample::*field, std::string_view name);
void quote_json(std::ostream& out, std::string_view value);
void write_creation(std::ostream& out, const std::optional<RendererCreationSample>& creation);
void write_identity(std::ostream& out, const cgpui::RendererBackendInfo& info);
void write_display(std::ostream& out, const std::optional<DisplayFacts>& facts);
void write_sample_row(std::ostream& out, const Sample& sample, std::string_view phase);
void write_summary(std::ostream& out, const Options& options, const Report& report);
} // namespace renderer_probe

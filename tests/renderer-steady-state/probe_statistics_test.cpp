#include "probe_report_support.hpp"
#include <array>
#include <limits>
#include <sstream>

int main() {
  using namespace renderer_probe;
  if (statistics({})) return 1;
  const std::array<std::uint64_t, 1> single{17};
  const auto one = statistics(single);
  if (!one || one->count != 1 || one->minimum != 17 || one->p50 != 17 ||
      one->p95 != 17 || one->maximum != 17 || one->mean != 17) return 2;
  const std::array<std::uint64_t, 2> pair{100, 2};
  const auto two = statistics(pair);
  if (!two || two->p50 != 2 || two->p95 != 100 || two->mean != 51) return 3;
  std::array<std::uint64_t, 20> twenty{};
  for (std::size_t i = 0; i != twenty.size(); ++i) twenty[i] = twenty.size() - i;
  const auto many = statistics(twenty);
  if (!many || many->count != 20 || many->minimum != 1 || many->p50 != 10 ||
      many->p95 != 19 || many->maximum != 20 || many->mean != 10.5L) return 4;
  std::array<std::uint64_t, 21> twenty_one{};
  for (std::size_t i = 0; i != twenty_one.size(); ++i) twenty_one[i] = i + 1;
  if (statistics(twenty_one)->p95 != 20) return 5;
  const auto maximum = std::numeric_limits<std::uint64_t>::max();
  const std::array<std::uint64_t, 2> large{maximum, maximum};
  const auto wide = statistics(large);
  if (!wide || wide->minimum != maximum || wide->maximum != maximum ||
      wide->mean < static_cast<long double>(maximum) * 0.99L) return 6;
  std::ostringstream empty;
  write_statistic(empty, {}, &Sample::frame_wall_ns, "frame");
  return empty.str() == "\"frame\":null" ? 0 : 7;
}

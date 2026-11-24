#include "dx12_foreign_probe_control_test_support.hpp"
#include <charconv>
#include <string_view>

using namespace cgpui_test;
namespace {
bool mode(std::string_view value, ForeignProbeMode& selected) {
  if (value == "pumping") selected = ForeignProbeMode::pumping;
  else if (value == "parked") selected = ForeignProbeMode::parked;
  else return false;
  return true;
}
} // namespace
int main(int argc, char** argv) {
  ForeignProbeMode selected{};
  if (argc == 4 && std::string_view(argv[1]) == "--probe-child" && mode(argv[2], selected)) {
    std::uintptr_t value = 0;
    const std::string_view argument(argv[3]);
    const auto parsed = std::from_chars(argument.data(), argument.data() + argument.size(), value);
    if (parsed.ec != std::errc{} || parsed.ptr != argument.data() + argument.size() || !value) return 80;
    ForeignProbeHandle cancel(reinterpret_cast<HANDLE>(value));
    return foreign_probe_case(selected, cancel.get());
  }
  try {
    if (argc == 3 && std::string_view(argv[1]) == "--case" && mode(argv[2], selected))
      return foreign_probe_process(selected);
    if (argc != 1) return 80;
    const int pumping = foreign_probe_process(ForeignProbeMode::pumping);
    const int parked = foreign_probe_process(ForeignProbeMode::parked);
    if (pumping || parked) return pumping ? pumping : parked;
    return 0; // Parent exit plus both child completion records; no parent output I/O.
  } catch (...) { return 95; }
}

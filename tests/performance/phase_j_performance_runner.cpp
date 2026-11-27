#include "baseline_workloads.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>

int main(int argc, char** argv) {
  std::string platform = "portable";
  std::string output_path;
  std::size_t iterations = 16;
  try {
    for (int index = 1; index < argc; ++index) {
      const std::string_view argument = argv[index];
      if (argument == "--platform" && index + 1 < argc) platform = argv[++index];
      else if (argument == "--output" && index + 1 < argc) output_path = argv[++index];
      else if (argument == "--iterations" && index + 1 < argc) iterations = std::stoul(argv[++index]);
      else return 2;
    }
    const bool valid_platform = platform == "portable" || platform == "windows" ||
        platform == "wayland" || platform == "x11" || platform == "macos-arm64";
    if (iterations == 0 || iterations > 10000 || !valid_platform) return 3;

    std::vector<performance_baseline::Sample> samples;
    samples.reserve(8);
    performance_baseline::append_window_samples(samples, iterations);
    performance_baseline::append_content_samples(samples, iterations);
    performance_baseline::append_scheduling_samples(samples);
    if (!performance_baseline::validate_samples(samples, std::cerr)) return 4;
    // A failed workload must never publish a successful-looking report.
    if (output_path.empty()) {
      return performance_baseline::write_report(std::cout, platform, iterations, samples);
    }
    const std::filesystem::path output_file{output_path};
    if (output_file.has_parent_path()) std::filesystem::create_directories(output_file.parent_path());
    std::ofstream output(output_path, std::ios::binary);
    return performance_baseline::write_report(output, platform, iterations, samples);
  } catch (const std::exception& error) {
    std::cerr << "performance baseline: " << error.what() << '\n';
    return 4;
  }
}

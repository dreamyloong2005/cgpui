#if defined(_WIN32)
#include "probe_report_support.hpp"
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
  using namespace renderer_probe;
  const auto options = parse_options(argc, argv);
  if (!options) { std::cerr << options.error().message << '\n'; return 2; }
  std::error_code filesystem_error;
  if (!std::filesystem::create_directory(options->output, filesystem_error)) {
    std::cerr << "output must name a NEW directory with an existing parent: "
              << options->output << " (" << filesystem_error.message() << ")\n";
    return 3;
  }
  Report report;
  try {
    if (auto result = run(*options, report); !result) {
      report.error = "code=" + std::to_string(static_cast<int>(result.error().code)) + ": " + result.error().message;
    }
  } catch (const std::exception& error) {
    report.error = std::string("exception: ") + error.what();
  } catch (...) {
    report.error = "unknown exception";
  }
  if (auto written = write_report(*options, report); !written) {
    std::cerr << written.error().message << '\n'; return 4;
  }
  std::cout << "status=" << (report_valid(*options, report) ? "valid_baseline" : "failed")
            << " backend=" << backend_name(report.actual.backend)
            << " adapter=" << report.actual.adapter_name << " software=" << report.actual.software_adapter
            << " measured=" << report.samples.size() << '/' << options->samples
            << " profile_label=" << options->profile << '\n';
  if (!report.error.empty()) std::cerr << report.phase << ": " << report.error << '\n';
  return report_valid(*options, report) ? 0 : 1;
}
#else
#include <cstdio>
int main() {
  std::fputs("renderer_steady_state_probe is an explicit Windows-native bin; not run or certified here\n", stderr);
  return 2;
}
#endif

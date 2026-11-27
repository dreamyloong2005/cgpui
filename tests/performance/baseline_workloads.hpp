#pragma once

#include "baseline_sample.hpp"

#include <vector>

namespace performance_baseline {

void append_window_samples(std::vector<Sample>& samples, std::size_t iterations);
void append_content_samples(std::vector<Sample>& samples, std::size_t iterations);
void append_scheduling_samples(std::vector<Sample>& samples);

} // namespace performance_baseline

#pragma once

#include <algorithm>
#include <cmath>

// Independent mathematical pixel references, shared with Metal.
namespace cgpui_test {
[[nodiscard]] double gaussian_cdf(double x, double sigma);
[[nodiscard]] double blurred_rect_coverage(double x, double y,
    double x0, double y0, double x1, double y1, double sigma);
[[nodiscard]] double rounded_shadow_x(
    double x, double y, double sigma, double corner, double hx, double hy);
[[nodiscard]] double rounded_rect_shadow_coverage(double x, double y,
    double x0, double y0, double x1, double y1, double corner, double sigma);
} // namespace cgpui_test

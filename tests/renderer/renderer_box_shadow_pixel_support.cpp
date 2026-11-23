#include "renderer_box_shadow_pixel_support.hpp"

#include <algorithm>
#include <cmath>

// Expected box shadow coverage, computed independently of the renderer.
namespace cgpui_test {

double gaussian_cdf(double x, double sigma) {
  return 0.5 * (1.0 + std::erf(x / (sigma * std::sqrt(2.0))));
}

// Exact coverage of a sharp-cornered rect [x0,x1]x[y0,y1] blurred by a
// Gaussian with standard deviation sigma, at point (x, y).
double blurred_rect_coverage(
    double x, double y, double x0, double y0, double x1, double y1,
    double sigma) {
  return (gaussian_cdf(x1 - x, sigma) - gaussian_cdf(x0 - x, sigma)) *
         (gaussian_cdf(y1 - y, sigma) - gaussian_cdf(y0 - y, sigma));
}

// C++ port of box_shadow.slang's blurred_coverage for a uniform corner
// radius (Evan Wallace's rounded-rect shadow with 4 y samples).
double rounded_shadow_x(
    double x, double y, double sigma, double corner, double hx, double hy) {
  const double delta = (std::min)(hy - corner - std::abs(y), 0.0);
  const double curved =
      hx - corner + std::sqrt((std::max)(0.0, corner * corner - delta * delta));
  const double scale = std::sqrt(0.5) / sigma;
  return 0.5 * (std::erf((x + curved) * scale) - std::erf((x - curved) * scale));
}

double rounded_rect_shadow_coverage(
    double x, double y, double x0, double y0, double x1, double y1,
    double corner, double sigma) {
  const double hx = (x1 - x0) * 0.5;
  const double hy = (y1 - y0) * 0.5;
  const double px = x - (x0 + x1) * 0.5;
  const double py = y - (y0 + y1) * 0.5;
  const double start = (std::clamp)(-3.0 * sigma, py - hy, py + hy);
  const double end = (std::clamp)(3.0 * sigma, py - hy, py + hy);
  const double step = (end - start) / 4.0;
  double sample = start + step * 0.5;
  double value = 0.0;
  for (int index = 0; index < 4; ++index) {
    const double weight = std::exp(-(sample * sample) / (2.0 * sigma * sigma)) /
                          (std::sqrt(2.0 * 3.14159265358979) * sigma);
    value += rounded_shadow_x(px, py - sample, sigma, corner, hx, hy) *
             weight * step;
    sample += step;
  }
  return value;
}

} // namespace cgpui_test

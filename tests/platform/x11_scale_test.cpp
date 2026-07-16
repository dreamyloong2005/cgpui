#include "x11_scale_internal.hpp"

int main() {
  const auto dpi = cgpui::x11_parse_xft_dpi(
      "Xcursor.size:\t24\nXft.dpi:\t144\nXft.antialias:\t1\n");
  if (!dpi || *dpi != 144.0F) return 1;
  if (cgpui::x11_parse_xft_dpi("Xft.dpi:\tinvalid\n")) return 2;
  if (cgpui::x11_parse_xft_dpi("Xcursor.size:\t24\n")) return 3;
  return 0;
}

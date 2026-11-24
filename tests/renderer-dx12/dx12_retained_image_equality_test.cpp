#include "dx12_diagnostics_support.hpp"

int main() {
  using namespace cgpui_test;
  cgpui::RendererFrameDiagnosticSnapshot original{.retained_image_byte_count = 16};
  auto changed = original;
  if (!diagnostic_same(&changed, original)) return 1;
  changed.retained_image_byte_count = 32;
  if (diagnostic_same(&changed, original)) return 2;
  changed = original;
  changed.retained_image_byte_count_saturated = true;
  if (diagnostic_same(&changed, original)) return 3;
  changed = original;
  changed.retained_image_byte_count.reset();
  if (diagnostic_same(&changed, original)) return 4;
  original.retained_image_byte_count = 0;
  if (diagnostic_same(&changed, original)) return 5;
  return 0;
}

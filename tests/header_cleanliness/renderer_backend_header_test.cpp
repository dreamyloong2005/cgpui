#include "cgpui/renderer/renderer_backend.hpp"

int main() {
  cgpui::RendererBackendInfo info;
  return info.backend == cgpui::RendererBackend::unknown ? 0 : 1;
}

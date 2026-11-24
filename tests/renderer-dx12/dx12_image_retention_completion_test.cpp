#include "../../src/renderer/dx12/dx12_image_cache_internal.hpp"
#include "../../src/renderer/dx12/dx12_frame_diagnostics_internal.hpp"
#include "../../src/renderer/dx12/dx12_state_internal.hpp"
#include <iostream>
#include <stdexcept>

using namespace cgpui;
namespace {
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void seed(Dx12State& state) {
  const ImageAsset image{.id = {9810}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  require(bool(state.image_cache->upload(state, image)), "CPU cache seed failed");
}
void complete(Dx12State& state) {
  Dx12FrameDiagnostics candidate(state, {}, 0);
  candidate.complete_successful_frame();
  candidate.complete_successful_frame(); // Must not age the same logical success twice.
}
void ledger_is_not_completion() {
  Dx12State state;
  seed(state);
  for (unsigned index = 0; index != 124; ++index) {
    Dx12FrameDiagnostics ledger(state, {}, 0);
    ledger.publish();
    ledger.publish();
    require(state.diagnostics && !state.diagnostics->retained_image_byte_count &&
        !state.diagnostics->retained_image_byte_count_saturated,
        "CPU diagnostic publication claimed a real successful frame sample");
  }
  require(state.image_cache->retained_image_payload().byte_count == 4,
      "diagnostic-only publication aged image cache state");
}
void successful_aging() {
  Dx12State state;
  seed(state);
  complete(state); // First completed frame resets the touched upload's idle age.
  require(state.diagnostics->retained_image_byte_count == 4, "first completion missed upload-only payload");
  for (unsigned idle = 0; idle != 120; ++idle) {
    complete(state);
    require(state.diagnostics->retained_image_byte_count == 4,
        "duplicate completion aged twice or cache evicted before121 idle successes");
  }
  const auto previous = *state.diagnostics;
  {
    Dx12FrameDiagnostics unpublished(state, {}, 0);
    unpublished.uploaded(false, 9); // Candidate destruction is not completion.
  }
  require(state.diagnostics->retained_image_byte_count == previous.retained_image_byte_count &&
      state.image_cache->retained_image_payload().byte_count == 4,
      "unpublished candidate changed history or advanced cache eviction");
  complete(state);
  require(state.image_cache->retained_image_payload().byte_count == 0 &&
      state.diagnostics->retained_image_byte_count == 0 &&
      !state.diagnostics->retained_image_byte_count_saturated,
      "successful121st idle frame must publish post-aging zero without an extra-frame delay");
}
}
int main() {
  try {
    // These direct completion/ledger calls prove CPU ordering, not native Present.
    ledger_is_not_completion(); successful_aging(); return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

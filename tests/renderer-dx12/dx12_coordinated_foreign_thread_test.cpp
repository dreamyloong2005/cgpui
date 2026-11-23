#include "dx12_recovery_test_support.hpp"

#include <latch>

using namespace cgpui;
using namespace cgpui_test;

namespace {
struct ForeignPeer {
  std::latch ready{1}, release{1};
  RendererBackendInfo info;
  bool prepared = false;
  int failure = 0;
  std::thread worker{[this] {
    failure = run();
    if (!prepared) ready.count_down(); // Setup failures must also unblock the owner.
  }};
  ~ForeignPeer() { finish(); }
  int finish() {
    if (worker.joinable()) {
      release.count_down(); // Main's early returns must not leave the peer parked.
      worker.join();
      if (failure) dx12_fail(30 + failure, "foreign owner setup or borrowed capture changed");
    }
    return failure ? 30 + failure : 0;
  }
  int run() {
    Dx12Window window;
    if (!window.hwnd) return 1;
    auto renderer = create_dx12_renderer(window.descriptor());
    if (!renderer) return 2;
    auto frame = (*renderer)->begin_frame();
    if (!frame) return 3;
    (*frame)->clear({0, 1, 0, 1});
    if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
        !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {0, 255, 0, 255})) return 4;
    const auto* borrowed = (*renderer)->last_frame_pixels();
    const auto saved = borrowed->rgba8;
    info = (*renderer)->backend_info();
    prepared = true;
    ready.count_down();
    release.wait(); // No public renderer operation overlaps main's recovery call.
    if ((*renderer)->last_frame_pixels() != borrowed || borrowed->rgba8 != saved ||
        (*renderer)->backend_info().device_generation != info.device_generation) return 5;
    return 0; // Frame, renderer and HWND are destroyed on their creating thread.
  }
};
} // namespace

int main() {
  Dx12RecoveryDeadline deadline;
  Dx12Window window;
  if (!deadline.done || !window.hwnd) return dx12_fail(1, "foreign recovery fixture unavailable");
  auto renderer = create_dx12_renderer(window.descriptor());
  if (!renderer) return dx12_fail(2, renderer.error().message.c_str());
  const ImageAsset image{.id = {8504}, .logical_size = {1, 1},
      .bitmap = {.width = 1, .height = 1, .stride = 4, .pixels = {255, 0, 0, 255}}};
  auto warmup = (*renderer)->begin_frame();
  if (!warmup) return dx12_fail(3, "foreign recovery warmup begin failed");
  (*warmup)->upload_image(image);
  (*warmup)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
  if (!(*warmup)->request_pixel_capture() || !(*warmup)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}))
    return dx12_fail(4, "foreign recovery warmup pixels differ");
  const auto before = (*renderer)->backend_info();
  ForeignPeer peer;
  peer.ready.wait();
  if (!peer.prepared) return peer.finish();
  if (!before.adapter_luid || peer.info.adapter_luid != before.adapter_luid)
    return dx12_fail(5, "foreign peer did not select the same adapter");
  if (!dx12_remove_native_device(before)) return dx12_fail(6, "foreign native removal failed");
  const auto blocked = (*renderer)->begin_frame();
  if (blocked || blocked.error().code != ErrorCode::renderer_initialization_failed)
    return dx12_fail(7, "foreign-thread peer was purged or falsely returned frame retry");
  const auto pending = (*renderer)->backend_info();
  if (pending.device_generation != before.device_generation ||
      pending.adapter_luid != before.adapter_luid || (*renderer)->last_frame_pixels())
    return dx12_fail(8, "blocked foreign recovery published a generation or stale capture");
  if (const int failure = peer.finish()) return failure;
  const auto retry = (*renderer)->begin_frame();
  if (retry || retry.error().code != ErrorCode::frame_retry_required)
    return dx12_fail(9, "renderer did not recover after foreign owner destroyed its resources");
  const auto after = (*renderer)->backend_info();
  if (after.device_generation != before.device_generation + 1 || after.adapter_luid != before.adapter_luid)
    return dx12_fail(10, "foreign blocker consumed generations or changed adapter");
  auto frame = (*renderer)->begin_frame();
  if (!frame) return dx12_fail(11, "foreign-unblocked renderer is not ready");
  (*frame)->clear({0, 0, 1, 1});
  (*frame)->draw_image({.bounds = {{4, 4}, {24, 24}}, .asset = describe_image_asset(image)});
  if (!(*frame)->request_pixel_capture() || !(*frame)->present() ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 8, 8, {255, 0, 0, 255}) ||
      !dx12_pixel((*renderer)->last_frame_pixels(), 40, 40, {0, 0, 255, 255}))
    return dx12_fail(12, "cached image failed after foreign-owner blocker release");
  return dx12_recovered_native_device_clean(after) ? 0 : dx12_fail(13, "foreign recovery debug errors");
}

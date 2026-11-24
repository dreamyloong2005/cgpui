#pragma once

#include "dx12_recovery_test_support.hpp"
#include "cgpui/platform/platform.hpp"
#include <atomic>
#include <latch>

namespace cgpui_test {
enum class OwnerRecoveryCase { pumping, busy, parked };

struct ManagedRecoveryWindow {
  cgpui::Result<std::unique_ptr<cgpui::PlatformApplication>> application;
  std::unique_ptr<cgpui::PlatformWindow> window;
  std::unique_ptr<cgpui::Renderer> renderer;
  ManagedRecoveryWindow();
};

bool owner_recovery_pump();
bool owner_recovery_paint(cgpui::Renderer& renderer, bool upload);
int run_owner_recovery_case(OwnerRecoveryCase mode);

class OwnerRecoveryPeer {
 public:
  explicit OwnerRecoveryPeer(OwnerRecoveryCase mode);
  ~OwnerRecoveryPeer();
  int finish();
  cgpui::RendererBackendInfo info;
  bool prepared = false;
  std::latch ready{1};
  std::atomic<bool> pump{false}, busy_seen{false}, release_busy{false};
  std::atomic<bool> retired{false}, origin_rebuilt{false}, completed{false}, stop{false};
  int result = 0;
 private:
  int run();
  OwnerRecoveryCase mode_;
  std::thread worker_;
};
} // namespace cgpui_test

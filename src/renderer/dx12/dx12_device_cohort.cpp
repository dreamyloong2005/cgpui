#include "dx12_device_cohort_internal.hpp"
#include "dx12_state_internal.hpp"
#include <algorithm>
#include <mutex>

namespace cgpui {
namespace {
struct Entry {
  std::weak_ptr<Dx12State> state;
  std::uint64_t luid;
  DWORD window_thread;
};
struct Registry {
  std::mutex mutex;
  std::vector<Entry> entries;
};
Registry& registry() {
  static Registry value;
  return value;
}
} // namespace

void dx12_register_device_cohort(const std::shared_ptr<Dx12State>& state) {
  if (!state->window_thread_id) state->window_thread_id = GetWindowThreadProcessId(state->hwnd, nullptr);
  if (!state->info.adapter_luid || !state->window_thread_id) return;
  auto& records = registry();
  std::lock_guard lock(records.mutex);
  std::erase_if(records.entries, [](const Entry& entry) { return entry.state.expired(); });
  records.entries.push_back({state, *state->info.adapter_luid, state->window_thread_id});
}

std::vector<std::weak_ptr<Dx12State>> dx12_device_cohort_peers(const Dx12State& origin) {
  std::vector<std::weak_ptr<Dx12State>> peers;
  if (!origin.info.adapter_luid || !origin.window_thread_id ||
      origin.window_thread_id != GetCurrentThreadId()) return peers;
  auto& records = registry();
  std::lock_guard lock(records.mutex);
  std::erase_if(records.entries, [](const Entry& entry) { return entry.state.expired(); });
  for (const auto& entry : records.entries) {
    if (entry.luid == *origin.info.adapter_luid && entry.window_thread == origin.window_thread_id) {
      // Do not promote weak handles here: their last strong reference could
      // otherwise run a state destructor while the registry mutex is held.
      peers.push_back(entry.state);
    }
  }
  return peers;
}
} // namespace cgpui

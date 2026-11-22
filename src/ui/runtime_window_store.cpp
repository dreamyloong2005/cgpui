#include "runtime_window_store_internal.hpp"

#include <algorithm>

namespace cgpui {

WindowRuntimeRecord& WindowRuntime::RuntimeWindowStore::insert(WindowRuntimeRecord record) {
  records_.push_back(std::move(record));
  return records_.back();
}

WindowRuntimeRecord* WindowRuntime::RuntimeWindowStore::find(WindowRuntimeId id) {
  const auto found = std::find_if(records_.begin(), records_.end(),
      [id](const auto& record) { return record.runtime_id == id; });
  return found == records_.end() ? nullptr : &*found;
}

const WindowRuntimeRecord* WindowRuntime::RuntimeWindowStore::find(WindowRuntimeId id) const {
  const auto found = std::find_if(records_.begin(), records_.end(),
      [id](const auto& record) { return record.runtime_id == id; });
  return found == records_.end() ? nullptr : &*found;
}

std::list<WindowRuntimeRecord>& WindowRuntime::RuntimeWindowStore::records() { return records_; }
const std::list<WindowRuntimeRecord>& WindowRuntime::RuntimeWindowStore::records() const { return records_; }
std::size_t WindowRuntime::RuntimeWindowStore::size() const { return records_.size(); }

std::span<const WindowRuntimeRecord> WindowRuntime::RuntimeWindowStore::snapshot() const {
  snapshot_.assign(records_.begin(), records_.end());
  return snapshot_;
}

} // namespace cgpui

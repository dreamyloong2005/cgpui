#pragma once

#include <functional>
#include <memory>
#include <vector>

namespace cgpui {

class Win32Window;

// A window owns its registration, not its application. Snapshots can keep
// the token alive, but destruction nulls the pointer before native teardown.
struct Win32WindowRegistration {
  Win32Window* window = nullptr;
};

class Win32WindowRegistry {
 public:
  void add(Win32Window& window);
  // Removed entries are skipped; additions join the next dispatch. Callbacks
  // may destroy themselves or other windows without invalidating iteration.
  void visit(const std::function<void(Win32Window*)>& callback);

 private:
  void prune();
  std::vector<std::weak_ptr<Win32WindowRegistration>> windows_;
};

} // namespace cgpui

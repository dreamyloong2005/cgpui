#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
std::string read(const char* path) {
  std::ifstream file(std::filesystem::path(CGPUI_SOURCE_ROOT) / path);
  if (!file) throw std::runtime_error(std::string("missing recovery module: ") + path);
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& text, const char* token) { return text.find(token) != std::string::npos; }
void require(bool valid, const char* message) { if (!valid) throw std::runtime_error(message); }
} // namespace
int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_recovery_ticket_internal.hpp",
                            "src/renderer/dx12/dx12_recovery_ticket.cpp",
                            "src/renderer/dx12/dx12_recovery_work_internal.hpp",
                            "src/renderer/dx12/dx12_recovery_work.cpp"}) {
      const auto text = read(path);
      require(std::count(text.begin(), text.end(), '\n') <= 190, "recovery coordination must keep focused owners");
      require(!has(text, "WaitFor") && !has(text, "SendMessage") && !has(text, "Sleep(") &&
              !has(text, "RemoveDevice("), "coordination must never block, force removal or dispatch synchronously");
    }
    const auto ledger = read("src/renderer/dx12/dx12_recovery_ticket.cpp");
    require(!has(ledger, "ID3D12") && !has(ledger, "device->") && !has(ledger, "owner.lock()"),
            "lease ledger owns metadata, not native references or strong renderer ownership");
    require(has(ledger, "now + impl_->timeout") && has(ledger, "lease->released = true"),
            "fixed deadline and explicit release completion belong to the ledger");
    const auto work = read("src/renderer/dx12/dx12_recovery_work.cpp");
    require(has(work, "try_to_lock") && has(work, "operation_depth != 0") &&
            has(work, "GetDeviceRemovedReason()") && has(work, "expected != lease"),
            "owner callback must prove idleness, exact native lease and actual removal");
    require(has(work, "native_window_work_post") && !has(work, "dx12_create_device_on_adapter"),
            "retirement messages cannot eagerly reconstruct peer windows");
    const auto factory = read("src/renderer/dx12/dx12_initialization.cpp");
    require(factory.find("dx12_enroll_recovery_work(state)") < factory.find("dx12_create_device(*state,"),
            "partial factory attachment must be visible to recovery coordination");
    const auto device = read("src/renderer/dx12/dx12_adapter_device.cpp");
    require(device.find("tickets.begin_attach") < device.find("D3D12CreateDevice("),
            "lease must cover CreateDevice in flight");
    const auto retirement = read("src/renderer/dx12/dx12_native_retirement.cpp");
    require(retirement.find("state.device.Reset()") < retirement.find("released(state.recovery_lease)"),
            "native acknowledgement cannot precede actual release");
    const auto operation = read("src/renderer/dx12/dx12_operation.cpp");
    require(operation.find("lock_.unlock()") < operation.find("dx12_post_recovery_work("),
            "outer-operation retry must post only after releasing its gate");
    require(has(read("src/renderer/dx12/dx12_recovery.cpp"), "ErrorCode::renderer_recovery_pending"),
            "unfinished peer retirement must not report successful frame retry");
    return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}

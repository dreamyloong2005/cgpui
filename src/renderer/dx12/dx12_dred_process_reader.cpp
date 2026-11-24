#include "dx12_dred_memory_reader_internal.hpp"
#include <windows.h>

namespace cgpui {
namespace {
class ProcessReader final : public Dx12DredMemoryReader {
 public:
  Dx12DredReadResult read(std::uintptr_t address, std::span<std::byte> destination) noexcept override {
    SIZE_T copied = 0;
    const BOOL success = ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<const void*>(address),
        destination.data(), destination.size(), &copied);
    const DWORD error = success ? ERROR_SUCCESS : GetLastError();
    return {success != FALSE, copied, error};
  }
};
}
Dx12DredMemoryReader& dx12_dred_process_reader() noexcept {
  static ProcessReader reader;
  return reader;
}
} // namespace cgpui

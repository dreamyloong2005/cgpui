#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace {
const std::filesystem::path root{CGPUI_SOURCE_ROOT};
std::string read(const std::filesystem::path& relative) {
  std::ifstream file(root / relative);
  if (!file) throw std::runtime_error("missing synchronization owner: " + relative.string());
  return {std::istreambuf_iterator<char>(file), {}};
}
bool has(const std::string& source, const char* token) {
  return source.find(token) != std::string::npos;
}
std::string compact(std::string source) {
  std::erase_if(source, [](unsigned char value) { return std::isspace(value); });
  return source;
}
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
} // namespace

int main() {
  try {
    for (const auto* path : {"src/renderer/dx12/dx12_synchronization.cpp",
                             "src/renderer/dx12/dx12_synchronization_native.cpp",
                             "src/renderer/dx12/dx12_synchronization_internal.hpp",
                             "src/renderer/dx12/dx12_retirement.cpp",
                             "src/renderer/dx12/dx12_retirement_internal.hpp"}) {
      const auto source = read(path);
      const auto limit = std::filesystem::path(path).extension() == ".cpp" ? 150 : 200;
      require(std::count(source.begin(), source.end(), '\n') <= limit,
              "synchronization policy, native operations and retirement need focused owners");
      const auto code = compact(source);
      require(!has(code, "getenv(") && !has(code, "_dupenv_s(") &&
                  !has(code, "GetEnvironmentVariable"),
              "production synchronization cannot expose environment fault injection");
    }

    const auto api = read("src/renderer/dx12/dx12_synchronization_internal.hpp");
    require(has(api, "Dx12SynchronizationApi") && has(api, "Dx12NativeSynchronizationApi") &&
                !has(api, "Dx12NativeSynchronizationApi::"),
            "native synchronization operations need a private, out-of-line boundary");
    const auto native = compact(read("src/renderer/dx12/dx12_synchronization_native.cpp"));
    for (const auto* operation : {"Signal(", "GetCompletedValue(", "SetEventOnCompletion(",
                                 "WaitForSingleObject(", "GetDeviceRemovedReason("}) {
      require(has(native, operation), "native adapter must own the actual synchronization calls");
    }
    for (const auto* path : {"src/renderer/dx12/dx12_synchronization.cpp",
                             "src/renderer/dx12/dx12_retirement.cpp"}) {
      const auto source = compact(read(path));
      require(!has(source, ".Signal(") && !has(source, "->Signal(") &&
                  !has(source, "GetCompletedValue(") && !has(source, "SetEventOnCompletion(") &&
                  !has(source, "WaitForSingleObject(") && !has(source, "GetDeviceRemovedReason("),
              "synchronization and retirement policy must not absorb native operations");
    }
    const auto state = compact(read("src/renderer/dx12/dx12_state.cpp"));
    require(has(state, "Dx12State::~Dx12State(){Dx12OperationGuardoperation(*this);") &&
                has(state, "dx12_retire_submitted_work(*this);dx12_release_native_objects(*this);"),
            "state destruction must lock and retire before explicit native release");
    const auto retirement = compact(read("src/renderer/dx12/dx12_retirement_internal.hpp"));
    require(has(retirement, "dx12_retire_submitted_work(Dx12State&") &&
                has(retirement, "noexcept;"),
            "safe retirement needs a dedicated private, nonthrowing declaration");

    for (const auto& entry : std::filesystem::directory_iterator(root / "src/renderer/dx12")) {
      const auto extension = entry.path().extension();
      if (extension != ".cpp" && extension != ".hpp") continue;
      const auto source = read(entry.path());
      require(std::count(source.begin(), source.end(), '\n') <=
                  (extension == ".cpp" ? 280 : 200),
              "exceptional retirement cannot raise existing DX12 module budgets");
      require(!has(compact(source), "RemoveDevice("),
              "production retirement cannot force removal of a potentially shared device");
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root / "include/cgpui")) {
      if (!entry.is_regular_file() || entry.path().extension() != ".hpp") continue;
      const auto source = read(entry.path());
      require(!has(source, "Dx12SynchronizationApi") &&
                  !has(source, "Dx12NativeSynchronizationApi") &&
                  !has(source, "dx12_synchronization_internal.hpp") &&
                  !has(source, "dx12_retirement_internal.hpp") &&
                  !has(source, "dx12_retire_submitted_work"),
              "private synchronization seams and retirement must not leak into public headers");
    }
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

#include "dx12_loader_process_support.hpp"
#include "dx12_loader_cases_support.hpp"
#include "dx12_loader_retry_support.hpp"

#include <string_view>

using namespace cgpui_test;

int main(int argc, char** argv) {
  if (argc == 3 && std::string_view(argv[1]) == "--loader-case")
    return std::string_view(argv[2]) == "retry-vulkan" ? loader_retry_case() : loader_child_case(argv[2]);
  if (argc != 1) return dx12_fail(1, "invalid loader launcher arguments");
  if (GetModuleHandleW(L"vulkan-1.dll")) return dx12_fail(2, "launcher imported Vulkan before any factory call");
  const auto original = loader_self_executable();
  LoaderIsolationDirectory isolated(original);
  if (!isolated.valid()) return dx12_fail(3, "cannot create owned invalid-loader fixture");
  bool passed = true;
  for (const auto* mode : {L"dx12", L"vulkan", L"default", L"auto", L"dx12-invalid"}) {
    const auto log = isolated.owned_file((std::wstring(mode) + L".log").c_str());
    passed = loader_run_child(isolated.executable(), mode, log, isolated.directory()) && passed;
  }
  // This final poisoned-directory child removes only its verified marker after
  // one real failure, then retries in the same process. No later child needs it.
  const auto retry_log = isolated.owned_file(L"retry-vulkan.log");
  passed = loader_run_child(isolated.executable(), L"retry-vulkan", retry_log, isolated.directory()) && passed;
  // A distinct fresh child in the original executable directory uses the real
  // installed loader. This must not be treated as an optional successful skip.
  const auto log = isolated.owned_file(L"healthy-vulkan.log");
  passed = loader_run_child(original, L"healthy-vulkan", log, original.parent_path()) && passed;
  if (GetModuleHandleW(L"vulkan-1.dll")) return dx12_fail(4, "child execution loaded Vulkan into its launcher");
  if (!isolated.cleanup()) return dx12_fail(5, "owned loader fixture cleanup failed");
  return passed ? 0 : dx12_fail(6, "one or more isolated loader children failed");
}

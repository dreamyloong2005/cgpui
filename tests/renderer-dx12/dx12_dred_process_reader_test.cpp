#include "dx12_dred_breadcrumb_test_support.hpp"
using namespace cgpui;
using namespace cgpui_test;
int main() {
  std::array<D3D12_AUTO_BREADCRUMB_OP, 4> ops{D3D12_AUTO_BREADCRUMB_OP_DRAWINSTANCED,
      D3D12_AUTO_BREADCRUMB_OP_DISPATCH, D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE, D3D12_AUTO_BREADCRUMB_OP_RESOURCEBARRIER};
  UINT completed = 2;
  D3D12_AUTO_BREADCRUMB_NODE node{};
  node.BreadcrumbCount = 4; node.pLastBreadcrumbValue = &completed; node.pCommandHistory = ops.data();
  const auto valid = dx12_sample_dred_breadcrumbs(reinterpret_cast<std::uintptr_t>(&node), dx12_dred_process_reader());
  if (valid.summary.status != Dx12DredBreadcrumbStatus::sampled || valid.summary.nodes != 1 ||
      valid.nodes[0].completed != 2 || valid.nodes[0].operations[2] != D3D12_AUTO_BREADCRUMB_OP_COPYRESOURCE)
    return dred_breadcrumb_fail(1, "self-process adapter failed an owned valid buffer");
  void* page = VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!page) return 2;
  const auto address = reinterpret_cast<std::uintptr_t>(page);
  DWORD previous = 0;
  if (!VirtualProtect(page, 4096, PAGE_NOACCESS, &previous)) { VirtualFree(page, 0, MEM_RELEASE); return 3; }
  const auto inaccessible = dx12_sample_dred_breadcrumbs(address, dx12_dred_process_reader());
  const bool freed = VirtualFree(page, 0, MEM_RELEASE) != FALSE;
  const auto released = dx12_sample_dred_breadcrumbs(address, dx12_dred_process_reader());
  if (!freed || inaccessible.summary.status != Dx12DredBreadcrumbStatus::read_failed ||
      released.summary.status != Dx12DredBreadcrumbStatus::read_failed ||
      !inaccessible.summary.read_error || !released.summary.read_error ||
      inaccessible.summary.nodes || released.summary.nodes)
    return dred_breadcrumb_fail(4, "NOACCESS/freed memory must fail safely, never publish a sampled node");
  return 0;
}

#include "dx12_command_reuse_native_test_support.hpp"
#include <array>
#include <algorithm>

namespace cgpui_test {
namespace {
struct DrawFact {
  cgpui::Dx12CommandEventKind kind{};
  void* pipeline = nullptr; void* root = nullptr;
  void* heap = nullptr; void* samplers = nullptr;
  UINT64 count = 0, first = 0, width = 0, height = 0;
  UINT64 vertex = 0, vertex_bytes = 0, stride = 0, index = 0, index_bytes = 0, index_format = 0;
  UINT64 table = 0, sampler = 0, topology = 0;
};
void observe(DrawFact& fact, const cgpui::Dx12CommandEvent& event) {
  using K = cgpui::Dx12CommandEventKind;
  if (event.kind == K::pipeline) fact.pipeline = event.object;
  if (event.kind == K::root) { fact.root = event.object; fact.table = fact.sampler = 0; }
  if (event.kind == K::heaps) { fact.heap = event.object; fact.samplers = event.object2; }
  if (event.kind == K::topology) fact.topology = event.a;
  if (event.kind == K::constants) { fact.width = event.a; fact.height = event.b; }
  if (event.kind == K::vertex) { fact.vertex = event.a; fact.vertex_bytes = event.b; fact.stride = event.c; }
  if (event.kind == K::index) { fact.index = event.a; fact.index_bytes = event.b; fact.index_format = event.c; }
  if (event.kind == K::table && event.a == 1) fact.table = event.b;
  if (event.kind == K::table && event.a == 2) fact.sampler = event.b;
  if (event.kind == K::draw || event.kind == K::indexed_draw) {
    fact.kind = event.kind; fact.count = event.a; fact.first = event.b;
  }
}
bool same_draw(const DrawFact& actual, const DrawFact& direct) {
  using K = cgpui::Dx12CommandEventKind;
  return actual.kind == direct.kind && actual.pipeline == direct.pipeline && actual.root == direct.root &&
      actual.count == direct.count && actual.first == direct.first && actual.width == direct.width && actual.height == direct.height &&
      actual.topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST && actual.topology == direct.topology &&
      actual.vertex && actual.vertex != direct.vertex && actual.vertex_bytes == direct.vertex_bytes && actual.stride == direct.stride &&
      (actual.kind == K::draw ? actual.table && actual.table == direct.table && actual.sampler && actual.sampler == direct.sampler &&
          actual.heap && actual.heap == direct.heap && actual.samplers && actual.samplers == direct.samplers :
          actual.index && actual.index != direct.index && actual.index_bytes == direct.index_bytes &&
          actual.index_format == DXGI_FORMAT_R32_UINT && actual.index_format == direct.index_format);
}
}
bool reuse_native_history(std::span<const cgpui::Dx12CommandEvent> events,
                          std::span<const std::uint64_t> parents) {
  using K = cgpui::Dx12CommandEventKind;
  if (parents.size() != 3 || !parents[0] || parents[0] >= parents[1] || parents[1] >= parents[2]) return false;
  ID3D12GraphicsCommandList* bundle = nullptr;
  ID3D12CommandAllocator* allocator = nullptr;
  std::size_t creates = 0, closes = 0, native_draws = 0;
  std::array<unsigned, 3> execute{}, producer{}, tail{}, completed{};
  std::array<UINT64, 2> image_tables{}; unsigned images = 0;
  std::array<DrawFact, 4> direct{}; std::size_t direct_count = 0; DrawFact current, fresh;
  for (const auto& event : events) if (event.parent < parents[0] && event.type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
    observe(current, event);
    if (event.kind == K::draw || event.kind == K::indexed_draw) {
      if (direct_count >= direct.size()) return false;
      direct[direct_count++] = current;
    }
  }
  if (direct_count != direct.size()) return false;
  for (const auto& event : events) {
    const auto found = std::find(parents.begin(), parents.end(), event.parent);
    if (found == parents.end()) continue;
    const auto frame = std::size_t(found - parents.begin());
    if (event.kind == K::bundle_created) {
      if (frame || FAILED(event.result) || !event.list || !event.allocator) return false;
      bundle = event.list; allocator = event.allocator; ++creates;
    }
    if (event.kind == K::bundle_closed) {
      if (frame || event.list != bundle || event.allocator != allocator || FAILED(event.result)) return false;
      ++closes;
    }
    if (event.kind == K::draw || event.kind == K::indexed_draw) {
      if (frame || event.list != bundle || event.type != D3D12_COMMAND_LIST_TYPE_BUNDLE) return false;
      observe(fresh, event);
      if (native_draws >= direct.size() || !same_draw(fresh, direct[native_draws])) return false;
      ++native_draws;
    }
    if (event.list == bundle) observe(fresh, event);
    if (event.kind == K::table && event.a == 1 && event.list == bundle) {
      if (images >= 2 || !event.b) return false;
      image_tables[images++] = event.b;
    }
    if (event.kind == K::execute_bundle) {
      if (event.object != bundle || event.list == bundle || !bundle) return false;
      ++execute[frame];
    }
    if (event.kind == K::producer_execute) ++producer[frame];
    if (event.kind == K::tail_execute) ++tail[frame];
    if (event.kind == K::completed) { if (!event.a) return false; ++completed[frame]; }
    if (event.kind == K::bundle_reset || event.kind == K::bundle_released) return false;
  }
  return creates == 1 && closes == 1 && native_draws == 4 && images == 2 &&
      image_tables[0] != image_tables[1] && execute == std::array<unsigned, 3>{1, 1, 1} &&
      producer == execute && tail == execute && completed == execute;
}
} // namespace cgpui_test

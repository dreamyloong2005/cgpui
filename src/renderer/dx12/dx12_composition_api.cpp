#include "dx12_composition_api_internal.hpp"
#include "dx12_composition_internal.hpp"

namespace cgpui {
HRESULT dx12_bind_composition(Dx12CompositionHost& host, IUnknown* content) {
  return host.api ? host.api->bind(host.visual.Get(), content) : host.visual->SetContent(content);
}
HRESULT dx12_apply_composition(Dx12CompositionHost& host) {
  return host.api ? host.api->commit(host.device.Get()) : host.device->Commit();
}
} // namespace cgpui

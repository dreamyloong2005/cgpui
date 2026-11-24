#pragma once

#include "dx12_native_internal.hpp"
#include <dcomp.h>

namespace cgpui {
struct Dx12CompositionHost;
// Optional private failure/forwarding seam. Its owner must outlive the host and
// every frame retaining it. Production uses the native calls with a null seam.
class Dx12CompositionApi {
 public:
  virtual ~Dx12CompositionApi() = default;
  virtual HRESULT bind(IDCompositionVisual*, IUnknown*) = 0;
  virtual HRESULT commit(IDCompositionDevice*) = 0;
};
HRESULT dx12_bind_composition(Dx12CompositionHost&, IUnknown*);
HRESULT dx12_apply_composition(Dx12CompositionHost&);
} // namespace cgpui

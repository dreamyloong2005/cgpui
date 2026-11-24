#pragma once
#include "dx12_adapter_option_internal.hpp"
#include "dx12_native_internal.hpp"

namespace cgpui {
// Borrows the factory and owns temporary QI/adapter references only. Missing
// Factory6 or failed preference enumeration never falls back to EnumAdapters1.
HRESULT dx12_enumerate_preferred_adapter(IUnknown& factory, Dx12AdapterKind kind,
                                        UINT index, Dx12Ptr<IDXGIAdapter1>& adapter);
} // namespace cgpui

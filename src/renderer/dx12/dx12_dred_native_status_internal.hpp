#pragma once
#include "dx12_dred_status_internal.hpp"

struct IUnknown;
namespace cgpui {
// Native COM acquisition/release seam. Returns owned scalars, never an interface.
Dx12DredInterfaceResult dx12_query_dred_interface(IUnknown& source) noexcept;
} // namespace cgpui

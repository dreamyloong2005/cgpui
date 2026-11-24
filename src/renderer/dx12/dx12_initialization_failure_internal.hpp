#pragma once
#include "dx12_native_internal.hpp"

namespace cgpui {
enum class Dx12InitializationFailureClass { terminal, backend_unavailable };
Dx12InitializationFailureClass dx12_classify_initialization_failure(HRESULT result) noexcept;
Error dx12_initialization_error(HRESULT, const char* operation, Dx12InitializationFailureClass*);
// Validates initialization environment before any window mutation. A valid
// adapter override or enabled diagnostic request must not be silently bypassed.
Result<bool> dx12_initialization_fallback_allowed();
} // namespace cgpui

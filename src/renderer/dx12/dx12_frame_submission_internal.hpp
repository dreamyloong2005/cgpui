#pragma once
#include "dx12_native_internal.hpp"
namespace cgpui {
struct Dx12State;
class Dx12GlyphRecordingScope;
Result<void> dx12_execute_frame(Dx12State&, Dx12GlyphRecordingScope&, ID3D12Resource& target);
} // namespace cgpui

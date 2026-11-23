#pragma once
#include "dx12_native_internal.hpp"
#include <cstddef>

namespace cgpui {
Result<Dx12Ptr<ID3D12Resource>> dx12_create_buffer(
    ID3D12Device& device, UINT64 size, D3D12_HEAP_TYPE heap);
Result<Dx12Ptr<ID3D12Resource>> dx12_upload_buffer(
    ID3D12Device& device, const void* data, std::size_t size);
}

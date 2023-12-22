#pragma once
#include "header.h"

// 每个 CBResource 视作有 RESOURCE_ELEMENT_SIZE 个 子资源 的资源池。
// 总大小为 RESOURCE_ELEMENT_SIZE * alignByteSize 个字节.
#define RESOURCE_ELEMENT_SIZE 131072

struct CBResource
{
	UINT alignedByteSize;
	ID3D12Resource* pData;
};

class CBufferAllocator
{
public:
	CBufferAllocator(ID3D12Device* pDevice) {}
	~CBufferAllocator() {}

	// 创建一个CBuffer
	void Create(const std::string& name, UINT64 byteSize);

	// 为一个CBuffer分配内存
	void Alloc(const std::string& name, UINT64 allocSize);

private:
	UINT CalcBufferViewSize(UINT sizeInBytes) { return (sizeInBytes + 255) & ~255; }
private:
	ID3D12Device* m_pDevice;

	// CBuffer分配器允许创建多个资源
	std::unordered_map<std::string, CBResource> m_pResourcesMap;
};

#pragma once
#include "header.h"
#include "DescriptorAllocator.h"

// CBResource 视作有 CBUFFER_RESOURCE_ELEMENT_SIZE 个 子资源 的资源池。
// 总大小为 CBUFFER_RESOURCE_ELEMENT_SIZE * alignedByteSize 个字节.
#define CBUFFER_RESOURCE_ELEMENT_SIZE 131072

struct CBResource
{
	UINT blockByteSize; // 单个 Block 的字节大小。该值必须是 256 的倍数
	UINT byteOffset;
	ID3D12Resource* pData;
};

class CBufferAllocator
{
public:
	CBufferAllocator(ID3D12Device* pDevice, DescriptorAllocator* pDescriptorAllocator) : 
		m_pDevice(pDevice), m_pDescriptorAllocator(pDescriptorAllocator) {}
	~CBufferAllocator() {}

	// 分配 CBResource Pool。
	// blockByteSize 代表 Pool 中 每个 Block 的 ByteSize。
	void Init(UINT blockByteSize);

	// 分配一段CBV
	template<typename T>
	UINT AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr)
	{
		size_t blockByteMask = m_pResource.blockByteSize - 1;
		UINT actualBlockByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);

		// 创建CBV
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_pResource.pData->GetGPUVirtualAddress() + m_pResource.byteOffset; // 常量缓冲区的GPU虚拟地址
		cbvDesc.SizeInBytes = actualBlockByteSize;

		// 分配描述符
		auto cbvCpuHandle = m_pDescriptorAllocator->Alloc(DescriptorType_CBV, 1);
		g_pDevice->CreateConstantBufferView(&cbvDesc, cbvCpuHandle);

		UINT currCBDatabyteOffset = m_pResource.byteOffset;
		m_pResource.byteOffset += actualBlockByteSize;

		oGPUVirtualAddr = cbvDesc.BufferLocation;

		return currCBDatabyteOffset;
	}
	 
	template<typename T>
	void UpdateCBData(T& data, UINT cbDataByteOffset)
	{
		UINT8* pSrc;
		HRESULT hr = m_pResource.pData->Map(0, nullptr, reinterpret_cast<void**>(&pSrc));

		UINT8* pDest = pSrc + cbDataByteOffset;
		memcpy(pDest, &data, sizeof(T));
	}

private:
	ID3D12Device* m_pDevice;
	DescriptorAllocator* m_pDescriptorAllocator;

	CBResource m_pResource;
};

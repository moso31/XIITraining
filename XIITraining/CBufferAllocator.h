#pragma once
#include "header.h"
#include "DescriptorAllocator.h"

// CBResource ������ CBUFFER_RESOURCE_ELEMENT_SIZE �� ����Դ ����Դ�ء�
// �ܴ�СΪ CBUFFER_RESOURCE_ELEMENT_SIZE * alignedByteSize ���ֽ�.
#define CBUFFER_RESOURCE_ELEMENT_SIZE 131072

struct CBResource
{
	UINT blockByteSize; // ���� Block ���ֽڴ�С����ֵ������ 256 �ı���
	UINT byteOffset;
	ID3D12Resource* pData;
};

class CBufferAllocator
{
public:
	CBufferAllocator(ID3D12Device* pDevice, DescriptorAllocator* pDescriptorAllocator) : 
		m_pDevice(pDevice), m_pDescriptorAllocator(pDescriptorAllocator) {}
	~CBufferAllocator() {}

	// ���� CBResource Pool��
	// blockByteSize ���� Pool �� ÿ�� Block �� ByteSize��
	void Init(UINT blockByteSize);

	// ����һ��CBV
	template<typename T>
	UINT AllocCBV(T& data, D3D12_GPU_VIRTUAL_ADDRESS& oGPUVirtualAddr)
	{
		size_t blockByteMask = m_pResource.blockByteSize - 1;
		UINT actualBlockByteSize = (UINT)((sizeof(T) + blockByteMask) & ~blockByteMask);

		// ����CBV
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_pResource.pData->GetGPUVirtualAddress() + m_pResource.byteOffset; // ������������GPU�����ַ
		cbvDesc.SizeInBytes = actualBlockByteSize;

		// ����������
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

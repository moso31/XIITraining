#pragma once
#include "header.h"

// ÿ�� CBResource ������ RESOURCE_ELEMENT_SIZE �� ����Դ ����Դ�ء�
// �ܴ�СΪ RESOURCE_ELEMENT_SIZE * alignByteSize ���ֽ�.
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

	// ����һ��CBuffer
	void Create(const std::string& name, UINT64 byteSize);

	// Ϊһ��CBuffer�����ڴ�
	void Alloc(const std::string& name, UINT64 allocSize);

private:
	UINT CalcBufferViewSize(UINT sizeInBytes) { return (sizeInBytes + 255) & ~255; }
private:
	ID3D12Device* m_pDevice;

	// CBuffer�����������������Դ
	std::unordered_map<std::string, CBResource> m_pResourcesMap;
};

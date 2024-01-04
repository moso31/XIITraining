#pragma once
#include "Texture.h"
#include "DescriptorAllocator.h"

class TextureAllocator
{
	struct Page
	{
		Page(UINT dataSize) { freeIntervals.insert({ 0, dataSize - 1 }); }
		std::set<AllocatorRangeInterval> freeIntervals;

		ID3D12Resource* pData;
	};

public:
	TextureAllocator();
	~TextureAllocator();

	void Alloc(Texture* pTexture);

private:
	std::vector<Page> m_pages;
};

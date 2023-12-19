#include "Texture.h"
#include "DescriptorAllocator.h"

void Texture::Load(const std::filesystem::path& path, const std::string& name)
{
	HRESULT hr;
	TexMetadata metadata;
	std::unique_ptr<ScratchImage> pImage = std::make_unique<ScratchImage>();

	std::string strExt = path.extension().string();
	if (strExt == ".dds")
	{
		hr = LoadFromDDSFile(path.wstring().c_str(), DDS_FLAGS_NONE, &metadata, *pImage);
	}
	else if (strExt == ".png" || strExt == ".tga")
	{
		hr = LoadFromWICFile(path.wstring().c_str(), WIC_FLAGS_NONE, &metadata, *pImage);
	}
	else
	{
		pImage.reset();
		return;
	}

	// Create the texture.
	{
		// Describe and create a Texture2D.
		D3D12_RESOURCE_DESC desc = {};
		desc.Width = static_cast<UINT>(metadata.width);
		desc.Height = static_cast<UINT>(metadata.height);
		desc.MipLevels = static_cast<UINT16>(metadata.mipLevels);
		desc.DepthOrArraySize = (metadata.dimension == TEX_DIMENSION_TEXTURE3D)
			? static_cast<UINT16>(metadata.depth)
			: static_cast<UINT16>(metadata.arraySize);
		desc.Format = metadata.format;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.SampleDesc.Count = 1;
		desc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);

		CD3DX12_HEAP_PROPERTIES defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_HEAP_PROPERTIES uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

		g_pDevice->CreateCommittedResource(
			&defaultHeap,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_pTexture));

		UINT layoutSize = desc.DepthOrArraySize * desc.MipLevels;

		auto texDesc = m_pTexture->GetDesc();
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT* layouts = new D3D12_PLACED_SUBRESOURCE_FOOTPRINT[layoutSize];
		UINT* numRow = new UINT[layoutSize];
		UINT64* numRowSizeInBytes = new UINT64[layoutSize];
		size_t totalBytes;
		g_pDevice->GetCopyableFootprints(&texDesc, 0, layoutSize, 0, layouts, numRow, numRowSizeInBytes, &totalBytes);

		CD3DX12_RESOURCE_DESC uploadBuffer = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
		// Create the GPU upload buffer.
		g_pDevice->CreateCommittedResource(
			&uploadHeap,
			D3D12_HEAP_FLAG_NONE,
			&uploadBuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_pTextureUpload));
		m_pTextureUpload->SetName(L"textureUploadHeap temp");

		void* mappedData;
		m_pTextureUpload->Map(0, nullptr, &mappedData);

		for (UINT face = 0, index = 0; face < texDesc.DepthOrArraySize; face++)
		{
			for (UINT mip = 0; mip < texDesc.MipLevels; mip++, index++)
			{
				const Image* pImg = pImage->GetImage(mip, face, 0);
				const BYTE* pSrcData = pImg->pixels;
				BYTE* pDstData = reinterpret_cast<BYTE*>(mappedData) + layouts[index].Offset;

				for (UINT y = 0; y < numRow[index]; y++)
				{
					memcpy(pDstData + layouts[index].Footprint.RowPitch * y, pSrcData + pImg->rowPitch * y, numRowSizeInBytes[index]);
				}
			}
		}

		m_pTextureUpload->Unmap(0, nullptr);

		for (UINT l = 0; l < layoutSize; l++)
		{
			CD3DX12_TEXTURE_COPY_LOCATION dst(m_pTexture.Get(), l); // 目标纹理的mip级别
			CD3DX12_TEXTURE_COPY_LOCATION src(m_pTextureUpload.Get(), layouts[l]); // 上传堆的mip级别

			// 指定目标mip级别的x, y, z偏移量，通常为0
			g_pCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
		}

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_pTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		g_pCommandList->ResourceBarrier(1, &barrier);
	}

	std::wstring wName(name.begin(), name.end());
	m_pTexture->SetName(wName.c_str());
	pImage.reset();
}

void Texture::AddSRV(TextureType type)
{
	if (!m_pTexture) return;

	auto cpuHandle = g_descriptorAllocator->Alloc(DescriptorType_SRV, 1);

	const auto& texDesc = m_pTexture->GetDesc();

	// 创建 SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // 默认的映射
	srvDesc.Format = texDesc.Format; // 纹理的格式

	if (type == TextureType_1D)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
		srvDesc.Texture1D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture1D.MostDetailedMip = 0;
		srvDesc.Texture1D.ResourceMinLODClamp = 0.0;
	}
	else if (type == TextureType_2D)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0;
		srvDesc.Texture2D.PlaneSlice = 0;
	}
	else if (type == TextureType_3D)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
		srvDesc.Texture3D.MipLevels = texDesc.MipLevels;
		srvDesc.Texture3D.MostDetailedMip = 0;
		srvDesc.Texture3D.ResourceMinLODClamp = 0.0;
	}
	else if (type == TextureType_Cube)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0;
	}

	g_pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, cpuHandle);

	// 保存 SRV
	m_SRVs.push_back(cpuHandle.ptr);
}

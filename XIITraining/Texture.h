#pragma once
#include "header.h"

enum TextureType
{
    TextureType_Unknown = 0,
    TextureType_Buffer = 1,
    TextureType_1D = 2,
    TextureType_1DArray = 3,
    TextureType_2D = 4,
    TextureType_2DArray = 5,
    TextureType_2DMS = 6,
    TextureType_2DMSArray = 7,
    TextureType_3D = 8,
    TextureType_Cube = 9,
    TextureType_CubeArray = 10,
    TextureType_RayTracing_Acceleration_Structure = 11
};

class Texture
{
public:
	Texture() {};
	~Texture() {};

	void Load(const std::filesystem::path& path, const std::string& name);
	void AddSRV(TextureType type);
    size_t GetSRV(UINT index) const { return m_SRVs[index]; }

private:
	ComPtr<ID3D12Resource> m_pTexture;
	ComPtr<ID3D12Resource> m_pTextureUpload;

	std::vector<size_t> m_SRVs; // size_t == D3D12_CPU_DESCRIPTOR_HANDLE.ptr
};

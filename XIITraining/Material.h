#pragma once
#include "header.h"

class Material
{
public:
	Material() {};
	~Material() {};

	void Load(const std::filesystem::path& shaderPath, const std::string& vsEntry, const std::string& psEntry, const std::string& vsTarget, const std::string& psTarget);

private:
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	ComPtr<ID3D12PipelineState> m_pPipelineState;

	D3D12_ROOT_PARAMETER m_rootParameter;
};

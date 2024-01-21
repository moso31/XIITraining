#include "Material.h"
#include "Texture.h"
#include "Mesh.h"

void Material::Load(const std::filesystem::path& shaderPath, const std::string& vsEntry, const std::string& psEntry, const std::string& vsTarget, const std::string& psTarget)
{
	ComPtr<ID3DBlob> pErrorBlob;

	HRESULT hr;
	hr = D3DCompileFromFile(shaderPath.wstring().c_str(), nullptr, nullptr, vsEntry.c_str(), vsTarget.c_str(), D3DCOMPILE_DEBUG, 0, &m_pVSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			std::string str = reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer());
			OutputDebugStringA(str.c_str());
		}
	}

	hr = D3DCompileFromFile(shaderPath.wstring().c_str(), nullptr, nullptr, psEntry.c_str(), psTarget.c_str(), D3DCOMPILE_DEBUG, 0, &m_pPSBlob, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob)
		{
			std::string str = reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer());
			OutputDebugStringA(str.c_str());
		}
	}
}

void Material::Reprofile()
{
	CreateViewsGroup();
	CreateRootSignature();
	CreatePSO();
}

void Material::Render(const UINT swapChainBufferIndex)
{
	g_pCommandList->SetGraphicsRootSignature(m_pRootSignature.Get());
	g_pCommandList->SetPipelineState(m_pPipelineState.Get());

	// cbPerFrame
	g_pCommandList->SetGraphicsRootConstantBufferView(0, g_cbPerFrame[swapChainBufferIndex].GPUVirtualAddr);

	// cbOfMaterial
	g_pCommandList->SetGraphicsRootDescriptorTable(2, m_gpuHandle);

	for (auto& pMesh : m_refMeshes)
	{
		pMesh->Render(swapChainBufferIndex);
	}
}

void Material::CreateRootSignature()
{
	// ������̬������
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.ShaderRegister = 0; // HLSL�еļĴ����� (s0)
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL; // ֻ��������ɫ���пɼ�
	samplerDesc.RegisterSpace = 0;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // ������������Ĳ��֣����û��Ʒ�ʽ
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.MipLODBias = 0; // MipLODƫ��
	samplerDesc.MinLOD = 0.0f; // MipLOD���½�
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX; // MipLOD���Ͻ�
	samplerDesc.MaxAnisotropy = 1; // �������Թ���
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // �ȽϺ���
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;

	std::vector<D3D12_STATIC_SAMPLER_DESC> pSamplers = { samplerDesc };

	// �˲���ʹ��һ������Ϊ1 ��������������ֻ��һ��SRV����ʾ��ǰ����ʹ�õ�����
	CD3DX12_ROOT_PARAMETER rootParam[3];
	rootParam[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_ALL); // cbPerFrame, b0
	rootParam[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // cbPerObject, b1

	// material textures, ��Ӧ register t0...tn�������������ʵ��ֻ���� t0
	CD3DX12_DESCRIPTOR_RANGE range[1];
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, (UINT)m_viewsGroup.size(), 0); // m_viewsGroup ��SRV����slot 0��ʼ
	rootParam[2].InitAsDescriptorTable(_countof(range), range);

	// TODO: material cbuffer params, register b2...

	// ������׼���ã��Ϳ��Դ�������������
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParam), rootParam, (UINT)pSamplers.size(), pSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	HRESULT hr;
	hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, nullptr);
	hr = g_pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
}

void Material::CreatePSO()
{
	// il bs dss rs �������ô����д�������
	// ʵ���ϸ��Ƽ�����ר�ŵĹ������ദ�����ǡ�
	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc = {};
	defaultRenderTargetBlendDesc.BlendEnable = false;
	defaultRenderTargetBlendDesc.LogicOpEnable = false;
	defaultRenderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.DestBlend = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	defaultRenderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
	defaultRenderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	defaultRenderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;

	D3D12_BLEND_DESC blendState = {};
	blendState.AlphaToCoverageEnable = false;
	blendState.IndependentBlendEnable = false;
	defaultRenderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		blendState.RenderTarget[i] = defaultRenderTargetBlendDesc;

	D3D12_RASTERIZER_DESC rasterizerState = {};
	rasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerState.FrontCounterClockwise = false;
	rasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;
	rasterizerState.ForcedSampleCount = 0;
	rasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_DEPTH_STENCIL_DESC depthStencilState = {};
	depthStencilState.DepthEnable = true;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilState.StencilEnable = false;
	depthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	depthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
	{
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_ALWAYS
	};
	depthStencilState.FrontFace = defaultStencilOp;
	depthStencilState.BackFace = defaultStencilOp;


	// ����PSO
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = m_pRootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(m_pVSBlob->GetBufferPointer(), m_pVSBlob->GetBufferSize());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(m_pPSBlob->GetBufferPointer(), m_pPSBlob->GetBufferSize());
	psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.BlendState = blendState;
	psoDesc.RasterizerState = rasterizerState;
	psoDesc.DepthStencilState = depthStencilState;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.SampleMask = UINT32_MAX;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	g_pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));
}

void Material::CreateViewsGroup()
{
	m_viewsGroup.clear();

	// ��ȡ����ʹ�õ�SRV��������Ϊ���ʵ�ViewGroup��һ���֡�
	// Ϊ�˼����������ֻ����һ�������������ֻ��һ����ֻʹ����һ��SRV��
	m_viewsGroup.push_back(m_pTexture->GetSRV(0));
}

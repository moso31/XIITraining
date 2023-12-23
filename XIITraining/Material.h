#pragma once
#include "header.h"

class Mesh;
class Texture;
class Material
{
public:
	Material() {};
	~Material() {};

	void Load(const std::filesystem::path& shaderPath, const std::string& vsEntry, const std::string& psEntry, const std::string& vsTarget, const std::string& psTarget);

	// ÿ�μ��ز��ʣ�����Ҫ���ô˷���
	// ��������������ʱ�ı����������������һ����ʽ�ķ�������������������൱�ڵ��˱��밴ť��
	void Reprofile();

	void Render();

	void AddRefMesh(Mesh* pMesh) { m_refMeshes.push_back(pMesh); }
	const std::vector<Mesh*>& GetSubMeshes() const { return m_refMeshes; }

	void SetGPUHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) { m_gpuHandle = gpuHandle; }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return m_gpuHandle; }

	void SetTexture(Texture* pTexture) { m_pTexture = pTexture; }

	const size_t GetViewsGroupSize() const { return m_viewsGroup.size(); }
	const size_t* GetViewsGroup() const { return m_viewsGroup.data(); }

private:
	void CreateRootSignature();
	void CreatePSO();
	void CreateViewsGroup();

private:
	ComPtr<ID3D12RootSignature> m_pRootSignature;
	ComPtr<ID3D12PipelineState> m_pPipelineState;

	D3D12_ROOT_PARAMETER m_rootParameter;

	// ��¼ gpu-visible ���������е�ƫ������ÿ֡��Ⱦǰ���£�ÿ֡���Ʋ���ǰ���á�
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;

	ComPtr<ID3DBlob> m_pVSBlob;
	ComPtr<ID3DBlob> m_pPSBlob;

private:
	Texture* m_pTexture; // ��Ϊ�涨 �˲���ֻ����һ������

	// ��¼���ô˲��ʵ�����Mesh
	std::vector<Mesh*> m_refMeshes;

	// ��¼�˲��ʵ�����cpuHandle
	std::vector<size_t> m_viewsGroup;

	// ������ʹ�õ����и�����
	// Ŀǰ�Ĺ滮�ǣ�һ�����������¼���еĲ��ʣ�һ��CBV��¼
	std::vector<D3D12_ROOT_PARAMETER> m_rootParameters;

	std::vector<D3D12_DESCRIPTOR_RANGE> m_descriptorRange;
};

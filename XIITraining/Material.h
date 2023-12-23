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

	// 每次加载材质，就需要调用此方法
	// 可以理解成连连看时的编译操作，这里做成一个显式的方法，调了这个方法就相当于点了编译按钮。
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

	// 记录 gpu-visible 描述符堆中的偏移量，每帧渲染前更新，每帧绘制材质前调用。
	D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;

	ComPtr<ID3DBlob> m_pVSBlob;
	ComPtr<ID3DBlob> m_pPSBlob;

private:
	Texture* m_pTexture; // 人为规定 此材质只有这一张纹理

	// 记录引用此材质的所有Mesh
	std::vector<Mesh*> m_refMeshes;

	// 记录此材质的所有cpuHandle
	std::vector<size_t> m_viewsGroup;

	// 材质所使用的所有根参数
	// 目前的规划是，一张描述符表记录所有的材质，一个CBV记录
	std::vector<D3D12_ROOT_PARAMETER> m_rootParameters;

	std::vector<D3D12_DESCRIPTOR_RANGE> m_descriptorRange;
};

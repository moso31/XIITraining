# XIITraining

## 1. 介绍

这个项目的作用是个人练手，主要目标是尝试写一个基础的DX12框架，能基于材质，纹理，Mesh进行管理对DX12的底层API进行一定程度的封装管理。

这个Readme主要用于记录一些心得。

## 2. 主要结构

分为初始化阶段和渲染阶段：

- 初始化阶段 `D3D::Init()` 
	- 初始化DX设备，命令分配器，命令列表，命令队列
 	- 创建交换链的 RTV DSV 描述符堆
  - 创建描述符分配器
  - 创建常量资源分配器
  - 搭建场景：创建材质，纹理，Mesh，创建全局cb
- 渲染阶段
  - `D3D::Prepare()`：准备GPU可见描述符堆，每帧更新一次
  - `D3D::Update()`：负责每帧更新CB（WVP矩阵）
  - `D3D::Render()`：负责在命令列表中渲染，并提交命令列表

### 2.1. 初始化DX设备，命令分配器，命令列表，命令队列，创建交换链的 RTV DSV 描述符堆

这些内容读一下官方API或者龙书之类的教程，按部就班即可。

相关资料网络上已经很多，不再赘述。

### 2.2. 描述符分配器（DescriptorAllocator）

DescriptorAllocator 是描述符分配器，主要负责分配描述符。

在管理结构上，分为两个部分：按页存储的若干CPU描述符堆 `DescriptorAllocator::m_pages`，和GPU可见的描述符堆 DescriptorAllocator::m_renderHeap。

每帧的`D3D::Prepare()`方法中，将`m_pages`中需要渲染的那些描述符找出来，然后映射到GPU可见的`m_renderHeap`上。

#### 描述符分配器的实际使用

假设我们的工作流首先创建纹理，然后创建材质。

首先，在创建纹理的时候，通过 `Texture::AddSRV()` 方法，为纹理创建对应的 SRV。描述符分配器会负责分配一个 CPU 可见的 SRV，并存到 `DescriptorAllocator::m_heaps` 上。

然后，创建材质并编译。在这个 Demo 中，`Materials::Reprofile()` 类似在游戏中的材质 “编译” 的过程。

编译时，通过 `Material::CreateViewsGroup()` 方法，重新构建材质依赖的描述符。比如，如果材质在设计上依赖了多张纹理，这个时候就会获取这些 **纹理的SRV** 在 **描述符分配器** 上的 **CPU描述符**。为了方便渲染时调用，材质依赖的描述符会以数组的形式，存储到 `Material::m_viewsGroup` 中。

每帧渲染前（`D3D::Prepare`时），按遍历顺序，将每个材质的 `m_viewsGroup` 映射到 GPU 描述符堆上。

大致的结构如图所示：![image](https://github.com/moso31/XIITraining/assets/15684115/5be328f4-1fc2-4c2e-84fa-aac9a13a66c6)

> 调用`CreateViewsGroup()`方法时需要注意，每个材质依赖的shader代码中，SRV 寄存器（register(s0);） 的顺序，一定要和`m_viewsGroup`记录的CPU描述符的映射关系的顺序一致，不然 SRV 的加载顺序会乱。

每帧渲染时的流程大致如下：

```C++
void D3D::Prepare()
{
	for (auto& pMaterial : m_pMaterials)
	{
		// 获取这个材质使用的所有 non-shader-visible (cpu) 描述符
		const size_t* pDescriptors = pMaterial->GetViewsGroup();
		const size_t pDescriptorsSize = pMaterial->GetViewsGroupSize();

		// 将这些 描述符 追加到 shader-visible (gpu) 的描述符堆 
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = g_pDescriptorAllocator->AppendToRenderHeap(pDescriptors, pDescriptorsSize);

		// 记录其在 GPU 描述符堆上的 首个GPU句柄
		pMaterial->SetGPUHandle(gpuHandle);
	}
}
```

每个材质会获得其在 GPU 描述符堆 `DescriptorAllocator::m_renderHeap` 上的 首个地址的句柄 `gpuHandle`。每帧渲染时，让描述符表使用这个 `gpuHandle` 即可：

```C++
void Material::Render()
{
	... 其它内容省略
 
	g_pCommandList->SetGraphicsRootDescriptorTable(2, m_gpuHandle);

	for (auto& pMesh : m_refMeshes)
	{
		pMesh->Render();
	}
}
```

### 2.3. CBuffer 分配器

CBuffer分配器类`CBufferAllocator`，结构上可以看做是一个按页存储的若干页CBuffer的集合。

该类中使用 `CBufferAllocator::m_pages` 管理这些页面。可以将其中每单个 `m_pages[i]` 都看成一个资源池。

当调用 AllocCBV() 时，会自动根据输入的 `T& data` 类型，在已经分配的页面里寻找是否有新的页面。如果没有，就再创建一个新的页面。

每个页对应的 ID3D12Resource 都以 上传堆 的形式创建，若调用 AllocCBV() 且分配成功，就会返回对应的 D3D12_GPU_VIRTUAL_ADDRESS 虚拟地址，以及这段资源具体位于哪个页面的哪段字节。

这样每帧渲染时，就可以通过这些信息寻址，实现对 GPU 上的数据（比如MVP矩阵）的更新：

```C++

// Mesh.cpp
void Mesh::Update()
{
	static float r = 0.0f;
	r += 0.0025f;

	Matrix mx = Matrix::CreateScale(m_scale);
	if (m_rotate) mx = mx * Matrix::CreateRotationX(-r) * Matrix::CreateRotationY(r);

	m_cbData.worldMatrix = mx;

	g_pCBufferAllocator->UpdateCBData(m_cbData, m_cbDataCBufferPageIndex, m_cbDataByteOffset);
}

// CBufferAllocator.h
template<typename T>
void UpdateCBData(T& data, UINT cbDataByteOffset)
{
	UINT8* pSrc;
	HRESULT hr = m_pResource.pData->Map(0, nullptr, reinterpret_cast<void**>(&pSrc));

	UINT8* pDest = pSrc + cbDataByteOffset;
	memcpy(pDest, &data, sizeof(T));
}
```


> p.s.: CBuffer分配器 和上面的 描述符分配器 的实现思路，主要受这个讨论：https://www.gamedev.net/forums/topic/708811-d3d12-best-approach-to-manage-constant-buffer-for-the-frame/ ，ddlox 提到的 'paging method' 的启发。

### 2.4 其它

相对上述内容的顶层调用而已，并不是核心内容。略了。

## 3. 效果

最终效果并不复杂，但概念相对比较完整：

首先创建两张纹理，一个 2d，一个 cube。

然后创建了两个材质，一个材质中连 2d 纹理，另一张连 Cube 纹理，采样方法不同，所以使用了两个不同的 Shader。

最后创建两个Mesh，并将材质绑到模型上。

这个Demo主要是我自己想捋一下，应该如何设计一套方法来手动管理 CBV 和 SRV。

![image](https://github.com/moso31/XIITraining/assets/15684115/af600d44-193b-4681-9e15-ac6bf0120f3f)
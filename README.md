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

### 2.2. 描述符分配器（CBufferAllocator）

CBufferAllocator 是描述符分配器，主要负责对场景内的。。。

#### 一个是 CPU 描述符堆集：`m_heaps`。

`m_heaps` 负责存储记录用到的描述符。说的更简单一点就是，当你给纹理资源创建 SRV 的时候，就在 `m_heaps` 同步注册一个 SRV；当你的 Mesh 初始化分配 CB 数据（用来存世界矩阵之类的东西）的时候，就在 `m_heaps` 注册一个 CBV。

> 受 DX API 限制，描述符堆数量有上限，所以需要使用数组的形式存储，以防到达上限。

> 目前我按个人直觉，规定了其中每个描述符堆是单类型的，即一个描述符堆只能存储 CBV/SRV/UAV 中的一种（但我不确定这是否有必要）。

#### 另一个是 GPU 描述符堆：`m_renderHeaps`

和 `m_heaps` 的主要区别在于，`m_renderHeaps` 是 GPU 可见的。只有 GPU 可见的描述符堆，才能被绑定到 DX12 的根签名。

每帧渲染开始之前，会通过 `DescriptorAllocator::AppendToRenderHeap()` 方法，将本帧用到的 CPU 的描述符从 `m_heaps` 里抽出来，拷贝到 `m_renderHeaps` 里。

#### 描述符分配器的实际使用

假设我们的工作流首先创建纹理，然后创建材质。

首先，在创建纹理的时候，通过 `Texture::AddSRV()` 方法，为纹理创建对应的 SRV。描述符分配器会负责分配一个 CPU 可见的 SRV，并存到 `DescriptorAllocator::m_heaps` 上。

然后，创建材质并编译。在这个 Demo 中，可以通过调用 `Materials::Reprofile()` 方法，模拟在游戏中的材质 “编译” 的过程。

编译时，通过 `Material::CreateViewsGroup()` 方法，重新构建材质依赖的描述符。比如，如果材质在设计上依赖了多张纹理，这个时候就会获取这些 **纹理的SRV** 在 **描述符分配器** 上的 **CPU描述符**。为了方便渲染时调用，材质依赖的描述符会以数组的形式，存储到 `Material::m_viewsGroup` 中。

每帧渲染前，遍历这些映射关系。最终，调用 `DescriptorAllocator::AppendToRenderHeap()` 映射到 GPU 描述符堆上即可。

大致的结构如图所示：![image](https://github.com/moso31/XIITraining/assets/15684115/5be328f4-1fc2-4c2e-84fa-aac9a13a66c6)

> 注意，每个材质依赖的shader代码中，SRV 寄存器（register(s0);） 的顺序，一定要和自己的`m_viewsGroup`记录的CPU描述符的映射关系的顺序一致。

然后，记录其在 GPU 描述符堆上的 首个GPU句柄 即可。

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

等之后渲染时，这个 GPU句柄 是 DX12的命令列表 可以直接使用的：

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

### 2.3. CBuffer 分配器（有待完善）

> 更完善的 CBuffer 分配器应该考虑进一步的动态管理：https://www.gamedev.net/forums/topic/708811-d3d12-best-approach-to-manage-constant-buffer-for-the-frame/

> 我的实现目前是个简易版本，相当于只实现了上述帖子中的 "large buffer method"。

CBuffer分配器类`CBufferAllocator`，在概念上可以理解成使用一个超大的 CB，一个资源池。

在初始化阶段，通过`Init()`，这个超大的CB其实就准备好了。剩下的其实只是根据渲染场景的情况，往这个大的资源池里分配CBV，并通过在 Mesh 中记录 CBV 字节偏移量（`m_cbDataByteOffset`）的方法，记录这个 CBV 是哪个 Mesh 的。

渲染时，在 `Mesh::Update()` 方法遍历更新每个 Mesh 的时候，就可以通过 Mesh 所记录的字节偏移量`m_cbDataByteOffset`，通过`memcpy()`，刷新 超大CB中 对应当前mesh的部分的值：

```C++
template<typename T>
void UpdateCBData(T& data, UINT cbDataByteOffset)
{
	UINT8* pSrc;
	HRESULT hr = m_pResource.pData->Map(0, nullptr, reinterpret_cast<void**>(&pSrc));

	UINT8* pDest = pSrc + cbDataByteOffset;
	memcpy(pDest, &data, sizeof(T));
}
```

#### CBuffer分配器的实际使用

其实只需要理解两件事：CBV 是什么时候分配的，每个CBV对应的CBData是怎么更新的。

在这个小框架中，需要 CBV 的地方目前只有两处：每帧更新的 cbPerFrame（相机的VP矩阵），和每个Obj独立更新的cbPerObject（Mesh的世界矩阵）。

在每个Mesh自己初始化的时候，就通过 `Mesh::CreateCBuffer()` 方法，在CBuffer分配器中，创建了自己的 CBV，并
- 通过 `Mesh::m_cbDataByteOffset` 确定了该CBV在资源池中的字节偏移量
- 通过 `Mesh::m_cbDataGPUVirtualAddr` 确定了该CBV在GPU上的虚拟地址

渲染时，只需要将 GPU 上的虚拟地址指向想要设置的 根描述符 上即可：
```C++
void Mesh::Render()
{
	// cbPerObject
	g_pCommandList->SetGraphicsRootConstantBufferView(1, m_cbDataGPUVirtualAddr);

	... 其它省略
}
```

### 2.4 其它

相对上述内容的顶层调用而已，并不是核心内容。略了。

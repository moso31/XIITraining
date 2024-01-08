# XIITraining

## 1. 介绍

这个项目的作用是个人练手，主要目标是尝试写一个基础的DX12框架，能基于材质，纹理，Mesh进行管理对DX12的底层API进行一定程度的封装管理。

这个Readme主要用于记录一些心得。

## 2. 主要结构

分为初始化阶段和渲染阶段：

- 初始化阶段 `D3D::Init()` 
	- 初始化DX设备，命令分配器，命令列表，命令队列
 	- 创建交换链的 RTV DSV 描述符堆
  - 显存分配（重要）
    - 分配器
    - 分配器基类 XAllocator
    - 描述符分配器（DescriptorAllocator）
    - 纹理分配器（PlacedResourceAllocator）
    - 常量资源分配器（CommittedResourceAllocator）
  - 搭建场景：创建材质，纹理，Mesh，创建全局cb
- 渲染阶段
  - `D3D::Prepare()`：准备GPU可见描述符堆，每帧更新一次
  - `D3D::Update()`：负责每帧更新CB（WVP矩阵）
  - `D3D::Render()`：负责在命令列表中渲染，并提交命令列表

### 2.1. 初始化DX设备，命令分配器，命令列表，命令队列，创建交换链的 RTV DSV 描述符堆

这些内容读一下官方API或者龙书之类的教程，按部就班即可。

相关资料网络上已经很多，不再赘述。

### 2.2. 显存分配（重要）

DX12 和 DX11 相比，一个最明显的区别就是，12需要亲手设计自己的显存分配机制，而11则几乎完全交由驱动层管理。

在当今的计算机架构中，购买一块独立显卡，意味着你电脑的内存系统可以被划分为三个主要部分：**CPU内存、共享内存、GPU内存（显存）**。

- CPU内存是最直观的部分。例如，c++ new一下，就是在分配内存空间；
- GPU内存（显存）则关联于创建的图形资源（如Buffer、Texture）。当通过DirectX API等进行读取时，相应的显存空间会由显卡驱动自动在GPU内存中分配。举例来说，DirectX 11中的D3D11Device::CreateBuffer/Texture1D/2D/3D函数的作用就是在GPU内存中为这些资源分配空间。
- 共享内存在硬件层面上是普通内存，但在显存不足时，它可以作为一种备用显存来使用。

### 2.2.1. 通信速率：显卡>内存>总线>硬盘(2024年）（主要是扯硬件的，可跳过）

首先来聊一下2024年的硬件通信速率吧。就通信速度而言，在当今（2024年）大多数计算机系统中，显存的速度优于内存，内存的速度优于总线，而总线的速度又优于硬盘。

相信搞DX的人很多都见过龙书给的一张图：

![image](https://github.com/moso31/XIITraining/assets/15684115/1d30e297-3234-4d12-9f61-3cbcff02dfcd)

这张图的数据并不准，它只是明确指出了一个概念：显卡通信速率最快，内存通信次之，最慢是总线，硬盘的部分没提。

考虑到这张上古老图都快有15年历史了，并且数据不准，我们不妨更新一下它。现在假设有一台非常好的顶级机器，所有设置都按顶配的条件安排：

- RTX4090 显卡
- KLEVV DDR5 32G 旗舰内存
- ROG STRIX Z790-A GAMING WIFI 主板
- Samsung 980 PRO 2TB NVMe 硬盘

然后我们认真计算一下这四者的通信传输效率吧。

#### 通信速率：显卡(GPU-VRAM)：

一般的显卡都会在技术参数中明确描述自己的带宽。

以N卡为例，4090的实际带宽，按照官网数据计算，是1008GB/s。

作为横向比对、2080的带宽是448.0 GB/s；1060(6GB版)的带宽则是192.2GB/s。

#### 通信速率：内存(CPU-RAM)：

以KLEVV CRAS V RGB DDR5-8000 32 GB Review，这是目前比较好的内存了。

对内存而言，一般关注内存的速度等级，因为从该值可以直接推导该内存理论上的最大传输速率。该内存的数值是8000MT/s。

内存的传输速率计算需要考虑的条件比较多，除了上面的传输速率，还需要考虑数据率、总线宽度、以及内存和主板是否支持双通道和四通道。计算步骤如下：

1. 由于DDR=double data rate表示双倍数据率，所以实际的数据传输速率是该值的两倍。
2. 另外，DDR5内存模块的总线宽度通常为64位。
3. 最后，当今大多数机器的主板是双通道的，所以还要乘以2。一些发烧机器和主板可能会组四通道或者八通道，这种情况下就乘以4或者8。

我们这里考虑这次的主板，ROG STRIX Z790-A GAMING WIFI主板，内存最多只支持双通道，所以最终的速度是8000*2*64*2=2048000b/s=256GB/s。

这在理论上已经比相当多的显卡的通信速率还快。但这是完全理想的情况，实际运行时内存的运行效率远不能达到这么高的理论值，并且拿顶配内存对比平民显卡，多少有点耍流氓了……

至少在现在2024年，仍然可以认为，显卡的通信速度还是数倍快于内存的。

#### 通信速率：总线(CPU-GPU)

总线的通信效率是一个相对复杂的问题，同时取决于你的显卡、主板、甚至硬盘。

首先，明确一下PCIe协议如何换算成传输速率。这玩意通常每代翻一倍。5.0 x16 是 4.0x16 的两倍，3.0x16的四倍；同理，5.0x16 = 4.0x8 = 3.0x4。
按照维基百科的说法，PCIe 5.0 x16，相当于通信速率是63GB/s。

然后看显卡，显卡有自己的PCIe规格，比如我们假设的这台机器，4090显卡的官方技术参数表示，支持PCIe 4.0 x16，这意味着该显卡本身最多只能让总线通信这么多带宽。

再说主板的部分。ROG STRIX Z790-A GAMING WIFI的PCIe插槽有四个：

- 1 x PCIe 5.0 x16 slot
- 2 x PCIe 4.0 x16 slots (x4 mode，换句话说每个插槽实际上只支持PCIe 4.0 x4的性能)
- 1 x PCIe 3.0 x1 slot

考虑到4090最高支持PCIe 4.0 x16，这就意味着要追求CPU-GPU的最高带宽，就必须将4090插在那根PCIe 5.0 x16的槽上。

因为5.0是4.0的两倍，所以最终，CPU-GPU的通信速率为31.5GB/s。

> p.s. 对某些主板而言，还需要考虑硬盘的部分，因为NVMe的硬盘也使用PCIe插槽。这意味着如果你的机器上插着太多这样的硬盘，就有可能挤占显卡使用的PCIe空间。但对我们假设的机器来说，这完全不是问题。且不说主板本身就没有这么多硬盘插槽，就算有，至少需要几十个硬盘，再多插一两张其他显卡，才有可能需要考虑带宽相互挤占的问题。我们先不考虑这种情景。

#### 通信速率：硬盘

硬盘更常见的说法一般还是读写速度。以前面要求的 Samsung 980 PRO 2TB 为例，顺序读写速度为：7000MB/s和5100MB/s，按6GB/s算吧。

当然也需要考虑 PCIe 的通信速率。

多提一句：千万别插在 PCIE 3.0 的插槽上，不然会因为木桶效应减慢硬盘的读写速度。

还有，插在那两个 PCIe 4.0 x16 slots (x4 mode) 上面，其实也发挥不出硬盘的最大读写速度。因为这两个槽实际上只有 PCIe 4.0 x4 的通信速率，实际上只有 3.9375GB/s 的通信速率，同样达不到硬盘的 6GB/s

所以其实我们可以得出一个结论：我给的这台机器主板其实配错了（笑）。想要硬盘发挥最大性能，其实不能使用这里的 Z790-A；作为正确答案之一，可以将主板换成 Z790-E，因为该卡提供了一个 PCIE 5.0x4 级别的M.2接口。

#### 通信速率：结论

所以综上，对当今大多数机器而言，显卡(1TB/s)>内存(256GB/s)>总线(31GB/s)>硬盘(6GB/s)。个人估计这里面内存的数据水分最大、硬盘次之。

所以，龙书十几年前的100:10:1的比例已经失准了，确实只能参考一下概念（当然人家写那段本来就只是让你参考概念的）。

#### 2.2.1 分配器

即使是在DX11的年代，要将一个纹理资源传到显存上，其实底层也是这样一个流程：首先从硬盘读取到内存，再由内存加载到共享内存，最后由共享内存加载到显存。

这个Demo中设计了一个通用的内存分配机制。

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

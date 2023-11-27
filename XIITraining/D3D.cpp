#include "D3D.h"

void D3D::Init()
{
	// ���� D3D12 ���Բ�
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();
#endif

	HRESULT hr;

	// ���� DXGI ����
	ComPtr<IDXGIFactory4> pDXGIFactory;
	hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&pDXGIFactory));

	// �� DXGI ������ ö�ٵ�һ����������Ĭ�ϵ��Կ���
	ComPtr<IDXGIAdapter1> pAdapter;
	pDXGIFactory->EnumAdapters1(0, &pAdapter);

	// ת���� IDXGIAdapter4��
	// IDXGIAdapter �����ʼ�汾�����Ƽ�ʹ�ã�
	// IDXGIAdapter1����������Կ��Ĺ�Ӧ��ID���豸ID����ϵͳID�ȡ�
	// IDXGIAdapter2��������Կ����й��Ĺ���
	// IDXGIAdapter3����һ�������˶��Դ�ʹ������Ĳ�ѯ���ܡ�
	// IDXGIAdapter4 �ṩ�˶� GPU ���߼���Ŀ��ƺͼ��ӡ��ر��Ƕ�Ӳ���������ݵķ��ʡ�
	ComPtr<IDXGIAdapter4> pAdapter4;
	hr = pAdapter.As(&pAdapter4);

	// ���� D3D12 �豸
	// ID3D12Device Ҳ�ֳ����ɰ汾�����ڰ汾̫���ˣ�������Ҫ��˵��
	// �� ID3D12Device4 ��ʼ�������˶Թ���׷�ٹ��ܵĽӿ�֧�֡�
	// Խ�ϵİ汾�����Ժ��ȶ���Խǿ���Լ�����ѡ��
	hr = D3D12CreateDevice(pAdapter4.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_pDevice));
}

void D3D::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; 
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr;

	// �����������
	hr = m_pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pCommandQueue));

	// �������������
	hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCommandAllocator));

	// ���������б�������� ��������� ����
	// ͬʱ�趨��ʼ��Ⱦ����״̬ = nullptr��Ĭ��״̬��
	hr = m_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_pCommandList));

	// ��ʼĬ�Ϲر������б�
	// ��Ȼ���ǹٷ��涨��������һ������ʵ��������
	// ��ʼ�ر������б���Ҫ������б��ʱ��ʹ�� Reset ���´�����Ȼ���ټ�¼�µ����
	// �������Ա��� CommandList �ں��������б������߼��������������Ⱦ����
	// Ҫ����Close�����������������б�֮��ĵ����߼��ǳ���ȷ�����ǣ��α��أ�
	m_pCommandList->Close();
}

void D3D::CreateSwapChain()
{
}

#include "Direct3D9.h"
#include "Texture.h"
#include "Device.h"

HOOK(void, __cdecl, LoadPictureData, 0x743DE0,
    hh::mr::CPictureData* pPictureData, const uint8_t* pData, size_t length, hh::mr::CRenderingInfrastructure* pRenderingInfrastructure)
{
    if (!pPictureData || (pPictureData->m_Flags & 1) != 0 || (pData && *(uint32_t*)pData != MAKEFOURCC('D', 'D', 'S', ' ')))
        return originalLoadPictureData(pPictureData, pData, length, pRenderingInfrastructure);

    const ComPtr<Device> device = (Device*)pRenderingInfrastructure->m_RenderingDevice.m_pD3DDevice;

    ComPtr<ID3D12Resource> resource;
    std::vector<D3D12_SUBRESOURCE_DATA> subResources;

    const HRESULT result = DirectX::LoadDDSTextureFromMemory(device->GetD3D12Device(), pData, length, resource.GetAddressOf(), subResources);

    if (SUCCEEDED(result) && resource != nullptr)
    {
        pPictureData->m_pD3DTexture = (DX_PATCH::IDirect3DBaseTexture9*)(new Texture(device, resource));
        pPictureData->m_Type = hh::mr::ePictureType_Texture;
    }
    else
    {
        pPictureData->m_pD3DTexture = nullptr;
        pPictureData->m_Type = hh::mr::ePictureType_Invalid;
    }

    pPictureData->m_Flags |= 1;
}

HOOK(HRESULT, __stdcall, FillTexture, 0xA55270, Texture* d3dTexture, void* function, void* data)
{
    return S_OK;
}

HOOK(Direct3D9*, __cdecl, Direct3DCreate, 0xA5EDD0, UINT SDKVersion)
{
    return new Direct3D9(SDKVersion);
}

extern "C" __declspec(dllexport) void Init()
{
    INSTALL_HOOK(LoadPictureData);
    INSTALL_HOOK(FillTexture);
    INSTALL_HOOK(Direct3DCreate);
}
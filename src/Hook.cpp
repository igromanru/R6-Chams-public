
#include "Hook.hpp"

#ifdef _DEBUG
    #include <cstdio>
#endif

#include "minhook/include/MinHook.h"
#include "Models.hpp"

namespace Hook
{
    constexpr void SafeRelease(IUnknown* dxObject)
    {
        if (dxObject)
        {
            dxObject->Release();
            dxObject = nullptr;
        }
    }

    constexpr void SafeReleaseSRVs(ID3D11ShaderResourceView* SRVs[])
    {
        for (int i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
        {
            if (SRVs[i] == nullptr)
            {
                break;
            }
            SafeRelease(SRVs[i]);
        }
    }

    constexpr int CountSRVs(ID3D11ShaderResourceView* SRVs[])
    {
        auto i = 0;
        for (;i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
        {
            if (!SRVs[i])
            {
                break;
            }
        }
        return i;
    }

    DWORD WINAPI Init()
    {
        auto result = FALSE;

        auto mhStatus = MH_Initialize();
        if (mhStatus == MH_OK)
        {
            if (const auto moduleHandle = GetModuleHandleW(L"d3d11.dll"))
            {
    #ifdef _DEBUG
                printf("d3d11.dll module handle: %p\n", reinterpret_cast<void*>(moduleHandle));
    #endif
                // ToDo add FindPattern here, if needed
                DrawIndexedInstancedIndirectAddress = reinterpret_cast<LPVOID>(reinterpret_cast<uintptr_t>(moduleHandle) + 0x154300);

                if (DrawIndexedInstancedIndirectAddress)
                {
#ifdef _DEBUG
                    printf("Found DrawIndexedInstancedIndirect function at: %p\n", DrawIndexedInstancedIndirectAddress);
#endif
                    mhStatus = MH_CreateHook(DrawIndexedInstancedIndirectAddress, DetourDrawIndexedInstancedIndirect, reinterpret_cast<LPVOID*>(&OriginalDrawIndexedInstancedIndirect));
                    if (mhStatus == MH_OK)
                    {
                        mhStatus = MH_EnableHook(DrawIndexedInstancedIndirectAddress);
                        result = mhStatus == MH_OK;
                        if (mhStatus == MH_OK)
                        {
#ifdef _DEBUG
                            printf("DrawIndexedInstancedIndirect hook enabled\n");
#endif
                        }
                        else
                        {
#ifdef _DEBUG
                            printf("Error: Failed MH_EnableHook for DrawIndexedInstancedIndirect! Status: %d\n", mhStatus);
#endif
                        }
                    }
                    else
                    {
#ifdef _DEBUG
                        printf("Error: Failed MH_CreateHook for DrawIndexedInstancedIndirect! Status: %d\n", mhStatus);
#endif
                    }
                }
                else
                {
#ifdef _DEBUG
                    printf("Error: Couldn't find DrawIndexedInstancedIndirect function!\n");
#endif
                }
            }
            else
            {
#ifdef _DEBUG
                printf("Error: Failed to get d3d11.dll module handle!\n");
#endif
            }
        }
        else
        {
#ifdef _DEBUG
            printf("Error: Failed to initialize MinHook! Status: %d\n", mhStatus);
#endif                    
        }

        return result;
    }

    void Unload()
    {
        if (DrawIndexedInstancedIndirectAddress)
        {
            MH_DisableHook(DrawIndexedInstancedIndirectAddress);
        }

        SafeRelease(wallHackDSS);

        SafeRelease(redSRV);
        SafeRelease(greenSRV);
    }

    void CreateColorSRV(ID3D11Device* device, const RGBA& color, ID3D11ShaderResourceView** shaderResourceView)
    {
        if (device && shaderResourceView)
        {
            D3D11_SUBRESOURCE_DATA initData;
            initData.pSysMem = &color;
            initData.SysMemPitch = sizeof(color);
            initData.SysMemSlicePitch = 0;

            ID3D11Texture2D* texture = nullptr;
            D3D11_TEXTURE2D_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            desc.Width = 1;
            desc.Height = 1;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.SampleDesc.Count = 1;
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            device->CreateTexture2D(&desc, &initData, &texture);

            if (texture)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
                ZeroMemory(&SRVDesc, sizeof(SRVDesc));
                SRVDesc.Format = desc.Format;
                SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                SRVDesc.Texture2D.MipLevels = 1;
                device->CreateShaderResourceView(texture, &SRVDesc, shaderResourceView);
                SafeRelease(texture);
            }
        }
    }

    void CreateColorSRVs(ID3D11Device* device)
    {
        if (device && !greenSRV && !redSRV)
        {
            const RGBA red(255, 0, 0);
            const RGBA green(0, 255, 0);

            CreateColorSRV(device, red, &redSRV);
            CreateColorSRV(device, green, &greenSRV);
        }
    }

    void CreateDepthStencilStates(ID3D11Device* device, ID3D11DepthStencilState* originalDSS)
    {
        if (device && !wallHackDSS && originalDSS)
        {
            D3D11_DEPTH_STENCIL_DESC desc;
            ZeroMemory(&desc, sizeof(desc));
            originalDSS->GetDesc(&desc);
            desc.DepthEnable = FALSE;
            desc.StencilEnable = FALSE;
            device->CreateDepthStencilState(&desc, &wallHackDSS);
        }
    }

    bool DrawChams(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs, ID3D11ShaderResourceView* SRVs[], ID3D11ShaderResourceView* srvWhenInvisible, ID3D11ShaderResourceView* srvWhenVisible, ID3D11DepthStencilState* visibleDSS)
    {
        if (pContext && pBufferForArgs && SRVs)
        {
            const auto srvCount = CountSRVs(SRVs);
            if (srvWhenInvisible)
            {
                for (auto i = 0; i < srvCount; i++)
                {
                    pContext->PSSetShaderResources(i, 1, &srvWhenInvisible);
                }
                pContext->OMSetDepthStencilState(wallHackDSS, 0);
                OriginalDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
            }

            if (srvWhenVisible && visibleDSS)
            {
                for (auto i = 0; i < srvCount; i++)
                {
                    pContext->PSSetShaderResources(i, 1, &srvWhenVisible);
                }
                pContext->OMSetDepthStencilState(visibleDSS, 0);
                OriginalDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);

                return true;
            }
        }

        return false;
    }

    void __stdcall DetourDrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
        ID3D11Device* device;
        pContext->GetDevice(&device);
        if (!device)
        {
            return OriginalDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
        }

        CreateColorSRVs(device);

        ID3D11Buffer* veBuffer = nullptr;
        UINT Stride;
        UINT veBufferOffset;
        pContext->IAGetVertexBuffers(0, 1, &veBuffer, &Stride, &veBufferOffset);
        if (veBuffer)
        {
            SafeRelease(veBuffer);
        }

        ID3D11Buffer* pscBuffer = nullptr;
        D3D11_BUFFER_DESC pscDesc;
        UINT pscWidth = 0;

        pContext->PSGetConstantBuffers(0, 1, &pscBuffer);
        if (pscBuffer)
        {
            pscBuffer->GetDesc(&pscDesc);
            pscWidth = pscDesc.ByteWidth;
            SafeRelease(pscBuffer);
        }

        ID3D11Buffer* vscBuffer = nullptr;
        D3D11_BUFFER_DESC vscDesc;
        UINT vscWidth = 0;

        pContext->VSGetConstantBuffers(0, 1, &vscBuffer);
        if (vscBuffer)
        {
            vscBuffer->GetDesc(&vscDesc);
            vscWidth = vscDesc.ByteWidth;
            SafeRelease(vscBuffer);
        }

        const auto compVscWidth = vscWidth / 10;
        const auto compPscWidth = pscWidth / 10;

        ID3D11ShaderResourceView* originalSRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
        ZeroMemory(originalSRVs, sizeof(originalSRVs));
        pContext->PSGetShaderResources(0, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT, originalSRVs);

        if (Models::Body1.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            || Models::Body2.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            || Models::Skin.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            )
        {
            ID3D11DepthStencilState* originalDSS = nullptr;
            UINT originalStencilRef = 0;
            pContext->OMGetDepthStencilState(&originalDSS, &originalStencilRef);

            const auto dontDrawAgain = DrawChams(pContext, pBufferForArgs, AlignedByteOffsetForArgs, originalSRVs, redSRV, greenSRV, originalDSS);
            pContext->OMSetDepthStencilState(originalDSS, originalStencilRef);
            SafeRelease(originalDSS);

            if (dontDrawAgain)
            {
                SafeReleaseSRVs(originalSRVs);
                return;
            }
        }
        else if (Models::DroneWorldOverlay.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            || Models::WorldOverlay1.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            || Models::WorldOverlay2.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            || Models::WorldOverlay3.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            || (Models::WorldOverlay4.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs) && !Models::Scope.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs))
            )
        {
            SafeReleaseSRVs(originalSRVs);
            return;
        }

        SafeReleaseSRVs(originalSRVs);
        OriginalDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
    }
}

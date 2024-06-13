/*
 *  R6 Chams by Igromanru
 *  Copyright (C) 2024 Igromanru
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
 *  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "Hook.hpp"

#ifdef _DEBUG
    #include <cstdio>
    #define DEBUG_PRINT(format, ...) printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...) (void)0
#endif

#include "minhook/include/MinHook.h"
#include "FindPattern/FindPattern.h"
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
        auto mhStatus = MH_Initialize();
        if (mhStatus != MH_OK)
        {
            DEBUG_PRINT("Error: Failed to initialize MinHook! Status: %d\n", mhStatus);
            return false;
        }

        // const auto gameHandle = GetCurrentProcess();
        // if (!gameHandle)
        // {
        //     DEBUG_PRINT("Error: Failed to get game module handle. LastError: %lu\n", GetLastError());
        //     return false;
        // }
        // DEBUG_PRINT("Game module handle: %p\n", reinterpret_cast<void*>(gameHandle));

        const auto moduleHandle = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"d3d11.dll"));
        if (!moduleHandle)
        {
            DEBUG_PRINT("Error: Failed to get d3d11.dll module handle. LastError: %lu\n", GetLastError());
            return false;
        }
        DEBUG_PRINT("d3d11.dll module handle: %p\n", reinterpret_cast<void*>(moduleHandle));

        // MODULEINFO info;
        // ZeroMemory(&info, sizeof(info));
        // if (!GetModuleInformation(gameHandle, moduleHandle, &info, sizeof(MODULEINFO)))
        // {
        //     DEBUG_PRINT("Error: Failed to get module information. LastError: %lu\n", GetLastError());
        //     return false;
        // }
        // DEBUG_PRINT("d3d11.dll module size: %lu\n", info.SizeOfImage);

        const auto searchStart = moduleHandle + 0x100000;
        // const auto searchLength = static_cast<size_t>(moduleHandle + info.SizeOfImage - searchStart - 0x1000);
        const auto searchLength = moduleHandle + 0x200000 - searchStart;
        auto functionAddress = reinterpret_cast<LPVOID>(IgroWidgets::FindPattern(searchStart, searchLength,
            reinterpret_cast<const uint8_t*>("\x48\x89\x5C\x24\x08\x48\x89\x74\x24\x10\x57\x48\x83\xEC\x00\x41\x8B\xF0\x48\x8B\xDA\x48\x8D\xB9\x00\x00\x00\xFF\x48\x8B\xCF\xE8\x00\x00\x00\x00\x84\xC0\x74\x00\x48\x85\xDB\x74\x00\x8B\x8B\x00\x00\x00\x00\x8B\x93"),
            "xxxxxxxxxxxxxx?xxxxxxxxx???xxxxx????xxx?xxxx?xx????xx"));
        if (!functionAddress)
        {
            DEBUG_PRINT("Couldn't find DrawIndexedInstancedIndirect with Windows 11 pattern, trying with Windows 10...\n");
            functionAddress = reinterpret_cast<LPVOID>(IgroWidgets::FindPattern(searchStart, searchLength,
                reinterpret_cast<const uint8_t*>("\x48\x83\xEC\x00\x4C\x8B\xD1\x48\x85\xD2\x74\x00\x8B\x82"),
                "xxx?xxxxxxx?xx"));
        }

        DrawIndexedInstancedIndirectAddress = functionAddress;
        if (!DrawIndexedInstancedIndirectAddress)
        {
            DEBUG_PRINT("Error: Couldn't find DrawIndexedInstancedIndirect function!\n");
            return false;
        }
        DEBUG_PRINT("Found DrawIndexedInstancedIndirect function at: %p\n", DrawIndexedInstancedIndirectAddress);

        mhStatus = MH_CreateHook(DrawIndexedInstancedIndirectAddress, DetourDrawIndexedInstancedIndirect, reinterpret_cast<LPVOID*>(&OriginalDrawIndexedInstancedIndirect));
        if (mhStatus != MH_OK)
        {
            DEBUG_PRINT("Error: Failed MH_CreateHook for DrawIndexedInstancedIndirect! Status: %d\n", mhStatus);
            return false;
        }

        mhStatus = MH_EnableHook(DrawIndexedInstancedIndirectAddress);
        if (mhStatus != MH_OK)
        {
            DEBUG_PRINT("Error: Failed MH_EnableHook for DrawIndexedInstancedIndirect! Status: %d\n", mhStatus);
            return false;
        }

        DEBUG_PRINT("DrawIndexedInstancedIndirect hook enabled\n");
        return true;
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

    void CreateDepthStencilStates(ID3D11Device* device)
    {
        if (device && !wallHackDSS)
        {
            D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            depthStencilDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
            depthStencilDesc.StencilEnable = FALSE;
            depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
            depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
            // Stencil operations if pixel is front-facing
            depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            // Stencil operations if pixel is back-facing
            depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
            depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
            device->CreateDepthStencilState(&depthStencilDesc, &wallHackDSS);
        }
    }

    bool DrawChams(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs, ID3D11ShaderResourceView* SRVs[], ID3D11ShaderResourceView* srvWhenInvisible, ID3D11ShaderResourceView* srvWhenVisible)
    {
        if (pContext && pBufferForArgs && SRVs)
        {
            const auto srvCount = CountSRVs(SRVs);
            if (srvWhenInvisible)
            {
                ID3D11DepthStencilState* originalDSS = nullptr;
                UINT originalStencilRef = 0;
                pContext->OMGetDepthStencilState(&originalDSS, &originalStencilRef);

                for (auto i = 0; i < srvCount; i++)
                {
                    pContext->PSSetShaderResources(i, 1, &srvWhenInvisible);
                }
                pContext->OMSetDepthStencilState(wallHackDSS, originalStencilRef);
                OriginalDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
                
                pContext->OMSetDepthStencilState(originalDSS, originalStencilRef);
                SafeRelease(originalDSS);
            }

            if (srvWhenVisible)
            {
                for (auto i = 0; i < srvCount; i++)
                {
                    pContext->PSSetShaderResources(i, 1, &srvWhenVisible);
                }
                OriginalDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
                return true;
            }
        }

        return false;
    }

    void __stdcall DetourDrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs)
    {
#ifdef _DEBUG
        static auto printOnce = true;
        if (printOnce)
        {
            DEBUG_PRINT("DetourDrawIndexedInstancedIndirect was called, confirmation that the right function was hooked\n");
            printOnce = false;
        }
#endif

        ID3D11Device* device;
        pContext->GetDevice(&device);
        if (!device)
        {
            return OriginalDrawIndexedInstancedIndirect(pContext, pBufferForArgs, AlignedByteOffsetForArgs);
        }

        CreateColorSRVs(device);
        CreateDepthStencilStates(device);

        ID3D11Buffer* veBuffer = nullptr;
        UINT Stride;
        UINT veBufferOffset;
        pContext->IAGetVertexBuffers(0, 1, &veBuffer, &Stride, &veBufferOffset);
        SafeRelease(veBuffer);

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
            || Models::Skin1.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            || Models::Skin2.IsModel(Stride, compVscWidth, compPscWidth, originalSRVs)
            )
        {
            if (DrawChams(pContext, pBufferForArgs, AlignedByteOffsetForArgs, originalSRVs, redSRV, greenSRV))
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

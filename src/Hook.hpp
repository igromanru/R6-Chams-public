#pragma once

#include <d3d11.h>
#include "RGBA.hpp"


typedef void(__stdcall* D3D11DrawIndexedInstancedIndirect) (ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);

namespace Hook
{
    inline D3D11DrawIndexedInstancedIndirect OriginalDrawIndexedInstancedIndirect = nullptr;
    inline LPVOID DrawIndexedInstancedIndirectAddress = nullptr;

    inline ID3D11DepthStencilState* wallHackDSS = nullptr;

    inline ID3D11ShaderResourceView* redSRV = nullptr;
    inline ID3D11ShaderResourceView* greenSRV = nullptr;

    DWORD WINAPI Init();
    void Unload();

    void CreateColorSRV(ID3D11Device* device, const RGBA& color, ID3D11ShaderResourceView** shaderResourceView);
    void CreateColorSRVs(ID3D11Device* device);

    void CreateDepthStencilStates(ID3D11Device* device, ID3D11DepthStencilState* originalDSS);

    bool DrawChams(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs, ID3D11ShaderResourceView* SRVs[],
        ID3D11ShaderResourceView* srvWhenInvisible, ID3D11ShaderResourceView* srvWhenVisible, ID3D11DepthStencilState* visibleDSS);

    void __stdcall DetourDrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);
}

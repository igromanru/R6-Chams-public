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

    void CreateDepthStencilStates(ID3D11Device* device);

    bool DrawChams(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs, ID3D11ShaderResourceView* SRVs[],
        ID3D11ShaderResourceView* srvWhenInvisible, ID3D11ShaderResourceView* srvWhenVisible);

    void __stdcall DetourDrawIndexedInstancedIndirect(ID3D11DeviceContext* pContext, ID3D11Buffer* pBufferForArgs, UINT AlignedByteOffsetForArgs);
}

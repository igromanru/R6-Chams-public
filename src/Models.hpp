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

#pragma warning(disable:4267)

namespace Models
{
    constexpr bool MatchSRVFormat(ID3D11ShaderResourceView* srv, int targetFormat)
    {
        if (srv)
        {
            if (targetFormat > -1)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC desc;
                srv->GetDesc(&desc);
                return desc.Format == targetFormat;
            }
            return true;
        }
        return false;
    }

    class Model
    {
    public:
        int Stride;
        int VscWidth;
        int PscWidth;
        int SrvFormat;
        int SrvIndex;

        Model(int stride, int vscWidth, int pscWidth, int psSrvFormat = -1, int srvIndex = -1)
            : Stride(stride), VscWidth(vscWidth), PscWidth(pscWidth), SrvFormat(psSrvFormat), SrvIndex(srvIndex)
        {
        }

        bool IsModel(UINT stride, UINT vscWidth, UINT pscWidth, ID3D11ShaderResourceView* SRVs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT]) const
        {
            auto result = (Stride < 0 || stride == static_cast<unsigned>(Stride)) && (VscWidth < 0 || vscWidth == static_cast<unsigned>(VscWidth)) && (PscWidth < 0 || pscWidth == static_cast<unsigned>(PscWidth));
            if (result)
            {
                if (SRVs && (SrvIndex >= 0 || SrvFormat >= 0))
                {
                    if (SrvIndex < 0)
                    {
                        result = false;
                        for (auto i = 0; i < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT; i++)
                        {
                            if (!SRVs[i])
                            {
                                break;
                            }

                            if (MatchSRVFormat(SRVs[i], SrvFormat))
                            {
                                result = true;
                                break;
                            }
                        }
                    }
                    else
                    {
                        result = SrvIndex < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT && MatchSRVFormat(SRVs[SrvIndex], SrvFormat);
                    }
                }
            }

            return result;
        }
    };

    inline Model Skin1(8, 62, 4, 77, 2);
    inline Model Skin2(8, 62, 4, 83, 2);
    inline Model Body1(8, 62, 12, 83);
    inline Model Body2(8, 62, 14, 83);

    inline Model DroneWorldOverlay(8, 49, 3);
    inline Model WorldOverlay1(8, 49, 4);
    inline Model WorldOverlay2(8, 49, 12);
    inline Model WorldOverlay3(8, 49, 14);
    inline Model WorldOverlay4(8, 49, 49);

    inline Model Scope(8, 49, 49, 77, 2); // Scope textures
}
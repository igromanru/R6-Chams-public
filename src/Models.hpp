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
                if (SRVs && SrvIndex >= 0 && SrvFormat >= 0)
                {
                    result = SrvIndex < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT && MatchSRVFormat(SRVs[SrvIndex], SrvFormat);
                }
            }

            return result;
        }
    };

    inline Model Skin(8, 62, 4);
    inline Model Body1(8, 62, 12);
    inline Model Body2(8, 62, 14);

    inline Model DroneWorldOverlay(8, 49, 3);
    inline Model WorldOverlay1(8, 49, 4);
    inline Model WorldOverlay2(8, 49, 12);
    inline Model WorldOverlay3(8, 49, 14);
    inline Model WorldOverlay4(8, 49, 49);

    inline Model Scope(8, 49, 49, 77, 2); // Scope textures
}
#pragma once

#include <d2d1.h>

class ImageRenderer
{
public:
    ImageRenderer();
    virtual ~ImageRenderer();

    HRESULT Initialize(HWND hwnd, ID2D1Factory* pD2DFactory, int sourceWidth, int sourceHeight, int sourceStride);
    HRESULT Draw(BYTE* pImage, unsigned long cbImage);

private:
    HWND                     m_hWnd;
    UINT                     m_sourceHeight;
    UINT                     m_sourceWidth;
    LONG                     m_sourceStride;
    ID2D1Factory*            m_pD2DFactory;
    ID2D1HwndRenderTarget*   m_pRenderTarget;
    ID2D1Bitmap*             m_pBitmap;

    HRESULT EnsureResources();
    void DiscardResources();
}; 
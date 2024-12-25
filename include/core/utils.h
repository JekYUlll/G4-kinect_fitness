#ifndef KF_CORE_UTILS_H
#define KF_CORE_UTILS_H

#define NOMINMAX
#include <Windows.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

namespace kfc {

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
    if (pInterfaceToRelease)
    {
        pInterfaceToRelease->Release();
        pInterfaceToRelease = nullptr;
    }
}

} // namespace kfc

#endif // KF_CORE_UTILS_H 
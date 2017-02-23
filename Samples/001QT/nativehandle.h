#ifndef NATIVEHANDLE
#define NATIVEHANDLE

#if defined(_WIN32)
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#endif

struct NativeHandle
{
    #if defined(_WIN32)
    HINSTANCE hInst;
    HWND hWnd;
    #endif
};


#endif // NATIVEHANDLE


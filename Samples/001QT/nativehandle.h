#ifndef NATIVEHANDLE
#define NATIVEHANDLE

#if defined(_WIN32)
#include <windows.h>
#endif

struct NativeHandle
{
    #if defined(_WIN32)
    HINSTANCE hInst;
    HWND hWnd;
    #endif
};


#endif // NATIVEHANDLE


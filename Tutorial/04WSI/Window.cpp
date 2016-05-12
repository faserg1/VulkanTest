#include "Window.h"
#include <cstdio>

Window::Window()
{
}

Window::~Window()
{
}

void Window::SetFPS(float fps)
{
	char txt[80];
    sprintf(txt, "FPS: %f", fps);
    #if defined(_WIN32)
    SetWindowTextA(this->hWnd, txt);
    #endif
}

void Window::GetWindowSize(unsigned int &width, unsigned int &height)
{
	#if defined(_WIN32)
	RECT rect;
    GetClientRect(this->hWnd, &rect);
    width = rect.right - rect.left;
    height = rect.bottom - rect.top;
	#endif // defined
}

#if defined(_WIN32)
HWND Window::GetHandle()
{
	return hWnd;
}
#endif

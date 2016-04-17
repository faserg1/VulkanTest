#ifndef WINDOW_H
#define WINDOW_H

#if defined(_WIN32)
#define _WIN32_WINNT 0x0601 //Windows 7
#include <windows.h>
#endif


class Window
{
	#if defined(_WIN32)
	HWND hWnd;
	#endif
	friend class Application;
public:
	Window();
	~Window();
	
	#if defined(_WIN32)
	HWND GetHandle();
	#endif
};

#endif // WINDOW_H

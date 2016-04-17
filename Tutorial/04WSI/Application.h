#ifndef APPLICATION_H
#define APPLICATION_H

#if defined(_WIN32)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601 //Windows 7
#include <windows.h>
#endif
#include <string>
#include "Window.h"

class Application
{
	#if defined(_WIN32)
	static LRESULT CALLBACK AppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	#endif
	std::string app_name;
public:
	Application(std::string app_name);
	~Application();
	bool InitApplication();
	bool DeinitApplication();
	Window *CreateAppWindow(uint32_t sizeX, uint32_t sizeY);
	void DestroyAppWindow(Window **w);
	bool Loop();
	std::string GetAppName();
	#if defined(_WIN32)
	HINSTANCE GetHandle();
	#endif
};

#endif // APPLICATION_H

#include "Application.h"
#include <stdexcept>
#include <cstdlib>
#include "common.h"

/* Конечно, можно было AppProc настроить на UserData (GWLP_USERDATA), когда прилетает сообщение, но сделаем проще:
 * через глобаьные переменные. А также, я их скрыл из видимости линкощика, поместив их в безымянное пространство имён.
*/

namespace
{
	static uint8_t app_count = 0;
	Application *app = NULL;
	#if defined(_WIN32)
	HINSTANCE hInst = NULL;
	MSG msg;
	#endif
}

Application::Application(std::string app_name)
{
	if (app_count > 0)
		throw std::runtime_error("Creating too much applications (more than one)!");
	else
	{
		app_count++;
		app = this;
		this->app_name = app_name;
	}
}

Application::~Application()
{
	if (app_count > 0)
		app_count--;
	app = NULL;
}

bool Application::InitApplication()
{
	bool result = false;
	#if defined(_WIN32)
	hInst = GetModuleHandle(0); //
	WNDCLASSEXW wc;
	ZM(wc);
	wc.cbSize = sizeof(wc);
	wc.hInstance = hInst;
	wc.lpfnWndProc = AppProc;
	wc.lpszClassName = L"cls_vk_tutorial";
	wc.style = CS_DROPSHADOW | CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
	if (RegisterClassExW(&wc))
		result = true;
	ZM(msg);
	#endif
	return result;
}

bool Application::DeinitApplication()
{
	bool result = false;
	#if defined(_WIN32)
	if (UnregisterClassW(L"cls_vk_tutorial", hInst))
		result = true;
	#endif
	return result;
}

bool Application::Loop()
{
	bool result = false;
	#if defined(_WIN32)
	if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			result = true;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	#endif
	return result;
}

std::wstring tows(std::string str)
{
	size_t len = str.size();
	wchar_t *buffer = new wchar_t[len+1];
	mbstowcs(buffer, str.data(), len+1);
	std::wstring wstr(buffer, len);
	delete [] buffer;
	return wstr;
}

Window *Application::CreateAppWindow(uint32_t sizeX, uint32_t sizeY)
{
	#if defined(_WIN32)
	//Стили окна
	DWORD style, ex_style;
	style = WS_OVERLAPPEDWINDOW;
	ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	RECT wr = { 0, 0, LONG(sizeX), LONG(sizeY) };
	//Добавим размеры границ, чтобы sizeX, sizeY стали размерами дочерней области (области рисования, а не всего окна)
	AdjustWindowRectEx(&wr, style, FALSE, ex_style);
	HWND temp = CreateWindowExW(ex_style,
		L"cls_vk_tutorial", tows(this->app_name).c_str(),
		style,
		CW_USEDEFAULT, CW_USEDEFAULT,
		wr.right - wr.left,
		wr.bottom - wr.top,
		NULL, NULL, hInst, NULL);
	if (temp)
	{
		ShowWindow(temp, SW_SHOW);
		SetForegroundWindow(temp);
		SetFocus(temp);
		Window *w = new Window;
		w->hWnd = temp;
		return w;
	}
	#endif
	return NULL;
}

void Application::DestroyAppWindow(Window **w)
{
	delete (*w);
	*w = NULL;
}


std::string Application::GetAppName()
{
	return app_name;
}

HINSTANCE Application::GetHandle()
{
	return hInst;
}

#if defined(_WIN32)
LRESULT CALLBACK Application::AppProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
        default:
			break;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}
#endif

#include "Logger.h"
#include "common.h"
#include "Render.h"
#include <stdexcept>
#include <iostream>
#if defined(_WIN32)
#include <windows.h>
#endif

Logger::Logger()
{
	callback = VK_NULL_HANDLE;
	file.open("VulkanLog.txt", std::fstream::in | std::fstream::out | std::fstream::trunc);
	if (!file.is_open())
		throw std::runtime_error("Cannot open log file!");
}

Logger::~Logger()
{
	file.close();
}

void Logger::Init(Render *r)
{
	if (r->IsDebugEnabled())
	{
		VkInstance instance = r->GetInstance();
		//Получение функций
		fvkCreateDebugReportCallbackEXT =
			(PFN_vkCreateDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT =
			(PFN_vkDestroyDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (!(fvkCreateDebugReportCallbackEXT || fvkDestroyDebugReportCallbackEXT))
			throw std::runtime_error("Cannot fetch debug_report functions!");
	}
}

//Debug Report Callback
VKAPI_ATTR VkBool32 VKAPI_CALL Logger::DebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t object,
	size_t location,
	int32_t messageCode,
	const char *pLayerPrefix,
	const char *pMessage,
	void *pUserData)
{
	if (pUserData)
		return ((Logger *) pUserData)->Log(flags, pLayerPrefix, pMessage);
	return false;
}

namespace
{


	enum Color
	{
		COLOR_RED, COLOR_YELLOW, COLOR_WHITE, COLOR_PINK, COLOR_GREEN
	};

	void SetDefaultColor()
	{
		#if defined(_WIN32)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
		#endif
	}

	void SetColor(Color clr)
	{
		#if defined(_WIN32)
		WORD t_color;
		switch (clr)
		{
			case COLOR_RED:
				t_color = FOREGROUND_RED | FOREGROUND_INTENSITY;
				break;
			case COLOR_YELLOW:
				t_color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			case COLOR_WHITE:
				t_color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY;
				break;
			case COLOR_PINK:
				t_color = FOREGROUND_RED | FOREGROUND_BLUE;
				break;
			case COLOR_GREEN:
				t_color = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
				break;
			default:
				t_color = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
		}
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), t_color);
		#endif
	}

}

bool Logger::Log(VkDebugReportFlagsEXT flags, const char *pLayerPrefix, const char *pMessage)
{
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		SetColor(COLOR_WHITE);
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		SetColor(COLOR_YELLOW);
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		SetColor(COLOR_PINK);
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		SetColor(COLOR_RED);
	if (!(flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT))
		std::cout << "[" << pLayerPrefix << "] " << pMessage << std::endl;
	file << "[" << pLayerPrefix << "] " << pMessage << std::endl;
	SetDefaultColor();
	return false;
}

void Logger::UserLog(const char *tag, const char *msg)
{
	SetColor(COLOR_GREEN);
	std::cout << "[" << tag << "] " << msg << std::endl;
	file << "[" << tag << "] " << msg << std::endl;
	SetDefaultColor();
}

void Logger::UserError(const char *tag, const char *msg)
{
	SetColor(COLOR_RED);
	std::cout << "[" << tag << "] " << msg << std::endl;
	file << "[" << tag << "] " << msg << std::endl;
	SetDefaultColor();
}

bool Logger::AttachLogger(Render *r)
{
	if (fvkCreateDebugReportCallbackEXT)
	{
		VkInstance instance = r->GetInstance();
		VkDebugReportCallbackCreateInfoEXT debug_report_callback_info;
		ZM(debug_report_callback_info);
		debug_report_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		debug_report_callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
			VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
		debug_report_callback_info.pfnCallback = DebugReportCallback;
		debug_report_callback_info.pUserData = this;

		if (fvkCreateDebugReportCallbackEXT(instance, &debug_report_callback_info,
			NULL, &callback) != VK_SUCCESS)
			return false;
		return true;
	}
	return false;
}

void Logger::DetachLogger(Render *r)
{
	if (fvkDestroyDebugReportCallbackEXT)
	{
		VkInstance instance = r->GetInstance();
		fvkDestroyDebugReportCallbackEXT(instance, callback, NULL);
	}
}

const char *Logger::GetRequiredExtension()
{
	return VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
}

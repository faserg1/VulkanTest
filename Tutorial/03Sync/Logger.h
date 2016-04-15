#ifndef LOGGER_H
#define LOGGER_H

#include <vulkan/vulkan.h> // Vulkan API
#include <fstream>

class Logger
{
	//функция Callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL
		DebugReportCallback(VkDebugReportFlagsEXT,VkDebugReportObjectTypeEXT,
		uint64_t,size_t,int32_t,const char*,const char*,void*);
	
	bool Log(VkDebugReportFlagsEXT flags, const char *pLayerPrefix, const char *pMessage);
	std::fstream file; //файл, в который также выводится лог
	
	//структура с данными о callback функции
	VkDebugReportCallbackEXT callback;
	
	//Необходимые функции
	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT;
public:
	Logger();
	~Logger();
	
	void Init(VkInstance instance); //инициализация отладки (получение функций)
	
	bool AttachLogger(VkInstance instance); //прикрепление отладки
	void DetachLogger(VkInstance instance); //отсоединение отладки
	static const char *GetRequiredExtension(); //получение имени необходимого расширения
};

#endif // LOGGER_H

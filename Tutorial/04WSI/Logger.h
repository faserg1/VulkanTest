#ifndef LOGGER_H
#define LOGGER_H

#include <vulkan/vulkan.h> // Vulkan API
#include <fstream>

class Render;

class Logger
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL
		DebugReportCallback(VkDebugReportFlagsEXT,VkDebugReportObjectTypeEXT,
		uint64_t,size_t,int32_t,const char*,const char*,void*);

	bool Log(VkDebugReportFlagsEXT flags, const char *pLayerPrefix, const char *pMessage);
	std::fstream file;

	VkDebugReportCallbackEXT callback;
	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT;
public:
	Logger();
	~Logger();

	void Init(Render *r);
	void UserLog(const char *tag, const char *msg);
	void UserError(const char *tag, const char *msg);

	bool AttachLogger(Render *r);
	void DetachLogger(Render *r);
	static const char *GetRequiredExtension();
};

#endif // LOGGER_H

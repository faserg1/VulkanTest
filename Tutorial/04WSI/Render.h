#ifndef RENDER_H
#define RENDER_H

//Прежду чем добавить Vulkan, ы должны настроить его заголовки под определённую платформу.
#include "platform.h"
#include <vulkan/vulkan.h> // Vulkan API
#include <vector>
#include <string>

class Application;
class Window;

class Render
{
	VkInstance instance;
	VkDevice device;

	uint32_t graphic_family_index; //семейство с графикой
	uint32_t present_family_index; //семейство, которое поддерживает surface платформы
	VkPhysicalDevice gpu;

	bool debug_enabled;

	struct
	{
		bool extensions_enabled;
		VkSurfaceKHR surface;
		VkSurfaceCapabilitiesKHR surface_capabilities;
		std::vector<VkSurfaceFormatKHR> available_formats;
		std::vector<VkPresentModeKHR> present_modes;
	} surface_data;

	struct
	{
		bool extensions_enabled;
		bool gpu_support;
		VkSwapchainKHR swapchain;
	} swapchain_data;

	std::vector<const char *> instance_layers;
	std::vector<const char *> instance_extensions;
	std::vector<const char *> device_layers;
	std::vector<const char *> device_extensions;
public:
	Render();
	~Render();

	void AddInstanceLayer(const char *name);
	void AddInstanceExtension(const char *name);
	void AddDeviceLayer(const char *name);
	void AddDeviceExtension(const char *name);

	bool RemoveInstanceLayer(const char *name);
	bool RemoveInstanceExtension(const char *name);
	bool RemoveDeviceLayer(const char *name);
	bool RemoveDeviceExtension(const char *name);

	bool CheckInstanceLayer(const char *name);
	bool CheckInstanceExtension(const char *layer_name, const char *name);
	bool CheckDeviceLayer(const char *name);
	bool CheckDeviceExtension(const char *layer_name, const char *name);

	bool CreateInstance(std::string app_name);
	bool CreateDevice();
	void DestroyInstance();
	void DestroyDevice();

	bool PrepareGPU();
	bool CreateSurface(Application *app, Window *w);
	void DestroySurface();

	bool PrepareSwapchain();
	bool CreateSwapchain(Window *w);
	void DestroySwapchain();

	void EnableDebug(bool enable);
	inline bool IsDebugEnabled() {return debug_enabled;}

	//возвращают состояниие (включены ли)
	bool EnableSurface(bool enable);
	bool EnableSwapchains(bool enable);

	const VkInstance GetInstance() const;
	const VkDevice GetDevice() const;
	const VkSwapchainKHR GetSwapchain() const;

	const VkQueue GetQueue(uint32_t index) const;
	inline int GetFamIndex(bool present) const
		{return (present ? present_family_index : graphic_family_index);}

	VkCommandPool CreateCommandPool(bool reset, bool transient);
	void DestroyCommandPool(VkCommandPool pool);
};

#endif // RENDER_H

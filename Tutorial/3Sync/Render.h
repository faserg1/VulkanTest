#ifndef RENDER_H
#define RENDER_H

#include <vulkan/vulkan.h> // Vulkan API
#include <vector>


class Render
{
	VkInstance instance;
	VkDevice device;
	
	uint32_t family_index;
	
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
	
	bool CreateInstance();
	bool CreateDevice();
	void DestroyInstance();
	void DestroyDevice();
	
	
	void EnableDebug(bool enable);
	
	const VkInstance GetInstance() const;
	const VkDevice GetDevice() const;
	
	const VkQueue GetQueue(uint32_t index) const;
	
	VkCommandPool CreateCommandPool(bool reset, bool transient);
	void DestroyCommandPool(VkCommandPool pool);
};

#endif // RENDER_H

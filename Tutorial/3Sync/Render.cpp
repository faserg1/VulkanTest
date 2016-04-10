#include "Render.h"
#include "common.h"
#include "Logger.h"

Render::Render()
{
	instance = VK_NULL_HANDLE;
	device = VK_NULL_HANDLE;
	
	family_index = (uint32_t) -1;
}

Render::~Render()
{
}

void Render::AddInstanceLayer(const char *name)
{
	instance_layers.push_back(name);
}

void Render::AddInstanceExtension(const char *name)
{
	instance_extensions.push_back(name);
}

void Render::AddDeviceLayer(const char *name)
{
	device_layers.push_back(name);
}

void Render::AddDeviceExtension(const char *name)
{
	device_extensions.push_back(name);
}

#define VPCCI std::vector<const char *>::iterator

bool Render::RemoveInstanceLayer(const char *name)
{
	for (VPCCI i = instance_layers.begin(); i != instance_layers.end(); i++)
	{
		if (strcmp(*i, name) == 0)
		{
			instance_layers.erase(i);
			return true;
		}
	}
	return false;
}

bool Render::RemoveInstanceExtension(const char *name)
{
	for (VPCCI i = instance_extensions.begin(); i != instance_extensions.end(); i++)
	{
		if (strcmp(*i, name) == 0)
		{
			instance_extensions.erase(i);
			return true;
		}
	}
	return false;
}

bool Render::RemoveDeviceLayer(const char *name)
{
	for (VPCCI i = device_layers.begin(); i != device_layers.end(); i++)
	{
		if (strcmp(*i, name) == 0)
		{
			device_layers.erase(i);
			return true;
		}
	}
	return false;
}

bool Render::RemoveDeviceExtension(const char *name)
{
	for (VPCCI i = device_extensions.begin(); i != device_extensions.end(); i++)
	{
		if (strcmp(*i, name) == 0)
		{
			device_extensions.erase(i);
			return true;
		}
	}
	return false;
}

const VkInstance Render::GetInstance() const
{
	return instance;
}

const VkDevice Render::GetDevice() const
{
	return device;
}

void Render::EnableDebug(bool enable)
{
	if (enable)
		AddInstanceExtension(Logger::GetRequiredExtension());
	else
		RemoveInstanceExtension(Logger::GetRequiredExtension());
}

bool Render::CreateInstance()
{
	VkApplicationInfo app_info; 
	ZM(app_info);
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan Tutorian. Sync. © GrWolf.";
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 8);
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	
	VkInstanceCreateInfo instance_info;
	ZM(instance_info);
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	
	instance_info.enabledLayerCount = instance_layers.size();
	instance_info.ppEnabledLayerNames = instance_layers.data();
	
	instance_info.enabledExtensionCount = instance_extensions.size();
	instance_info.ppEnabledExtensionNames = instance_extensions.data();
	
	if (vkCreateInstance(&instance_info, NULL, &instance) != VK_SUCCESS)
		return false;
	return true;
}

bool Render::CreateDevice()
{
	//Поиск GPU
	std::vector<VkPhysicalDevice> gpu_list;
	uint32_t gpu_count;
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		return false;
	}
	gpu_list.resize(gpu_count);
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
	{
		return false;
	}
	VkPhysicalDevice gpu = gpu_list[0];
	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, VK_NULL_HANDLE);
	std::vector<VkQueueFamilyProperties> family_properties_list(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_properties_list.data());
	uint32_t valid_family_index = (uint32_t) -1;
	for (uint32_t i = 0; i < family_count; i++)
	{
		VkQueueFamilyProperties &properties = family_properties_list[i];
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (valid_family_index == (uint32_t) -1)
				valid_family_index = i;
		}
	}
	if (valid_family_index == (uint32_t) -1)
	{
		return false;
	}
	
	//Индекс нам понадобится для получения очередей.
	family_index = valid_family_index;
	
	//Настройка очередей
	VkDeviceQueueCreateInfo device_queue_info;
	ZM(device_queue_info);
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	float device_queue_priority[] = {1.0f};
	device_queue_info.queueCount = 1; 
	device_queue_info.queueFamilyIndex = valid_family_index; 
	device_queue_info.pQueuePriorities = device_queue_priority;
	
	//Настройка девайса
	VkDeviceCreateInfo device_info; 
    ZM(device_info);
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; 
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &device_queue_info;
	device_info.enabledLayerCount = device_layers.size();
	device_info.ppEnabledLayerNames = device_layers.data();
	device_info.enabledExtensionCount = device_extensions.size();
	device_info.ppEnabledExtensionNames = device_extensions.data();
	
	//Создание девайса
	if (vkCreateDevice(gpu, &device_info, NULL, &device) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

void Render::DestroyInstance()
{
	vkDestroyInstance(instance, NULL);
}

void Render::DestroyDevice()
{
	vkDestroyDevice(device, NULL);
}

const VkQueue Render::GetQueue(uint32_t index) const
{
	VkQueue queue;
	vkGetDeviceQueue(device, family_index, 0, &queue);
	return queue;
}

VkCommandPool Render::CreateCommandPool(bool reset, bool transient)
{
	VkCommandPoolCreateInfo pool_create_info;
	ZM(pool_create_info);
	pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.queueFamilyIndex = family_index;
	pool_create_info.flags = 
		(reset ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : 0) |
		(transient ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : 0);
	VkCommandPool pool = VK_NULL_HANDLE;
	vkCreateCommandPool(device, &pool_create_info, NULL, &pool);
	return pool;
}

void Render::DestroyCommandPool(VkCommandPool pool)
{
	vkDestroyCommandPool(device, pool, NULL);
}
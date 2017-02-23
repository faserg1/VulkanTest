#include "renderer.h"
#include "nativehandle.h"
#include <cstring>
#include <QDebug>

#include <vulkan/vulkan.h>

class RendererImp
{
	VkInstance vk_inst;
    VkDevice vk_dev;

    uint32_t gpu_count;
    std::vector<VkPhysicalDevice> gpu_list;

	VkDebugReportCallbackEXT callback;

	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT;

	struct
	{
		VkSurfaceKHR surface;
		VkSurfaceCapabilitiesKHR surface_capabilities;
		std::vector<VkSurfaceFormatKHR> available_formats;
		std::vector<VkPresentModeKHR> present_modes;
	} surface_data;

	VkSwapchainKHR swapchain;

	static VKAPI_ATTR VkBool32 VKAPI_CALL
		DebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT,
		uint64_t, size_t, int32_t,
		const char *pLayerPrefix, const char *pMessage, void *userData)
	{
		return static_cast<RendererImp*>(userData)->Log(flags, pLayerPrefix, pMessage);
	}

	bool Log(VkDebugReportFlagsEXT flags, const char *pLayerPrefix, const char *pMessage)
	{
		auto msgr = qInfo();
		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
			msgr = qDebug();
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
			msgr = qWarning();
		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
			msgr = qWarning();
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
			msgr = qCritical();
		msgr << "[" << pLayerPrefix << "] " << pMessage;
		return false;
	}

	std::vector<const char *> instance_layers;
	std::vector<const char *> instance_extensions;
	std::vector<const char *> device_layers;
	std::vector<const char *> device_extensions;

public:
	RendererImp()
	{
		vk_inst = VK_NULL_HANDLE;
		vk_dev = VK_NULL_HANDLE;
		swapchain = VK_NULL_HANDLE;
		gpu_count = 0;
	}

	void AddInstanceLayer(const char *name)
	{
		instance_layers.push_back(name);
	}

	void AddInstanceExtension(const char *name)
	{
		instance_extensions.push_back(name);
	}

	void AddDeviceLayer(const char *name)
	{
		device_layers.push_back(name);
	}

	void AddDeviceExtension(const char *name)
	{
		device_extensions.push_back(name);
	}

	bool RemoveInstanceLayer(const char *name)
	{
		for (auto i = instance_layers.begin(); i != instance_layers.end(); i++)
		{
			if (strcmp(*i, name) == 0)
			{
				instance_layers.erase(i);
				return true;
			}
		}
		return false;
	}

	bool RemoveInstanceExtension(const char *name)
	{
		for (auto i = instance_extensions.begin(); i != instance_extensions.end(); i++)
		{
			if (strcmp(*i, name) == 0)
			{
				instance_extensions.erase(i);
				return true;
			}
		}
		return false;
	}

	bool RemoveDeviceLayer(const char *name)
	{
		for (auto i = device_layers.begin(); i != device_layers.end(); i++)
		{
			if (strcmp(*i, name) == 0)
			{
				device_layers.erase(i);
				return true;
			}
		}
		return false;
	}

	bool RemoveDeviceExtension(const char *name)
	{
		for (auto i = device_extensions.begin(); i != device_extensions.end(); i++)
		{
			if (strcmp(*i, name) == 0)
			{
				device_extensions.erase(i);
				return true;
			}
		}
		return false;
	}

	bool CheckInstanceLayer(const char *name)
	{
		uint32_t available_instance_layer_count = 0;
		if (vkEnumerateInstanceLayerProperties(&available_instance_layer_count, VK_NULL_HANDLE) != VK_SUCCESS)
			return false;
		std::vector<VkLayerProperties> available_instance_layers(available_instance_layer_count);
		if (vkEnumerateInstanceLayerProperties(&available_instance_layer_count, available_instance_layers.data()) != VK_SUCCESS)
			return false;
		for (size_t i = 0; i < available_instance_layers.size(); i++)
		{
			if (strcmp(available_instance_layers[i].layerName, name) == 0)
				return true;
		}
		return false;
	}

	bool CheckInstanceExtension(const char *layer_name, const char *name)
	{
		uint32_t available_instance_extension_count = 0;
		if (vkEnumerateInstanceExtensionProperties
			(layer_name, &available_instance_extension_count, VK_NULL_HANDLE) != VK_SUCCESS)
			return false;
		std::vector<VkExtensionProperties> available_instance_extensions(available_instance_extension_count);
		if (vkEnumerateInstanceExtensionProperties
			(layer_name, &available_instance_extension_count, available_instance_extensions.data()) != VK_SUCCESS)
			return false;
		for (size_t i = 0; i < available_instance_extensions.size(); i++)
		{
			if (strcmp(available_instance_extensions[i].extensionName, name) == 0)
				return true;
		}
		return false;
	}

	bool CheckDeviceLayer(size_t gpu_num, const char *name)
	{
		VkPhysicalDevice gpu = gpu_list[gpu_num];
		uint32_t available_device_layer_count = 0;
		if (vkEnumerateDeviceLayerProperties(gpu, &available_device_layer_count, VK_NULL_HANDLE) != VK_SUCCESS)
			return false;
		std::vector<VkLayerProperties> available_device_layers(available_device_layer_count);
		if (vkEnumerateDeviceLayerProperties(gpu, &available_device_layer_count, available_device_layers.data()) != VK_SUCCESS)
			return false;
		for (size_t i = 0; i < available_device_layers.size(); i++)
		{
			if (strcmp(available_device_layers[i].layerName, name) == 0)
				return true;
		}
		return false;
	}

	bool CheckDeviceExtension(size_t gpu_num, const char *layer_name, const char *name)
	{
		VkPhysicalDevice gpu = gpu_list[gpu_num];
		uint32_t available_device_extension_count = 0;
		if (vkEnumerateDeviceExtensionProperties
			(gpu, layer_name, &available_device_extension_count, VK_NULL_HANDLE) != VK_SUCCESS)
			return false;
		std::vector<VkExtensionProperties> available_device_extensions(available_device_extension_count);
		if (vkEnumerateDeviceExtensionProperties
			(gpu, layer_name, &available_device_extension_count, available_device_extensions.data()) != VK_SUCCESS)
			return false;
		for (size_t i = 0; i < available_device_extensions.size(); i++)
		{
			if (strcmp(available_device_extensions[i].extensionName, name) == 0)
				return true;
		}
		return false;
	}

	bool attachLogging()
	{
		if (fvkCreateDebugReportCallbackEXT)
		{
			VkDebugReportCallbackCreateInfoEXT debug_report_callback_info;
			memset(&debug_report_callback_info, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
			debug_report_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			debug_report_callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
				VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
			debug_report_callback_info.pfnCallback = DebugReportCallback;
			debug_report_callback_info.pUserData = this;

			if (fvkCreateDebugReportCallbackEXT(vk_inst, &debug_report_callback_info,
				NULL, &callback) != VK_SUCCESS)
				return false;
			return true;
		}
		else
			return false;
	}

	bool detachLoggin()
	{
		if (fvkDestroyDebugReportCallbackEXT)
		{
			fvkDestroyDebugReportCallbackEXT(vk_inst, callback, NULL);
			return true;
		}
		return false;
	}

	bool initInstance()
	{
		if (!CheckInstanceExtension(nullptr, VK_KHR_SURFACE_EXTENSION_NAME))
			return false;
		AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);

		#if defined(VK_USE_PLATFORM_WIN32_KHR)
		if (!CheckInstanceExtension(NULL, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			return false;
		AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		#endif

		VkApplicationInfo app_info;
		memset(&app_info, 0, sizeof(app_info));
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.apiVersion = VK_MAKE_VERSION(1, 0, 11);
		app_info.pApplicationName = "VulkanTest.Sample001";
		app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 3);

		VkInstanceCreateInfo inst_info;
		memset(&inst_info, 0, sizeof(inst_info));
		inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		inst_info.pApplicationInfo = &app_info;

		inst_info.enabledLayerCount = instance_layers.size();
		inst_info.ppEnabledLayerNames = instance_layers.data();

		inst_info.enabledExtensionCount = instance_extensions.size();
		inst_info.ppEnabledExtensionNames = instance_extensions.data();

		auto result = vkCreateInstance(&inst_info, nullptr, &vk_inst);
		if (result != VK_SUCCESS)
			return false;

		#ifdef QT_DEBUG
		fvkCreateDebugReportCallbackEXT =
			(PFN_vkCreateDebugReportCallbackEXT)
			vkGetInstanceProcAddr(vk_inst, "vkCreateDebugReportCallbackEXT");
		fvkDestroyDebugReportCallbackEXT =
			(PFN_vkDestroyDebugReportCallbackEXT)
			vkGetInstanceProcAddr(vk_inst, "vkDestroyDebugReportCallbackEXT");
		#endif

		return true;
	}

	void destroyInstance()
	{
		if (vk_inst)
			vkDestroyInstance(vk_inst, nullptr);
	}

	bool createDevice()
	{
		if (vkEnumeratePhysicalDevices(vk_inst, &gpu_count, nullptr) != VK_SUCCESS)
			return false;
		gpu_list.resize(gpu_count);
		if (vkEnumeratePhysicalDevices(vk_inst, &gpu_count, gpu_list.data()) != VK_SUCCESS)
			return false;

		if (!CheckDeviceExtension(0, NULL, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			return false;
		AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		VkDeviceCreateInfo dev_info;
		memset(&dev_info, 0, sizeof(dev_info));

		VkDeviceQueueCreateInfo dev_q_info;
		memset(&dev_q_info, 0, sizeof(dev_q_info));

		dev_q_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

		uint32_t fam_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu_list[0], &fam_count, nullptr);
		if (fam_count < 1)
			return false;
		std::vector<VkQueueFamilyProperties> prop_list;
		prop_list.resize(fam_count);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu_list[0], &fam_count, prop_list.data());

		uint32_t fq_index = (uint32_t) -1; // -1 as not found

		for (uint32_t i = 0; i < prop_list.size(); i++)
		{
			if (prop_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				fq_index = i;
			}
		}

		if (fq_index == (uint32_t) -1) //still not found graphics bit
		{
			qCritical() << "You have not a graphic bit in your family!\n";
			return false;
		}
		else
			qInfo() << "I found a graphic bit in a " << fq_index << " family index.\n";

		float dev_q_pri[] = {1.0f};

		dev_q_info.queueCount = 1;
		dev_q_info.queueFamilyIndex = fq_index;
		dev_q_info.pQueuePriorities = dev_q_pri;

		dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		dev_info.queueCreateInfoCount = 1;
		dev_info.pQueueCreateInfos = &dev_q_info;

		dev_info.enabledLayerCount = device_layers.size();
		dev_info.ppEnabledLayerNames = device_layers.data();
		dev_info.enabledExtensionCount = device_extensions.size();
		dev_info.ppEnabledExtensionNames = device_extensions.data();

		if (vkCreateDevice(gpu_list[0], &dev_info, nullptr, &vk_dev) != VK_SUCCESS)
			return false;

		return true;
	}

	void destroyDevice()
	{
		if (vk_dev)
			vkDestroyDevice(vk_dev, nullptr);
	}

	bool createSurface(NativeHandle *handle)
	{
		VkResult res = VK_RESULT_MAX_ENUM;
		#if defined(VK_USE_PLATFORM_WIN32_KHR)
		VkWin32SurfaceCreateInfoKHR surface_info;
		memset(&surface_info, 0, sizeof(surface_info));
		surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surface_info.hinstance = handle->hInst;
		surface_info.hwnd = handle->hWnd;
		res = vkCreateWin32SurfaceKHR(vk_inst,
			&surface_info, nullptr, &surface_data.surface);
		#endif
		if (res != VK_SUCCESS)
			return false;
		return true;
	}

	void destroySurface()
	{
		vkDestroySurfaceKHR(vk_inst, surface_data.surface, nullptr);
	}

	bool createSwapchain()
	{
		VkPhysicalDevice gpu = gpu_list[0];
		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu,
			surface_data.surface, &surface_data.surface_capabilities) != VK_SUCCESS)
			return false;
		uint32_t formats_count = 0;
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu,
			surface_data.surface, &formats_count, NULL) != VK_SUCCESS)
			return false;
		surface_data.available_formats.resize(formats_count);
		if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface_data.surface,
			&formats_count, surface_data.available_formats.data()) != VK_SUCCESS)
			return false;
		uint32_t present_modes_count = 0;
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu,
			surface_data.surface, &present_modes_count, NULL) != VK_SUCCESS)
			return false;
		surface_data.present_modes.resize(present_modes_count);
		if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface_data.surface,
			&present_modes_count, surface_data.present_modes.data()) != VK_SUCCESS)
			return false;

		VkSurfaceCapabilitiesKHR &sc = surface_data.surface_capabilities;
		if (sc.maxImageCount < 1)
				return false;

		uint32_t images_count = sc.minImageCount + 1;

		if (images_count > sc.maxImageCount)
			images_count = sc.maxImageCount;

		VkSurfaceFormatKHR surface_format =
			{VK_FORMAT_UNDEFINED,
			 VK_COLORSPACE_SRGB_NONLINEAR_KHR};

		if (surface_data.available_formats.size() == 1
			&& surface_data.available_formats[0].format == VK_FORMAT_UNDEFINED)
		{
			surface_format = {VK_FORMAT_R8G8B8A8_UNORM,
							  VK_COLORSPACE_SRGB_NONLINEAR_KHR};
		}
		for (size_t i = 0; i < surface_data.available_formats.size(); i++)
		{
			if (surface_data.available_formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
				surface_format = surface_data.available_formats[i];
		}
		if (surface_format.format == VK_FORMAT_UNDEFINED)
			surface_format = surface_data.available_formats[0];

		VkExtent2D sc_extent;
		//w->GetWindowSize(sc_extent.width, sc_extent.height);
		if (sc.currentExtent.height == (uint32_t) -1
			&& sc.currentExtent.width == (uint32_t) -1)
		{
			if (sc_extent.height > sc.maxImageExtent.height)
				sc_extent.height = sc.maxImageExtent.height;
			if (sc_extent.width > sc.maxImageExtent.width)
				sc_extent.width = sc.maxImageExtent.width;
		}
		else
		{
			sc_extent.height = sc.currentExtent.height;
			sc_extent.width = sc.currentExtent.width;
		}
		VkImageUsageFlags flags = 0;
		if (sc.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		else
			flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		if (!(sc.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR))
			return false;
		VkPresentModeKHR pm = VK_PRESENT_MODE_IMMEDIATE_KHR;
		for (size_t i = 0; i < surface_data.present_modes.size(); i++)
		{
			if (surface_data.present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				pm = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (surface_data.present_modes[i] == VK_PRESENT_MODE_FIFO_KHR)
			{
				if (pm != VK_PRESENT_MODE_FIFO_RELAXED_KHR)
					pm = VK_PRESENT_MODE_FIFO_KHR;
			}
			if (surface_data.present_modes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
			{
				pm = VK_PRESENT_MODE_FIFO_KHR;
			}
		}
		VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		VkSwapchainCreateInfoKHR swapchain_create_info;
		memset(&swapchain_create_info, 0, sizeof(swapchain_create_info));
		swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.imageExtent = sc_extent;
		swapchain_create_info.imageFormat = surface_format.format;
		swapchain_create_info.imageColorSpace = surface_format.colorSpace;
		swapchain_create_info.imageUsage = flags;
		swapchain_create_info.presentMode = pm;
		swapchain_create_info.preTransform = transform;
		swapchain_create_info.minImageCount = images_count;
		swapchain_create_info.surface = surface_data.surface;
		swapchain_create_info.oldSwapchain = swapchain;
		swapchain_create_info.imageArrayLayers = 1;
		swapchain_create_info.compositeAlpha = alpha;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (vkCreateSwapchainKHR(vk_dev,
			&swapchain_create_info, nullptr, &swapchain) != VK_SUCCESS)
			return false;
		return true;
	}

	void destroySwapchain()
	{
		vkDestroySwapchainKHR(vk_dev, swapchain, nullptr);
	}
};

Renderer::Renderer() : imp(nullptr),
	native_handle(nullptr)
{
	this->imp = new RendererImp;
	this->native_handle = new NativeHandle;
}

Renderer::~Renderer()
{
    if (this->native_handle)
        delete this->native_handle;
	imp->destroySwapchain();
	imp->destroySurface();
	#ifdef QT_DEBUG
	imp->detachLoggin();
	#endif
	imp->destroyDevice();
	imp->destroyInstance();
	delete imp;
}

void Renderer::load()
{
	#ifdef QT_DEBUG
	imp->AddInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	#endif

	if (!imp->initInstance())
		qCritical() << "Cannot create Vulkan Instance.\n";
	else
		qInfo() << "Created Vulkan!\n";

	#ifdef QT_DEBUG
	imp->attachLogging();
	#endif

	imp->createSurface(native_handle);

	if (!imp->createDevice())
		qCritical() << "Cannot create Vulkan Device.\n";
	else
		qInfo() << "Vulkan have his own device.\n";

	if (!imp->createSwapchain())
		qCritical() << "Cannot create Vulkan Swapchain.\n";
	else
		qInfo() << "Swapchain created.\n";
}

void Renderer::set_window(NativeHandle &nhandle)
{
    *(this->native_handle) = nhandle;
}

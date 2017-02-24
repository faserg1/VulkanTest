#include "renderer.h"
#include "nativehandle.h"
#include <cstring>
#include <QDebug>
#include <QThread>
#include <vulkan/vulkan.h>

class RendererImp
{
	VkInstance vk_inst;
    VkDevice vk_dev;
	VkCommandPool cmdPool;

    uint32_t gpu_count;
    std::vector<VkPhysicalDevice> gpu_list;
	uint32_t graphic_family_index;

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

	uint32_t image_count;
	std::vector<VkImage> images;
	std::vector<VkCommandBuffer> cmd_buffers;

	VkSemaphore sem_render_done, sem_image_available;

	VkPipelineStageFlags wait_dst_stage_mask;
	VkSubmitInfo submit_info;
	VkPresentInfoKHR present_info;
	VkQueue queue;

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
		cmdPool = VK_NULL_HANDLE;
		swapchain = VK_NULL_HANDLE;
		sem_render_done = sem_image_available = VK_NULL_HANDLE;
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
			qCritical() << "You have not a graphic bit in your family!";
			return false;
		}
		else
			qInfo() << "I found a graphic bit in a " << fq_index << " family index.";

		graphic_family_index = fq_index;

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

	void createCommandPool()
	{
		VkCommandPoolCreateInfo pool_create_info;
		memset(&pool_create_info, 0, sizeof(VkCommandPoolCreateInfo));
		pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_create_info.queueFamilyIndex = graphic_family_index;
		vkCreateCommandPool(vk_dev, &pool_create_info, nullptr, &cmdPool);
	}

	void destroyCommandPool()
	{
		vkDestroyCommandPool(vk_dev, cmdPool, nullptr);
	}

	bool getImages()
	{
		if (vkGetSwapchainImagesKHR(vk_dev, swapchain, &image_count, nullptr) != VK_SUCCESS)
			return false;
		images.resize(image_count);
		cmd_buffers.resize(image_count);
		if (vkGetSwapchainImagesKHR(vk_dev, swapchain, &image_count, images.data()) != VK_SUCCESS)
			return false;
		return true;
	}

	bool createSemaphores()
	{
		VkSemaphoreCreateInfo sem_info;
		memset(&sem_info, 0, sizeof(sem_info));
		sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkResult rsem1, rsem2;
		rsem1 = vkCreateSemaphore(vk_dev, &sem_info, nullptr, &sem_render_done);
		rsem2 = vkCreateSemaphore(vk_dev, &sem_info, nullptr, &sem_image_available);
		if (rsem1 != VK_SUCCESS || rsem2 != VK_SUCCESS)
			return false;
		return true;
	}

	void destroySemaphores()
	{
		vkDestroySemaphore(vk_dev, sem_image_available, nullptr);
		vkDestroySemaphore(vk_dev, sem_render_done, nullptr);
	}

	void waitDeviceIdle()
	{
		vkDeviceWaitIdle(vk_dev);
	}

	bool recordCommandBuffers()
	{
		VkCommandBufferBeginInfo cmd_buffer_begin_info;
		memset(&cmd_buffer_begin_info, 0, sizeof(VkCommandBufferBeginInfo));
		cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		VkClearColorValue clear_color = {
			{ 0.0f, 1.0f, 1.0f, 0.5f }
		};

		VkImageSubresourceRange image_subresource_range;
		memset(&image_subresource_range, 0, sizeof(VkImageSubresourceRange));
		image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_subresource_range.layerCount = 1;
		image_subresource_range.levelCount = 1;

		VkCommandBufferAllocateInfo cmd_buffer_allocate_info;
		memset(&cmd_buffer_allocate_info, 0, sizeof(VkCommandBufferAllocateInfo));
		cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_buffer_allocate_info.commandBufferCount = image_count;
		cmd_buffer_allocate_info.commandPool = cmdPool;
		if (vkAllocateCommandBuffers(vk_dev, &cmd_buffer_allocate_info, cmd_buffers.data()) != VK_SUCCESS)
			return false;
		for( uint32_t i = 0; i < image_count; i++ )
		{
			VkImageMemoryBarrier barrier_from_present_to_clear;
			memset(&barrier_from_present_to_clear, 0, sizeof(VkImageMemoryBarrier));
			barrier_from_present_to_clear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier_from_present_to_clear.srcAccessMask = 0;
			barrier_from_present_to_clear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier_from_present_to_clear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier_from_present_to_clear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier_from_present_to_clear.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier_from_present_to_clear.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier_from_present_to_clear.image = images[i];
			barrier_from_present_to_clear.subresourceRange = image_subresource_range;
			VkImageMemoryBarrier barrier_from_clear_to_present;
			barrier_from_clear_to_present = barrier_from_present_to_clear;
			barrier_from_clear_to_present.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier_from_clear_to_present.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier_from_clear_to_present.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier_from_clear_to_present.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			vkBeginCommandBuffer(cmd_buffers[i], &cmd_buffer_begin_info);
			vkCmdPipelineBarrier(cmd_buffers[i],
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier_from_present_to_clear);
			vkCmdClearColorImage(cmd_buffers[i], images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color,
				1, &image_subresource_range);
			vkCmdPipelineBarrier(cmd_buffers[i],
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier_from_clear_to_present);
			if (vkEndCommandBuffer(cmd_buffers[i]) != VK_SUCCESS)
				return false;
		}
		return true;
	}

	void prepareDrawInfo()
	{
		wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;

		memset(&submit_info, 0, sizeof(VkSubmitInfo));
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
		submit_info.pWaitSemaphores = &sem_image_available;
		submit_info.commandBufferCount = 1;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &sem_render_done;

		memset(&present_info, 0, sizeof(VkPresentInfoKHR));
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &sem_render_done;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain;

		vkGetDeviceQueue(vk_dev, graphic_family_index, 0, &queue);
	}

	void draw()
	{
		VkResult result;
		uint32_t image_index;
		result = vkAcquireNextImageKHR(vk_dev,
			swapchain,
			UINT64_MAX,
			sem_image_available,
			VK_NULL_HANDLE,
			&image_index);
		switch (result)
		{
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			//recreating
			break;
		case VK_SUCCESS:
			break;
		case VK_TIMEOUT:
			return;
		default:
			return;
		}
		submit_info.pCommandBuffers = &cmd_buffers[image_index];
		present_info.pImageIndices = &image_index;
		result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
			return;
		result = vkQueuePresentKHR(queue, &present_info);
		switch (result)
		{
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			//recreating
			break;
		case VK_SUCCESS:
			break;
		default:
			return;
		}
	}
};

Renderer::Renderer() : imp(nullptr),
	native_handle(nullptr)
{
	imp = new RendererImp;
	native_handle = new NativeHandle;
	renderThread = new QThread();

	rendering = false;
}

Renderer::~Renderer()
{
	imp->waitDeviceIdle();
    if (this->native_handle)
        delete this->native_handle;
	imp->destroySemaphores();
	imp->destroyCommandPool();
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
		qCritical() << "Cannot create Vulkan Instance.";
	else
		qInfo() << "Created Vulkan!";

	#ifdef QT_DEBUG
	imp->attachLogging();
	#endif

	imp->createSurface(native_handle);

	if (!imp->createDevice())
		qCritical() << "Cannot create Vulkan Device.";
	else
		qInfo() << "Vulkan have his own device.";

	if (!imp->createSwapchain())
		qCritical() << "Cannot create Vulkan Swapchain.";
	else
		qInfo() << "Swapchain created.";

	/// TODO: Preparing to draw.
	imp->createCommandPool();

	if (!imp->getImages())
		qCritical() << "Cannot get swapchain images.";
	else
		qInfo() << "Swapchain images taken.";

	if (!imp->createSemaphores())
		qCritical() << "Cannot create semaphores.";
	else
		qInfo() << "Semaphores created.";

	if (!imp->recordCommandBuffers())
		qCritical() << "Cannot record command buffers.";
	else
		qInfo() << "Command buffers recorded.";

	connect(renderThread, SIGNAL(started()),
		this, SLOT(drawFrame()), Qt::DirectConnection);
}

void Renderer::drawFrame()
{
	imp->prepareDrawInfo();
	while (rendering)
	{
		imp->draw();
	}
}

void Renderer::set_window(NativeHandle &nhandle)
{
    *(this->native_handle) = nhandle;
}

void Renderer::startRender()
{
	rendering = true;
	renderThread->start();
}

void Renderer::stopRender()
{
	rendering = false;
	renderThread->quit();
}

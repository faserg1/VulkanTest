#include "Render.h"
#include "common.h"
#include "Logger.h"
#include "Application.h"
#include "Window.h"
#include <iostream>

Render::Render()
{
	instance = VK_NULL_HANDLE;
	device = VK_NULL_HANDLE;
	gpu = VK_NULL_HANDLE;

	graphic_family_index = (uint32_t) -1;
	present_family_index = (uint32_t) -1;

	surface_data.surface = VK_NULL_HANDLE;
	surface_data.extensions_enabled = false;

	swapchain_data.extensions_enabled = false;
	swapchain_data.gpu_support = false;
	swapchain_data.swapchain = VK_NULL_HANDLE;

	debug_enabled = false;
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

bool Render::CheckInstanceLayer(const char *name)
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

bool Render::CheckInstanceExtension(const char *layer_name, const char *name)
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

bool Render::CheckDeviceLayer(const char *name)
{
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

bool Render::CheckDeviceExtension(const char *layer_name, const char *name)
{
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

const VkInstance Render::GetInstance() const
{
	return instance;
}

const VkDevice Render::GetDevice() const
{
	return device;
}

const VkSwapchainKHR Render::GetSwapchain() const
{
	return swapchain_data.swapchain;
}

void Render::EnableDebug(bool enable)
{
	if (enable)
		AddInstanceExtension(Logger::GetRequiredExtension());
	else
		RemoveInstanceExtension(Logger::GetRequiredExtension());
	debug_enabled = enable;
}

bool Render::EnableSurface(bool enable)
{
	if (enable)
	{
		/* В Vulkan плоскость — это некая абстракция, куда мы будем в последствии помещать изображение. Для плоскостей существуют
		 * расширения, так какплоскости бывают привязаны к разным оконным системам, а могут быть также привязаны напрямую к
		 * дисплею. К слову говоря, для того, чтобы рисовать напрямую в дисплей, мы игнорируем любые другие оконные системы,
		 * но для этого нужно отдельное расширение VK_KHR_display для экземпляра Vulkan, VK_KHR_display_swapchain для устройства.
		 * Эти два расширения недоступны для карт NVIDIA, так как их поддержка всё ещё в разработке.
		 * Источник: https://devtalk.nvidia.com/default/topic/925605/linux/nvidia-364-12-release-vulkan-glvnd-drm-kms-and-eglstreams/
		 * А чтобы проверить доступные возможности, можно воспользоваться приложением vulkaninfo.
		 *
		 * Если некоторые функции, которые используют WSI, вернули VK_ERROR_SURFACE_LOST_KHR, то нам нужно уничтожить плоскость,
		 * и вместе с этим и цепочки переключений (swapchain), которую эту плоскость используют, так как они больше
		 * не пригодны к использованию.
		 * Если функции вернули VK_ERROR_OUT_OF_DATE_KHR, то это означает, что сама плосколсть не пропала, но изменилась.
		 * Рисовать в неё, конечно, можно, но это отнимает часть производительности, так как приходится вручную преобразовывать
		 * изображение. Чтобы этого не было нужно пересоздать цепочку переключений с новыми параметрами.
		 *
		 * Для начала добавим неободимые расширения. Первым делом, добавим общее (базовое) расширение для плоскостей.
		 * Для этого у нас есть специальное макроимя. Прежде, чем мы добавим это расширение, проверим его на доступность.
		*/
		if (!CheckInstanceExtension(NULL, VK_KHR_SURFACE_EXTENSION_NAME))
			return false;
		AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
		/* Затем, нужно добавить расширение плоскости, специальное для каждой платформы.
		 * Но перед этим нам нужно настроить заголовки Vulkan под нашу платформу, поэтому в Render.h, перед тем, как добавить
		 * vulkan.h, добавлен заголовок platform.h, который настраивает макроимена. Иначе нам просто не будут доступны необходимые
		 * функции и структуры.
		*/
		#if defined(VK_USE_PLATFORM_WIN32_KHR)
		if (!CheckInstanceExtension(NULL, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			return false;
		AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		#endif
		//Готово!
		surface_data.extensions_enabled = true;
	}
	else
	{
		RemoveInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
		#if defined(VK_USE_PLATFORM_WIN32_KHR)
		RemoveInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		#endif
		surface_data.extensions_enabled = false;
	}
	return surface_data.extensions_enabled;
}

bool Render::CreateInstance(std::string app_name)
{
	VkApplicationInfo app_info;
	ZM(app_info);
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name.c_str();
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 11);
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

bool Render::CreateSurface(Application *app, Window *w)
{
	if (!surface_data.extensions_enabled)
		return false;
	VkResult res = VK_RESULT_MAX_ENUM;
	#if defined(VK_USE_PLATFORM_WIN32_KHR)
	/* Для создания необходимой win32 плоскости нам понадобиться: HWND & HINSTANCE.
	 * Их мы возьмём их наших параметров.
	*/
	VkWin32SurfaceCreateInfoKHR surface_info;
	ZM(surface_info);
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = app->GetHandle();
	surface_info.hwnd = w->GetHandle();
	res = vkCreateWin32SurfaceKHR(instance, &surface_info, NULL, &surface_data.surface);
	#endif
	if (res != VK_SUCCESS)
		return false;
	//Готово!
	return true;
}

void Render::DestroySurface()
{
	vkDestroySurfaceKHR(instance, surface_data.surface, NULL);
}

bool Render::EnableSwapchains(bool enable)
{
	if (enable)
	{
		/* Мы также проверим, возможно ли подключить необходимые расширения для нашего устройства.
		*/
		if (!CheckDeviceExtension(NULL, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			return false;
		AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		swapchain_data.extensions_enabled = true;
	}
	else
	{
		RemoveDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		swapchain_data.extensions_enabled = false;
	}
	return swapchain_data.extensions_enabled;
}

bool Render::PrepareGPU()
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
	gpu = gpu_list[0];

	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, VK_NULL_HANDLE);
	std::vector<VkQueueFamilyProperties> family_properties_list(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_properties_list.data());
	for (uint32_t i = 0; i < family_count; i++)
	{
		VkQueueFamilyProperties &properties = family_properties_list[i];
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (graphic_family_index == (uint32_t) -1)
				graphic_family_index = i;
		}
		/* Теперь, после того, как мы проверили семейство на совместимость с графикой, проверим, поддерживает ли это семейство
		 * презентацию на плоскость.
		*/
		VkBool32 support_present_to_surface;
		/* Следующая функция, проверяет, можно ли отправлять изображения на плоскость (поддерживает ли наше устройство такую
		 * возможность). Параметры:
		 * физическое устройство (входной)
		 * индекс семейства (входной)
		 * плоскость, что мы создали (входной)
		 * логическая переменная, значение которой и показывает возможность поддержки (выходной)
		*/
		if (vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface_data.surface, &support_present_to_surface) != VK_SUCCESS)
		{
			//А здесь возможна обработка ошибки, если мы потеряли плоскость или хосту/девайсу не хватает памяти.
		}
		if (support_present_to_surface)
		{
			//Также, мы можем проверить, может ли наше устройсво поддерживать плоскость нашей платформы.
			#if defined(VK_USE_PLATFORM_WIN32_KHR)
			/* Для Windows есть функция vkGetPhysicalDeviceWin32PresentationSupportKHR.
			 * Параметры:
			 * физическое устройство
			 * индекс семейства
			 * Возвращает VkBool32. VK_TRUE, если поддерживает.
			*/
			if (vkGetPhysicalDeviceWin32PresentationSupportKHR(gpu, i) == VK_TRUE
				&& present_family_index == (uint32_t) -1)
				present_family_index = i;
			#else
			present_family_index = i;
			#endif
		}
	}
	if (graphic_family_index == (uint32_t) -1)
		return false;
	if (present_family_index != (uint32_t) -1)
		swapchain_data.gpu_support = true;
	return true;
}

bool Render::CreateDevice()
{
	//Настройка очередей
	VkDeviceQueueCreateInfo device_queue_info;
	ZM(device_queue_info);
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	float device_queue_priority[] = {1.0f};
	device_queue_info.queueCount = 1;
	device_queue_info.queueFamilyIndex = graphic_family_index;
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
		return false;
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

bool Render::PrepareSwapchain()
{
	/* Теперь, чтобы мы потом могли создать цепочку переключений, нам нужно собрать информацию о нашей плоскости.
	 * Для этого есть следубщие функции:
	 * vkGetPhysicalDeviceSurfaceCapabilitiesKHR
	 * vkGetPhysicalDeviceSurfaceFormatsKHR
	 * vkGetPhysicalDeviceSurfacePresentModesKHR
	*/
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface_data.surface, &surface_data.surface_capabilities) != VK_SUCCESS)
		return false;
	uint32_t formats_count = 0;
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface_data.surface, &formats_count, NULL) != VK_SUCCESS)
		return false;
	surface_data.available_formats.resize(formats_count);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface_data.surface,
		&formats_count, surface_data.available_formats.data()) != VK_SUCCESS)
		return false;
	uint32_t present_modes_count = 0;
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface_data.surface, &present_modes_count, NULL) != VK_SUCCESS)
		return false;
	surface_data.present_modes.resize(present_modes_count);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface_data.surface,
		&present_modes_count, surface_data.present_modes.data()) != VK_SUCCESS)
		return false;
	VkSurfaceCapabilitiesKHR &sc = surface_data.surface_capabilities;
	/* И теперь немного покопаемся:
	 * Для начала, посмотрим, сколько изображений может содержать наш Swapchain. Это может понадобиться, для того, чтобы
	 * сделать тройную буферизацию, если такое возможно. Обычно, minImageCount = 1.
	*/
	std::wcout << L">> Минимальное кол-во изображений: " << surface_data.surface_capabilities.minImageCount << L".\n";
	std::wcout << L">> Максимальное кол-во изображений: " << surface_data.surface_capabilities.maxImageCount << L".\n";
	/* Теперь, нам допустим нужно узнать размеры области, в которую мы хотим что-то отображать, не особо заморачиваясь с
	 * функциями вашего оконного API. Для этого есть currentExtent с параметрами width и height.
	 * Также, есть minImageExtent и maxImageExtent, которые показывают минимально и максимально возможное разрешение
	 * изображения соответственно (не для текущей плоскости, а вообще возможной).
	*/
	std::wcout << L">> Размер области рисования: " <<
		sc.currentExtent.width << L"x" <<
		sc.currentExtent.height << L".\n";
	std::wcout << L">> Минимальный размер области рисования: " <<
		sc.minImageExtent.width << L"x" <<
		sc.minImageExtent.height << L".\n";
	std::wcout << L">> Максимальный размер области рисования: " <<
		sc.maxImageExtent.width << L"x" <<
		sc.maxImageExtent.height << L".\n";
	/* Также, у изображения могут быть слои, и максимальное их кол-во задаётся парамтером maxImageArrayLayers.
	 * Подробнее о слоях изображения будет рассказано в одном из следующих уроков.
	*/
	std::wcout << L">> Максимальное кол-во слоёв изображения: " <<
		sc.maxImageArrayLayers << L".\n";
	/* У изображения также есть разные преобразования (трансформации), т.е. вращение и/или отраение изображения.
	 * За поддерживаемые форматы отвечает битовая маска supportedTransforms, а за текущею трансформацию currentTransform.
	*/
	if (sc.supportedTransforms)
	{
		std::wcout << L">> Есть поддержка трансформаций:\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
			std::wcout << L">>> Без преобразований, VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR)
			std::wcout << L">>> Вращение на 90 градусов по часовой, VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR)
			std::wcout << L">>> Вращение на 180 градусов по часовой, VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
			std::wcout << L">>> Вращение на 270 градусов по часовой, VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR)
			std::wcout << L">>> Отражение по горизонтали, VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR)
			std::wcout << L">>> Отражение по горизонтали + 90 по часовой, " <<
				"VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR)
			std::wcout << L">>> Отражение по горизонтали + 180 по часовой, " <<
				"VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR)
			std::wcout << L">>> Отражение по горизонтали + 270 по часовой, " <<
				"VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR.\n";
		if (sc.supportedTransforms & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR)
			std::wcout << L">>> Трансформация определяется платформой, VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR.\n";
	}
	std::wcout << L">> Текущая трансформация:\n";
	switch (sc.currentTransform)
	{
		case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR.\n";
			break;
		case VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR:
			std::wcout << L">>> VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR.\n";
			break;
		default:
			break;
	}
	/* Также можно настроить смешивание альфа каналов изображения и плоскости окна, если такое поддерживается.
	 * Для этого есть битовая маска supportedCompositeAlpha, в которой содержаться все поддерживаемые режимы композиции.
	*/
	if (sc.supportedCompositeAlpha)
	{
		std::wcout << L">> Доступны режимы смешивания альфа канала:\n";
		if (sc.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			std::wcout << L">>> Альфа канал изображения игнорируется, если такой есть (приравнивается к 1.0).\n";
		//Ожидается, что приложение уже смешало не-альфа каналы изображения и альфа канал приложения.
		if (sc.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
			std::wcout << L">>> Альфа канал смешивается (Pre).\n";
		//В этом случае, не ожидается смешивание от приложения, и это делается во время композиции.
		if (sc.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
			std::wcout << L">>> Альфа канал смешивается (Post).\n";
		if (sc.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
			std::wcout << L">>> Смешивание каналов происходит через функции платформы.\n";
	}
	/* Далее по списку — флаги использования supportedUsageFlags, которые показывают, в каких целях можно использовать
	 * изображения в Swapchain. Обычно всегда есть флаг VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, но платформа может предоставить
	 * и другие. <<Подробнее об этих флагах расскажу позже, поэтому я поленюсь и сделаю макровызов.>>
	*/
	std::wcout << L">> Возможные Usage флаги:\n";
	#define PrintIfExists(var, mask) if ((var) & (mask)) std::wcout << L">>> " << (#mask) << L".\n"
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_SAMPLED_BIT);
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_STORAGE_BIT);
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
	PrintIfExists(sc.supportedUsageFlags, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
	#undef PrintIfExists
	//Теперь можно посмотреть на возможные форматы... хотелось бы сказать...
	std::vector<VkSurfaceFormatKHR> &af = surface_data.available_formats;
	std::wcout << L">> Кол-во форматов: " << af.size() << L".\n";
	for (size_t i = 0; i < af.size(); i++)
	{
		VkSurfaceFormatKHR &sf = af[i];
		/* Различных форматов бесчисленное множество, так что перечислять и проверять их все здесь — безумие.
		 * Но существует и другое. На момент спецификации 1.0.10 было замечено, что Vulkan возможно будет поддерживать другие
		 * цветовые схемы, кроме sRGB, но на данный момент, sRGB — это единственная возможная.
		*/
		if (sf.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			std::wcout << L">>> Поддержка цветовой схемы sRGB.\n";
	}
	/* Последним проверим режимы презентации изображений. С ними проще: всего 4 возможных.
	 * Режим фактически определяет, каким способом будет обновлятся наше изображение.
	*/
	std::vector<VkPresentModeKHR> &pm = surface_data.present_modes;
	std::wcout << L">> Кол-во режимов показа: " << pm.size() << L".\n";
	for (size_t i = 0; i < pm.size(); i++)
	{
		VkPresentModeKHR &mode = pm[i];
		switch (mode)
		{
			case VK_PRESENT_MODE_IMMEDIATE_KHR:
				/* В данном случае, изоражение отображается немедленно, ничего не ожидая. Поэтому могут быть заметны разрывы,
				 * между предыдущем кадром и следующем.
				*/
				std::wcout << L">>> Изображение обновляется немедленно.\n";
				break;
			case VK_PRESENT_MODE_MAILBOX_KHR:
				/* В данном случае, ожидается следующий "Кадровый гасящий импульс" (Vertical blanking period, VBI),
				 * чтобы сменилось изображение на плоскости. Тем не менее, очередь состоит только из 1 изображения,
				 * и если пришло новое, то старое (в очереди) заменяется новым, и выбывшее уходит на переиспользование.
				 * Также на переиспользование уходит изображение, которое уже отобразилось на плоскости. Т.е. эти
				 * изображения вновь стали доступны для использования приложением.
				*/
				std::wcout << L">>> Изображение ждёт VBI. Очередь состоит из 1 изображения.\n";
				break;
			case VK_PRESENT_MODE_FIFO_KHR:
				/* В данном случае, работает абсолютно также, как и предудущее, только теперь очередь больше.
				 * Если очередь заполнена, то новое изображение заменяет старое, т.е. изображение в начале очереди удаляется,
				 * и новое помещается в конец очереди.
				 * Спецификация говорит, что это всё равно, что в {wgl|glX|egl}SwapBuffers использовать интервал смены 1.
				*/
				std::wcout << L">>> Изображение ждёт VBI. Очередь состоит из (numSwapchainImages - 1) изображений.\n";
				break;
			case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
				/* В этом случае, если изображение уже можно поместить на плоскость (т.е. недавно прошёл период VBI),
				 * то изображение отобразится на плоскость сразу же, без ожидания следующего VBI. Очередь работает также,
				 * как и с предыдущим вариантом.
				 * Спецификация говорит, что это всё равно, что в {wgl|glX|egl}SwapBuffers использовать интервал смены -1.
				*/
				std::wcout << L">>> Изображение может ждать VBI. Очередь состоит из (numSwapchainImages - 1) изображений.\n";
				break;
			default:
				break;
		}
	}

	return true;
}

bool Render::CreateSwapchain(Window *w)
{
	/* Настало время создать нашу цепочку переключений. Фактически, эта цепочка позволяет нам переключаться между изображениями,
	 * что лежат в очереди этой цепочки. Одно изображение сменяется другим. Одно изображение сейчас на плоскости, другое в обработке,
	 * а остальные ждут своего момента, чтобы заменить первое, а потом вновь пойти на обработку.
	 * Поэтому, для начала укажем, какое кол-во изображений будет в нашей цепочке.
	*/
	VkSurfaceCapabilitiesKHR &sc = surface_data.surface_capabilities;
	//Проверим, есть ли мообще поддерживаемые изображения для Swapchain
	if (sc.maxImageCount < 1)
		return false;
	//В данном случае, укажем 2 изображения (хотя может быть и больше, если minImageCount > 1)
	uint32_t images_count = sc.minImageCount + 1;
	/* Эта проверка нужна для случаев, когда мы хотим указать больше, чем 2 изображения в цепочке (например 3, для тройной
	 * буферезации), но наша плоскость может и не поддерживать такое кол-во.
	*/
	if (images_count > sc.maxImageCount)
		images_count = sc.maxImageCount;
	/* Затем подберём формат нашей плоскости. Так как их может быть несколько (или неопределён вообще), с этим возникнут
	 * небольшие осложнения.
	*/
	VkSurfaceFormatKHR surface_format = {VK_FORMAT_UNDEFINED, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
	/* И так, допустим в списке у нас только один формат VK_FORMAT_UNDEFINED. На самом деле, это значит, что можно для
	 * плоскости выбрать любой формат (формат изображения, конечно же).
	*/
	if (surface_data.available_formats.size() == 1 && surface_data.available_formats[0].format == VK_FORMAT_UNDEFINED)
	{
		surface_format = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR};
	}
	/*В противном случае нам надо подобрать подходящий формат. Обычно, это VK_FORMAT_R8G8B8A8_UNORM, он
	 * довольно часто используется.
	*/
	for (size_t i = 0; i < surface_data.available_formats.size(); i++)
	{
		if (surface_data.available_formats[i].format == VK_FORMAT_R8G8B8A8_UNORM)
			surface_format = surface_data.available_formats[i];
	}
	//Если такого нет...
	if (surface_format.format == VK_FORMAT_UNDEFINED)
		surface_format = surface_data.available_formats[0]; //...то не будем заморачиваться!
	/* Теперь надо настроить разрешение изображений. Тут в принципе всё просто, когда оно задаётся через
	 * currentExtent, но когда у этого параметра ширина и высота равны -1, тогда вы должны задать разрешение
	 * сами, но упираясь в рамки. Это выглядит примерно так:
	*/
	VkExtent2D sc_extent;
	w->GetWindowSize(sc_extent.width, sc_extent.height);
	if (sc.currentExtent.height == (uint32_t) -1 && sc.currentExtent.width == (uint32_t) -1)
	{
		/* Можно, конечно, проверить и на минимумы, но зачем? Обычно, минимум равен {1,1}, и даже если превышает
		 * это число, то не должен превышать максимум. Но кто знает, может здесь затаился великий математик?
		*/
		if (sc_extent.height > sc.maxImageExtent.height)
			sc_extent.height = sc.maxImageExtent.height;
		if (sc_extent.width > sc.maxImageExtent.width)
			sc_extent.width = sc.maxImageExtent.width;
	}
	/* Флаги использвания. В общем, нам всего лишь нужно проверить, можно ли копировать данные в изображение,
	 * за это отвечает флаг VK_IMAGE_USAGE_TRANSFER_DST_BIT. Но здесь обязательно есть флаг использвания цвета,
	 * т.е. VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, если существует какой-либо другой.
	*/
	VkImageUsageFlags flags = 0;
	if (sc.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	else
		flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//Вращать/отражать изображение мне совершенно не хочеться. Тем более, это влияет на производительность.
	VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	if (!(sc.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR))
		return false;
	/* Способ отображения с минимальной задержкой — VK_PRESENT_MODE_MAILBOX_KHR.
	 * (Проверил: Clear Screen cпособоб выдавал на 200-300 кадров в секунду больше, чем с FIFO)
	 * Хотя, этот способ может оказаться недоступным, так что FIFO будет подобным способом,
	 * но чуть менее быстрым.
	*/
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
			pm = VK_PRESENT_MODE_FIFO_KHR;
		}
	}
	//Такой способ работы с альфа-каналом наверняка есть везде.
	VkCompositeAlphaFlagBitsKHR alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	// Пришло время заполнить структуру данными!
	VkSwapchainCreateInfoKHR swapchain_create_info;
	ZM(swapchain_create_info);
	swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	/* flags зарезервированы, должны быть NULL, pNext должен быть NULL или указатель на структуру из расширения.
	 * Заполним структуру данными, которые мы получили.
	*/
	swapchain_create_info.imageExtent = sc_extent;
	swapchain_create_info.imageFormat = surface_format.format;
	swapchain_create_info.imageColorSpace = surface_format.colorSpace;
	swapchain_create_info.imageUsage = flags;
	swapchain_create_info.presentMode = pm;
	swapchain_create_info.preTransform = transform;
	swapchain_create_info.minImageCount = images_count;
	//Укажем плоскость, к которой будет привязана наша цепочка.
	swapchain_create_info.surface = surface_data.surface;
	/* Если мы пересоздём swapchain на этой же плоскости, то необходимо указать старый. Это позволит избежать лишних задержек,
	 * ошибок и так далее.
	*/
	swapchain_create_info.oldSwapchain = swapchain_data.swapchain;
	//У изображений также есть слои. Их можно использовать для мульти/стерео-изображений. Но для обычного ставим 1.
	swapchain_create_info.imageArrayLayers = 1;
	//Альфа-канал
	swapchain_create_info.compositeAlpha = alpha;
	/* Позволяет включить механизм перекрытия, когда другое окно (или другой объект) перекрывает пиксели нашей плоскости.
	 * Хотя, включение этого не гарантирует, что такое обязательно произойдёт. Когда пиксели перестанут быть перекрытыми другим
	 * объектом, их содержание станет неопределённым.
	 * В противном случае, если не включать перекрытие, то оно станет невозможным — swapchain арендует все пиксели полностью.
	*/
	swapchain_create_info.clipped = VK_TRUE;
	/* Далее, укажем, что наш swapchain будет использовать только одна очередь за всё время его жизни.
	 * В противном случае, нам нужно будет поставить VK_SHARING_MODE_CONCURRENT, а также
	 * указать сколько очередей будет использовать swapchain (queueFamilyIndexCount), и сами очереди через их индексы
	 * (pQueueFamilyIndices).
	*/
	swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	/* Создём цепочку. Внимание! Могут возникнуть ошибки, если недостаточно памяти, или потеряно утсройство.
	 * Также, могут возникнуть ошибки, если потеряна плоскость (VK_ERROR_SURFACE_LOST_KHR) или она
	 * находится в использовании другого объекта (VK_ERROR_NATIVE_WINDOW_IN_USE_KHR), будь то объект Vulkan
	 * или любой другой.
	*/
	if (vkCreateSwapchainKHR(device, &swapchain_create_info, NULL, &swapchain_data.swapchain) != VK_SUCCESS)
		return false;
	return true;
}


void Render::DestroySwapchain()
{
	vkDestroySwapchainKHR(device, swapchain_data.swapchain, NULL);
}

const VkQueue Render::GetQueue(uint32_t index) const
{
	VkQueue queue;
	vkGetDeviceQueue(device, graphic_family_index, 0, &queue);
	return queue;
}

VkCommandPool Render::CreateCommandPool(bool reset, bool transient)
{
	VkCommandPoolCreateInfo pool_create_info;
	ZM(pool_create_info);
	pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_create_info.queueFamilyIndex = graphic_family_index;
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

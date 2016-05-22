/* В этом уроке раскажется о том, как создавать различные ресурсы в Vulkan.
 * Это такие ресурсы, как буфер (будь то вершинный или индексный) или изображение.
 * Также, здесь снова всё раскидано по функциям и структурам, чтобы особо не мудрить с классами.
 * Все интересующие вас подробности смотрите здесь:
 * https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf
 * © Серый Волк.
*/

#include <iostream> //Ввод-вывод информации.
#include <string.h> //Необходимые инструменты (memset)
#include <vector> //Удобно храним массивы
#include <cstdlib> //exit
#include <vulkan/vulkan.h> // Vulkan API

//Макрос для очистки памяти, zero memory
#define ZM(what) memset(&what, 0, sizeof(what))

//Для начала, буду хранить все необходимые хэндлы и т.д. здесь, в структуре
struct _vkglobals
{
	VkInstance instance;
	VkDevice device;
	VkDebugReportCallbackEXT debug_report_callback;
	
	uint32_t family_index;
	
	std::vector<const char *>
		instance_layers, instance_extensions,
		device_layers, device_extensions;
		
	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT;
	
	//Зполнение нулями. хотя здесь можно было бы и memset применеть на всю структуру.
	inline _vkglobals() 
	{
		instance = VK_NULL_HANDLE;
		device = VK_NULL_HANDLE;
		debug_report_callback = VK_NULL_HANDLE;
		
		family_index = (uint32_t) -1;
		
		fvkCreateDebugReportCallbackEXT = NULL;
		fvkDestroyDebugReportCallbackEXT = NULL;
	}
} vkGlobals;

char app_name[] = "Vulkan Tutorian. Resource creation © GrWolf.";

//Создам прототипы для инициализации и уничтожения экземпляра, устройства, дебага и т.д.
void PrepareLayersAndExtensions();
bool InitInstance();
bool InitDebug();
bool InitDevice();
void DestroyInstance();
void DestroyDebug();
void DestroyDevice();

//Прототип для дебаг репортов
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT,VkDebugReportObjectTypeEXT,
	uint64_t,size_t,int32_t,const char*,const char*,void*);

int main()
{
	//Сначала настроем всё как надо.
	setlocale(LC_ALL, "Russian");
	PrepareLayersAndExtensions(); 
	if (!InitInstance()) 
		return 1; 
	#ifdef __DEBUG 
	if (!InitDebug()) 
		return 2; 
	#endif
	if (!InitDevice()) 
		return 3;
	/* Теперь настало время разобраться с ресурсами Vulkan, как они создаются и уничтожаются.
	 * Vulkan предоставляет два вида ресурсов. Это буфер (VkBuffer) и изображение (VkImage).
	 * Каждый из них имеет разные форматы и размер. Буфер — фактически лишь массив из байтов,
	 * который может быть представлен в различных видах. В то время как изображение — всегда
	 * имеет определённый формат. Тем не менее, разрешено копирование из одного в другое и между
	 * собой.
	*/ 
	VkResult res;
	VkBufferCreateInfo buffer_info;
	VkBuffer buffer = VK_NULL_HANDLE;
	ZM(buffer_info);
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	buffer_info.size = 16;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	res = vkCreateBuffer(vkGlobals.device, &buffer_info, NULL, &buffer);
	if (res == VK_SUCCESS)
	{
		//vkBindBufferMemory()
	}
	DestroyDevice();
	#ifdef __DEBUG
	DestroyDebug();
	#endif
	DestroyInstance();
	return 0;
}

//Подготовка слоёв и расширений
void PrepareLayersAndExtensions()
{
	#ifdef __DEBUG // Макрос объявлен в настройках
	vkGlobals.instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	vkGlobals.device_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	vkGlobals.instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	#endif
}


bool InitInstance()
{
	VkApplicationInfo app_info; 
	ZM(app_info);
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name;
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 5);
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	
	VkInstanceCreateInfo instance_info;
	ZM(instance_info);
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	
	instance_info.enabledLayerCount = vkGlobals.instance_layers.size();
	instance_info.ppEnabledLayerNames = vkGlobals.instance_layers.data();
	
	instance_info.enabledExtensionCount = vkGlobals.instance_extensions.size();
	instance_info.ppEnabledExtensionNames = vkGlobals.instance_extensions.data();
	
	if (vkCreateInstance(&instance_info, NULL, &vkGlobals.instance) != VK_SUCCESS)
		return false;
	return true;
}

bool InitDebug()
{
	//Получение функций
	vkGlobals.fvkCreateDebugReportCallbackEXT =
		(PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(vkGlobals.instance, "vkCreateDebugReportCallbackEXT");
	vkGlobals.fvkDestroyDebugReportCallbackEXT =
		(PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(vkGlobals.instance, "vkDestroyDebugReportCallbackEXT");
	
	//Настройка callback'а
	VkDebugReportCallbackCreateInfoEXT debug_report_callback_info;
	ZM(debug_report_callback_info);
	debug_report_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_report_callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	debug_report_callback_info.pfnCallback = DebugReportCallback;
	
	//Создание callback
	if (vkGlobals.fvkCreateDebugReportCallbackEXT(vkGlobals.instance, &debug_report_callback_info,
		NULL, &vkGlobals.debug_report_callback) != VK_SUCCESS)
		return false;
	return true;
}

bool InitDevice()
{
	//Поиск GPU
	std::vector<VkPhysicalDevice> gpu_list;
	uint32_t gpu_count;
	if (vkEnumeratePhysicalDevices(vkGlobals.instance, &gpu_count, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		std::wcerr << L"Посчитать физические устройства не удалось.\n";
		return false;
	}
	gpu_list.resize(gpu_count);
	if (vkEnumeratePhysicalDevices(vkGlobals.instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
	{
		std::wcerr << L"Получить физические устройства не удалось.\n";
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
		std::wcerr << L"Подходящее семейство не найдено.\n";
		return false;
	}
	
	vkGlobals.family_index = valid_family_index;
	
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
	device_info.enabledLayerCount = vkGlobals.device_layers.size();
	device_info.ppEnabledLayerNames = vkGlobals.device_layers.data();
	device_info.enabledExtensionCount = vkGlobals.device_extensions.size();
	device_info.ppEnabledExtensionNames = vkGlobals.device_extensions.data();
	
	//Создание девайса
	if (vkCreateDevice(gpu, &device_info, NULL, &vkGlobals.device) != VK_SUCCESS)
	{
		std::wcerr << L"Создать устройство не удалось.\n";
		return false;
	}
	return true;
}

void DestroyInstance()
{
	vkDestroyInstance(vkGlobals.instance, NULL);
}

void DestroyDebug()
{
	vkGlobals.fvkDestroyDebugReportCallbackEXT(vkGlobals.instance, vkGlobals.debug_report_callback, NULL);
}

void DestroyDevice()
{
	vkDestroyDevice(vkGlobals.device, NULL);
}

//Debug Report Callback
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objectType, 
	uint64_t object, 
	size_t location, 
	int32_t messageCode, 
	const char *pLayerPrefix, 
	const char *pMessage, 
	void *pUserData) 
{
	std::cout << "[" << pLayerPrefix << "] " << pMessage << std::endl;
	return false;
}

/* В общем, спасибо, что уделили этому уроку время. Khronos Group, Niko Kauppi, и тем, кто делает примеры — Большое Спасибо!
 * А дальше классика:
 * Если вы хотите меня поддержать, прикрепляю кошель с Я.Денег:
 * 410012557544062
 * Ну или просто жмя по ссылке:
 * https://money.yandex.ru/to/410012557544062
 * Хе-хе.
*/
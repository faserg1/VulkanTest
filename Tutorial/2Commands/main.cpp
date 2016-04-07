/* В этом уроке теперь всё будет более-менее расфасовано по функциям,
 * чтобы не нагромождать сильно с кодом.
 * Тематика этого урока: команды и командные буферы (command buffers).
 * Все интересующие вас подробности смотрите здесь:
 * https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf
 * © Серый Волк.
*/

#include <iostream> //Ввод-вывод информации.
#include <string.h> //Необходимые инструменты (memset)
#include <vector> //Удобно храним массивы
#include <cstdlib> //exit
#include <vulkan/vulkan.h> // Vulkan API

//Для начала, буду хранить все необходимые хэндлы и т.д. здесь, в структуре
struct _vkglobals
{
	VkInstance instance;
	VkDevice device;
	VkDebugReportCallbackEXT debug_report_callback;
	
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
		
		fvkCreateDebugReportCallbackEXT = NULL;
		fvkDestroyDebugReportCallbackEXT = NULL;
	}
} vkGlobals;

char app_name[] = "Vulkan Tutorian. Commands. © GrWolf.";

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
	//Настроем вывод.
	setlocale(LC_ALL, "Russian");
	PrepareLayersAndExtensions(); //настройка слоёв и расширений
	if (!InitInstance()) //инициализация экземпляра
		return 1; //при ошибках будем выдавать уникальный код.
	#ifdef __DEBUG // Макрос объявлен в настройках проекта
	if (!InitDebug()) //инициализация отладки
		return 2; //
	#endif
	if (!InitDevice()) //инициализация устройства
		return 3;
	
	std::cout << "Working.\n";
	

	DestroyDevice(); //разрушение устройства
	#ifdef __DEBUG // Макрос объявлен в настройках проекта
	DestroyDebug(); //отключение отладки
	#endif
	DestroyInstance(); //разрешение экземпляра
	return 0;
}

//Подготовка слоёв и расширений
void PrepareLayersAndExtensions()
{
	#ifdef __DEBUG // Макрос объявлен в настройках
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	device_layers.push_back("VK_LAYER_LUNARG_standard_validation");
	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	#endif
}


bool InitInstance()
{
	VkApplicationInfo app_info; 
	memset(&app_info, 0, sizeof(app_info));
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name;
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 5);
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	
	VkInstanceCreateInfo instance_info;
	memset(&instance_info, 0, sizeof(instance_info));
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	
	instance_info.enabledLayerCount = vkGlobals.instance_layers.size();
	instance_info.ppEnabledLayerNames = vkGlobals.instance_layers.data();
	
	instance_info.enabledExtensionCount = vkGlobals.instance_extensions.size();
	instance_info.ppEnabledExtensionNames = vkGlobals.instance_extensions.data();
	
	if (vkCreateInstance(&instance_info, VK_NULL_HANDLE, &vkGlobals.instance) != VK_SUCCESS)
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
	memset(&debug_report_callback_info, 0, sizeof(debug_report_callback_info));
	debug_report_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_report_callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	debug_report_callback_info.pfnCallback = DebugReportCallback;
	
	//Создание callback
	if (vkGlobals.fvkCreateDebugReportCallbackEXT(vkGlobals.instance, &debug_report_callback_info,
		VK_NULL_HANDLE, &vkGlobals.debug_report_callback) != VK_SUCCESS)
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
	
	//Настройка очередей
	VkDeviceQueueCreateInfo device_queue_info;
    memset(&device_queue_info, 0, sizeof(device_queue_info));
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	float device_queue_priority[] = {1.0f};
	device_queue_info.queueCount = 1; 
	device_queue_info.queueFamilyIndex = valid_family_index; 
	device_queue_info.pQueuePriorities = device_queue_priority;
	
	//Настройка девайса
	VkDeviceCreateInfo device_info; 
    memset(&device_info, 0, sizeof(device_info)); 
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; 
	device_info.queueCreateInfoCount = 1; 
	device_info.pQueueCreateInfos = &device_queue_info;
	device_info.enabledLayerCount = vkGlobals.device_layers.size();
	device_info.ppEnabledLayerNames = vkGlobals.device_layers.data();
	device_info.enabledExtensionCount = vkGlobals.device_extensions.size();
	device_info.ppEnabledExtensionNames = vkGlobals.device_extensions.data();
	
	//Создание девайса
	if (vkCreateDevice(gpu, &device_info, VK_NULL_HANDLE, &vkGlobals.device) != VK_SUCCESS)
	{
		std::wcerr << L"Создать устройство не удалось.\n";
		return false;
	}
	return true;
}

void DestroyInstance()
{
	vkDestroyInstance(vkGlobals.instance, VK_NULL_HANDLE);
}

void DestroyDebug()
{
	vkGlobals.fvkDestroyDebugReportCallbackEXT(vkGlobals.instance, vkGlobals.debug_report_callback, VK_NULL_HANDLE);
}

void DestroyDevice()
{
	vkDestroyDevice(vkGlobals.device, VK_NULL_HANDLE);
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
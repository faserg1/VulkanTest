/* В этом уроке: подробнее про слои и расширения Vulkan.
 * Все интересующие вас подробности смотрите здесь:
 * https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf
 * © Серый Волк.
*/

#include <iostream> //Ввод-вывод информации.
#include <string.h> //Необходимые инструменты (memset)
#include <vector> //Удобно храним массивы
#include <cstdlib> //exit
#include <vulkan/vulkan.h> // Vulkan API

//Имя приложения.
char app_name[] = "Vulkan Tutorian. Layers And Extensions. © GrWolf.";

//Прототип функции для debug-report'а.
VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT,VkDebugReportObjectTypeEXT,
	uint64_t,size_t,int32_t,const char*,const char*,void*);

int main()
{
	//Настроем вывод.
	setlocale(LC_ALL, "Russian");
	//Подготовим данные приложения
	VkApplicationInfo app_info;
	memset(&app_info, 0, sizeof(app_info));
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name;
	#ifdef VK_API_VERSION_1_0
	app_info.apiVersion = VK_API_VERSION_1_0;
	#else
	app_info.apiVersion = VK_API_VERSION;
	#endif
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 25);

	VkResult res; //заранее подготовим переменную для результата, она нам понадобится несколько раз.

	//Данные экземпляра
	VkInstanceCreateInfo instance_info;
	memset(&instance_info, 0, sizeof(instance_info));
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	uint32_t available_instance_layer_count = 0;
	//узнаём кол-во установленных слоёв.
	res = vkEnumerateInstanceLayerProperties(&available_instance_layer_count, nullptr);
	if (res != VK_SUCCESS)
	{
		std::wcout << "Не удалось получить кол-во слоёв.\n";
		return -1;
	}
	//настраиваем массив
	std::vector<VkLayerProperties> available_instance_layers(available_instance_layer_count);
	//Получаем слои
	res = vkEnumerateInstanceLayerProperties(&available_instance_layer_count, available_instance_layers.data());
	if (available_instance_layer_count)
	{
		//А теперь выводим на экран.
		std::wcout << L"\n\nСлои экземпляра:\n";
		for (uint32_t i = 0; i < available_instance_layer_count; i++)
		{
			VkLayerProperties &properties = available_instance_layers[i];
			std::cout << properties.layerName << (strlen(properties.layerName) < 24 ? "\t" : "")
			<< (strlen(properties.layerName) < 32 ? "\t" : "")
			<< "\t|" << properties.description << "\n";
		}
	}
	std::cout << "\n\n";

	std::vector<const char *> instance_layers;
	instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");


	//И затем отправим их в стурктуру с информацией.
	instance_info.enabledLayerCount = instance_layers.size();
	instance_info.ppEnabledLayerNames = instance_layers.data();

	//Кол-во расширений.
	uint32_t available_instance_extension_count = 0;
	//Забираем кол-во расширений.
	res = vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extension_count, nullptr);
	if (res != VK_SUCCESS) //Проверка
	{
		std::wcout << "Не удалось получить кол-во расширений.\n";
		return -1;
	}

	//настраиваем массив
	std::vector<VkExtensionProperties> available_instance_extensions(available_instance_extension_count);
	//Получаем расширения
	res = vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extension_count, available_instance_extensions.data());
	if (available_instance_extension_count)
	{
		//Вывод на экран.
		std::wcout << L"\n\nРасширения экземпляра:\n";
		for (uint32_t i = 0; i < available_instance_extensions.size(); i++)
		{
			VkExtensionProperties &properties = available_instance_extensions[i];
			std::cout << properties.extensionName << "\n";
		}
		std::cout << "\n\n";
	}

	//Настройка расширений
	std::vector<const char *> instance_extensions;

	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	instance_info.enabledExtensionCount = instance_extensions.size();
	instance_info.ppEnabledExtensionNames = instance_extensions.data();

	VkInstance instance = VK_NULL_HANDLE;
	res = vkCreateInstance(&instance_info, NULL, &instance);
	if (res != VK_SUCCESS) //С проверками особо заморачиваться не будем.
	{
		std::wcerr << L"Что-то не так...\n";
		return -1;
	}
	else
		std::wcout << L"Экземпляр Vulkan создан.\n";

	//Создадим указатели на функции таким образом.
	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = NULL;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = NULL;

	//И получим их.
	fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");

	/* Отлично, мы получили эти функции! Теперь, нам нужно подготовить информацию для callback'ов, а также
	 * оставить хэндл, который мы конечно же потом уничтожим.
	*/

	//Структура, которую мы должны заполнить
	VkDebugReportCallbackCreateInfoEXT debug_report_callback_info;
	memset(&debug_report_callback_info, 0, sizeof(debug_report_callback_info));
	debug_report_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	debug_report_callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	// Настало время прикрепить наш прототип здесь:
	debug_report_callback_info.pfnCallback = DebugReportCallback;

	VkDebugReportCallbackEXT debug_report_callback = VK_NULL_HANDLE;
	//И наконец-таки привзяываем наш Callback:
	res = fvkCreateDebugReportCallbackEXT(instance, &debug_report_callback_info, NULL, &debug_report_callback);
	if (res != VK_SUCCESS)
	{
		std::wcerr << L"Не удалось создать debug-report callback.\n";
		return -1;
	}

	std::vector<VkPhysicalDevice> gpu_list; //здесь будем хранить физические устройства.
	uint32_t gpu_count; //кол-во девайсов
	//получаем колв-о девайсов и сразу проверяем на удачу.
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		std::wcerr << L"Посчитать физические устройства не удалось :(\n";
		return -1;
	}
	gpu_list.resize(gpu_count); //заранее изменим размер под самый оптимальный.
	//Забираем физические девайсы
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
	{
		std::wcerr << L"Заполучить твои физические устройства не удалось, но я приду за ними в следующий раз!\n";
		return -1;
	}
	//Выбираем видеокарту
	VkPhysicalDevice gpu = gpu_list[0];
	//Теперь, семейства.
	uint32_t family_count = 0; //кол-во семейтсв
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, nullptr);
	std::vector<VkQueueFamilyProperties> family_properties_list(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_properties_list.data());
	//Листаем семейтсва и получаем нужное.
	uint32_t valid_family_index = (uint32_t) -1; //значение -1 будем использовать как "не найдено, 404".
	for (uint32_t i = 0; i < family_count; i++) //листаем все семейства.
	{
		VkQueueFamilyProperties &properties = family_properties_list[i];
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (valid_family_index == (uint32_t) -1)
				valid_family_index = i;
		}
	}
	//И если наш индекс всё ещё не перезаписан, то..
	if (valid_family_index == (uint32_t) -1)
		return -1;

	//Описываем очереди.
	VkDeviceQueueCreateInfo device_queue_info;
    memset(&device_queue_info, 0, sizeof(device_queue_info));
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

	float device_queue_priority[] = {1.0f};
	device_queue_info.queueCount = 1;
	device_queue_info.queueFamilyIndex = valid_family_index;
	device_queue_info.pQueuePriorities = device_queue_priority;

	VkDeviceCreateInfo device_info;
    memset(&device_info, 0, sizeof(device_info));
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &device_queue_info;

	uint32_t available_device_layer_count = 0;
	res = vkEnumerateDeviceLayerProperties(gpu, &available_device_layer_count, VK_NULL_HANDLE);
	if (res != VK_SUCCESS)
	{
		std::wcout << "Не удалось получить кол-во слоёв.\n";
		return -1;
	}
	//настраиваем массив
	std::vector<VkLayerProperties> available_device_layers(available_device_layer_count);
	res = vkEnumerateDeviceLayerProperties(gpu, &available_device_layer_count, available_device_layers.data());
	if (res != VK_SUCCESS)
	{
		std::wcout << "Не удалось получить слои.\n";
		return -1;
	}
	if (available_device_layer_count)
	{
		//А теперь выводим на экран.
		std::wcout << L"\n\nСлои устройства:\n";
		for (uint32_t i = 0; i < available_device_layer_count; i++)
		{
			//Сделаем красивый вывод для имени и описания слоя.
			VkLayerProperties &properties = available_device_layers[i];
			std::cout << properties.layerName << (strlen(properties.layerName) < 24 ? "\t" : "")
			<< (strlen(properties.layerName) < 32 ? "\t" : "")
			<< "\t|" << properties.description << "\n";
		}
	}
	std::cout << "\n\n";

	//Кол-во расширений.
	uint32_t available_device_extension_count = 0;
	//Забираем кол-во расширений.
	res = vkEnumerateDeviceExtensionProperties(gpu, NULL, &available_device_extension_count, VK_NULL_HANDLE);
	if (res != VK_SUCCESS) //Проверка
	{
		std::wcout << "Не удалось получить кол-во расширений.\n";
		return -1;
	}

	//настраиваем массив
	std::vector<VkExtensionProperties> available_device_extensions(available_device_extension_count);
	//Получаем расширения
	res = vkEnumerateDeviceExtensionProperties(gpu, NULL, &available_device_extension_count, available_device_extensions.data());
	if (available_device_extension_count)
	{
		//Вывод на экран.
		std::wcout << L"\n\nРасширения устройства:\n";
		for (uint32_t i = 0; i < available_device_extensions.size(); i++)
		{
			VkExtensionProperties &properties = available_device_extensions[i];
			std::cout << properties.extensionName << "\n";
		}
		std::cout << "\n\n";
	}

	std::vector<const char *> device_layers = instance_layers;

	//Расширения и слои
	device_info.enabledLayerCount = device_layers.size();
	device_info.ppEnabledLayerNames = device_layers.data();

	//А расширения устройства оставим и на сей раз пустым.
	std::vector<const char *> device_extensions;

	device_info.enabledExtensionCount = device_extensions.size();
	device_info.ppEnabledExtensionNames = device_extensions.data();
	VkDevice device = VK_NULL_HANDLE;
	if (vkCreateDevice(gpu, &device_info, NULL, &device) != VK_SUCCESS)
		return -1;

	vkDestroyDevice(device, NULL);
	fvkDestroyDebugReportCallbackEXT(instance, debug_report_callback, NULL);
	vkDestroyInstance(instance, NULL);
	std::wcout << L"Пожарено!\n";
	return 0;
}

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
	//Сделаем простенький вывод:
	std::cout << "[" << pLayerPrefix << "] " << pMessage << std::endl;
	return false;
}
/* Готово. Разобрались.
 *
 * В общем, спасибо, что уделили этому уроку время. Khronos Group, Niko Kauppi, и тем, кто делает примеры — Большое Спасибо!
 * А дальше классика:
 * Если вы хотите меня поддержать, прикрепляю кошель с Я.Денег:
 * 410012557544062
 * Ну или просто жмя по ссылке:
 * https://money.yandex.ru/to/410012557544062
 * Хе-хе.
*/

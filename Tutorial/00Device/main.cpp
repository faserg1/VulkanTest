/* В этом уроке будет показано создание Vulkan Device & Vulkan Instance.
 * Каждый из уроков также будет обильно насыщен комментариями.
 * Также будут небольшие пояснения, зачем нужны те или иные вещи в Vulkan.
 * Все интересующие вас подробности смотрите здесь:
 * https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf
 * © Серый Волк.
*/

#include <iostream> //Ввод-вывод информации.
#include <string.h> //Необходимые инструменты (memset)
#include <vector> //Удобно храним массивы
#include <cstdlib> //exit
#include <vulkan/vulkan.h>

//Имя нашего приложения. Вывел в глобальные.
char app_name[] = "Vulkan Tutorian. Device. © GrWolf.";

inline void getVersion(uint32_t version, uint32_t &major, uint32_t &minor, uint32_t &patch)
{
	major = VK_VERSION_MAJOR(version);
	minor = VK_VERSION_MINOR(version);
	patch = VK_VERSION_PATCH(version);
}

int main()
{
	//Прежде всего, настроем вывод.
	setlocale(LC_ALL, "Russian");

	//Информация о приложении

	VkApplicationInfo app_info; //Вот структура, которая содержит всю эту информацию.
	memset(&app_info, 0, sizeof(app_info)); //Очистим структуру (заполним нулями).
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name;
	#ifdef VK_API_VERSION_1_0
	app_info.apiVersion = VK_API_VERSION_1_0;
	#else
	app_info.apiVersion = VK_API_VERSION;
	#endif
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 35);
	//Информация о экземпляре

	VkInstanceCreateInfo instance_info; //Создаём...
	memset(&instance_info, 0, sizeof(instance_info)); //... и заполним её нулями.
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; //зададим тип структуры.
	instance_info.pApplicationInfo = &app_info;

	//Создание экземпляра

	VkResult create_instance_result; //тут будем хранить  и обрабатывать результат.
	VkInstance instance = VK_NULL_HANDLE;
	create_instance_result = vkCreateInstance(&instance_info, NULL, &instance);

	if (create_instance_result != VK_SUCCESS) //проверяем на удачу..
	{
		switch (create_instance_result) //.. воспользуемся стандартной переключалкой..
		{
			case VK_ERROR_INCOMPATIBLE_DRIVER: //..делаем case..
				std::wcerr << L"Дрова поставь!!!\n"; //..обрабатываем ошибку..
			default:
				std::wcerr << L"Что-то не так...\n"; //..ну и так далее..
		}
		return -1; //..ведь в конце концов приложение всё равно дало сбой.
	}
	else
		std::wcout << L"Экземпляр Vulkan создан.\n";

	//Физические устройства

	std::vector<VkPhysicalDevice> gpu_list; //здесь будем хранить
	uint32_t gpu_count; //кол-во девайсов
	//получаем кол-во девайсов и сразу проверяем на удачу.
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		std::wcerr << L"Посчитать физические устройства не удалось :(\n";
		return -1;
	}
	//тут мы можем немного поиздеваться над юзверями.
	std::wcout << L"Человек, я всё от тебе знаю!\n";
	std::wcout << L"Даже то, что у тебя " << gpu_count << L" видеокарт.\n";
	gpu_list.resize(gpu_count); //заранее изменим размер под самый оптимальный.
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
	{
		std::wcerr << L"Заполучить твои физические устройства не удалось, но я приду за ними в следующий раз!\n";
		return -1;
	}
	//Свойства устройоств
	VkPhysicalDeviceProperties prop;
	for (size_t i = 0; i < gpu_count; i++)
	{
		vkGetPhysicalDeviceProperties(gpu_list[i], &prop);
		std::wcout << L"Смотрим на ";
		std::cout << prop.deviceName << ".\n";
		uint32_t major, minor, patch;
		getVersion(prop.driverVersion, major, minor, patch);
		std::wcout << L"Драйвер версии " << major << L"." << minor << L"." << patch << ".\n";
		getVersion(prop.apiVersion, major, minor, patch);
		std::wcout << L"Поддержка API версии " << major << L"." << minor << L"." << patch << ".\n";
	}

	//Семейства

	/* Так как это хэндл (на подобии HWND в Win32 API) не является каким-либо объектом класса,
	 * а является указателем..
	*/
	VkPhysicalDevice gpu = gpu_list[0]; //мы просто копируем его.
	uint32_t family_count = 0; //Кол-во семейств.
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, nullptr);
	std::wcout << L"Я нашёл " << family_count << L" семейств. Похвали меня!\n";
	std::vector<VkQueueFamilyProperties> family_properties_list(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_properties_list.data());
	/* Теперь, можно бы и пролистать их и посмотреть на свойства, но для начала, нам нужно кое-что, что
	 * не потеряет индекс семейства, который может поддерживать графику.
	*/
	uint32_t valid_family_index = (uint32_t) -1; //значение -1 будем использовать как "не найдено".
	for (uint32_t i = 0; i < family_count; i++) //листаем все семейства.
	{
		VkQueueFamilyProperties &properties = family_properties_list[i];
		std::wcout << L"В семействе с индексом " << i << L" существует ";
		//queueCount показывает, сколько всего очередей есть у этого семейства.
		std::wcout << properties.queueCount << L" очередей.\n";
		std::wcout << L"Они поддерживают:\n";
		//queueFlags — флаг с битами, поэтому используем так:
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			std::wcout << L"Графику\n";
			if (valid_family_index == (uint32_t) -1)
				valid_family_index = i;
		}
		if (properties.queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			std::wcout << L"Вычисления\n";
		}
		if (properties.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			std::wcout << L"Копирование\n";
		}
		if (properties.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
		{
			std::wcout << L"Редкие рерсурсы\n";
		}
	}
	//И если наш индекс всё ещё не перезаписан, то..
	if (valid_family_index == (uint32_t) -1)
	{
		std::wcerr << L"Мы не нашли подходящее семейство! Сворачиваемся!\n";
		return -1;
	}
	else //Ну а иначе всё хорошо..
	{
		std::wcout << L"Мы будем использовать " << valid_family_index << L" индекс.\n";
	}

	//Очереди

	/* Хорошо, теперь мы нашли наш индекс волшебства, теперь, прежде чем приступать к
	 * созданию логического устройства, нам нужно создать для него очереди. Как же без них?
	 * Иначе нам просто некуда будет отсылать команды! Для начала создадим структуру.
	*/
	VkDeviceQueueCreateInfo device_queue_info;
    memset(&device_queue_info, 0, sizeof(device_queue_info));
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	float device_queue_priority[] = {1.0f};
	device_queue_info.queueCount = 1;
	device_queue_info.queueFamilyIndex = valid_family_index; //укажем индекс семейства этих очередей
	//и помни — массив, это указатель на первый слот самого массива.
	device_queue_info.pQueuePriorities = device_queue_priority;

	//Устройство

	VkDeviceCreateInfo device_info; //Создаём.
    memset(&device_info, 0, sizeof(device_info)); //Очищаем.
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; //Задаём.
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &device_queue_info;

	//Настало время создать логическое устройство.
	VkDevice device = VK_NULL_HANDLE;
	if (vkCreateDevice(gpu, &device_info, NULL, &device) != VK_SUCCESS)
	{
		std::wcerr << L"Чёрт! А я был так близко...\n";
		return -1;
	}
	std::wcout << L"Ура! Получилось! Device наш!\n";

	//Разрушение

	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
	std::wcout << L"Пожарено!\n";
	return 0;
}

/*
 * В общем, спасибо, что уделили этому уроку время.
 * Khronos Group, Niko Kauppi, и тем, кто делает примеры — Большое Спасибо!
 * А дальше классика:
 * Если вы хотите меня поддержать, прикрепляю кошель с Я.Денег:
 * 410012557544062
 * Ну или просто по ссылке:
 * https://money.yandex.ru/to/410012557544062
 * Хе-хе.
*/

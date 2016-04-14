/* В этом уроке раскажу подробнее про слои Vulkan на примере Google & LunarG
 * Validation Layers (проверочные слои), т.е. фактически — отладочные.
 * Этот урок также будет с большим количеством комментариев, но тем не меннее,
 * для изученного будет уделён лишь небольшой комментарий.
 * Все интересующие вас подробности смотрите здесь:
 * https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf
 * © Серый Волк.
*/

#include <iostream> //Ввод-вывод информации.
#include <string.h> //Необходимые инструменты (memset)
#include <vector> //Удобно храним массивы
#include <cstdlib> //exit
#include <vulkan/vulkan.h> // Vulkan API

//Имя нашего приложения. Вывел в глобальные.
char app_name[] = "Vulkan Tutorian. Layers And Extensions. © GrWolf.";

/* Придётся потихоньку забегать вперёд, ведь сегодня речь пойдёт о fetch-функциях,
 * callback'ах, отладке и так далее.
 * Вначале я создам здесь прототип функции для debug-report'а. Но подробнее о нём будет ниже.
*/ 

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
	/* Можно использовать новую версию таким способом, но учтите, что это влияет только на драйвер.
	 * Поэтому, если у вас не работает из-за старых драйверов, можно опустить версию до 1.0.3
	*/ 
	app_info.apiVersion = VK_MAKE_VERSION(1, 0, 5);
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	
	VkResult res; //заранее подготовим переменную для результата, она нам понадобится несколько раз.
	
	//Данные экземпляра
	VkInstanceCreateInfo instance_info;
	memset(&instance_info, 0, sizeof(instance_info));
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;

	/* И дальше начинается самое интересное — включение слоёв и расширений.
	 * Для начала посмотрим, какие слои вообще установлены в систему. Для этого воспользуемся функцией
	 * vkEnumerateInstanceLayerProperties, которая работает, как и все остальные vkEnumerate*:
	 * Первый раз вызываем, чтобы посчитать кол-во, второй раз — забираем всё.
	 * Конечно же, можно и подругому, но в данном случае этот вариант самый оптимальный и разумный.
	 * И да,
	*/
	uint32_t available_instance_layer_count = 0;
	//узнаём кол-во установленных слоёв.
	res = vkEnumerateInstanceLayerProperties(&available_instance_layer_count, VK_NULL_HANDLE);
	if (res != VK_SUCCESS)
	{
		std::wcout << "Не удалось получить кол-во слоёв.\n";
		exit(1);
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
			/* Структура VkLayersProperties имеет четыре члена, причём, всё настолько запутано, что даже в спецификации
			 * немного накосячили, а man вообще пуст. Я надеюсь, эти косяки быстро уберут.
			 * Тем не менее, можно разобраться методом тыка. И так:
			 * layerName — имя слоя.
			 * implementationVersion — его версия, да, его версия, а не версия Vulkan API, на которую он расчитан.
			 * description — описание слоя, даже в UTF-8.
			 * specVersion — версия Vulkan API, на которую он расчитан.
			 * К слову, если версия Vulkan API сделана через макросы, то версия как слоёв, так и расширений написана
			 * вручную, причём, пока ещё непонятным образом, как её надо читать.
			 * Сделаем красивый вывод для имени и описания слоя.
			*/
			VkLayerProperties &properties = available_instance_layers[i];
			std::cout << properties.layerName << (strlen(properties.layerName) < 24 ? "\t" : "")
			<< (strlen(properties.layerName) < 32 ? "\t" : "")
			<< "\t|" << properties.description << "\n";
		}
	}
	std::cout << "\n\n";
	
	/* Лично я получил такой результат (после установки нового SDK, версии 1.0.5):
	 * Слои экземпляра:
	 * VK_LAYER_LUNARG_api_dump                |LunarG debug layer
	 * VK_LAYER_LUNARG_device_limits           |LunarG Validation Layer
	 * VK_LAYER_LUNARG_draw_state              |LunarG Validation Layer
	 * VK_LAYER_LUNARG_image                   |LunarG Validation Layer
	 * VK_LAYER_LUNARG_mem_tracker             |LunarG Validation Layer
	 * VK_LAYER_LUNARG_object_tracker          |LunarG Validation Layer
	 * VK_LAYER_LUNARG_param_checker           |LunarG Validation Layer
	 * VK_LAYER_LUNARG_screenshot              |LunarG image capture layer
	 * VK_LAYER_LUNARG_swapchain               |LunarG Validation Layer
	 * VK_LAYER_GOOGLE_threading               |Google Validation Layer
	 * VK_LAYER_GOOGLE_unique_objects          |Google Validation Layer
	 * VK_LAYER_LUNARG_vktrace                 |Vktrace tracing library
	 * VK_LAYER_RENDERDOC_Capture              |Debugging capture layer for RenderDoc
	 * VK_LAYER_VALVE_steam_overlay            |Steam Overlay Layer
	 * VK_LAYER_LUNARG_standard_validation     |LunarG Standard Validation Layer
	 * 
	 * И да, имена слоёв могут различаться в зависимости от версии Vulkan, но об этом чуть позже.
	 * Validation слои нужны фактически для отслеживания ошибок, подробности об этом возможно будут позже.
	 * В общем, штука хорошая. И главное — по умолчанию, все эти слои ОТКЛЮЧЕНЫ.
	 * Т.е. если вы выпускаете игру (или что там у вас?) в релиз — можно просто не включать отладочные и проверочные слои,
	 * чтобы не было лишней нагрузки. Если же вы хотите отладить проект — определённые слои можно включить, но с одной оговоркой:
	 * для instance нужны либо все стандартные проверочные слои, либо ни один.
	 * И так, более подробно о стандартном наборе. В Vulkan есть специально сокращение для слоёв:
	 * VK_LAYER_LUNARG_standard_validation, которое включает за собой стандартный набор слоёв с оптимально подобраным порядком.
	 * Перечислю имена, которые входят в этот набор:
	 * VK_LAYER_LUNARG_threading (после 1.0.5 — VK_LAYER_GOOGLE_threading)
	 * VK_LAYER_LUNARG_param_checker
	 * VK_LAYER_LUNARG_device_limits
	 * VK_LAYER_LUNARG_object_tracker
	 * VK_LAYER_LUNARG_image
	 * VK_LAYER_LUNARG_mem_tracker
	 * VK_LAYER_LUNARG_draw_state
	 * VK_LAYER_LUNARG_swapchain
	 * VK_LAYER_GOOGLE_unique_objects
	 * 
	 * Эти слои мы вскоре вручную добавим в наш экземпляр Vulkan. И так, снова предупреждаю, либо каждый из этих слоёв
	 * (или просто VK_LAYER_LUNARG_standard_validation), плюс можно добавить дополнительные, либо ни один из них.
	 * Почему так — ищите ответ сами.
	 * Теперь о работе. Каждый слой мы будем добавлять сверху вниз, как и обычно, тем не менее, работать они будут снизу вверх.
	 * А именно: вы вызвали функцию, после этого вызов обрабатывается слоём VK_LAYER_GOOGLE_unique_objects, потом выше и выше,
	 * пока не достигнет VK_LAYER_LUNARG_threading/VK_LAYER_GOOGLE_threading, а затем вызов уйдёт в ядро Vulkan.
	 * 
	 * Ну что, пробуем! Для начало нам нужно хранить эжи слои в массиве строк. В общем, как обычно, используем вектор.
	*/ 
	std::vector<const char *> instance_layers;
	
	/* Пару слов о том, как выбираются слои:
	 * После того, как я поставил новый Vulkan API SDK, появились новые слои, и появились изменения.
	 * Например, теперь я должен писать не LUNARG_threading, а GOOGLE_threading. Если что — откоментируйте
	 * то, что нужно, закомментируйте ненужное.
	*/
	
	instance_layers.push_back("VK_LAYER_GOOGLE_threading");
	//instance_layers.push_back("VK_LAYER_LUNARG_threading");
	instance_layers.push_back("VK_LAYER_LUNARG_param_checker");
	instance_layers.push_back("VK_LAYER_LUNARG_device_limits");
	instance_layers.push_back("VK_LAYER_LUNARG_object_tracker");
	instance_layers.push_back("VK_LAYER_LUNARG_image");
	instance_layers.push_back("VK_LAYER_LUNARG_mem_tracker");
	instance_layers.push_back("VK_LAYER_LUNARG_draw_state");
	instance_layers.push_back("VK_LAYER_LUNARG_swapchain");
	instance_layers.push_back("VK_LAYER_GOOGLE_unique_objects");
	
	
	//И затем отправим их в стурктуру с информацией.
	instance_info.enabledLayerCount = instance_layers.size();
	instance_info.ppEnabledLayerNames = instance_layers.data();
	
	/* Хорошо, теперь давайте разберёмся с расширениями! И с ними не так всё просто. По умолчанию, Vulkan Loader даёт
	 * только ядро Vulkan + частично WSI. И чтобы использовать эти расширения, необходимо получить (fetch) функции из них.
	 * Но об этом чуть попозже. Давайте для начала посмотрим, какие у нас есть расширения.
	 * Для этого у нас конечно же есть функция vkEnumerateInstanceExtensionProperties, но теперь она чуть-чуть другая:
	 * первым параметром можно опционально задать определённый слой, для которого мы будем искать расширения. Но в данном
	 * случае нам пока что интересны все.
	*/
	//Кол-во расширений.
	uint32_t available_instance_extension_count = 0;
	//Забираем кол-во расширений.
	res = vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extension_count, VK_NULL_HANDLE);
	if (res != VK_SUCCESS) //Проверка
	{
		std::wcout << "Не удалось получить кол-во расширений.\n";
		exit(1);
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
			/* Структура VkExtensionProperties имеет два члена: extensionName, соответсвенно имя расширения,
			 * и specVersion, его версию. Странно, да? Со слоями было всё иначе!
			 * Выводим имя.
			*/
			VkExtensionProperties &properties = available_instance_extensions[i];
			std::cout << properties.extensionName << "\n";
		}
		std::cout << "\n\n";
	}
	
	/* Теперь посмотрим, что есть из расширений:
	 * Расширения экземпляра:
	 * VK_KHR_surface
	 * VK_KHR_win32_surface
	 * VK_EXT_debug_report
	 * Отлично, теперь немного опишу, что это за расширения. Перове — вывод на поверхность. Например, на экран,
	 * окно или ещё куда-нибудь. Но к какой либо оконной системе оно не привязано, это лишь базовое расширение.
	 * Следующее как раз позволяет прикрепить эту поверхность к окну Win32, т.е. Windows окну (масло маслянное).
	 * И последнее, позволяет делать debug-report'ы. Ну, почему бы и нет? Давайте сделаем.
	*/ 
	
	//Подготовим массив
	std::vector<const char *> instance_extensions;
	
	/*И добавим расширение для отладочных репортов, используя макроимя.
	 * Оно, как и полагается, содержит имя необходимого нам расширения.
	*/
	instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	
	//Настроим наши расширения.
	instance_info.enabledExtensionCount = instance_extensions.size();
	instance_info.ppEnabledExtensionNames = instance_extensions.data();
	
	/* Отлично. Теперь, чтобы работать с этим расширеним, нам уже нужно создать экземпляр Vulkan,
	 * чтобы впоследствии через Vulkan Loader получить функции этого расширения. Поехали.
	*/
	
	VkInstance instance = VK_NULL_HANDLE;
	res = vkCreateInstance(&instance_info, NULL, &instance);
	if (res != VK_SUCCESS) //С проверками особо заморачиваться не будем.
	{
		std::wcerr << L"Что-то не так...\n"; 
		exit(1); 
	}
	else
		std::wcout << L"Экземпляр Vulkan создан.\n";
	
	/* Отлично, теперь эти репорты надо настроить. Для этого воспользуемся функцией vkGetInstanceProcAddr.
	 * Первый параметр — наш экземпляр, второй — имя функции, которую мы хотим получить.
	 * К слову говоря, подключать Vulkan можно динамическим путём. Через эту функцию можно получить и те,
	 * которые не работают с экземпляром. Например, вышеиспользованные функции слоёв, расширений и создания
	 * самого экземпляра можно получить, если оставить первый параметр пустым. Все остальные функции,
	 * которые используют экземпляр, получается с указанием экземпляра. Но обычно все функции доступны
	 * сразу же при линковке Vulkan. Все, кроме функций расширений. Настало время их получить.
	 * Для этого нам уже немного упростили жизнь, создав специальные типы указателей на функции.
	 * Причём, на ВСЕ функции. Можете сами посмотреть.
	*/
	
	//Создадим указатели на функции таким образом.
	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = NULL;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = NULL;
	
	//И получим их.
	fvkCreateDebugReportCallbackEXT = 
		(PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	fvkDestroyDebugReportCallbackEXT =
		(PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	
	/* Отлично, мы получили эти функции! Теперь, нам нужно подготовить информацию для callback'ов, а также
	 * оставить хэндл, который мы конечно же потом уничтожим.
	*/
	
	//Структура, которую мы должны заполнить
	VkDebugReportCallbackCreateInfoEXT debug_report_callback_info;
	memset(&debug_report_callback_info, 0, sizeof(debug_report_callback_info));
	debug_report_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT; //сразу зададим тип структуры
	
	/* И так, флаги! VkDebugReportFlagsEXT Даёт нам следующие флаги:
	 * VK_DEBUG_REPORT_INFORMATION_BIT_EXT — информационные сообщения.
	 * VK_DEBUG_REPORT_WARNING_BIT_EXT — предупреждения
	 * VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT — предупреждения о производительности
	 * VK_DEBUG_REPORT_ERROR_BIT_EXT — ошибки
	 * VK_DEBUG_REPORT_DEBUG_BIT_EXT — отладочные сообщения
	 * 
	 * Самое время включить отладку на полную!
	*/
	debug_report_callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
		VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
	// Настало время прикрепить наш прототип здесь:
	debug_report_callback_info.pfnCallback = DebugReportCallback;
	/* pNext зарезервирован, как всегда. Также, можно всегда прикрепить любые свои данные
	 * через void *pUserData, об этом тоже можно сразу догадаться.
	 * Тем не менее, сейчас man на эту тему вообще умалчивает.
	 * Создаём хэндл:
	*/ 
	VkDebugReportCallbackEXT debug_report_callback = VK_NULL_HANDLE;
	//И наконец-таки привзяываем наш Callback:
	res = fvkCreateDebugReportCallbackEXT(instance, &debug_report_callback_info, NULL, &debug_report_callback);
	if (res != VK_SUCCESS)
	{
		std::wcerr << L"Не удалось создать debug-report callback.\n"; 
		exit(1); 
	}
	/* Отлично, мы привязали callback. Он  распространяется по всему Vulkan, поэтому привязывать ещё раз ничего не надо.
	 * Если быть более точным, то это расширение привязывается к слоям, причём, не каждый слой может поддерживать
	 * это расширение. Саму привязку allback'а к слоям можно увидеть в выводе: для каждого слоя будет размещено сообщение
	 * [DebugReport] Added callback. И да, теоритически, Vulkan Core тоже является слоем, при этом, в любом случае самым
	 * последним. Мы лишь можем добавить эти слои в середину между нашим приложением и самим ядром Vulkan.
	 * 
	 * Теперь пора вновь взяться за создание нашего устройства. Менять ничего не будем, только привяжем новые слои и
	 * расширения.
	*/ 
	std::vector<VkPhysicalDevice> gpu_list; //здесь будем хранить физические устройства.
	uint32_t gpu_count; //кол-во девайсов
	//получаем колв-о девайсов и сразу проверяем на удачу.
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		std::wcerr << L"Посчитать физические устройства не удалось :(\n";
		exit(1);
	}
	std::wcout << L"У тебя " << gpu_count << L" видеокарт.\n";
	gpu_list.resize(gpu_count); //заранее изменим размер под самый оптимальный.
	//Забираем физические девайсы
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
	{
		std::wcerr << L"Заполучить твои физические устройства не удалось, но я приду за ними в следующий раз!\n";
		exit(1);
	}
	//Выбираем видеокарту
	VkPhysicalDevice gpu = gpu_list[0];
	//Теперь, семейства.
	uint32_t family_count = 0; //кол-во семейтсв
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, VK_NULL_HANDLE); //забираем кол-во
	std::wcout << L"Найдено " << family_count << L" семейств.\n";
	//Создадим буфер, который будет хранить свойства этих семейств.
	std::vector<VkQueueFamilyProperties> family_properties_list(family_count);
	//И получим их все.
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_properties_list.data());
	//Листаем семейтсва и получаем нужное.
	uint32_t valid_family_index = (uint32_t) -1; //значение -1 будем использовать как "не найдено, 404".
	for (uint32_t i = 0; i < family_count; i++) //листаем все семейства.
	{
		VkQueueFamilyProperties &properties = family_properties_list[i];
		std::wcout << L"В семействе с индексом " << i << L" существует ";
		std::wcout << properties.queueCount << L" очередей.\n";
		std::wcout << L"Они поддерживают:\n";
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
		exit(1);
	}
	else //Ну а иначе всё хорошо..
	{
		std::wcout << L"Мы будем использовать " << valid_family_index << L" индекс.\n";
	}
	//Описываем очереди.
	VkDeviceQueueCreateInfo device_queue_info;
    memset(&device_queue_info, 0, sizeof(device_queue_info));
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	
	float device_queue_priority[] = {1.0f}; //Приоритет очередей
	device_queue_info.queueCount = 1; //кол-во
	device_queue_info.queueFamilyIndex = valid_family_index; //укажем индекс этих очередей
	device_queue_info.pQueuePriorities = device_queue_priority; //прикрепим приоритет

	//Теперт информация об устройстве
	VkDeviceCreateInfo device_info; //Создаём.
    memset(&device_info, 0, sizeof(device_info)); //Очищаем.
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; //Задаём.
	device_info.queueCreateInfoCount = 1; //Кол-во структур информации об очередях
	device_info.pQueueCreateInfos = &device_queue_info; //сами структуры
	
	/* Теперь снова к нашим баранам — расширения и слои. Так как мы в прошлый раз перечисляли все доступные,
	 * в этот раз сделаем тоже самое.
	*/
	uint32_t available_device_layer_count = 0;
	/* Только теперь первым параметром встанет наше физическое устройство. Причём, этот параметр является
	 * обязательным, так что по каждому устройству нужно будет смотреть возможные слои.
	 * Далее, как всегда, кол-во слоёв и сами слои. Сначала получим кол-во.
	*/ 
	res = vkEnumerateDeviceLayerProperties(gpu, &available_device_layer_count, VK_NULL_HANDLE);
	if (res != VK_SUCCESS)
	{
		std::wcout << "Не удалось получить кол-во слоёв.\n";
		exit(1);
	}
	//настраиваем массив
	std::vector<VkLayerProperties> available_device_layers(available_device_layer_count);
	res = vkEnumerateDeviceLayerProperties(gpu, &available_device_layer_count, available_device_layers.data());
	if (res != VK_SUCCESS)
	{
		std::wcout << "Не удалось получить слои.\n";
		exit(1);
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
	/* В данный момент у меня вывелось такое-же кол-во слоёв, что и приэкземпляре.
	 * Поэтому едем дальше — расширения.
	*/
	//Кол-во расширений.
	uint32_t available_device_extension_count = 0;
	//Забираем кол-во расширений.
	res = vkEnumerateDeviceExtensionProperties(gpu, NULL, &available_device_extension_count, VK_NULL_HANDLE);
	if (res != VK_SUCCESS) //Проверка
	{
		std::wcout << "Не удалось получить кол-во расширений.\n";
		exit(1);
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
	/* Список расширений теперь уже отличается:
	 * Расширения устройства:
	 * VK_KHR_swapchain
	 * VK_NV_glsl_shader
	 * VK_KHR_sampler_mirror_clamp_to_edge
	 * 
	 * Подробнее о последних двух будем говорить в следующих уроках.
	 * Теперь, давайте прикрепим слои к нашему устройству. Слои будут абсолютно те же, так что просто скопируем вектор.
	*/
	
	std::vector<const char *> device_layers = instance_layers;
	
	//Расширения и слои
	device_info.enabledLayerCount = device_layers.size();
	device_info.ppEnabledLayerNames = device_layers.data();
	
	//А расширения устройства оставим и на сей раз пустым.
	std::vector<const char *> device_extensions;
	
	device_info.enabledExtensionCount = device_extensions.size();
	device_info.ppEnabledExtensionNames = device_extensions.data();
	
	//Создаём пустой хэндл..
	VkDevice device = VK_NULL_HANDLE;
	//Создаём устройство
	if (vkCreateDevice(gpu, &device_info, NULL, &device) != VK_SUCCESS)
	{
		std::wcerr << L"Чёрт! А я был так близко...\n";
		exit(1);
	}
	std::wcout << L"Ура! Получилось! Device наш!\n";
	
	//*Здесь могла быть ваша реклама* Шучу. Здесь должен быть рендер и т.д.
	
	//Разрешаем устройство
	vkDestroyDevice(device, NULL);
	/* Разрушаем связь с Callback.
	 * Первый параметр — экземпляр.
	 * Второй — хэндл нашего debug-report callback.
	 * Третий — управление памятью. Оставим по умолчанию.
	*/ 
	fvkDestroyDebugReportCallbackEXT(instance, debug_report_callback, NULL);
	//Разрушаем экземпляр.
	vkDestroyInstance(instance, NULL);
	std::wcout << L"Пожарено!\n";
	//Готово. Теперь возвращаемся к функции callback.
	return 0;
}


/* И так, для того, чтобы функция работала везде и всегда, нужны для неё специальные приставки,
 * которые в свою очередь по разному опрделяются в vulkan_platform.h, т.е. в зависимости от платформы.
 * Разберёимся с параметрами:
*/ 

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT flags, //Это флаги нашего сообщения
	VkDebugReportObjectTypeEXT objectType, //Это тип объекта, который вызвал сообщение
	uint64_t object, //Сам объект, в основном указатель на него, хэндл
	size_t location, //Что это?! man и спецификация пустует, нигде нет информации. Khronos Group, вы где?!
	//(хотя, по идее, это может быть например строчка кода, где произошла ошибка)
	
	int32_t messageCode, //Код сообщения
	const char *pLayerPrefix, //Это префикс сообщения, или тэг, как иногда пишут
	const char *pMessage, //Само сообщение
	void *pUserData) //И конечно же ваши собственные данные, которые вы сюда привязали
{
	//Сделаем простенький вывод:
	std::cout << "[" << pLayerPrefix << "] " << pMessage << std::endl;
	/* По умолчанию мы всегда возвращаем false.
	 * Если вернуть true, о тогда команда, при котрой возникла ошибка просто не пойдёт дальше (в следующий слой или ядро Vulkan).
	 * Возвращение false позволяет командам работать дальше.
	*/ 
	return false;
}
/* Готово. Разобрались. Из-за того, что я пока что не нашёл более полного описания некоторых вещей, туториал не вышел столь
 * подробным, скорлько и предыдущий.
 * 
 * В общем, спасибо, что уделили этому уроку время. Khronos Group, Niko Kauppi, и тем, кто делает примеры — Большое Спасибо!
 * А дальше классика:
 * Если вы хотите меня поддержать, прикрепляю кошель с Я.Денег:
 * 410012557544062
 * Ну или просто жмя по ссылке:
 * https://money.yandex.ru/to/410012557544062
 * Хе-хе.
*/
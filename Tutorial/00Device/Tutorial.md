#Начало: подключение библиотеки.
И так, для начала разберёмся с подключением библиотек. После установки SDK у нас будут 3 папки, которые надо будет подключить для поиска заголовочных файлов и библиотек.
* Папка Include — в ней будет поиск заголовочных файлов.
* Папка Bin — в ней будет поиск библиотек (x86_64, т.е. для 64 битных приложений).
* Папка Bin32 — в ней будет поиск библиотек (x86, т.е. для 32 битных приложений).

После этого, нужно подключить одну библиотеку: vulkan-1. Конечно, для Android приложений нужно будет привязывать эту библиотеку динамически, но для всех остальных можно и просто подключить эту библиотеку.

Затем, в исходниках, где будет использоваться Vulkan, нужно подключить заголовочный файл *vulkan.h*:

	#include <vulkan/vulkan.h>

Всё, теперь можно приступать к работе.

#Создание экземпляра Vulkan. Кратко о получении функций.
Как уже говорилось ранее, не всегда функции, которые мы хотим использовать, могут быть получены в результате статической линковки. Поэтому есть функция `vkGetInstanceProcAddr`, которую нужно получить в первую очередь, используя системно-зависимые функции (например, `GetProcAddress` для *Win32 API*). Этой функцией можно получить все остальные функции, работающие с экземпляром и без него.
**Экземпляр** (*Instance*) можно получить из функции `vkCreateInstance`, но прежде, чем мы будем её использовать, нужно заполнить информацию о том, какой должен быть наш экземпляр. И в Vulkan для создания любого объекта нужно заполнять определённые структуры.
Первым делом, мы можем указать информацию о нашем приложении. Вот так выглядит эта структура:

	typedef struct VkApplicationInfo {
		VkStructureType sType;
		const void* pNext;
		const char* pApplicationName;
		uint32_t applicationVersion;
		const char* pEngineName;
		uint32_t engineVersion;
		uint32_t apiVersion;
	} VkApplicationInfo;
	
В каждой структуре всегда есть два члена: `sType`, а также `pNext`. Обо всём по порядку:
+ `sType` — тип структуры. В нашем случае мы должны указать тип `VK_STRUCTURE_TYPE_APPLICATION_INFO`, так мы скажем Vulkan, какие данные и где находятся в нашей структуре. Зачем постоянно указывать тип? Фишка в том, что для Vulkan есть расширения, и некоторые функции могут принимать структуры разного типа. Сами значения — беззнаковые целые, вплоть до `VK_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF`. Часто в таких перечеслениях есть специальные значения, такие как `VK_STRUCTURE_TYPE_BEGIN_RANGE`, `VK_STRUCTURE_TYPE_END_RANGE` и `VK_STRUCTURE_TYPE_RANGE_SIZE`.
+ `pNext` — указатель на следующую структуру, содержащую дополнительную информацию из расширения. В данном случае `nullptr`. В описании расширения может быть указано, что `pNext` в определённой структуре должен ссылаться на другую структуру, специфичную для расширения. И там `sType` также должен быть заполнен, но уже другим значением, определённым в расширении.
+ `pApplicationName` — имя приложения, которое вы можете указать. В данном случае это *Null Terminated* (последний символ должен быть `\0`) *C-строка* (указатель на char). Можно указать `nullptr`.
+ `applicationVersion` — версия приложения. Можно указать `0`.
+ `pEngineName` — имя используемого движка (если таковой имеется), задаётся также, как и имя приложения. Можно указать `nullptr`.
+ `engineVersion` — версия используемого движка. Можно указать `0`.
+ `apiVersion` — версия API.

Теперь о версиях. Версии имеют три параметра: **главный** (*major*), **дополнительный** (*minor*) и **патч** (*patch*). Для создания версии есть макрос `VK_MAKE_VERSION(major, minor, patch)`, который компонует все три числа в `uint32_t`. Для того, чтобы получить отдельные числа из версии, есть макросы `VK_VERSION_MAJOR(version)`, `VK_VERSION_MINOR(version)`, `VK_VERSION_PATCH(version)`. Так можно создать версию для приложения, движка и используемого API. К слову, версия API будет влиять на работу драйвера, но об этом переживать не стоит, если версия отличается лишь последней цифрой (patch). Поэтому можено просто указать `VK_API_VERSION_1_0` (в предыдущих SDK — `VK_API_VERSION`).
Есть также версия заголовочного файла *vulkan.h*: `VK_HEADER_VERSION`.

Теперь, заполним структуру. Для начала, я вывел в глобальные имя приложения:

	char app_name[] = "Vulkan Tutorian. Device. © GrWolf.";
	
А потом заполнил структуру таким образом:

	VkApplicationInfo app_info;
	memset(&app_info, 0, sizeof(app_info));
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = app_name;
	#ifdef VK_API_VERSION_1_0
	app_info.apiVersion = VK_API_VERSION_1_0;
	#else
	app_info.apiVersion = VK_API_VERSION;
	#endif
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	
К слову, я заполнил её нулями сразу, чтобы не указывать эти нули потом. Тем не менее, можно указать параметры структуры и таким образом:

	VkApplicationInfo app_info = {
		VK_STRUCTURE_TYPE_APPLICATION_INFO,		// VkStructureType 	sType
		nullptr,								// const void 		*pNext
		app_name,								// const char 		*pApplicationName
		VK_MAKE_VERSION(1, 0, 0 ),				// uint32_t 		applicationVersion
		nullptr,								// const char 		*pEngineName
		0,										// uint32_t 		engineVersion
		VK_API_VERSION							// uint32_t 		apiVersion
	};

Хорошо, а на кой нам вообще указывать всё это? Чтобы было! На самом деле, некоторые драйвера могут быть оптимизированы под определённые движки и/или игры, чтобы приложение работало ещё быстрее. А так, как уже говорилось, версия API влияет на то, будет ли вообще драйвер поддерживать приложение, а если будет, то как поддерживать. Но об этом чуть позже.
Теперь о структуре, отвечающей за экземпляр.

	typedef struct VkInstanceCreateInfo {
		VkStructureType             sType;
		const void*                 pNext;
		VkInstanceCreateFlags       flags;
		const VkApplicationInfo*    pApplicationInfo;
		uint32_t                    enabledLayerCount;
		const char* const*          ppEnabledLayerNames;
		uint32_t                    enabledExtensionCount;
		const char* const*          ppEnabledExtensionNames;
	} VkInstanceCreateInfo;
	
Разбираем:
+ `sType` — тип структуры. В данном случае `VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO`.
+ `pNext` —  указатель на структуру из расширения, если таковые имеются. В данном случае `nullptr`.
+ `flags` — флаги. Их пока не существует, зарезервированы для будущего использования. В данном случае `0`.
+ `pApplicationInfo` — указатель на информацию о приложении. Можно указать `nullptr`.

Далее идёт инофрмация о слоях и расширениях, о которых будет рассказно в следующем уроке. А пока `nullptr` и `0`.
Заполняем:

	VkInstanceCreateInfo instance_info;
	memset(&instance_info, 0, sizeof(instance_info));
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	
Ничего сложного. Далее создадим экземпляр. Нам дадут рабочий хэндл, если создание будет успешным.
Отдельно стоит рассказать о handle'ах в Vulkan. Каждый такой хэндл создаётся через макровызовы `VK_DEFINE_HANDLE` или `VK_DEFINE_NON_DISPATCHABLE_HANDLE`, из-за чего в некоторых IDE подсветка работает неправильно (например, CodeLite, который я недавно из-за его причуд бросил). Нулевой хэндл (т.е. нерабочий или несуществующий) обозначается через `VK_NULL_HANDLE`. Сами типы хэндлов задаются через эти макросы:

	#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;

	#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
			#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
	#else
			#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
	#endif
        
Как уже видно, хэндлы распределяются на **отправляемые** (*dispatchable*) и **не отправляемые** (*non-dispatchable*). Первые отправляются в функции как главный объект, с которым мы работаем, вторые наоборот — являются второстепенными. Также первые, это указатель на реально существующие данные, а вслучае со вторыми, это не обязательно указатель на существующие данные. В нашем случае, отправляемыми являются: `VkInstance`, `VkPhysicalDevice`, `VkDevice`, `VkQueue`, `VkCommandBuffer`. Все остальные являются не отправляемыми.


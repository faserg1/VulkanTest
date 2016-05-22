#Начало: подключение библиотеки.
И так, для начала разберёмся с подключением библиотек. После установки SDK у нас будут 3 папки, которые надо будет подключить для поиска заголовочных файлов и библиотек.
* Папка Include — в ней будет поиск заголовочных файлов.
* Папка Bin — в ней будет поиск библиотек (x86_64, т.е. для 64 битных приложений).
* Папка Bin32 — в ней будет поиск библиотек (x86, т.е. для 32 битных приложений).

После этого, нужно подключить одну библиотеку: vulkan-1. Конечно, для Android приложений нужно будет привязывать эту библиотеку динамически, но для всех остальных можно и просто подключить эту библиотеку.

Затем, в исходниках, где будет использоваться Vulkan, нужно подключить заголовочный файл *vulkan.h*:

	#include <vulkan/vulkan.h>

Всё, теперь можно приступать к работе.

#Создание экземпляра Vulkan. Кратко о получении функций
##Получение функций
Как уже говорилось ранее, не всегда функции, которые мы хотим использовать, могут быть получены в результате статической линковки. Поэтому есть функция `vkGetInstanceProcAddr`, которую нужно получить в первую очередь, используя системно-зависимые функции (например, `GetProcAddress` для *Win32 API*). Этой функцией можно получить все остальные функции, работающие с экземпляром и без него. Подробнее:

	PFN_vkVoidFunction vkGetInstanceProcAddr(
		VkInstance		instance,
		const char*		pName);
		
+ `instance` — экземпляр Vulkan. Если это функции, работающие без него (например, функция создания экземпляра), тогда нужно отправить `VK_NULL_HANDLE`.
+ `pName` — имя функции. Например, `"vkCreateInstance"`.

Это возвращаемый тип:

	typedef void (VKAPI_PTR *PFN_vkVoidFunction)(void);
	
В последствии, его надо будет приводить к другим типам указателей на функции.

##Экземпляр

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
Объявим хэндл:

	VkInstance instance = VK_NULL_HANDLE;
	
Пока он будет нулевым. Теперь результат:

	VkResult create_instance_result;
	
От этого результата будет зависить дальнейшая работа приложения. Создаём экземпляр:

	create_instance_result = vkCreateInstance(&instance_info, nullptr, &instance);
	
Теперь подробнее рассмотрим функцию `vkCreateInstance`.

	VkResult vkCreateInstance(
		const VkInstanceCreateInfo*                 pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkInstance*                                 pInstance);
	
+ `pCreateInfo` — это указатель на обязательно заполненную структуру `VkInstanceCreateInfo`.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью. Если нам это не нужно, можно оставить `nullptr`.
+ `pInstance` — указатель на хэндл, который мы получим при успешном завершении функции.

И так, какие могут быть результаты?

+ `VK_SUCCESS` — всё хорошо, всё получилось.
+ `VK_ERROR_OUT_OF_HOST_MEMORY` — не хватает памяти хоста (оперативной памяти).
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY` — не хватает памяти устройства (видеопамяти).
+ `VK_ERROR_INITIALIZATION_FAILED` — магия, провалилась инициализация, внутренняя ошибка.
+ `VK_ERROR_LAYER_NOT_PRESENT` — указанный слой не может быть загружен.
+ `VK_ERROR_EXTENSION_NOT_PRESENT` — указанное расширение не может быть загружено.
+ `VK_ERROR_INCOMPATIBLE_DRIVER` — пользователь не поставил новые драйвера или всё гораздо хуже. Т.е. нет драйвера, который бы поддерживал заданную версию `apiVersion` в `VkApplicationInfo` или драйвер отсутствует вовсе.

После вызова функции, можно проверить результат `create_instance_result`. В добавок, можно сказать, что различные экземпляры друг на друга не влияют, а также, прежде чем разрушить экземпляр, нужно разрушить и все дочерние объекты.
А теперь, к главному!

#Устройства, семейства, очереди
**Устройство** — это инструмент, позволяющий рисовать и вычислять. В Vulkan, в отлчиии от других API есть разделение на **физическое** и **логическое** устройство. Прежде чем создать логическое устройсто, которое будет использовано в приложении, необходимо выбрать для него физическое, подобрав необходимые параметры. Поэтому, для начала нужно получить список физических устройств с помощью функции `vkEnumeratePhysicalDevices`. 

##Физические устройства
Физическим устройством может являтся видеокарта (дискретный GPU), встроенный графический процессор или что-либо ещё (подробнее об этом чуть ниже).

	VkResult vkEnumeratePhysicalDevices(
		VkInstance                                  instance,
		uint32_t*                                   pPhysicalDeviceCount,
		VkPhysicalDevice*                           pPhysicalDevices);
		
+ `instance` — экземпляр Vulkan.
+ `pPhysicalDeviceCount` — указатель на количество физических устройств.
+ `pPhysicalDevices` — массив фиических устройств.

Эту функцию можно использовать двумя способами. Первый способ — это получить количество физических устройств. Для этого нужно `pPhysicalDevices` оставить `VK_NULL_HANDLE`, но `pPhysicalDeviceCount` должен быть действительным указателем. Так, `*pPhysicalDeviceCount` (т.е. значение, на которое ссылается указатель) станет равным количеству физических устройств.

Второй способ — получить сами устройства. Тогда `pPhysicalDevices` должен быть действительным указателем на область памяти/массивом, который будет принимать значения, а также `*pPhysicalDeviceCount` — максимальное количество, которое может принять массив (и меньше, если необходимо).

Результатами могут быть:
+ `VK_SUCCESS` — всё хорошо, нет проблем.
+ `VK_INCOMPLETE` — всё хорошо, но были получены не все данные (в данном случае — не все хэндлы физических устройств).
+ `VK_ERROR_OUT_OF_HOST_MEMORY` — не хватает памяти хоста.
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY` — не хватает памяти устройства.
+ `VK_ERROR_INITIALIZATION_FAILED` — магия.

Вот пример, как можно получить все хэндлы без потерь:

	uint32_t gpu_count;
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, VK_NULL_HANDLE) != VK_SUCCESS)
		return;
	std::vector<VkPhysicalDevice> gpu_list(gpu_count);
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
		return;
		
Для этого нужно будет вызвать функцию два раза. Первый раз, чтобы узнать точное количество физических устройств и подготовить для хэндлов место, второй раз, получить все физические устройства.

Теперь, можно узнать все подробности об устройстве. Есть несколько функций, которые позволяют получить эти подробности:

+ `vkGetPhysicalDeviceFeatures` — возвращает поддерживаемый функционал.
+ `vkGetPhysicalDeviceFormatProperties` — возвращает поддерживаемые форматы.
+ `vkGetPhysicalDeviceImageFormatProperties` — возвращает поддерживаемые форматы изображений.
+ `vkGetPhysicalDeviceProperties` — возвращает свойства устройства (немного подробнее ниже).
+ `vkGetPhysicalDeviceMemoryProperties` — возвращает свойства памяти устройства.
+ `vkGetPhysicalDeviceQueueFamilyProperties` — возвращает свойства семейств очередей устройства (подробнее ниже).

Например, можно узнать тип устройства, его имя и лимиты. Для этого есть функция `vkGetPhysicalDeviceProperties`.

	void vkGetPhysicalDeviceProperties(
		VkPhysicalDevice				physicalDevice,
		VkPhysicalDeviceProperties*		pProperties);
		
+ `physicalDevice` — хэндл физичского устройства, свойства которого вы хотите получить.
+ `pProperties` — указатель на структуру, принимающую свойства.

Функция не возвращает значения, ибо она всегда должна выполнятся стабильно. Сама структура выглядит таким образом:

	typedef struct VkPhysicalDeviceProperties {
		uint32_t							apiVersion;
		uint32_t							driverVersion;
		uint32_t							vendorID;
		uint32_t							deviceID;
		VkPhysicalDeviceType				deviceType;
		char								deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE];
		uint8_t								pipelineCacheUUID[VK_UUID_SIZE];
		VkPhysicalDeviceLimits				limits;
		VkPhysicalDeviceSparseProperties	sparseProperties;
	} VkPhysicalDeviceProperties;
	
+ `apiVersion` — версия API, которую поддерживает драйвер этого устройства. Расшифровывается через макровызовы, упомянутые выше.
+ `driverVersion` — версия драйвера для этого устройства. Расшифровывается через макровызовы, упомянутые выше.
+ `vendorID` — уникальный идентификатор для vendor'а (издателя).
+ `deviceID` — уникальный идентификатор устройства.
+ `deviceType` — тип устройства.
+ `deviceName` — имя устройства.
+ `pipelineCacheUUID` — уникальная и универсальная подпись, обозначающая комбинацию физического устройства и драйвера.
+ `limits` — лимиты физического устройства. 
+ `sparseProperties` — свойства разрежженой памяти.

Нменого подробнее о последних двух структурах:

	typedef struct VkPhysicalDeviceLimits {
		uint32_t              maxImageDimension1D;
		uint32_t              maxImageDimension2D;
		uint32_t              maxImageDimension3D;
		uint32_t              maxImageDimensionCube;
		uint32_t              maxImageArrayLayers;
		uint32_t              maxTexelBufferElements;
		uint32_t              maxUniformBufferRange;
		uint32_t              maxStorageBufferRange;
		uint32_t              maxPushConstantsSize;
		uint32_t              maxMemoryAllocationCount;
		uint32_t              maxSamplerAllocationCount;
		VkDeviceSize          bufferImageGranularity;
		VkDeviceSize          sparseAddressSpaceSize;
		uint32_t              maxBoundDescriptorSets;
		uint32_t              maxPerStageDescriptorSamplers;
		uint32_t              maxPerStageDescriptorUniformBuffers;
		uint32_t              maxPerStageDescriptorStorageBuffers;
		uint32_t              maxPerStageDescriptorSampledImages;
		uint32_t              maxPerStageDescriptorStorageImages;
		uint32_t              maxPerStageDescriptorInputAttachments;
		uint32_t              maxPerStageResources;
		uint32_t              maxDescriptorSetSamplers;
		uint32_t              maxDescriptorSetUniformBuffers;
		uint32_t              maxDescriptorSetUniformBuffersDynamic;
		uint32_t              maxDescriptorSetStorageBuffers;
		uint32_t              maxDescriptorSetStorageBuffersDynamic;
		uint32_t              maxDescriptorSetSampledImages;
		uint32_t              maxDescriptorSetStorageImages;
		uint32_t              maxDescriptorSetInputAttachments;
		uint32_t              maxVertexInputAttributes;
		uint32_t              maxVertexInputBindings;
		uint32_t              maxVertexInputAttributeOffset;
		uint32_t              maxVertexInputBindingStride;
		uint32_t              maxVertexOutputComponents;
		uint32_t              maxTessellationGenerationLevel;
		uint32_t              maxTessellationPatchSize;
		uint32_t              maxTessellationControlPerVertexInputComponents;
		uint32_t              maxTessellationControlPerVertexOutputComponents;
		uint32_t              maxTessellationControlPerPatchOutputComponents;
		uint32_t              maxTessellationControlTotalOutputComponents;
		uint32_t              maxTessellationEvaluationInputComponents;
		uint32_t              maxTessellationEvaluationOutputComponents;
		uint32_t              maxGeometryShaderInvocations;
		uint32_t              maxGeometryInputComponents;
		uint32_t              maxGeometryOutputComponents;
		uint32_t              maxGeometryOutputVertices;
		uint32_t              maxGeometryTotalOutputComponents;
		uint32_t              maxFragmentInputComponents;
		uint32_t              maxFragmentOutputAttachments;
		uint32_t              maxFragmentDualSrcAttachments;
		uint32_t              maxFragmentCombinedOutputResources;
		uint32_t              maxComputeSharedMemorySize;
		uint32_t              maxComputeWorkGroupCount[3];
		uint32_t              maxComputeWorkGroupInvocations;
		uint32_t              maxComputeWorkGroupSize[3];
		uint32_t              subPixelPrecisionBits;
		uint32_t              subTexelPrecisionBits;
		uint32_t              mipmapPrecisionBits;
		uint32_t              maxDrawIndexedIndexValue;
		uint32_t              maxDrawIndirectCount;
		float                 maxSamplerLodBias;
		float                 maxSamplerAnisotropy;
		uint32_t              maxViewports;
		uint32_t              maxViewportDimensions[2];
		float                 viewportBoundsRange[2];
		uint32_t              viewportSubPixelBits;
		size_t                minMemoryMapAlignment;
		VkDeviceSize          minTexelBufferOffsetAlignment;
		VkDeviceSize          minUniformBufferOffsetAlignment;
		VkDeviceSize          minStorageBufferOffsetAlignment;
		int32_t               minTexelOffset;
		uint32_t              maxTexelOffset;
		int32_t               minTexelGatherOffset;
		uint32_t              maxTexelGatherOffset;
		float                 minInterpolationOffset;
		float                 maxInterpolationOffset;
		uint32_t              subPixelInterpolationOffsetBits;
		uint32_t              maxFramebufferWidth;
		uint32_t              maxFramebufferHeight;
		uint32_t              maxFramebufferLayers;
		VkSampleCountFlags    framebufferColorSampleCounts;
		VkSampleCountFlags    framebufferDepthSampleCounts;
		VkSampleCountFlags    framebufferStencilSampleCounts;
		VkSampleCountFlags    framebufferNoAttachmentsSampleCounts;
		uint32_t              maxColorAttachments;
		VkSampleCountFlags    sampledImageColorSampleCounts;
		VkSampleCountFlags    sampledImageIntegerSampleCounts;
		VkSampleCountFlags    sampledImageDepthSampleCounts;
		VkSampleCountFlags    sampledImageStencilSampleCounts;
		VkSampleCountFlags    storageImageSampleCounts;
		uint32_t              maxSampleMaskWords;
		VkBool32              timestampComputeAndGraphics;
		float                 timestampPeriod;
		uint32_t              maxClipDistances;
		uint32_t              maxCullDistances;
		uint32_t              maxCombinedClipAndCullDistances;
		uint32_t              discreteQueuePriorities;
		float                 pointSizeRange[2];
		float                 lineWidthRange[2];
		float                 pointSizeGranularity;
		float                 lineWidthGranularity;
		VkBool32              strictLines;
		VkBool32              standardSampleLocations;
		VkDeviceSize          optimalBufferCopyOffsetAlignment;
		VkDeviceSize          optimalBufferCopyRowPitchAlignment;
		VkDeviceSize          nonCoherentAtomSize;
	} VkPhysicalDeviceLimits;
	
	typedef struct VkPhysicalDeviceSparseProperties {
		VkBool32    residencyStandard2DBlockShape;
		VkBool32    residencyStandard2DMultisampleBlockShape;
		VkBool32    residencyStandard3DBlockShape;
		VkBool32    residencyAlignedMipSize;
		VkBool32    residencyNonResidentStrict;
	} VkPhysicalDeviceSparseProperties;
	
Узнать о них поближе вы сможете прочитав спецификацию Vulkan. А вот информация про типы устройств: 

#Семейства очередей
И так, что же за семейства очередей и с чем их идят? Напоминаем, что устройства могут разделятся по предназначениям, хотя многие из них будут универсальными. Например, устройства разделяются на 4 вида работы:
+ *графика* — устройство может рисовать 3D/2D объекты.
+ *вычисления* — устройство может производить вычисления.
+ *копирование* — устройство может копировать и переносить информацию.
+ *работа с разрежженой памятью* — это по большей части уникальная особенность функционала, но тем не менее, флаг находится здесь.

Все эти флаги собираются в различные семейства, которые могут или не могут выполнять те или иные возможности, указанные флагами. Сама очередь представляет собой место, куда попадают команды, чтобы в последствии они могли быть исполнены устройством. Команды, принадлежащие определённому семейству, не могут быть посланы в очередь, которая не поддерживает это семейство. Семейство может содержать как один флаг, так и все. Vulkan старается объединять семейства с одинаковыми способности в одно целое. Каждое семейство может иметь ограниченное число очередей.

К примеру: NVIDIA GeForce GTX 760 содержит 1 семейство поддерживающее все флаги.  Семейство содержит 16 очередей. Так обычно и со всеми другими картами NVIDIA.

Получить информацию о семействах может следующая функция:

	void vkGetPhysicalDeviceQueueFamilyProperties(
		VkPhysicalDevice			physicalDevice,
		uint32_t*					pQueueFamilyPropertyCount,
		VkQueueFamilyProperties*	pQueueFamilyProperties);
		
+ `physicalDevice` — хэндл физического устройства.
+ `pQueueFamilyPropertyCount` — указатель на количество семейств.
+ `pQueueFamilyProperties` — семейства.

Как вы уже догадались, эта функция работает также, как и `vkEnumeratePhysicalDevices`. Напомню, что если последний аргумент — `VK_NULL_HANDLE`, то второй получит количество семейств, иначе, третий получит столько семейств, сколько указано во втором. Подробнее о структуре:

	typedef struct VkQueueFamilyProperties {
		VkQueueFlags    queueFlags;
		uint32_t        queueCount;
		uint32_t        timestampValidBits;
		VkExtent3D      minImageTransferGranularity;
	} VkQueueFamilyProperties;
	
+ `queueFlags` — флаги семейства (поддерживаемая работа).
+ `queueCount` — максимальное количество очередей.
+ `timestampValidBits` — число доступных битов для `vkCmdWriteTimestamp`, 0 если вовсе не поддерживает.
+ `minImageTransferGranularity` — минимальная "зернистость" поддерживаемая при копировании изображения.

Сами флаги бывают следующими:

+ `VK_QUEUE_GRAPHICS_BIT` — поддерживает графику.
+ `VK_QUEUE_COMPUTE_BIT` — поддерживают вычисления.
+ `VK_QUEUE_TRANSFER_BIT` — поддерживает копирование.
+ `VK_QUEUE_SPARSE_BINDING_BIT` — поддерживает работу с разрежженной памятью.

В примере будет задействована лишь одно, первое по индексу устройство.

	VkPhysicalDevice gpu = gpu_list[0];
	uint32_t family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, VK_NULL_HANDLE);
	std::vector<VkQueueFamilyProperties> family_properties_list(family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_properties_list.data());
	
	uint32_t valid_family_index = (uint32_t) -1;
	for (uint32_t i = 0; i < family_count; i++) //листаем все семейства.
	{
		VkQueueFamilyProperties &properties = family_properties_list[i];
		if (properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			if (valid_family_index == (uint32_t) -1)
				valid_family_index = i;
		}
	}
	if (valid_family_index == (uint32_t) -1)
		return;
		
Так можно определить, поддерживает ли устройство по индексу 0 графику вообще, а также определить индекс семейства, которое поддерживает графику. Так же, вам может понадобится хранить индексы семейств (или проверять уже полученный), которые могут вычислять, копировать, поддерживать рарежженную память, а также выводить картинку на экран (о том, как проверить семейство на это будет в уроке 04).

В предыдущем примере мы получили `valid_family_index`, теперь мы точно знаем, что устройство, а именно это семейство поддерживает графику. Теперь нужно создать логическое устройство, но для начала нам нужно указать для него информацию об очередях.

#Очереди

Информацию об очередях, которые должны быть в устройстве, Vulkan получает с помощью этой структуры:

	typedef struct VkDeviceQueueCreateInfo {
		VkStructureType             sType;
		const void*                 pNext;
		VkDeviceQueueCreateFlags    flags;
		uint32_t                    queueFamilyIndex;
		uint32_t                    queueCount;
		const float*                pQueuePriorities;
	} VkDeviceQueueCreateInfo;
	
+ `sType` — тип структуры, в данном случае `VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO`.
+ `pNext` — `nullptr` или указатель на структуру из расширения.
+ `flags` — флаги зарезервированы для будущего использования.
+ `queueFamilyIndex` — индекс семейства, к которому будут пренадлежать очереди.
+ `queueCount` — количество очередей, которые нужно создать с этим семейством.
+ `pQueuePriorities` — приоритеты семейств.

Пример. Вы хотите создать *одну* очередь с семейством, которое было недавно получено (**`valid_family_index`**):

	float device_queue_priority[] = {1.0f}; //приоритеты
	
	VkDeviceQueueCreateInfo device_queue_info;
    memset(&device_queue_info, 0, sizeof(device_queue_info));
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	
	device_queue_info.queueCount = 1;
	device_queue_info.queueFamilyIndex = valid_family_index;
	device_queue_info.pQueuePriorities = device_queue_priority;
	
**Приоритеты** — это массив `float` значений от **0.f** (*низшего приоритета*) до **1.f** (*высшего приоритета*). Чем выше приоритет — тем больше времени будет уделяться этой очереди.

#Устройство

Для создания устройства, нам нужно заполнить следующую структуру:

	typedef struct VkDeviceCreateInfo {
		VkStructureType                    sType;
		const void*                        pNext;
		VkDeviceCreateFlags                flags;
		uint32_t                           queueCreateInfoCount;
		const VkDeviceQueueCreateInfo*     pQueueCreateInfos;
		uint32_t                           enabledLayerCount;
		const char* const*                 ppEnabledLayerNames;
		uint32_t                           enabledExtensionCount;
		const char* const*                 ppEnabledExtensionNames;
		const VkPhysicalDeviceFeatures*    pEnabledFeatures;
	} VkDeviceCreateInfo;
	
+ `sType` — тип структуры, в данном случае `VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO`.
+ `pNext` — `nullptr` или указатель на структуру из расширения.
+ `flags` — флаги зарезервированы для будущего использования.
+ `queueCreateInfoCount` — количество структур `VkDeviceQueueCreateInfo`, которые вы хотите отправить.
+ `pQueueCreateInfos` — структуры `VkDeviceQueueCreateInfo`. Таким образом, можно создать несколько очередей с разными семействами (напомню, что одна структура `VkDeviceQueueCreateInfo` может принять лишь одно семейство).
+ `pEnabledFeatures` — функционал устройства. Если хотите оставить только **нужный** (*required*) — оставьте `nullptr`. Все доступные возможности можно получить с помощью функции `vkGetPhysicalDeviceFeatures`.
 
Слои и расширения:

+ `enabledLayerCount`
+ `ppEnabledLayerNames`
+ `enabledExtensionCount`
+ `ppEnabledExtensionNames`

Пример:

	VkDeviceCreateInfo device_info;
    memset(&device_info, 0, sizeof(device_info));
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &device_queue_info;
	
Далее устройство можно создать. За это отвечает следующая функция:

	VkResult vkCreateDevice(
		VkPhysicalDevice                            physicalDevice,
		const VkDeviceCreateInfo*                   pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkDevice*                                   pDevice);

+ `physicalDevice` — физическое устройство, на основе которого создаётся логическое.
+ `pCreateInfo` — структура, содержащая информацию об устройстве.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.
+ `pDevice` — указатель на хэндл устройства.

Создать устройство можно таким образом:

	VkDevice device = VK_NULL_HANDLE;
	if (vkCreateDevice(gpu, &device_info, NULL, &device) != VK_SUCCESS)
		return;
		
И так мы получим `device`. Теперь, разберём, какие ошибки могут быть.

+ `VK_SUCCESS
+ `VK_ERROR_OUT_OF_HOST_MEMORY
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY
+ `VK_ERROR_INITIALIZATION_FAILED — неправильно заданы параметры или магия.
+ `VK_ERROR_LAYER_NOT_PRESENT
+ `VK_ERROR_EXTENSION_NOT_PRESENT
+ `VK_ERROR_TOO_MANY_OBJECTS — подали на создание **слишком много ~~яблок~~ объектов** (например — очередей).
+ `VK_ERROR_DEVICE_LOST — устройство ~~потрачено~~ потеряно.

##Функции устройства

Получить функции, с которыми работает устройство, может эта функция:

	PFN_vkVoidFunction vkGetDeviceProcAddr(
		VkDevice		device,
		const char*		pName);
		
+ `device` — хэндл устройство. Не может быть `VK_NULL_HANDLE`
+ `pName` — имя функции, которая работает с устройством.

#Потеря устройства

Теперь немного об ошибке device lost (потеря устройства). Сколько помню DX9, то такая ошибка могла возникнуть даже при закрытии окна, в которое мы что-либо рисовали, если оно было крепко привязано к устройству. В Vulkan потеря устройства может быть лишь по такому ряду причин:

+ Ошибка в устройстве, возможно вызванная вашими командами из приложения. В этом случае нужно уничтожить устройство, но перед этим нужно также уничтожить все его дочернии объекты — их хэндлы всё ещё дейсвительны, а некоторые команды могут также продолжать работу, и также могут вернуть как удачу, так и error — device lost. После этого также возможно создание логического устройства заново, и можно продолжить работу, если это не 2 пункт.
+ Отсоединение и/или потеря физического устройства. В этом случае создать логическое устройство заново нельзя — так как физическое устройство уже не может работать.
+ Ошибка в системе, повреждение памяти. В этом случае Vulkan не гарантирует стабильное выполнение команд.

Во всех остальных случаях устройство потеряно быть не может.

#Уничтожение (cleaning up)

Для уничтожения устройства понадобится следующая функция:

	void vkDestroyDevice(
		VkDevice						device,
		const VkAllocationCallbacks*	pAllocator);

+ `device` — хэндл устройства.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.

Для уничтожения экземпляра:

	void vkDestroyInstance(
		VkInstance						instance,
		const VkAllocationCallbacks*	pAllocator);
	
+ `device` — хэндл экземпляра.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.

То есть ничего сложного:

	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
	
#Заключение

Готово! Кумамон одобряет, мы сожгли всё, что породили. Даже пройдя через такие трудные методы.
К слову, один человек во время GDC 2016 сказал: "Vulkan API старались сделать максимально понятным. Но понятный — не значит простой." Да, Vulkan API действительно отличается от того же DirectX или OpenGL, хотя, конечно же, есть и некоторые схожие места. Но для чего же его усложнили? Для того, чтобы программист решал, что и в какой мере нагружать, это раз, а второе, чтобы оптимизировать работу и "общение" между видеокартой и процессором. Теперь мы можем просто отправить пачкой команды в видеокарту, а не каждый раз теребить по одной команде "сделай то, сделай это". Ну и другие полезные вещи в Vulkan API тоже есть.

Спасибо за прочтение урока.
Если вы хотите меня поддержать, прикрепляю кошель с Я.Денег: 410012557544062.
Ну или просто жмя по [ссылке](https://money.yandex.ru/to/410012557544062 "Яндекс.Деньги").

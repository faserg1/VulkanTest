/* Этот урок покажет, как можно использовать произвольное выделение и высвобождение памяти
 * с помощью структуры VkAllocationCallbacks. Данный урок — лишь пример с пояснениями,
 * и использовать его в реальных целях не возможно по причине того, что одна функция просто опущена
 * и в ней отсутствует реализация.
 * В этот урок скоро также войдёт управление памятью устройства.
 * Все интересующие вас подробности смотрите здесь:
 * https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf
 * © Серый Волк.
*/

#include <iostream> //Ввод-вывод информации.
#include <string.h> //Необходимые инструменты (memset)
#include <vector> //Удобно храним массивы
#include <cstdlib> //exit
#include <vulkan/vulkan.h> //Vulkan SDK

#define ZM(what) memset(&what, 0, sizeof(what))

/* Начнём с контроля хост-памяти. Если вы желаете, чтобы ваша память выделялась по особому, или вы просто
 * хотите контролировать её выделение — можно создать специальный контроллер (или просто функции), которые будут
 * запрашивать память, и передавать дополнительную информацию. К слову говоря, вы можете выделить какой-то участок
 * памяти заранее, и уже там отправлять отдельные кусочки этой памяти в Vulkan. Тем не менее, есть места, где вы
 * таким образом не сможете управлять памятью.
 * 
 * Для начала объявим класс, который будет заниматься всем управлением. Это может быть конечно же не класс, а просто
 * отдельные функции.
*/
class MemoryController;

/* В качестве примера, я буду использовать и передавать указатель на структуру ниже, в которой будет храниться сам класс
 * управления памятью.
*/ 
struct MemoryControllerData
{
	MemoryController *ctrl;
};

/* Ну и соответственно сам класс. Фактически, всю реализицию я вообще разместил в статические функции (функции класса,
 * естественно, передать нельзя).
*/ 
class MemoryController
{
	MemoryControllerData data; //вот сами данные
	/* А вот в этой структуре будут лежать указатели на наши функции, и указатель на эту структуру мы будем передвать
	 * функциям Vulkan.
	*/ 
	VkAllocationCallbacks callbacks;
	/* И так, для начала реализуем функции самого класса. В нём мы будем просто логгировать в консоль все действия.
	 * Забегая вперёд, скажу, что кроме размера необходимой памяти, прилетает ещё другая полезная и не очень информация.
	 * 
	 * Выделение памяти в Vulkan можно распределить по сфере применения:
	*/ 
	void log_scope(VkSystemAllocationScope scope)
	{
		switch (scope)
		{
			case VK_SYSTEM_ALLOCATION_SCOPE_COMMAND:
				std::wcout << L"Эта память была выделена для какой-то команды Vulkan.\n";
				break;
			case VK_SYSTEM_ALLOCATION_SCOPE_OBJECT:
				std::wcout << L"Эта память была выделена для объекта Vulkan, который был создан или использован.\n";
				break;
			case VK_SYSTEM_ALLOCATION_SCOPE_CACHE:
				std::wcout << L"Эта память была выделена для кэша конвейера.\n";
				break;
			case VK_SYSTEM_ALLOCATION_SCOPE_DEVICE:
				std::wcout << L"Эта память была выделена для устройства Vulkan.\n";
				break;
			case VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE:
				std::wcout << L"Эта память была выделена для экземпляра Vulkan.\n";
				break;
			default:
				//Ну а тут какой-нибудь бред, так как ничего больше не используется, а warning'и глазу мешают.
				break;
		}
		
	}
	//И по типу использвания:
	void log_type(VkInternalAllocationType allocationType)
	{
		switch (allocationType)
		{
			case VK_INTERNAL_ALLOCATION_TYPE_EXECUTABLE:
				std::wcout << L"Эта память была выделена для использования на хосте.\n";
				break;
			default:
				break;
		}
	}
	//Далее сами функции логгирования:
	void log(size_t size, size_t align, VkSystemAllocationScope scope)
	{
		std::wcout << L"Память выделена с размером " << size << L" байт.\n";
		log_scope(scope);
	}
	void log_free()
	{
		std::wcout << L"Высвободили память.\n";
	}
	void ilog(bool is_free, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
	{
		if (is_free)
		{
			std::wcout << L"Внутри Vulkan высвободилась память.\n";
		}
		else
		{
			std::wcout << L"Внутри Vulkan память была выделена память.\n";
		}
		log_type(allocationType);
		log_scope(allocationScope);
	}
public:
	void *GetData() //С помощью этой функции мы просто получаем pUserData
	{
		return (void *) &data;
	}
	VkAllocationCallbacks *GetCallbacks() //А с помощью этой — указатель на структуру с callback'ами
	{
		return &callbacks;
	}
	/* Теперь сами функции. Для начала разберём функцию выделения памяти.
	 * Для начала, она должна возвращать void *, т.е. указатель на неопределённый тип.
	 * Она должна обязательно быть с соглашением вызовов VKAPI_PTR (для Win32 — __stdcall), иначе может произойти
	 * неправильный вызов и в резульате вы получите SIGSEGV.
	 * Она может принять, но не обязательно в ней будет не нулевым pUserData, так как вы можете использовать
	 * одни и те же функции с разным pUserData (например, при создании устройства вы можете оставить pUserData
	 * пустым, а при создании экземпляра — нет).
	 * Впрочем, можно также использовать и разные функции.
	 * И так, параметры.
	*/ 
	static void VKAPI_PTR *mc_alloc(void *pUserData, //данные, которые вы можете передать в структуре с callback'ами
		size_t size, //размер запрашиваемой памяти
		size_t alignment, //выравнивание памяти. Гарантировано, что число будет иметь степень двойки.
		VkSystemAllocationScope scope) //сфера применения памяти
	{
		if (pUserData)
			static_cast<MemoryControllerData *>(pUserData)->ctrl->log(size, alignment, scope);
		/* А дальше идёт код, который я стырил на StackOverflow. Но всё равно его поясню, как этот код устроен.
		 * Хотя, конечно же, можно реализовать всё это иначе.
		*/ 
		void* res = 0; //Для начала подготовим указатель, который будем возвращать.
		/* Потом мы выделяем память с необходимым размером и специальным запасом, помещая адресс во временный указатель.
		 * Этот запас нужен для того, чтобы мы могли правильно выравнить нашу память и поместить туда адресс
		 * на реальное начало памяти.
		*/
		void* ptr = malloc(size+alignment+sizeof(void**));
        // Проверяем, выделилась ли память, и если да, то нужно обработать адрес.
		if(ptr != 0)
        {
            res = reinterpret_cast<void*>(
				/* Здесь выполняем битовые операции, которые фактически приводят к смещению за начало допустимой
				 * памяти.
				*/
				(reinterpret_cast<size_t>(ptr) & ~(size_t(alignment-1)))
				+ alignment); //поэтому мы прибавим сюда наш запас для выравнивания.
			//А теперь помещаем указатель перед началом нашей используемой памяти фактически в запас для указателя.
            *(reinterpret_cast<void**>(res) - 1) = ptr;
			//Но всё равно этот код можно немного оптимизировать по памяти и т.д.
        }
        return res;
	}
	static void VKAPI_PTR *mc_realloc(void *pUserData,
		void *pOriginal,
		size_t size,
		size_t alignment,
		VkSystemAllocationScope scope)
	{
		/* В данном случае, так как эта функция всё равно не используется (мы лишь создаём и уничтожаем экземпляр Vulkan),
		 * то я оставлю эту функцию пустой.
		 * Главное, чтобы она и другие функции при неудаче возвращали NULL.
		*/ 
		return NULL;
	}
	/* С освобождением памяти кроме pUserData и указателя больше мы ничего не получим.
	 * Поэтому в этой функции мы просто берём реальный указатель и высвобождаем память.
	*/ 
	static void VKAPI_PTR mc_free(void *pUserData, void *memory)
	{
		if (pUserData)
			static_cast<MemoryControllerData *>(pUserData)->ctrl->log_free();
		if(memory != 0)
            free(*(reinterpret_cast<void**>(memory)-1));
	}
	/* Далее функции, которым не нужно выделять дополнитульную память:
	 * Эта память "выделяется" внутри уже выделенной, а нам лишь приходит сообщение об этом.
	*/ 
	static void VKAPI_PTR mc_ialloc(void *pUserData, size_t size,
		VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
	{
		if (pUserData)
			static_cast<MemoryControllerData *>(pUserData)->ctrl->ilog(false, allocationType, allocationScope);
	}
	static void VKAPI_PTR mc_ifree(void *pUserData, size_t size,
		VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
	{
		if (pUserData)
			static_cast<MemoryControllerData *>(pUserData)->ctrl->ilog(true, allocationType, allocationScope);
	}
	//Конструктор, который соберёт всё воедино
	MemoryController()
	{
		data.ctrl = this; //присвоит указатель на контроллер
		callbacks.pUserData = GetData(); //присвоим для pUserData указатель на наши данные
		//соберём функции
		callbacks.pfnAllocation = (PFN_vkAllocationFunction) mc_alloc;
		callbacks.pfnReallocation = (PFN_vkReallocationFunction) mc_realloc;
		callbacks.pfnFree = (PFN_vkFreeFunction) mc_free;
		/*Следующие указатели могут быть NULL, так как лишь рассказывают о том, что внутри уже выделенной памяти взяли кусочек
		 * для чего-то ещё.
		*/
		callbacks.pfnInternalAllocation = (PFN_vkInternalAllocationNotification) mc_ialloc;
		callbacks.pfnInternalFree = (PFN_vkInternalFreeNotification) mc_ifree;
	}
};

//Функция для красоты консоли
void PrintDelimiter(uint8_t width, char c = '=')
{
	for (uint8_t delimeter_i = 0; delimeter_i < (width); delimeter_i++)
	{
		std::cout << c;
	}
	std::cout << "\n";
}

//Функция создания логического устройства
bool CreateDevice(VkPhysicalDevice gpu, uint32_t &fam_index, VkDevice &device)
{
	//Поиск семейств
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
	
	fam_index = valid_family_index;
	
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
	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = 0;
	device_info.enabledExtensionCount = 0;
	device_info.ppEnabledExtensionNames = 0;
	
	//Создание девайса
	if (vkCreateDevice(gpu, &device_info, NULL, &device) != VK_SUCCESS)
	{
		std::wcerr << L"Создать устройство не удалось.\n";
		return false;
	}
	return true;
} 

//Проверим работу контроллера:
bool MemoryHost()
{
	MemoryController controller; //создадим контроллер памяти
	VkInstanceCreateInfo instance_info; //информация о экзмепляре
	ZM(instance_info);
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VkInstance instance = VK_NULL_HANDLE;
	//создаём экземпляр с нашими callback'ами
	auto ret = vkCreateInstance(&instance_info, controller.GetCallbacks(), &instance);
	if (ret != VK_SUCCESS)
		return false;
	//уничтожаем экземпляр, также отправляя наши callback'и
	vkDestroyInstance(instance, controller.GetCallbacks());
	//работает нормально.
	return true; 
}

/* Теперь поговорим о памяти устройства. Vulkan позволяет контроллировать память устройства. Хотя, слово "позволяет"
 * здесь вписывается слабо: вам придётся её контроллировать. Для того, чтобы вы могли использовать ресурсы (такие, как
 * буферы и изображения), вам придётся запросить у устройства память для них, а потом привязать эту память к вашим ресурсам.
 * Забегая вперёд, скажу, что вы сначала создаёте виртуальный ресурс, который не может ничего в себе содержать до привязки к
 * памяти, и вы должны спросить, какие требования у него есть к памяти, и потом выдать необходимую память.
 * 
 * Поэтому посмотрим на работу с памятью устройства, а также памятью хоста.
*/

#define SIZE_B(size) (size)
#define SIZE_KB(size) ((SIZE_B(size))/1024)
#define SIZE_MB(size) ((SIZE_KB(size))/1024)
#define SIZE_GB(size) ((SIZE_MB(size))/1024)

#define SIZE_B_FROM(size) (size)
#define SIZE_KB_FROM(size) (SIZE_B_FROM(size)*1024)
#define SIZE_MB_FROM(size) (SIZE_KB_FROM(size)*1024)
#define SIZE_GB_FROM(size) (SIZE_MB_FROM(size)*1024)

bool MemoryDevice()
{
	VkInstanceCreateInfo instance_info; //информация о экзмепляре
	ZM(instance_info);
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VkInstance instance = VK_NULL_HANDLE;
	auto ret = vkCreateInstance(&instance_info, NULL, &instance);
	if (ret != VK_SUCCESS)
		return false;
	//Первым делом, посмотрим список физических устройств, а также свойства их памяти.
	std::vector<VkPhysicalDevice> gpu_list;
	uint32_t gpu_count = 0;
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr) != VK_SUCCESS)
        return false;
    gpu_list.resize(gpu_count);
    if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
        return false;
	std::wcout << L"Найдено " << gpu_list.size() << L" видеокарт.\n";
	//Заранее объявим переменные выбранного утройства
	VkPhysicalDevice gpu;
	VkPhysicalDeviceMemoryProperties memory_properties_choosed;
	uint32_t mem_type_index = -1;
	uint32_t mem_type_index_host = -1;
	//Просматриваем устройства
	for (size_t gpu_index = 0; gpu_index < gpu_list.size(); gpu_index++)
	{
		std::wcout << L">>> Смотрим информацию о устройстве с индексом " << gpu_index << L".\n";
		VkPhysicalDeviceMemoryProperties memory_properties;
		uint32_t mem_type_index_local = -1;
		uint32_t mem_type_index_host_local = -1;
		vkGetPhysicalDeviceMemoryProperties(gpu_list[gpu_index], &memory_properties);
		/* В структуре можно найти информацию о кучах (heap) и типах памяти.
		 * Кучи бывают разные, но в основном их не так много.
		*/
		std::wcout << L"В устройстве " << memory_properties.memoryHeapCount << L" куч.\n";
		for (uint32_t heap_index = 0; heap_index < memory_properties.memoryHeapCount; heap_index++)
		{
			PrintDelimiter(80/4, '+');
			VkMemoryHeap &heap = memory_properties.memoryHeaps[heap_index];
			std::wcout << L"В куче " << heap_index << L".\n";
			uint32_t bytes = SIZE_B(heap.size);
			//uint32_t kbytes = SIZE_KB(heap.size);
			uint32_t mbytes = SIZE_MB(heap.size);
			uint32_t gbytes = SIZE_GB(heap.size);
			std::wcout << L"Размер кучи " << bytes << L" байт (т.е. "<< mbytes << L" мегабайт, или " <<
				gbytes << L" гигабайт).\n";
			/* Локальная память устройства отличается от памяти хоста по характеристикам производительности.
			 * Так что если здесь нет такого флага — скорее всего это память хоста, которая дополнительно
			 * используется устройством.
			*/
			if (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
				std::wcout << L"Это локальная память устройства.\n";
			PrintDelimiter(80/4, '+');
		} //heap
		std::wcout << L"В устройстве " << memory_properties.memoryTypeCount << L" типов памяти.\n";
		for (uint32_t type_index = 0; type_index < memory_properties.memoryTypeCount; type_index++)
		{
			PrintDelimiter(80/4, '*');
			VkMemoryType &type = memory_properties.memoryTypes[type_index];
			std::wcout << L"В типе " << type_index << L".\n";
			std::wcout << L"Использует кучу с индексом " << type.heapIndex << L".\n";
			if (type.propertyFlags == 0)
			{
				/* Обычно это некэшированная память хоста.
				 * Выделение этой памяти равносильно malloc/free.
				*/ 
				std::wcout << L"Это обычная память.\n";
			}
			if (type.propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				/* Если тип имеет такой флаг, то это наиболее подходящий по производительности вариант, так
				 * как находится прямо в устройстве.
				*/ 
				std::wcout << L"Это локальная память устройства.\n";
				//сохраним этот тип памяти
				mem_type_index_local = type_index;
			}
			if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				/* Такую память можно прочитать с хоста (управлять ей полностью вручную).
				 * О том, как это делается, будет показано позже.
				*/ 
				std::wcout << L"Эта память просматриваемая для хоста.\n";
				mem_type_index_host_local = type_index;
			}
			if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			{
				/* В этом случае, команды контроля кэша на хосте vkFlushMappedMemoryRanges и vkInvalidateMappedMemoryRanges
				 * не обязаны делать записи хоста видимыми для устройства и записи устройства видимыми для хоста, соответсвенно.
				 * Т.е. все данные скрыты памяти скрыты друг от друга.
				*/ 
				std::wcout << L"Это память привязана к хосту.\n";
			}
			if (type.propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
			{
				/* Вся память, созданная с этим типом кэшируется на хосте. Нужна для ускореня запросов к памяти,
				 * так как запросы к некэшуремой памяти медленней, чем к кэшированной. Так или иначе, нэкэшированная память
				 * привязана к хосту.
				*/ 
				std::wcout << L"Это память кэшируется на хосте.\n";
			}
			if (type.propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			{
				/* Этот тип памяти доступен только устойсства, поэтому он не может содержать также флаг
				 * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT (так как это протеворечие).
				*/ 
				std::wcout << L"Это приватная память устройства.\n";
			}
			PrintDelimiter(80/4, '*');
		} //type
		PrintDelimiter(80, '-');
		/* А теперь пора создать наше устройство.
		 * Тем не менее, в будущем нам обязательно пригодится структура с данными о памяти, хоть и не сейчас.
		 * Предстваим, что нам нужно взять локальну память устройства. Переменная mem_type_index_local уже хранит нужный индекс,
		 * и если всё правильно, то мы его заберём вместе с физическим устройством и другими данными.
		*/
		if (mem_type_index_local != (uint32_t) -1)
		{
			gpu = gpu_list[gpu_index];
			memory_properties_choosed = memory_properties;
			mem_type_index = mem_type_index_local;
			mem_type_index_host = mem_type_index_host_local;
			break;
		}
	} //gpu
	//Создадим устройство
	VkDevice device;
	uint32_t fam_index = (uint32_t) -1;
	if (!CreateDevice(gpu, fam_index, device))
		return false;
	/* И так, память устройства можно и нужно распределять вручную. Допустим, мы хотим выделить память для нескольких ресурсов,
	 * а потом распределить её. Допустим, нам хватит 16 МБ, т.е. 16777216 байт. Т.е. кол-во выделяемой памяти задаётся, естесственно,
	 * всегда в байтах. Для выделения памяти нам нужно заполнить информацию о ней (снова бюрократия, но это нормально).
	*/ 
	VkMemoryAllocateInfo alloc_info;
	ZM(alloc_info);
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = SIZE_MB_FROM(16);
	alloc_info.memoryTypeIndex = mem_type_index;
	VkDeviceMemory memory;
	ret = vkAllocateMemory(device, &alloc_info, NULL, &memory);
	if (ret != VK_SUCCESS)
		return false;
	/* Так мы выделили память устройства. Ровно 16 МБ. К слову говоря, мы, так или иначе, не можем её прочитать напрямую, если она
	 * не Host visible. Всё, что мы можем сделать — скопировать память туда или обратно, но не записывать, используя указатели
	 * языка (например, void * на память устройства) или что-либо ещё.
	*/ 
	vkFreeMemory(device, memory, NULL);
	/* Тем не менее, есть и память, которую мы можем прочитать прочитать и асоциировать, такая память называется
	 * отображаемой (mappable). Мы можем просто получить указатель на память, и работать с этой памятью, как с обычной
	 * памятью хоста. Обычно, это и есть память хоста, но имплементация разрешает вариант отображаемой памяти устройства.
	 * Таким образом, мы можем получить указатель на эту память и работать с ней через этот указатель.
	*/
	alloc_info.memoryTypeIndex = mem_type_index_host;
	ret = vkAllocateMemory(device, &alloc_info, NULL, &memory);
	if (ret != VK_SUCCESS)
		return false;
	void *pMemory = nullptr;
	ret = vkMapMemory(device, memory, //хэндлы девайса и памяти
		0, alloc_info.allocationSize, //смещение от начала и размер, необходимый для работы
		0, &pMemory); //флаги (зарезервированы) и указатель на указатель, последний из которых будет содержать адресс памяти
	if (ret != VK_SUCCESS)
		return false;
	/* Учтите, что вам придётся синхронизировать доступ к памяти, так как иногда она бывает не дступной для записи/чтения,
	 * так как с ней может работать устройство. Как всё это дело синхронизировать? Покажем на примере!
	 * Для прикола запишем туда 4 рандомных байта:
	*/
	uint8_t test[4] = {12, 18, 54, 45};
	memcpy(pMemory, test, 4);
	/* Далее, убеждаемся, что мы не собираемся использовать эту память в девайсе (для чтения/записи) и не собираемся записывать
	 * в память что-то ещё (если наше приложение многопоточное). После этого "отправляем" эту память — т.е. гарантируем, что память
	 * доступна устройству для чтения. Мы можем одновременно отправялть на подтверждение сразу несколько диапазонов памяти.
	 * Для каждого из диапазонов используется следующая структура:
	*/
	VkMappedMemoryRange mem_range;
	ZM(mem_range);
	mem_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mem_range.memory = memory; //память, которую мы используем
	mem_range.offset = 0; //смещение в памяти
	mem_range.size = 4; //размер
	//После этого отправляем все необходимые структуры:
	ret = vkFlushMappedMemoryRanges(device, 1, &mem_range);
	if (ret != VK_SUCCESS)
		return false;
	/* Также, есть функция, которая используется для обратного назначения — чтобы гарантировать, что все записи сделанные
	 * устройством будут видны и на хосте.
	*/
	ret = vkInvalidateMappedMemoryRanges(device, 1, &mem_range);
	if (ret != VK_SUCCESS)
		return false;
	/* Как только мы добились гарантии — можно снова использовать тот полученный нами указатель. Да-да, он никуда
	 * не терялся, и мы можем использовать его до сих пор, не смотря на то, что устройство могло взаимодействовать с той памятью.
	 * Ну, а если нам больше не нужен доступ к памяти, мы можем отвзяать память от указателя.
	 * И после этого работать с указателем, естественно, нельзя.
	 * Конечно же, есть и последовательная (coherent) паммять, для которой не нужно использовать функции vkFlushMappedMemoryRanges и
	 * vkInvalidateMappedMemoryRanges.
	*/ 
	vkUnmapMemory(device, memory); //отвязываем указатель pMemory
	vkFreeMemory(device, memory, NULL); //освобождаем саму память
	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
	return true;
}

int main()
{
	setlocale(LC_ALL, "Russian");
	if (!MemoryHost())
		return -1;
	PrintDelimiter(80);
	if (!MemoryDevice())
		return -1;
	PrintDelimiter(80);
	return 0;
}


/* В общем, спасибо, что уделили этому уроку время. Khronos Group, Niko Kauppi, и тем, кто делает примеры — Большое Спасибо!
 * А дальше классика:
 * Если вы хотите меня поддержать, прикрепляю кошель с Я.Денег:
 * 410012557544062
 * Ну или просто жмя по ссылке:
 * https://money.yandex.ru/to/410012557544062
 * Хе-хе.
*/
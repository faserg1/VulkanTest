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

/* Для начала объявим класс, который будет заниматься всем управлением. Это может быть конечно же не класс, а просто
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
		std::wcout << L"Память выделена.\n";
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
			((MemoryControllerData *) pUserData)->ctrl->log(size, alignment, scope);
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
			((MemoryControllerData *) pUserData)->ctrl->log_free();
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
			((MemoryControllerData *) pUserData)->ctrl->ilog(false, allocationType, allocationScope);
	}
	static void VKAPI_PTR mc_ifree(void *pUserData, size_t size,
		VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
	{
		if (pUserData)
			((MemoryControllerData *) pUserData)->ctrl->ilog(true, allocationType, allocationScope);
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

//А теперь проверим работу:

int main()
{
	setlocale(LC_ALL, "Russian");
	MemoryController controller;
	VkInstanceCreateInfo instance_info;
	memset(&instance_info, 0, sizeof(instance_info));
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VkInstance instance = VK_NULL_HANDLE;
	auto ret = vkCreateInstance(&instance_info, controller.GetCallbacks(), &instance);
	if (ret != VK_SUCCESS)
		return 1;
	vkDestroyInstance(instance, controller.GetCallbacks());
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
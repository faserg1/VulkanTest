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

/* Не смотря на то, что я часто буду использовать пространство имём std,
 * using я ставить не буду, по крайней мере, на данный момент.
*/

/* Для Vulkan изначально необходим лишь самый главный заголовочный файл.
 * Все остальные расширения (extension) на данный момент добавляются через
 * указатели на функции и написание структур и т.д. вручную. Но об этом
 * немного позже. Также, прошу заметить, что в Vulkan есть так называемые
 * "слои" (layers). И это не одно и тоже, что и расширения. Принцип работы
 * слоёв такой: сначала пользователь вызывает любую функцию из Vulkan, затем
 * этот вызов обрабатывается первым привязанным слоем, затем вторым и т.д.,
 * пока не достигнут самого ядра Vulkan. Эти слои в основном необходимы для
 * отладки вашего приложения. Расширения же нужны для других целей. Например,
 * привязка поверхности (surface) к окну/экрану делается через специальное
 * расширение, для каждой оконной системы — своё. О привязке к окнам поговрим
 * намного позже, так как для начала лучше разобраться с основами.
*/
#include <vulkan/vulkan.h> 

//Имя нашего приложения. Вывел в глобальные.
char app_name[] = "Vulkan Tutorian. Device. © GrWolf.";

int main()
{
	//Прежде всего, настроем вывод.
	setlocale(LC_ALL, "Russian");
	/* Для начала подготовим необходимые данные. Все данные в Vulkan сильно 
	 * пакуются: для этого есть некотрое кол-во структур, содержащие в себе
	 * всю необходимую информацию для создания/получения того или иного объекта.
	 * Также, структуры содержат sType с типом
	 * typedef enum VkStructureType. В этом перечеслении (enum) содержаться все
	 * типы структур (коих на самом деле не так много). Сами значения — беззнаковые
	 * целые числа, вплоть до VK_STRUCTURE_TYPE_MAX_ENUM = 0x7FFFFFFF. Существуют
	 * также и другие специальные значения, такие как VK_STRUCTURE_TYPE_BEGIN_RANGE,
	 * VK_STRUCTURE_TYPE_END_RANGE и VK_STRUCTURE_TYPE_RANGE_SIZE (на момент 
	 * написания урока, конечно же, так что если интересно, загляните в заголовочный
	 * файл сами).
	 * Сам sType нужен для указания типа структуры (т.е. также и её размера).
	 * Зачем? Подробностей не знаю, могу лишь сделать намёк на Windows, где для
	 * ex-структур (расширенных, т.е. структуры для расширенного функционала Win32 API)
	 * нужно было первым параметром обязательно задавать сам размер этих структур.
	*/
	/* Первым делом подготовим информацию о нашем приложении: мы можем сказать 
	 * Vulkan'у некоторые подробности нашего приложения, такие как версию, имя,
	 * версию используемого Vulkan SDK, версию и имя движка, если используется 
	 * игровой движок (GameEngine).
	*/
	VkApplicationInfo app_info; //Вот структура, которая содержит всю эту информацию.
	memset(&app_info, 0, sizeof(app_info)); //Очистим нашу структуру (заполним нулями).
	/*Настроим тип структуры. В нашем случае это VK_STRUCTURE_TYPE_APPLICATION_INFO.
	 * Естественно, если оставить его в любом другом значении, ваше приложение не будет
	 * работать стабильно (или не будет работать вообще ;D).
	*/
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	/* Имя приложения может быть не задано. Тем не менее, если вы задаёте имя приложения,
	 * убедитесь, что строка кончается символом конца строки, т.е. это должна быть
	 * null-terminated string.
	*/
	app_info.pApplicationName = app_name;
	/* С именем движка тоже самое, что и с именем приложения: если не NULL, то тогда
	 * null-termanated string.
	 */
	app_info.pEngineName;
	/* Немного о pNext. Они существуют хоть и в (почти?) каждой структуре, но не везде 
	 * она может быть использована. В данном случае (и на момент написания урока), член
	 * этой структуры зарезевирован, и должен иметь значение NULL.
	*/
	app_info.pNext;
	/* В Vulkan API имеется свой макрос для создания версий, и также макрос текущей
	* версии API. На самом деле, apiVersion можно оставить NULL, правда, точной 
	* информации, как это повлияет на функционал Vulkan я ещё не нашёл. Поэтому не
	* будем особо с этим мудрить и споставим текущую версию: VK_API_VERSION.
	* На момент написания используется версия 1.0.3.
	* 
	* Тем не менее, если поставить другую версию, может выйти ошибка о несовместимости
	* с драйвером: возможно, что вы запросили версию, которую текущий драйвер не поддерживает.
	*/
	app_info.apiVersion = VK_API_VERSION;
	/*
	 * Немного о версиях и макросах (макровызовах) для них в Vulkan API.
	 * Для начала рассмотрим VK_MAKE_VERSION. Первый параметр — major version,
	 * т.е. первая цифра версии, самая главная и приоритетная. Затем следует minor,
	 * т.е. если в вашем приложении существуют может и не столь знаменательные
	 * изменения, но и не маленькие. И последняя — patch. Маленькие патчи, фиксы
	 * и всё что в этом духе. Примерно так и используется версии в Vulkan.
	 * Также, если вам нужно будет разобрать версию, используете эти макросы:
	 * VK_VERSION_MAJOR, VK_VERSION_MINOR, VK_VERSION_PATCH.
	 * Сам applicationVersion можно оставить нулём, особой роли вся эта информация
	 * здесь не играет.
	*/
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	//Также, как и с applicationVersion. Оставляю NULL.
	app_info.engineVersion;
	/* И так, собрана вся информация о приложении. На кой? Чтобы было, что в
	 * debug/crash-report написать, или ещё куда-нибудь. Т.е. чисто для обратной
	 * связи (кроме apiVersion).
	 * Теперь необходимо создать "экземпляр" нашего Vulkan (instance), затем
	 * через него мы создадим device. В этом Vulkan несколько похож на DirectX,
	 * только здесь не используется ООП (ИМХО, ибо знаком только с DX9).
	 * Для начала подготовим структуру VkInstanceCreateInfo.
	*/
	VkInstanceCreateInfo instance_info; //Создаём...
	memset(&instance_info, 0, sizeof(instance_info)); //... и заполним её нулями.
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; //зададим тип структуры.
	/* В данном случае pNext каким-то образом связан с расширениями и зарезервирован для них.
	 * Тем не менее, он должен быть со значением NULL.
	*/
	instance_info.pNext;
	//Зарезервировано. Должно быть NULL.
	instance_info.flags;
	/*
	 * А вот и pApplicationInfo! Хотя, на самом деле, может спокойно быть NULL.
	 * Но если не NULL, то там должна содержаться информация о приложении. Даже
	 * если это нули, кроме sType.
	*/
	instance_info.pApplicationInfo = &app_info;
	/*
	 * Немного о расширениях: здесь как раз мы укажем (правда, не в этом уроке),
	 * сколько и какие расширения мы будем использваоть. В enabledExtensionCount,
	 * как уже ясно, нужно указать кол-во используемых расширений. Дальше,
	 * ppEnabledExtensionNames — это фактически массив строк. Допустим, const char[][].
	 * Ну, или std::vector<const char*>, как мы для удобства потом и поступим.
	 * Каждая строка — имя используемого расширения.
	*/
	instance_info.enabledExtensionCount;
	instance_info.ppEnabledExtensionNames;
	/* Что касается слоёв, с ними абсолютно такая же штука: кол-во и имена.
	 * Подробнее о слоях будет рассказано в следующем уроке.
	*/
	instance_info.enabledLayerCount;
	instance_info.ppEnabledLayerNames;
	/* Готово! Информация для создания экземпляра Vulkan готова к применению.
	 * Теперь подготовим принимающие объекты.
	*/
	VkResult create_instance_result; //тут будем хранить  и обрабатывать результат.
	/* Отдельно стоит рассказать о handle'ах в Vulkan. Каждый такой хэндл создаётся
	 * через VK_DEFINE_HANDLE, из-за чего в некоторых IDE подсветка работает неправильно.
	 * Но так как Win32 API использует почти такую же фишку с созданием типа данных через
	 * макровызов, то в Visual Studio всё должно работать прекрасно (в отличии от Code::Blocks
	 * или Code Lite, которым я пользуюсь в данный момент). Для них есть также специальный
	 * макрос VK_NULL_HANDLE, который заменяет привычный NULL.
	 * Хэндл сам по себе, как это показывается в vulkan.h — указатель на структуру, если это
	 * Dispatchable хэндл, которыми являются:
	 * VkInstance
	 * VkPhysicalDevice
	 * VkDevice
	 * VkQueue
	 * VkCommandBuffer
	 * В другом случае зависит от битности (32/64). Поэтому на хэндлах не рекомендуется использовать
	 * nullptr из C++11 и выше. И получается, что все остальные хэндлы являются Non-dispatchable,
	 * также, такой тип хэндлов необязательно является реальным указателем (имеется в виду, в самой
	 * реализации Vulkan API).
	*/
	VkInstance instance = VK_NULL_HANDLE;
	/* Теперь, нужно вызвать саму функцию для создания. Первый аргументом идёт ссылка
	 * на информацию о создаваемом экземпляре. Последним — указатель на хэндл, который
	 * в последствии перестанет быть нулевым, если всё порошло удачно.
	 * Что касается второго параметра (const VkAllocationCallbacks *), то это фактически
	 * управление памятью. Нет, не видеопамятью, а оперативной. Если вы "не доверяете"
	 * Vulkan, а точнее тем, как он распределяет память — сделайте свой обработчик.
	 * Подробнее о том, как его сделать — напишу в следующих уроках. А сейчас мы оставим
	 * NULL. Причём, именно NULL, а не VK_NULL_HANDLE — это обычная структура, а не хэндл.
	*/
	create_instance_result = vkCreateInstance(&instance_info, NULL, &instance);
	/* И так, наш экземпляр получен... а может быть и нет! Теперь нужно проверить всё на
	 * ошибки, вдруг пользователь (коем вы частично являетесь) использует настолько старый,
	 * слабый или просто перегруженный компьютер, что даже экземпляр Vulkan не смог появится!
	 * И так, в данном случае могут быть такие результаты:
	 * VK_SUCCESS — всё хорошо, всё получилось.
	 * VK_ERROR_OUT_OF_HOST_MEMORY — не хватает оперативной памяти.
	 * VK_ERROR_OUT_OF_DEVICE_MEMORY — не хватает видеопамяти.
	 * VK_ERROR_INITIALIZATION_FAILED — магия, провалилась инициализация, внутренняя ошибка.
	 * VK_ERROR_LAYER_NOT_PRESENT — заданный слой не может быть загружен.
	 * VK_ERROR_EXTENSION_NOT_PRESENT — заданное расширение не может быть загружено.
	 * VK_ERROR_INCOMPATIBLE_DRIVER — юзер не поставил новые дрова или всё гораздо хуже.
	 * Ну что ж, сделаем проверку!
	*/
	if (create_instance_result != VK_SUCCESS) //проверяем на удачу..
	{
		switch (create_instance_result) //.. воспользуемся стандартной переключалкой..
		{
			case VK_ERROR_INCOMPATIBLE_DRIVER: //..делаем case..
				std::wcerr << L"Дрова поставь!!!\n"; //..обрабатываем ошибку..
			default:
				std::wcerr << L"Что-то не так...\n"; //..ну и так далее..
		}
		exit(1); //..ведь в конце концов приложение всё равно дало сбой.
	}
	else
		std::wcout << L"Экземпляр Vulkan создан.\n";
	/* И так, для чего же нам может понадобится экземпляр Vulkan? Во-первых, так мы можем создать
	 * устройство (device), которое будет рисовать и заниматься прочими нужными делами. Во-вторых,
	 * так мы можем узнать, сколько у пользователя GPU и что они из себя представляют, т.е. и их
	 * возможности (capabilities) тоже. В-третьих, получить специальные указатели на функции, которые
	 * не представлены в обычном виде, но так или иначе используют instance (например, функции расширений).
	 * 
	 * В дополнение скажу, что разрушение экземпляра не разрушит за собой все его дочерние объекты, в том числе
	 * и устройство. Поэтому прежду чем уничтожить экземпляр, нужно уничтожить все устройства, созданные этим 
	 * экземпляром. И конечно же, разные экземпляры Vulkan не общаются между собой и никак на друг друга не влияют.
	 * 
	 * И вроде бы, на этом всё. Поэтому начнём создавать устройство-device.
	 *
	 * Теперь немного об устройстве. В Vulkan есть разделение на физическое устройство, так и на логическое.
	 * Примерно как с жёскими дисками: есть как физический диск и логический. Причём, логических дисков может быть
	 * несколько на одном физическом. Тоже самое и с Vulkan. Сначала мы узнаем подробности о физическом устройстве,
	 * и только потом создаём "на нём" логическое.
	 * 
	 * Для создания логического устройства нам необходимо будет указать следующее: какие в нём будут слои и расширения,
	 * какие очереди (queue) из какого семейтсва (family) будут использованы, их приоритет и количество,
	 * и самое главное — какое физическое устройство для этого будет использовано. Да, грубо говоря, Vulkan
	 * позваляет фактически распараллелить вашу отрисовку на разные GPU.
	 * Ну, можно ещё как-нибудь применить эту полезную возможность. Главное для вас, это посмотреть на их
	 * информацию, убедиться, что с ними всё хорошо и они подходят, и вперёд!
	 * 
	 * Для начала необходимо выяснить, сколько и какие физические устройства есть у пользователя.
	 * VkPhysicalDevice — хэндл. Поэтому будем хранить... ну допустим весь список наших хэндлов.
	*/
	std::vector<VkPhysicalDevice> gpu_list; //здесь будем хранить
	/* Для того, чтобы посчитать и перечислить все девайсы, используется функция vkEnumeratePhysicalDevices.
	 * У неё бывают такие результаты:
	 * VK_SUCCESS — всё хорошо.
	 * VK_INCOMPLETE — всё хорошо, но получен не весь возможный список.
	 * и дальше по старинке:
	 * VK_ERROR_OUT_OF_HOST_MEMORY
	 * VK_ERROR_OUT_OF_DEVICE_MEMORY
	 * VK_ERROR_INITIALIZATION_FAILED
	 * И так, параметры функции такие: VkInstance, указатель на uint32_t, т.е. указатель на кол-во хэндлов,
	 * и также указатель на хэнлд или хэндлы, в зависимости от кол-ва. Работает простым способом — если 
	 * указатель на хэндлы пуст, то функция заполнит значение по адресу pPhysicalDeviceCount, т.е. кол-во
	 * существуемых у пользователя физических устройств. Так и поступим.
	*/
	uint32_t gpu_count; //кол-во девайсов
	//получаем кол-во девайсов и сразу проверяем на удачу.
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		std::wcerr << L"Посчитать физические устройства не удалось :(\n";
		exit(1);
	}
	//тут мы можем немного поиздеваться над юзверями.
	std::wcout << L"Человек, я всё от тебе знаю!\n";
	std::wcout << L"Даже то, что у тебя " << gpu_count << L" видеокарт.\n";
	gpu_list.resize(gpu_count); //заранее изменим размер под самый оптимальный.
	/* А теперь, чтобы всё таки получить все наши девайсы, нам нужно вызывать функцию ещё раз!
	 * Но сейчас мы не будем отправлять VK_NULL_HANDLE третьим аргументом, а второй параметр уже не
	 * выходной, а входной, т.е. pPhysicalDeviceCount — сколько наш массив может выдержать
	 * или сколько мы хотим получить в данный момент.
	*/
	if (vkEnumeratePhysicalDevices(instance, &gpu_count, gpu_list.data()) != VK_SUCCESS)
	{
		std::wcerr << L"Заполучить твои физические устройства не удалось, но я приду за ними в следующий раз!\n";
		exit(1);
	}
	/* Получив список всех видеокарт, мы же не будем выбирать видеокарту случайным образом?
	 * Хотя, кто вас знает... Но если вы всё же хороший программист и не ищете гневных отзывов 
	 * о том, что ваше приложение ничего не может, то всё же вам придётся получать информацию о
	 * возможностях каждой из видеокарт и в дальнейшем, решать, что из этого выбрать.
	 * За получение информации о видеокарте (хотя, на самом деле это могут быть не только видеокарты)
	 * отвечают следующие функции:
	 * vkGetPhysicalDeviceFeatures — получение информации о возможностях устройства (лимиты, форматы и т.д.)
	 * vkGetPhysicalDeviceFormatProperties — проверка определённого формата
	 * vkGetPhysicalDeviceImageFormatProperties — проверка определённого формата изображения
	 * vkGetPhysicalDeviceProperties — получение свойств устройства (версия драйвера, совместимый API, имя видеокарты)
	 * vkGetPhysicalDeviceMemoryProperties — получение информации о памяти устройства
	 * vkGetPhysicalDeviceQueueFamilyProperties — получение информации о семействах очередей. 
	 * И последняя функция — для нас самая важная, ибо теперь всё не так просто, как в DX/GL.
	 * Для начала, нам нужны устройства не invalid'ы, т.е. нам нужно понять, могут ли они вообще работать
	 * с графикой. Для этого нам нужно проверить, содержит ли это устройство семейство, которое поддерживает,
	 * разные графические команды (допустим те же команды отрисовки), и затем, указать индекс этого
	 * семейства при создании одной или более очередей.
	 * И на данный момент, вас сейчас уже должно беспокоить, что же это за очередь такая и с чем её едят.
	 * 
	 * Объясняем. Очередь будет содерать в себе команды, которые будет выполнять наше устройство, причём,
	 * этих самых очередей может быть использованно несколько для разных целей или одинаковых целей.
	 * Эти самые очереди могут принадлежать одному семейству, хотя может быть и несколько очередей одного семейства.
	 * Короче говоря, семейство, это фактически вид, или тип очереди, которые в свою очередь
	 * могут иметь как разные так и одинаковые возможности:
	 * графические (команды рисования, привязка графических конвейеров и графических.. статов? состояний?)
	 * вычислительные (привязка вычеслительных конвейеров и состояний)
	 * передающие (копирование данных и изображений)
	 * редкие (использование памяти для редких изображений и буферов, оптимизация).
	 * В спецификации написано, что обычно семейства с одинаковыми возможностями объеденяются. Тем не менее,
	 * возможно появление семейств с одинаковыми возможностями, и это вполне нормально.
	 * 
	 * Теперь сделаем некое упрощение: не будем переберать все физические устройства и сравнивать их возможности.
	 * Предположим (хотя так почти всегда и бывает), что у пользователя одна видеокарта, которая легко находится
	 * по индексу 0, также она поддерживает всё, что нам необходимо, и мы пока не станем проверять её точные возомжности.
	 * Возьмём эту видеокарту, как основную, и про остальные (если они существуют) забудем. 
	*/
	//Так как это хэндл (на подобии HWND в Win32 API) не является каким-либо объектом класса, а является указателем..
	VkPhysicalDevice gpu = gpu_list[0]; //мы просто копируем его.
	/* Теперь, мы узнаем, какие семейтва есть у этой видеокарты.
	 * Для этого мы воспользуемся vkGetPhysicalDeviceQueueFamilyProperties, причём дважды и всё по тем же причинам:
	 * сначала мы узнаем кол-во семейств, а затем получим их свойства. Всё достаточно просто, когда к этому привыкаешь.
	*/
	uint32_t family_count = 0; //Кол-во семейств.
	//Функция void'ная, значений не возвращает.
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, VK_NULL_HANDLE);
	std::wcout << L"Я нашёл " << family_count << L" семейств. Похвали меня!\n";
	//Шикарно. Теперь создадим буфер, который будет хранить свойства этих семейств.
	std::vector<VkQueueFamilyProperties> family_properties_list(family_count);
	//И получим их все.
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_properties_list.data());
	/* Теперь, можно бы и пролистать их и посмотреть на свойства, но для начала, нам нужно кое-что, что
	 * не потеряет индекс семейства, который может поддерживать графику.
	*/
	uint32_t valid_family_index = (uint32_t) -1; //значение -1 будем использовать как "не найдено".
	for (uint32_t i = 0; i < family_count; i++) //листаем все семейства.
	{
		VkQueueFamilyProperties &properties = family_properties_list[i];
		std::wcout << L"В семействе с индексом " << i << L" существует ";
		//queueCount оказывает, сколько всего очередей есть у этого семейства.
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
		exit(1);
	}
	else //Ну а иначе всё хорошо..
	{
		std::wcout << L"Мы будем использовать " << valid_family_index << L" индекс.\n";
	}
	/* Хорошо, теперь мы нашли наш индекс волшебства, теперь, прежде чем приступать к
	 * созданию логического устройства, нам нужно создать для него очереди. Как же без них?
	 * Иначе нам просто некуда будет отсылать команды! Для начала создадим структуру.
	*/
	VkDeviceQueueCreateInfo device_queue_info;
    memset(&device_queue_info, 0, sizeof(device_queue_info)); //очистим её.
	device_queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; //зададим её тип.
	
	/* Теперь кое-что поинтересней. У очередей есть приоритет выполнения, т.е, какой из очереди
	 * будет уделяться больше внимания. Приоритет регулируется float значением от 0, т.е. самого
	 * низкого приоритета, до 1, самого выского. Одинаковый приоритет нескольким очередям выдавать можно,
	 * но Vulkan API ничего не обещает насчёт чтения ваших мыслей о том, как правильно это должно работать.
	 * Сами приоритеты — массив float значений. Мы создадим только одну очередь, поэтому и значение у нас одно.
	*/
	float device_queue_priority[] = {1.0f};
	/* Укажем кол-во очередей, которые нам нужно, сейчас нам хватит и одной.
	 * Причём, если указать больше, то они будут иметь одинаковое семейство.
	 */ 
	device_queue_info.queueCount = 1;
	device_queue_info.queueFamilyIndex = valid_family_index; //укажем индекс семейства этих очередей
	//и помни — массив, это указатель на первый слот самого массива.
	device_queue_info.pQueuePriorities = device_queue_priority; 
	//Флаги зарезервированы для будущего использования.
	device_queue_info.flags;
	//
	device_queue_info.pNext;
	/* Теперь всё очень круто! Прям зашибись! Теперь нам осталось только заполнить информацию о нашем логическом
	 * устройстве, и всё в наших руках!
	*/ 
	
	VkDeviceCreateInfo device_info; //Создаём.
    memset(&device_info, 0, sizeof(device_info)); //Очищаем.
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; //Задаём.
	/* Указываем кол-во структур, которые содержат информацию о очередях и их количестве.
	 * Это нужно для того, чтобы мы могли за раз создать очереди с разными семействами, так как одна VkDeviceQueueCreateInfo
	 * поддерживает только один queueFamilyIndex. Помните об этом.
	*/
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &device_queue_info;
	//дальше параметры по старинке:
	device_info.flags; //для будущего использования
	device_info.pNext; //для расширений
	//Расширения и слои
	device_info.enabledLayerCount;
	device_info.ppEnabledLayerNames;
	device_info.enabledExtensionCount;
	device_info.ppEnabledExtensionNames;
	/* А дальше кое-что поинтересней. Но все подробности будут оглашены потом.
	 * Можно включать и выключать у логичекого устройства некоторые возможности с помощью следующего:
	*/
	device_info.pEnabledFeatures;
	/* Это указатель на структуру VkPhysicalDeviceFeatures. Как указано в спецификации:
	 * Если вы хотите включить все возможные (поддерживаемые) фичи: отправьте указатель на структуру,
	 * полученную из vkGetPhysicalDeviceFeatures. Если вы хотите включить только необходимые (required) фичи,
	 * отправьте NULL.
	*/ 
	//Настало время создать наше логическое устройство. Для этого нам нужен его пустой хэндл..
	VkDevice device = VK_NULL_HANDLE;
	/* .. и конечно же функция создания! Первый параметр — физическое устройство. Убедитесь, что вы используете
	 * family_index именного с этого, а не иного устройства. Даже если у вас и того и другого по одной штуке.
	 * Второй параметр — информация о будущем логическом устройстве. Третий — управление памятью.
	 * Четвёртый, выходной параметр — логическое устройтво.
	 * Могут быть такие результаты:
	 * VK_SUCCESS
	 * VK_ERROR_OUT_OF_HOST_MEMORY
	 * VK_ERROR_OUT_OF_DEVICE_MEMORY
	 * VK_ERROR_INITIALIZATION_FAILED
	 * VK_ERROR_LAYER_NOT_PRESENT
	 * VK_ERROR_EXTENSION_NOT_PRESENT
	 * VK_ERROR_TOO_MANY_OBJECTS — подали на создание СЛИШКОМ МНОГО ЯБЛОК
	 * VK_ERROR_DEVICE_LOST — устройство потеряно. Подробнее об этом будет расказно позже.
	 * И так, создание!
	*/ 
	if (vkCreateDevice(gpu, &device_info, NULL, &device) != VK_SUCCESS)
	{
		std::wcerr << L"Чёрт! А я был так близко...\n";
		exit(1);
	}
	std::wcout << L"Ура! Получилось! Device наш!\n";
	/* Теперь немного об ошибке device lost (потеря устройства). Сколько помню DX9, то такая ошибка могла возникнуть даже
	 * при закрытии окна, в которое мы что-либо рисовали, если оно было крепко привязано к устройству. В Vulkan
	 * потеря устройства может быть лишь по такому ряду причин:
	 * 1) Ошибка в устройстве, возможно вызванная вашими командами из приложения.
	 * В этом случае нужно уничтожить устройство, но перед этим нужно также уничтожить все его дочернии объекты —
	 * их хэндлы всё ещё дейсвительны, а некоторые команды могут также продолжать работу, и также могут вернуть
	 * как удачу, так и error — device lost.
	 * После этого также возможно создание логического устройства заново, и можно продолжить работу, если это не 2 пункт.
	 * 2) Отсоединение и/или потеря физического устройства.
	 * В этом случае создать логическое устройство заново нельзя — так как физическое устройство уже не может работать.
	 * 3) Ошибка в системе, повреждение памяти.
	 * В этом случае Vulkan не гарантирует стабильное выполнение команд.
	 * 
	 * Во всех остальных случаях устройство потеряно быть не может.
	 * 
	 * И так, на этом этапе, мы сделали наше устройство. Дальше его можно использовать как вашей душе угодно...
	 * Почти. Я надеюсь, вы не собираетесь его сжечь/пристрелить/убить? Что? Собираетесь? Ну ладно.
	 * Давайте убьём всё это!
	 * 
	 * Для уничтожения нам понадобятся:
	 * void vkDestroyDevice
	 * void vkDestroyInstance
	 * И с этим мудрить ничего не надо. Только если необходимо, задать AllocationInfo вторым аргументом. Поехали!
	*/
	vkDestroyDevice(device, NULL);
	vkDestroyInstance(instance, NULL);
	/* Готово! Кумамон одобряет, мы сожгли всё, что породили. Даже пройдя через такие трудные методы.
	 * К слову, один человек во время GDC 2016 сказал:
	 * "Vulkan API старались сделать максимально понятным. Но понятный — не значит простой."
	 * Да, Vulkan API действительно отличается от того же DirectX или OpenGL, хотя, конечно же, есть и
	 * некоторые схожие места. Но для чего же его усложнили? Для того, чтобы программист решал, что и в какой мере
	 * нагружать, это раз, а второе, чтобы оптимизировать работу и "общение" между видеокартой и процессором.
	 * Теперь мы можем просто отправить пачкой команды в видеокарту, а не каждый раз теребить по одной команде
	 * "сделай то, сделай это". Ну и другие полезные вещи в Vulkan API тоже есть.
	*/
	std::wcout << L"Пожарено!\n";
	return 0;
}

/*
 * В общем, спасибо, что уделили этому уроку время. Khronos Group, Niko Kauppi, и тем, кто делает примеры — Большое Спасибо!
 * А дальше классика:
 * Если вы хотите меня поддержать, прикрепляю кошель с Я.Денег:
 * 410012557544062
 * Ну или просто жмя по ссылке:
 * https://money.yandex.ru/to/410012557544062
 * Хе-хе.
*/
| | | |
|:---|:---:|---:|
|[Назад][Prev]|[Наверх][Up]|[Вперёд][Next]|

#Небольшая предыстория
И так, что же за слои и расширения в Vulkan?
+ **Слоем** называют специальное дополнение, которым передают вызов той или функции Vulkan, прежде чем она достигнет ядра. Слоёв может быть несколько, и каждый занят своей определённой работой. Причём, при определённом условии слой может не передать вызов функции дальше (например, при ошибке, чтобы не навредить тем самым приложению, драйверу и/или устройству). Слоёв может быть несколько, и вызов функции должен будет пройти через все слои, прежде чем достичь ядра Vulkan. Тем не менее, по умолчанию все слои отключены, дабы ускорить процесс отрисовки кадра. Тем не менее, слой обычно не добавляет дополнительный функционал, а лишь исследует вызов функции, поэтому, основное их предназночение — проверка и отладка. И разрабочики слоёв бывают разные. Их мужно определить по названию: VK_LAYER_[имя_разработчика]_[название_слоя]. Перечеслю мне известные слои:
  + `VK_LAYER_GOOGLE_unique_objects` — помещает все объекты в unique pointer (уникальный указатель), и разыменовывает при использовании.
  + `VK_LAYER_LUNARG_api_dump` — выводит все вызовы функций, их параметры и значения. 
  + `VK_LAYER_LUNARG_device_limits` — проверяет, а не слишком и вы обжорливый. Т.е. проверка ваших параметров на то, чтобы они соответствовали лимитам устройства.
  + `VK_LAYER_LUNARG_core_validation` — проверяет сеты дескрипторов (descriptor set), состояния конвейера (pipeline state), проверяет интерфейсы между модулем SPIR-V и графическим конвейером, выполняет трасировку и проверку GPU памяти, а также её привязку ко объектам и командным буферам.
  + `VK_LAYER_LUNARG_image` — проверяет форматы текстур и целей отрисовки (render target).
  + `VK_LAYER_LUNARG_object_tracker` — трассирует объекты Vulkan, неверные флаги и утечки памяти объектов.
  + `VK_LAYER_LUNARG_parameter_validation` — проверяет параметры, передаваемые в функции.
  + `VK_LAYER_LUNARG_swapchain` — проверяет WSI расширение swapchain на правильное использование.
  + `VK_LAYER_GOOGLE_threading` — проверяет правильное использование API в мульти-поточном приложении.
  + `VK_LAYER_LUNARG_standard_validation` — мета-слой, при включении которого включает следующие слои:
    + `VK_LAYER_GOOGLE_threading`
    + `VK_LAYER_LUNARG_parameter_validation`
    + `VK_LAYER_LUNARG_device_limits`
    + `VK_LAYER_LUNARG_object_tracker`
    + `VK_LAYER_LUNARG_image`
    + `VK_LAYER_LUNARG_core_validation`
    + `VK_LAYER_LUNARG_swapchain`
    + `VK_LAYER_GOOGLE_unique_objects`
  + `VK_LAYER_LUNARG_vktrace` — слой, который не нужно включать вручную. Он нужен для трассировки приложения через утилиту vktrace. После этого можно будет воспользоваться утилитой vkreplay (подробнее об этом будет в одном из следующих уроков).
  + `VK_LAYER_RENDERDOC_Capture` — этот слой также не следует включать вручную. Нужен для программы RenderDoc, которое сделает отладку приложения ещё более удобной.
  + `VK_LAYER_NV_optimus` — включает технологию NVIDIA Optimus. ~~Экономия заряда батареи? Что? Что зелёные курят и почему со мной не поделились?~~
  + `VK_LAYER_VALVE_steam_overlay` — слой для Steam Overlay. Как он работает и что делает? А хрен его знает!
+ **Расширение** — это дополнение, которое предоставляет дополнительный функционал в Vulkan. Дополнения могут быть совершенно разными, например:
  + WSI — несколько расширений, с помощью которых настраивается вывод в окно или на экран (Vulkan по умолчанию не привязывается к окну).
  + Debug Report — можно указать свои callback-функции, которые будут вызываться из проверочных слоёв для передачи определённой инормации.
  + [Расширение растерезации от AMD](http://gpuopen.com/unlock-the-rasterizer-with-out-of-order-rasterization/ "Перейти к описанию") — похоже, доступно только для обладателей *"красных"* видеокарт. Что это такое и как оно работает — можете прочитать потом сами.
  + Расширение от NVIDIA — поддержка GLSL шейдера напрямую, без трансляции в SPIR-V (короче — **костыль**, забудьте про это расширение, когда захотите сделать что-то серьёзное).
  + [Расширение от Khronos Group](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/xhtml/vkspec.html#_vk_khr_sampler_mirror_clamp_to_edge "Подробнее"), которое позволяет создавать семплеры, где их UVW пространство может быть отражено.
  + И другие, которые возможно уже вышли, а возможно ещё нет.

Кстати, некоторые слои могут использовать расширения, если такие включены в работу. Например, некоторые проверочные слои могут использовать отладочное расширение. И поэтому я хочу рассказать в этом уроке именно об этом — отладке с помощью проверочных слоёв и отладочного расширения.

## Лирическое отступление
Раньше, до версии 1.0.13 существовало разделение на instance-слои и device-слои. Начиная с версии 1.0.13 такого разделения нет, и все слои считаются глобальными, а некоторые методы — устаревшими (deprecated). Поэтому все слои, добавленные в instance будут работать и на дочерних объектах (т.е. также и на устройстве), тем не менее, методы пока не удаляли, дабы была поддержка предыдущих версий API.

#Поиск слоёв и расширений
Для начала воспользуемся функциями Vulkan для того, чтобы найти все доступные слои и расширения.

## Поиск слоёв
Слои Vulkan (а также их свойства) можно найти с помощью функции
``` c++
VkResult vkEnumerateInstanceLayerProperties(
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties);
```
Я думаю, вы уже догадались как она работает. Первый параметр может быть как принимающим в функцию(in), так и получающим из функции(out). А второй может быть получающим, если первый — принимающий (in).
		
+ `pPropertyCount` — количество структур `VkLayerProperties`, которые нужно заполнить (in) или количество доступных слоёв (out).
+ `pProperties` — струтукры свойств слоёв. Может быть `nullptr`.

К слову, теперь эти слои считаются глобальными — работают как в экземпляре, так и в устройстве.

Структура выглядит так:
``` c++
typedef struct VkLayerProperties {
	char        layerName[VK_MAX_EXTENSION_NAME_SIZE];
	uint32_t    specVersion;
	uint32_t    implementationVersion;
	char        description[VK_MAX_DESCRIPTION_SIZE];
} VkLayerProperties;
```
+ `layerName` — имя слоя.
+ `specVersion` — версия Vulkan, для которой был написан этот слой.
+ `implementationVersion` — версия слоя (обычный unsigned integer, инкрементируемый, если произошли какие-то изменения).
+ `description` — описание слоя в UTF-8.

Вот пример, как можно вывести в консоль все доступные слои:
``` c++
uint32_t available_instance_layer_count = 0;
res = vkEnumerateInstanceLayerProperties(&available_instance_layer_count, nullptr);
if (res != VK_SUCCESS)
	return;
std::vector<VkLayerProperties> available_instance_layers(available_instance_layer_count);
res = vkEnumerateInstanceLayerProperties(&available_instance_layer_count, available_instance_layers.data());
if (available_instance_layer_count)
{
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
```
Я получил такой вывод:
```
Слои экземпляра:
VK_LAYER_LUNARG_api_dump                |LunarG debug layer
VK_LAYER_LUNARG_core_validation         |LunarG Validation Layer
VK_LAYER_LUNARG_device_limits           |LunarG Validation Layer
VK_LAYER_LUNARG_image                   |LunarG Validation Layer
VK_LAYER_LUNARG_object_tracker          |LunarG Validation Layer
VK_LAYER_LUNARG_parameter_validation    |LunarG Validation Layer
VK_LAYER_LUNARG_screenshot              |LunarG image capture layer
VK_LAYER_LUNARG_swapchain               |LunarG Validation Layer
VK_LAYER_GOOGLE_threading               |Google Validation Layer
VK_LAYER_GOOGLE_unique_objects          |Google Validation Layer
VK_LAYER_LUNARG_vktrace                 |Vktrace tracing library
VK_LAYER_RENDERDOC_Capture              |Debugging capture layer for RenderDoc
VK_LAYER_NV_optimus                     |NVIDIA Optimus layer
VK_LAYER_VALVE_steam_overlay            |Steam Overlay Layer
VK_LAYER_LUNARG_standard_validation     |LunarG Standard Validation Layer
```
Для устройства существует другая функция, но она устаревшая, так что в скором времени её могут убрать вообще:
``` c++
VkResult vkEnumerateDeviceLayerProperties(
	VkPhysicalDevice                            physicalDevice,
	uint32_t*                                   pPropertyCount,
	VkLayerProperties*                          pProperties);
```
+ `physicalDevice` — физическое устройство. 
+ `pPropertyCount` — количество структур `VkLayerProperties`, которые нужно заполнить (in) или количество доступных слоёв (out).
+ `pProperties` — струтукры свойств слоёв. Может быть `nullptr`.
	
##Поиск расширений
	
Для расширений есть другая функция.
``` c++
VkResult vkEnumerateInstanceExtensionProperties(
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties);
```	
+ `pLayerName` — имя слоя, для которого будут искаться совместимые расширения. Можно оставить nullptr, чтобы вывести все.
+ `pPropertyCount` — количество структур `VkExtensionProperties`, которые нужно заполнить (in) или количество доступных расширений (out).
+ `pProperties` — струтукры свойств расширений.

В отличии от слоёв, эти расширения распространяются только на экземпляр.

Кстати, если задавая параметр playerName можно проверить, какие расширение какие слои используют. Например, при перечеслении слоёв на каждый слой вызывать функцию перечесления расширений.

Сама структура имеет вид:
``` c++
typedef struct VkExtensionProperties {
	char        extensionName[VK_MAX_EXTENSION_NAME_SIZE];
	uint32_t    specVersion;
} VkExtensionProperties;
```
+ `extensionName` — имя расширения.
+ `specVersion` — версия расширения (обычный unsigned integer, инкрементируемый, если произошли какие-то изменения).

Смотрим на доступные расширения:
``` c++
uint32_t available_instance_extension_count = 0;
res = vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extension_count, nullptr);
if (res != VK_SUCCESS)
	return;
std::vector<VkExtensionProperties> available_instance_extensions(available_instance_extension_count);
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
```
Я получаю вот такой вывод:
```
Расширения экземпляра:
VK_KHR_surface
VK_KHR_win32_surface
VK_EXT_debug_report
```
Расширение `VK_KHR_win32_surface` — плавтформозависимое, так что у вас может быть и другой результат.

Функция для получения расширений устройства. Она не устаревшая/deprecated, так как есть расширения, предназначенные только для устройства, и есть расширения, предназначенные только для экземпляра (хотя могут быть и общие).
``` c++
VkResult vkEnumerateDeviceExtensionProperties(
	VkPhysicalDevice                            physicalDevice,
	const char*                                 pLayerName,
	uint32_t*                                   pPropertyCount,
	VkExtensionProperties*                      pProperties);
```
+ `physicalDevice` — физическое устройство. 
+ `pLayerName` — имя слоя, для которого будут искаться совместимые расширения. Можно оставить nullptr, чтобы вывести все.
+ `pPropertyCount` — количество структур `VkExtensionProperties`, которые нужно заполнить (in) или количество доступных расширений (out).
+ `pProperties` — струтукры свойств расширений.
		
##Включение слоёв и расширений
Для этого мы должны отправить список слоёв и расширений в инфо-структуру об экземпляре. Для начала подготовим массивы, который будет хранить имена слоёв и расширений:
``` c++
std::vector<const char *> instance_layers;
std::vector<const char *> instance_extensions;

std::vector<const char *> device_layers; //deprecated
std::vector<const char *> device_extensions;
```
Добавим необходимые слои и расширения. Пусть это будут слои стандартной проверки `VK_LAYER_LUNARG_standard_validation` и отладочное расширение `VK_EXT_debug_report`, для которого есть даже специальный макрос VK_EXT_DEBUG_REPORT_EXTENSION_NAME.
``` c++
instance_layers.push_back("VK_LAYER_LUNARG_standard_validation");
instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

device_layers = instance_layers; //deprecated
```
Затем, добавим все эти списки в информацию о экземпляре:
``` c++
VkInstanceCreateInfo instance_info;
//...
instance_info.enabledLayerCount = instance_layers.size();
instance_info.ppEnabledLayerNames = instance_layers.data();

instance_info.enabledExtensionCount = instance_extensions.size();
instance_info.ppEnabledExtensionNames = instance_extensions.data();
//...
```
А также в информацию об устройстве:
``` c++
VkDeviceCreateInfo device_info;
//...

device_info.enabledLayerCount = device_layers.size();
device_info.ppEnabledLayerNames = device_layers.data();

device_info.enabledExtensionCount = device_extensions.size();
device_info.ppEnabledExtensionNames = device_extensions.data();
//...
```
Готово. Слои и расширения подключены. Если какой-то из них не найден, могут быть ошибки `VK_ERROR_LAYER_NOT_PRESENT` или `VK_ERROR_EXTENSION_NOT_PRESENT`.

#Отладочное расширение

Не смотря на то, то у нас теперь были добавлены слои, нужно теперь с них получать информацию. Конечно, можно использовать слой api dump для вывода в консоль/терминал (stdout), но так мы потеряем несколько возможностей, которые даёт нам расширение Debug Report. Для начала, нужно получить две важные функции — функции расширения для добавления и удаления Callback'ов. Эти функции не линкуются библиотекой по умолчанию, так что в любом случае нам нужно будет получить указатель на функцию. Для начала создадим эти указатели:
``` c++
	PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = NULL;
	PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = NULL;
```
`PFN_` — Pointer Function, указатель на функцию. А `f` в начале указывает на то, что это fetch-функция, т.е. полученая. Хотя называйте как хотите, это влияет только на понимание кода. Подробнее об этих типах данных:
``` c++
typedef VkResult (VKAPI_PTR *PFN_vkCreateDebugReportCallbackEXT)(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback);
typedef void (VKAPI_PTR *PFN_vkDestroyDebugReportCallbackEXT)(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator);
typedef void (VKAPI_PTR *PFN_vkDebugReportMessageEXT)(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage);
```
Длинновато, не так ли? После этого, нужно получить адреса этих функций.
``` c++
fvkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)
	vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
fvkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)
	vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
```	
Функции получены. Теперь необходимо заполнить структуру, которая содержит информацию о нашей функции обратного вызова (callback function):
``` c++
typedef struct VkDebugReportCallbackCreateInfoEXT {
	VkStructureType                 sType;
	const void*                     pNext;
	VkDebugReportFlagsEXT           flags;
	PFN_vkDebugReportCallbackEXT    pfnCallback;
	void*                           pUserData;
} VkDebugReportCallbackCreateInfoEXT;
```
+ `sType` — тип структуры , т.е. `VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT`.
+ `pNext` — пустует (`nullptr`).
+ `flags` — Флаги сообщений, которые Callback функция будет принимать. Например, можно указать, что функция будет принимать только сообщения об ошибках.
+ `pfnCallback` — указатель на Callback функцию.
+ `pUserData` — просто указатель. Можно оставить `nullptr`, а можно что-то поместить. Если он не `nullptr`, то этот указатель будет поступать в callback функцию.

Подробнее о флагах:
``` c++
typedef enum VkDebugReportFlagBitsEXT {
	VK_DEBUG_REPORT_INFORMATION_BIT_EXT = 0x00000001,
	VK_DEBUG_REPORT_WARNING_BIT_EXT = 0x00000002,
	VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT = 0x00000004,
	VK_DEBUG_REPORT_ERROR_BIT_EXT = 0x00000008,
	VK_DEBUG_REPORT_DEBUG_BIT_EXT = 0x00000010,
} VkDebugReportFlagBitsEXT;
typedef VkFlags VkDebugReportFlagsEXT;
```
+ `VK_DEBUG_REPORT_INFORMATION_BIT_EXT` — простые ифнормационные сообщения.
+ `VK_DEBUG_REPORT_WARNING_BIT_EXT` — предупреждения.
+ `VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT` — предупреждения о производительности.
+ `VK_DEBUG_REPORT_ERROR_BIT_EXT` — ошибки.
+ `VK_DEBUG_REPORT_DEBUG_BIT_EXT` — отладочные сообщения (сообщения от самого расширения).

Учтите, что вывод всех сообщений в консоль вызовет огромную задержку, так что старайтесь избегать этого или можно отключить несколько флагов.

Теперь посмотрим, как должна выглядеть сама функция:
``` c++
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
	//..
	return VK_FALSE;
}
```
И так, зачем нужны `VKAPI_ATTR` и `VKAPI_CALL`? Всё просто, это соглашения о вызовах и аттрибуты функции. Ставить лучше и то и другое, дабы сохранить переносимость кода. Хотя в Windows используются только соглашения о вызовах, то в Linux — аттрибуты.

Возвращаемый тип — логический (bool). Если вернуть `VK_TRUE`, то передача вызова в следующий слой (а в следствии и ядро Vulkan) прекратиться. Следственно, `VK_FALSE` будет означать обратное — выполнение вызова продолжится.

Разберём, какие параметры у функции:

+ `flags` — это флаг сообщения (обычно один). Так можно определить тип сообщения, которое пришло.
+ `objectType` — тип объекта, с которым что-то произошло.
+ `object` — сам объект.
+ `location` — местонахождение объекта... тут я не в силах найти внятное описание этому происшествию. Строчка кода? Адрес памяти? Кто знает...
+ `messageCode` — код сообщения.
+ `pLayerPrefix` — слой (а точнее его префикс — сокращённое имя), в котором образовалось сообщение.
+ `pMessage` — само сообщение.
+ `pUserData` — указатель, который был до этого прикреплён в структуре с информацией о callback'е.

И так, всё необходимое есть. Можно прицеплять! Для начала рассмотрим саму функцию создания (без излишеств):
``` c++
VkResult vkCreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugReportCallbackEXT* pCallback);
```	
+ `instance` — хэндл экземпляра, к которому привяжется callback.
+ `pCreateInfo` — указатель на структуру с ифнормацией о callback.
+ `pAllocator` — callback'и для управления памятью. Может быть `nullptr`.
+ `pCallback` — возвращаемый хэндл для callback, чтобы была возможность отвязать callback (а также не путать их).

Прицепляем:
``` c++
VkDebugReportCallbackCreateInfoEXT debug_report_callback_info;
memset(&debug_report_callback_info, 0, sizeof(debug_report_callback_info));
debug_report_callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
debug_report_callback_info.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT |
	VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
	VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
debug_report_callback_info.pfnCallback = DebugReportCallback;

VkDebugReportCallbackEXT debug_report_callback = VK_NULL_HANDLE;
res = fvkCreateDebugReportCallbackEXT(instance, &debug_report_callback_info, nullptr, &debug_report_callback);
```
Прикреплять можно сколько угодно раз. Но достаточно и одного. Этот Callback распростроянется на весь Vulkan.
	
Когда нам уже не нужен будет callback, его нужно отцепить и разрушить хэндл. Сама функция разрешения выглядит так:
``` c++
void vkDestroyDebugReportCallbackEXT(
	VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks* pAllocator);
```	
+ `instance` — хэндл экземпляра, к которому привязан callback.
+ `callback` — хэндл полученный при привязке callback-функции.
+ `pAllocator` — callback'и для управления памятью. Может быть `nullptr`.

Ну и разрушается таким способом:
``` c++
fvkDestroyDebugReportCallbackEXT(instance, debug_report_callback, nullptr);
```
#Заключение
Отлаживайте приложения правильно. А если вам не нужны лишние затраты, и приложение уже должно выйти в свет — отключайте проверочные слои и Debug Report. Это даст может и не большое, но преимущество.

| | | |
|:---|:---:|---:|
|[Назад][Prev]|[Наверх][Up]|[Вперёд][Next]|

[К Readme][Readme]

[Up]: ../Readme.md "Наверх"
[Prev]: ../00Device/Tutorial.md "Назад"
[Next]: ../02Commands/Tutorial.md "Вперёд"
[Readme]: ./Readme.md "К Readme"
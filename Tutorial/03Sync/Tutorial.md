#Синхронизация в Vulkan
Вспомним некоторые вещи из предыдущего урока: заполненые командами буферы поступают в очередь, после этого эти команды запускаются, но не ждут другие команды, т.е. в Vulkan каждая команда не зависит друг от друга, также, не зависит друг от друга устройство и хост — они работают параллельно друг другу, т.е. при отправке команд в устройство, хост не ждёт выполнения всех команд, а работа его продолжается. Но есть инструменты Vulkan'а, которые позволяют синхронизировать хост и девайс, команды и т.д — это 4 примитима синхронизации:
+ **Забор** (*fence*) — самый примитивный примитив. Позволяет проверить с хсста выполнение какого-либо процесса на устройстве (выполнились ли командные буферы, например).
+ **Семафор** (*semaphores*) — позволяет синхронизировать выполнение командных буферов на устройстве. Но синхронизировать хост и устройство с помощью них нельзя.
+ **Событие** (*event*) — тонкая настройка — можно настроить зависимости между хостом, устройством, командами и т.д.
+ **Барьер** (*barrier*) — создаёт зависимости между наборами команд.

Каждый примитив может находится в двух состояниях:
+ **Не-сигнальное** (*unsignaled*) — обычное состояние примитива. Фактически означает, что ничего важного для этого примитива не произошло.
+ **Сигнальное** (*signaled*) — состояние, которое означает, что произошло что-то, на что примитив был настроен.

##Забор
На данный момент, забор может использоваться в 3 местах:
+ Посылка командных буферов в очередь.
+ Асссоциирование разрежженых ресурсов.
+ Ожидание доступности изображения из цепочки переключений (WSI: Swapchain).

Так что заборы могут пыть полезны в очень редких случаях. Тем не менее, лучше узнать про них подробнее.
###Создание и уничтожение забора
Для создания забора, нужно заполнить следующую структуру.
``` c++
typedef struct VkFenceCreateInfo {
	VkStructureType sType;
	const void* pNext;
	VkFenceCreateFlags flags;
} VkFenceCreateInfo;
```
+ `sType` — тип структуры, в данном случае `VK_STRUCTURE_TYPE_FENCE_CREATE_INFO`.
+ `pNext` — указатель на ESS.
+ `flags` — флаги создания.

Флаги могут быть такими:
``` c++
typedef enum VkFenceCreateFlagBits {
	VK_FENCE_CREATE_SIGNALED_BIT = 0x00000001,
} VkFenceCreateFlagBits;
```
+ `VK_FENCE_CREATE_SIGNALED_BIT` — означает, что забор должен быть в сигнальном состоянии с самого начала. По умолчанию состояние забора — не-сигнальное.
``` c++
VkResult vkCreateFence(
	VkDevice device,
	const VkFenceCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkFence* pFence);
```		
+ `device` — хэндл устройства.
+ `pCreateInfo` — информация о создании забора.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.
+ `pFence` — возвращаемый хэндл забора.

Функция может вернуть:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

Разрушить забор может эта функция:
``` c++
void vkDestroyFence(
	VkDevice device,
	VkFence fence,
	const VkAllocationCallbacks* pAllocator);
```	
+ `device` — хэндл устройства.
+ `fence` — хэндл забора, который необходимо разрушить.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.

Забор нельзя разрушить, если он всё ещё используется.

###Сброс забора
Если один и тот же забор будет использован несколько раз, перед повторным использованием его нужно сбросить. Для этого есть функция:
``` c++
VkResult vkResetFences(
	VkDevice device,
	uint32_t fenceCount,
	const VkFence* pFences);
```	
+ `device` — хэндл устройства.
+ `fenceCount` — количество заборов, которые нужно сбросить.
+ `pFences` — хэндлы заборов (массив).

После этого, все переданные заборы будут переведены в не-сигнальное состояние. Если забор был уже в не-сигнальном состоянии, его сброс не принисёт никакого эффекта. Функция возвращает:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

###Проверка и ожидание
После отправки забора можно проверить его текущее состояние (без ожидания):
``` c++
VkResult vkGetFenceStatus(
	VkDevice device,
	VkFence fence);
```
+ `device` — хэндл устройства.
+ `fence` — хэндл забора, который необходимо проверить.

Возвращаемые коды:

+ `VK_SUCCESS` — забор просигнален.
+ `VK_NOT_READY` — забор не просигнален.
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`
+ `VK_ERROR_DEVICE_LOST` — устройство потеряно.


А также, можно ждать один и более заборов:
``` c++
VkResult vkWaitForFences(
	VkDevice device,
	uint32_t fenceCount,
	const VkFence* pFences,
	VkBool32 waitAll,
	uint64_t timeout);
```	
+ `device` — хэндл устройства.
+ `fenceCount` — количество заборов, которые нужно ждать.
+ `pFences` — хэндлы заборов (массив).
+ `waitAll` — нужно ли ждать *все* заборы (`VK_TRUE`) или достаточно подожать *один* (`VK_FALSE`). Если забор один, данный параметр не имеет значения.
+ `timeout` — максимальное время ожидания в наносекундах (1 наносекунда — 10<sup>-9</sup> секунд, или 1/1.000.000.000 секунд). При времени ожидания 0 функция проверит состояние забора (или заборов) и сразу же вернёт управление. Если значение максимальное (`VK_WHOLE_SIZE`, или `~0ULL`), то функция будет ждать сигнала бесконечно.

Возвращает:

+ `VK_SUCCESS` — забор(ы) просигнален.
+ `VK_TIMEOUT` — время истекло.
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`
+ `VK_ERROR_DEVICE_LOST` — устройство потеряно.

###Пример
Создание забора:
``` c++
VkFence fence = VK_NULL_HANDLE;
VkFenceCreateInfo fence_create_info;
ZM(fence_create_info); //zero memory
fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
vkCreateFence(device, &fence_create_info, NULL, &fence);
```
Отправка команд вместе с забором:
``` c++
//...
vkQueueSubmit(queue, 1, &submit_info, fence);
```
Ожидание забора:
``` c++
vkWaitForFences(device, 1, &fence, false, VK_WHOLE_SIZE);
```
###Гарантии
Забор даёт гарантию на то, что все команды (или что там ещё...) выолнились. Но забор не даёт гарантий на то, что результат этих команд (а именно память) доступна для хоста. Для такой гарантии нужно применить барьер.

##Стадии конвейера и зависимости
###Стадии конвейера
Перед тем, как продолжить, сначала стоит пояснить несколько важных вещей. Конвейер, конечно, можно приостановить на определённой команде, но это бы привело к потере производительности, которой можно избежать. Для этого команды можно приостановить на определённой стадии. О том, как это работает и где можно использовать — чуть позже. А пока о стадиях.
``` c++
typedef enum VkPipelineStageFlagBits {
	VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT = 0x00000001,
	VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT = 0x00000002,
	VK_PIPELINE_STAGE_VERTEX_INPUT_BIT = 0x00000004,
	VK_PIPELINE_STAGE_VERTEX_SHADER_BIT = 0x00000008,
	VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT = 0x00000010,
	VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020,
	VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT = 0x00000040,
	VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT = 0x00000080,
	VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT = 0x00000100,
	VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT = 0x00000200,
	VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400,
	VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT = 0x00000800,
	VK_PIPELINE_STAGE_TRANSFER_BIT = 0x00001000,
	VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT = 0x00002000,
	VK_PIPELINE_STAGE_HOST_BIT = 0x00004000,
	VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT = 0x00008000,
	VK_PIPELINE_STAGE_ALL_COMMANDS_BIT = 0x00010000,
} VkPipelineStageFlagBits;
```
Вышеуказанные значения будут использваться как флаги масок, так как можно указывать несколько значений. А сейчас стоит разобрать каждый флаг подробнее.

+ `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT` — это флаг обозначает стадию конвейера, на которой команды только-только попали в очередь.
+ `VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT` — стадия, где используются данные от DrawInderect или DispatchInderect.
+ `VK_PIPELINE_STAGE_VERTEX_INPUT_BIT` — стадия, на которой используются данные вершинных и индесных буферов.
+ `VK_PIPELINE_STAGE_VERTEX_SHADER_BIT` — стадия, на которой включается вершинный шейдер.
+ `VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT` — на этой стадии включается шейдер контроля тесселяции.
+ `VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT` — стадия шейдера оценки тесселяции.
+ `VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT` — стадия геометрического шейдера.
+ `VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT` — стадия фрагментного (пиксельного шейдера).
+ `VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT` — стадия ранней фрагметной (пиксельной) проверки.
+ `VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT` — стадия поздней фрагметной (пиксельной) проверки.
+ `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` — стадия после смешивания (blending), когда подаются финальные значения на выход из конвейера. Эта стадия также включает в себя *resolve* операцию, которая происходит в конце подпрохода.
+ `VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT` — стадия, на которой запускается вычеслительный шейдер.
+ `VK_PIPELINE_STAGE_TRANSFER_BIT` — стадия, на которой выполняются команды копирования. Это такие команды, как:
  + `vkCmdCopyBuffer`
  + `vkCmdCopyImage`
  + `vkCmdBlitImage`
  + `vkCmdCopyBufferToImage`
  + `vkCmdCopyImageToBuffer`
  + `vkCmdUpdateBuffer`
  + `vkCmdFillBuffer`
  + `vkCmdClearColorImage`
  + `vkCmdClearDepthStencilImage`
  + `vkCmdResolveImage`
  + `vkCmdCopyQueryPoolResults`
+ `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` — завершающая стадия конвейера, где команды завершили своё исполнение.
+ `VK_PIPELINE_STAGE_HOST_BIT` — псевдо-стадия, означающая что хост будет читать/записывать память устройства.
+ `VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT` — работа всех графических стадий.
+ `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` — работа всех стадий, поддерживаемых очередью.

**Заметка 1.**
К слову, `VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT` и `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` не одно и тоже, что и `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT`. А именно, если `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` не задерживает другие стадии (так как сам флаг означает только одну), то флаги *All* будут (так как означают несколько стадий). Также, объявляя зависимость памяти, память будет доступна и/или видима на всех стадиях, если поставлен флаг *all*, в то время как флаг конца конвейера этого не означает, так как фактически не содержит в себе рабочую стадию. `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` можно использовать, дабы убедится в завершении конвейера.

**Заметка 2.**
Если импементация не может обновить состояние события на завершении текущей стадии, имплементация может сделать это по завершению следующей (к примеру, вместо сигнала после вершинного шейдера, сигнал после *color attachment*). Лимит — все графические команды. Также, если имплементация не может ждать определённую стадию, она будет ждать логически более раннюю стадию.
Также, если имплементация не может включить зависимость выполнения на определённых стадиях, она может включить зависимость дополнительных исходных (*source*) стадий и/или дополнительных конечных (*destenation*) стадий чтобы сохранить зависимость.
Если в имплементации происходит такое, то это не должно влиять на семантику зависимостей выполнения или памяти или барьеров изображений и буферов.


Некоторые стадии поддерживаются только очередями, которые поддерживают определённый набор операции. Ниже прдставлена таблица, что какие флаги поддержки должны быть у очереди, чтобы поддерживать определённый флаг стадии конвейера. Если во второй колонке указано несколько флагов, это значит, что стадия конвейера будет поддерживаться, если есть поддержка одного из указанных флагов очереди.

|Флаг стадии конвейера | Требуемый флаг возможностей очереди|
| -------------------- | -----------------------------------|
|`VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT`|Нет|
|`VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT`|`VK_QUEUE_GRAPHICS_BIT` или `VK_QUEUE_COMPUTE_BIT`|
|`VK_PIPELINE_STAGE_VERTEX_INPUT_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_VERTEX_SHADER_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT`|`VK_QUEUE_COMPUTE_BIT`|
|`VK_PIPELINE_STAGE_TRANSFER_BIT`|`VK_QUEUE_GRAPHICS_BIT`, `VK_QUEUE_COMPUTE_BIT` или `VK_QUEUE_TRANSFER_BIT`|
|`VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT`|Нет|
|`VK_PIPELINE_STAGE_HOST_BIT`|Нет|
|`VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT`|`VK_QUEUE_GRAPHICS_BIT`|
|`VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`|Нет|

###Зависимости
Команды синхронизации позволяют указать **явную** (*explicit*) зависимость между двумя наборами (set) команд, где второй набор команд зависит от первого. Наборами могут быть:
+ Первый набор: перед `vkCmdSetEvent`.<br>
  Второй набор: после `vkCmdWaitEvents` в той же очереди и с тем же событием.
+ Первый набор: команды внутри подпрохода с меньшим индексом (или перед проходом отрисовки).<br>
  Второй набор: команды внутри подпрохода с большим индексом, где прдеставлена зависимость между этими двумя подпроходами (или подпроходом и `VK_SUBPASS_EXTERNAL`).
+ Первый набор: команды перед барьером.<br>
  Второй набор: команды после барьера в той же очереди (по возможности также ограничены одним подпроходом).

**Зависимостью выполнения** (*execution dependency*) называют одиночную зависимость наборов исходных и конечных стадий конвейера, которая гарантирует, что вся работа набора стадий в `srcStageMask` будет выполнена в первом наборе команд, прежде чем будет выполнятся работа набора стадий в `dstStageMask` во втором наборе команд.

**Цепочкой зависимостей выполнения** (*execution dependency chain*) от набора исходных стадий A до набора конечных стадий B называют последовательность зависимостей выполнения послынных в очередь в промежутке между первым набором команд и вторым набором команд, сохраняя следующие условия:
+ первая зависимость включает в себя A или `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` или `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` в `srcStageMask`. И...
+ последняя зависимость включает в себя B или `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT` или `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` в `dstStageMask`. И...
+ для каждой зависимости в последовательности (исключая первую) как минимум одна соблюдать следующие условия:
  - `srcStageMask` текущей зависимости включает в себя хотя бы один бит C, который был в `dstStageMask` предыдущей зависимости. Или...
  - `srcStageMask` текущей зависимости включает в себя `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` или `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT`. Или...
  - `dstStageMask` предыдущей зависимости включает в себя `VK_PIPELINE_STAGE_ALL_COMMANDS_BIT` или `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT`. Или...
  - `srcStageMask` текущей зависимости включает в себя `VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT`, а `dstStageMask` предыдущей зависимости включает в себя как минимум один флаг графической стадии конвейера. Или...
  - `dstStageMask` предыдущей зависимости включает в себя  `VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT`, а `srcStageMask` текущей зависимости включает в себя как минимум один флаг графической стадии конвейера.
+ для каждой зависимости в последовательности (исключая первую) как минимум одна соблюдать следующие условия:
  - текущая зависимость — это пара `vkCmdSetEvent`/`vkCmdWaitEvents` (где `vkCmdWaitEvents` может быть как внутри, так и снаружи прохода отрисовки); или `vkCmdPipelineBarrier` снаружи прохода отрисовки; или зависимостью подпрохода отрисовки, где `srcSubpass` равен `VK_SUBPASS_EXTERNAL` для текущего прохода отрисовки, который начиается с команды `vkCmdBeginRenderPass`, где предыдущей зависимостью может быть любая из следующих:
    * пара `vkCmdSetEvent`/`vkCmdWaitEvents`  или `vkCmdPipelineBarrier`, любой из которых находится вне прохода отрисовки, который предшествует текущей зависимости в очереди порядка выполнения. Или...
	* зависимость подпрохода, где `dstSubpass` равен `VK_SUBPASS_EXTERNAL` для прохода отрисовки, который был завершён командой `vkCmdEndRenderPass` и который предшествует текущей зависимости в очереди порядка выполнения.
  - текущая зависимость — это зависимость подпрохода для прохода отрисовки, где предыдущей зависимостью может быть любая из следующих:
    * другая зависимость для того же прохода отрисовки, где `dstSubpass` равен `srcSubpass` текущей зависимости. Или...
	* `vkCmdPipelineBarrier` того же прохода отрисовки, записанный для подпрохода, указанном в `srcSubpass` текущей зависимости. Или...
	* пара `vkCmdSetEvent`/`vkCmdWaitEvents`, где `vkCmdWaitEvents` внутри того же прохода отрисовки и записанный для подпрохода, указанном в `srcSubpass` текущей зависимости.
  - текущая зависимость — `vkCmdPipelineBarrier` внутри подпрохода для прохода отрисвоки, где предыдущей зависимостью может быть любая из следующих:
    * зависимость подпрохода для того же прохода отрисовки, `dstSubpass` равен проходу с `vkCmdPipelineBarrier`. Или...
	* `vkCmdPipelineBarrier` того же прохода отрисовки, записанный для того же подпрохода перед текущей зависимостью. Или...
	* пара `vkCmdSetEvent`/`vkCmdWaitEvents`, где `vkCmdWaitEvents` внутри того же прохода отрисовки и записанный для того же подпрохода перед текущей зависимостью.


##Семафор
Семафоры используются, чтобы синхронизировать очереди и посылки (например, командных буферов) в очередь. Менять статус с сигнально на не-сигнальный и обратно могут только очереди (хост не может этого делать), а также ожидаются они только очередями.
##Событие
##Барьер
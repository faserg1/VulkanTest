| | | |
|:---|:---:|---:|
|[Назад][Prev]|[Наверх][Up]|[Вперёд][Next]|

#Очереди
**Очередь** — место, куда попадают команды. Их создание определяется ещё на моменте создания устройства: набор разных очередей с разным приоритетом и семейством. И поэтому, прежде, чем отправлять какие-либо команды в устройство, мы должны получить именно очередь, в которую они будут в последствии попадать. А точнее — её хэндл. 
##Получения хэндла
За это отвечает функция
``` c++
void vkGetDeviceQueue(
	VkDevice 	device,
	uint32_t 	queueFamilyIndex,
	uint32_t 	queueIndex,
	VkQueue* 	pQueue);
```	
 + `device` — хэндл устройства, у которого мы хотим взять хэндл очереди.
 + `queueFamilyIndex` — индекс семейства, из которого мы берём очередь.
 + `queueIndex` — индекс очереди (индексы очереди будут соответствовать индексам заданных приоритетов).
 + `pQueue` — полчучемый хэндл очереди.
 
Поэтому, нам нужно сохранять индексы семейств и очередей до определённого момента (а лучше — хранить до их уничтожения), так как они могут не раз пригодится. Вот, как это всё работает:
``` c++
VkQueue queue = VK_NULL_HANDLE;
vkGetDeviceQueue(vkGlobals.device, vkGlobals.family_index, 0, &queue);
```
Функции разрушения очереди не существует, ибо очереди (физически) существуют всегда. Но пользоваться ими можно только в течении жизни устройства. Логически устройства разрушаются вместе с разрушением устройства.

##Синхронизация очереди

Дабы убедится, что очередь сейчас ничем не занята, существует функция
``` c++
VkResult vkQueueWaitIdle(
	VkQueue queue);
```	
Результаты функции могут быть такие:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`
+ `VK_ERROR_DEVICE_LOST`

# Команды
Команды из неоткуда не берутся. Нам понадобится **командный пул**, чтобы из него выделять **командные буферы**, в которые и будут записываться команды. После этого командные буферы будут двумя способами отправлятся в очередь.

**Командный буфер** в свою очередь разделяется на два вида: **первичный** (*primory*) и **вторичный** (*secondary*).

+ **Первичный** командный буфер может быть отправлен прямиком в очередь, а также может запустить вторичный командный буфер, при условии, что последний успешно записан.
+ **Вторичный** командный буфер **НЕ** может быть отправлен в очередь напрямую, он может быть только запущен первичным.

##Важно
Важно понимать следующую вещь: Команды в командный буфер записываются в таком же порядке, в каком были вызваны команды записи. Командные буферы поступают в очередь в таком же порядке, в каком и были туда посланы. **Но**, команды лишь **запускают** процесс и **не дожидаются** завершения этого процесса, к тому же, некоторые команды могут запускаться вообще параллельно друг другу или раньше других, не зависимо от порядка, в котором они были записаны. Поэтому команды могут работать параллельно друг другу, и не все команды завершаются быстро или мгновенно — каждая команда тратит определённое время, свойственное ей. Поэтому, чтобы не было казусов, при котором одна команда пытается обработать результат, который ещё не был выпущен предыдущей командой, в Vulkan существует синхронизация. О том, как её использовать — будет в следующем уроке.

Командные буферы имеют три стадии:

+ **Изначальная** (*initial*) — командные буферы созданы или сброшены. Они не содержат никаких команд.
+ **Записываемая** (*recording*) — командные буферы ожидают записи команд. Любая функция `vkCmd...(...)` запишет в указаный командный буфер команду, с которой ассоциирована функция.
+ **Запускаемая** (*executable*) — командный буфер записан и может быть отправлен в очередь.

## Командные пулы
**Командный пул** (*command pool*) — место, из которого **выделяются** (именно *выделяются* (*allocate*), а не *создаются* (*create*)) командные буферы. Структура создания пула выглядит так:
``` c++
typedef struct VkCommandPoolCreateInfo {
	VkStructureType				sType;
	const void*					pNext;
	VkCommandPoolCreateFlags	flags;
	uint32_t					queueFamilyIndex;
} VkCommandPoolCreateInfo;
```
+ `sType` — тип структуры, `VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO`.
+ `pNext` — указатель на ESS (Extension-specific structure, специальная структура расширения) или `nullptr`.
+ `flags` — флаги пула. Напоминаю, что можно оставить 0, если нам не нужен ни один из флагов.
+ `queueFamilyIndex` — семейство очередей, к которому будут принадлежать командные буферы.

Здесь задаётся семейство очередей пулу, поэтому нельзя отправить командные буферы в очередь, которая пренадлежит другому семейству. Флаги бывают такими:
``` c++
typedef enum VkCommandPoolCreateFlagBits {
	VK_COMMAND_POOL_CREATE_TRANSIENT_BIT = 0x00000001,
	VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT = 0x00000002,
} VkCommandPoolCreateFlagBits;
typedef VkFlags VkCommandPoolCreateFlags;
```
+ `VK_COMMAND_POOL_CREATE_TRANSIENT_BIT` — указывает, что командные буферы, выделенные с этого пула — кратковременные. Они могут быть сброшены (reset) или освобождены (free) в относительно короткое время.
+ `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT` — указывает, что командные буферы, выделенные с этого пула могут быть сброшены индивидуально (для перезаписи). В противном случае нужно будет сбрасывать весь пул.

Создаётся пул с помощью этой функции:
``` c++
VkResult vkCreateCommandPool(
	VkDevice device,
	const VkCommandPoolCreateInfo* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkCommandPool* pCommandPool);
```
+ `device` — хэндл устройства.
+ `pCreateInfo` — указатель на информацию о командном пуле.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.
+ `pCommandPool` — получаемый хэндл командного пула.

Функция возвращает:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

Обратите внимание, что функция не имеют ~~магию~~ ошибки, что инициализация провалилась. Даже если вы зададите неверный индекс, пул всё равно будет создан.

Командный пул может быть сброшен, и тогда можно будет перезаписать все выделенные с него буферы, но при условии, что эти буферы не выполняются и не ожидают выполнения (их не должно быть в очереди).
``` c++
VkResult vkResetCommandPool(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandPoolResetFlags flags);
```
+ `device` — хэндл устройства.
+ `commandPool` — командный пул, который необходимо сбросить.
+ `flags` — флаги сброса.

Флаги сброса могут быть такими:
``` c++
typedef enum VkCommandPoolResetFlagBits {
	VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT = 0x00000001,
} VkCommandPoolResetFlagBits;
```
+ `VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT` — освобождает все занятые у системы ресурсы (в противном случае, ресурсы освобождены не будут).

Результаты выполнения:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

Разрушается пул таким образом:
``` c++
void vkDestroyCommandPool(
	VkDevice device,
	VkCommandPool commandPool,
	const VkAllocationCallbacks* pAllocator);
```
+ `device` — хэндл устройства.
+ `commandPool` — пул, который нужно разрушить.
+ `pAllocator` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.

Пример создания пула:
``` c++
VkCommandPoolCreateInfo pool_create_info;
ZM(pool_create_info); //zero memory
pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
pool_create_info.queueFamilyIndex = vkGlobals.family_index;
VkCommandPool pool = VK_NULL_HANDLE;
if (vkCreateCommandPool(vkGlobals.device, &pool_create_info, NULL, &pool) != VK_SUCCESS)
	return;
```
##Командные буферы

###Управление командными буферами

####Выделение командных буферов

Теперь можно выделить командные буферы с пула. Их можно выделять сразу несколько.
``` c++
typedef struct VkCommandBufferAllocateInfo {
	VkStructureType			sType;
	const void*				pNext;
	VkCommandPool			commandPool;
	VkCommandBufferLevel	level;
	uint32_t				commandBufferCount;
} VkCommandBufferAllocateInfo;	
```
+ `sType` — тип структуры, `VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO`.
+ `pNext` — указатель на ESS.
+ `commandPool` — пул, с которого нужно выделить буферы.
+ `level` — уровень буфера (первичный/вторичный).
+ `commandBufferCount` — количество буферов.

Уровень буфера определяется следующими значениями:
``` c++
typedef enum VkCommandBufferLevel {
	VK_COMMAND_BUFFER_LEVEL_PRIMARY = 0,
	VK_COMMAND_BUFFER_LEVEL_SECONDARY = 1,
} VkCommandBufferLevel;
```
+ `VK_COMMAND_BUFFER_LEVEL_PRIMARY` — первичный командный буфер.
+ `VK_COMMAND_BUFFER_LEVEL_SECONDARY` — вторичный командный буфер.

``` c++
VkResult vkAllocateCommandBuffers(
	VkDevice device,
	const VkCommandBufferAllocateInfo* pAllocateInfo,
	VkCommandBuffer* pCommandBuffers);
```
+ `device` — хэндл устройства.
+ `pAllocateInfo` — указатель на структуру `VkAllocationCallbacks`, содержащие адреса функций управления памятью.
+ `pCommandBuffers` — получаемые хэндлы командных буферов.

Результаты выполнения:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

#### Сброс командных буферов

Сброс можно совершить с помощью этой функции, при условии, что буфер был выделен из пула с флагом `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`.
``` c++
VkResult vkResetCommandBuffer(
	VkCommandBuffer commandBuffer,
	VkCommandBufferResetFlags flags);
```
+ `commandBuffer` — хэндл командного буфера.
+ `flags` — флаги сброса.

Флаги сброса, которые могут быть поставлены:
``` c++
typedef enum VkCommandBufferResetFlagBits {
	VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT = 0x00000001,
} VkCommandBufferResetFlagBits;
```
+ `VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT` — высвобождение всех ресурсов и возвращение их системе.

Буфер при сбросе не должен быть в очереди.

Результаты выполнения:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

#### Освобождение командных буфров

Буферы можно высвободить, если больше они не нужны. Конечно, после этого хэндлы буферов будут влятся недействительными.
``` c++
void vkFreeCommandBuffers(
	VkDevice device,
	VkCommandPool commandPool,
	uint32_t commandBufferCount,
	const VkCommandBuffer* pCommandBuffers);
```
+ `device` — хэндл устройства.
+ `commandPool` — хэндл пула.
+ `commandBufferCount` — количество командных буферов для освобождения.
+ `pCommandBuffers` — хэндлы командных буферов, запечатанные в массив.

Попытаетесь освободить буфер, что ещё на работе — Vulkan на вас обидится.

### Запись командных буферов

Прежде, чем вызвать функцию начала записи, нужно заполнить следующую структуру данными:
``` c++
typedef struct VkCommandBufferBeginInfo {
	VkStructureType sType;
	const void* pNext;
	VkCommandBufferUsageFlags flags;
	const VkCommandBufferInheritanceInfo* pInheritanceInfo;
} VkCommandBufferBeginInfo;
```
+ `sType` — тип структуры, `VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO`.
+ `pNext` — указатель на ESS.
+ `flags` — Флаги записи.
+ `pInheritanceInfo` — информация о наследовании. Используется, если записываемый буфер — вторичный. Если это первичный буфер, данный параметр игнорируется.

Флаги могут быть такими:
``` c++
typedef enum VkCommandBufferUsageFlagBits {
	VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT = 0x00000001,
	VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT = 0x00000002,
	VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT = 0x00000004,
} VkCommandBufferUsageFlagBits;
```
+ `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT` — Флаг, означающий,что последующие команды будут отправлены только один раз, и командный буфер будет сброшен и записан снова между каждой отправкой.
+ `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT` — означает, что вторичный командный буфер будет полностью внутри прохода отрисовки (что это такое, и как использовать — будет в следующих уроках). Если это первичный командный буфер — флаг игнорируется.
+ `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT` — в случае первичного командного буфера, флаг означает, что буфер будет отправлен в очередь несколько раз подряд (в противном случае, после записи его можно будет отправить только один раз, а потом ждать завершения его выполнения). В случае со вторичным, записанный буфер может быть прикреплён (т.е. будет в последствии вызван первичным) в разных первичных несколько раз (в противном случае, после прикрепления вторичного в одном из первичных, в других буферах прикрепить это вторичный уже будет нельзя, однако можно прикреплять один и тот же вторичный внутри одного первичного несколько раз).

И так, если мы записываем вторичный командный буфер, то она содержит информацию о наследовании:
``` c++
typedef struct VkCommandBufferInheritanceInfo {
	VkStructureType sType;
	const void* pNext;
	VkRenderPass renderPass;
	uint32_t subpass;
	VkFramebuffer framebuffer;
	VkBool32 occlusionQueryEnable;
	VkQueryControlFlags queryFlags;
	VkQueryPipelineStatisticFlags pipelineStatistics;
} VkCommandBufferInheritanceInfo;
```
+ `sType` — тип структуры, `VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO`.
+ `pNext` — указатель на ESS.
+ `renderPass` — Если указан флаг `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT`, то должен быть задан хэндл совместимого прохода отрисовки.
+ `subpass` — Если указан флаг `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT`, то должен быть указан идентификатор подпрохода.
+ `framebuffer` — Если указан флаг `VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT`, то можно задать фреймбуфер. Тем не менее, можно указать `VK_NULL_HANDLE`, если буфер неизвестен.
+ `occlusionQueryEnable` — при `VK_TRUE` означает, что указанный командный буфер может быть запущен из первичного, в котором есть проверка на перекрытие (occlusion query). Если `VK_FALSE`, то первичный командный буфер не должен содержать такой проверки.
+ `queryFlags` — ~~... что-то криво написана спецификация...~~ Означает, что определённые флаги могут быть использованы и в первичном командном буфере (в текущей проверке на перекрытие). Если определённого флага нет, то проврека в  первичном буфере также не должна содержать этот флаг.
+ `pipelineStatistics` — флаги статистики. Если определённый флаг есть, то значит, что первичный буфер с этим (или без) флагом может запустить этот буфер. Если определённого флага нет, то первичный буфер также не может содержать этого флага.

Теперь, заполнив всю информацию, можно начать записывать команды:
``` c++
VkResult vkBeginCommandBuffer(
	VkCommandBuffer commandBuffer,
	const VkCommandBufferBeginInfo* pBeginInfo);
```
+ `commandBuffer` — хэндл командного буфера, в который будут записываться команды.
+ `pBeginInfo` — указатель на структуру `VkCommandBufferBeginInfo`.

Функция возвращает следующие значения:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

Теперь можно вызывать функции `vkCmd...(...)`, которые запишут в командный буфер команды. Первым параметром таких функций будет являтся сам командный буфер, в который происходит запись. Все функции-команды не возвращают значений (подразумевается, что вы нигде не ошиблись и они обязательно выполнятся всегда успешно, ибо это всего лишь запись команд в буфер, а не их выполнение).
``` c++
VkResult vkEndCommandBuffer(
	VkCommandBuffer commandBuffer);
```
+ `commandBuffer` — командный буфер, записывание которого необходимо завершить.

Правильное использование:
+ Нельзя останавливать запись внутри прохода отрисовки.
+ Все проверки должны быть выключены перед завершением записи.

Функция также возвращает эти значения:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`

И снова — даже если что-то произошло не так, возвращаемое значение вас об этом не уведомит.

### Отправка командных буферов

Отправка командных буферов происходит партиями (ну или пачками, называйте, как хотите). Одну такую партию описывает следующая структура:
``` c++
typedef struct VkSubmitInfo {
	VkStructureType sType;
	const void* pNext;
	uint32_t waitSemaphoreCount;
	const VkSemaphore* pWaitSemaphores;
	const VkPipelineStageFlags* pWaitDstStageMask;
	uint32_t commandBufferCount;
	const VkCommandBuffer* pCommandBuffers;
	uint32_t signalSemaphoreCount;
	const VkSemaphore* pSignalSemaphores;
} VkSubmitInfo;
```
+ `sType` — тип структуры `VK_STRUCTURE_TYPE_SUBMIT_INFO`.
+ `pNext` — указатель на ESS.
+ `waitSemaphoreCount` — количество ожидаемых семафоров.
+ `pWaitSemaphores` — семафоры ожидания (массив хэндлов). Количество должно быть  равно `waitSemaphoreCount`.
+ `pWaitDstStageMask` — маски ожидания семафоров (массив масок). Количество должно быть  равно `waitSemaphoreCount`.
+ `commandBufferCount` — количесвто командных буферов в пачке.
+ `pCommandBuffers` — командные буферы (массив хэндлов). Только первичные!
+ `signalSemaphoreCount` — количество сигнальных семафоров.
+ `pSignalSemaphores` — сигнальные семафоры (массив хэндлов). Количество должно быть  равно `signalSemaphoreCount`.

Немного сложного материала, о котором подробнее будет рассказано позже: для каждого семафора существует маска стадий конвейера. Это означает, что прежде чем указанная стадия каждого из буферов начнёт выполнятся, должен быть просигнален соответствующий семафор. Пример: в пачке есть 2 буфера, 2 ожидающих семафора и маски для них. Назовём эти маски "*Стадия 1*" и "*Стадия 2*", а семафоры "*Семафор 1*" и "*Семафор 2*". Тогда, выполнение буферов на "*Стадии 1*" будет приостановлено до сигнала "*Семафора 1*", после сигнала, буферы продолжут работу, и потом, прежде, чем начнётся у буферов "*Стадия 2*" конвейера, они подождут сигнала "*Семафора 2*".

После того, как завершится выполнение всех указанных буферов в пачке, будут просигналены все семафоры, указанные в этой пачке.

Отправлять пачки будет следующая функция:
``` c++
VkResult vkQueueSubmit(
	VkQueue queue,
	uint32_t submitCount,
	const VkSubmitInfo* pSubmits,
	VkFence fence);
```
+ `queue` — очередь, в которую будут посланы все пачки.
+ `submitCount` — количесвто пачек.
+ `pSubmits` — пачки, массив структур `VkSubmitInfo`.
+ `fence` — забор, который будет просигнален, когда все пачки буферов будут выполнены и завершены. Если `submitCount` равен нулю, но забор всё равно задан, то он будет просигнален, когда завершаться предыдущие буферы, посланные в эту очередь.

Возвращает функция такие значения:

+ `VK_SUCCESS`
+ `VK_ERROR_OUT_OF_HOST_MEMORY`
+ `VK_ERROR_OUT_OF_DEVICE_MEMORY`
+ `VK_ERROR_DEVICE_LOST`

### Запуск вторичного командного буфера

Для запуска вторичного командного буфера, существует команда запуска, привязанная к следующей функции:
``` c++
void vkCmdExecuteCommands(
	VkCommandBuffer commandBuffer,
	uint32_t commandBufferCount,
	const VkCommandBuffer* pCommandBuffers);
```	
+ `commandBuffer` — хэндл первичного командного буфера, из когорого будет вызваны вторичные.
+ `commandBufferCount` — количество вторичных командных буферов.
+ `pCommandBuffers` — хэндлы вторичных командных буферов для запуска.

Как только эта функция была вызвана, то все вторичные буферы, указанные здесь не могут быть использованы в других буферах, если у них не указан флаг `VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT`.

###Пример и заключение

Допустим, выдилим один командный буфер.
``` c++
VkCommandBufferAllocateInfo command_buffers_info;
ZM(command_buffers_info); //zero memory
command_buffers_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
command_buffers_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
command_buffers_info.commandPool = pool;
command_buffers_info.commandBufferCount = 1;

VkCommandBuffer command_buffers[1];
if (vkAllocateCommandBuffers(vkGlobals.device, &command_buffers_info, command_buffers) != VK_SUCCESS)
	return;
```	
Запишем в этот буфер какую-нибудь фиговину:
``` c++
VkCommandBufferBeginInfo begin_info;
ZM(begin_info); //zero memory
begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
vkBeginCommandBuffer(command_buffers[0], &begin_info);
//...
vkEndCommandBuffer(command_buffers[0]);
```
А теперь отправим:
``` c++
VkSubmitInfo submit_info;
ZM(submit_info); //zero memory
submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
submit_info.commandBufferCount = 1;
submit_info.pCommandBuffers = command_buffers; 
vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
```	
Ну вот, собственно, и всё. Подробнее о заборах и семафорах вы узнаете в следующем уроке.

| | | |
|:---|:---:|---:|
|[Назад][Prev]|[Наверх][Up]|[Вперёд][Next]|

[К Readme][Readme]

[Up]: ../Readme.md "Наверх"
[Prev]: ../01LayersAndExtensions/Tutorial.md "Назад"
[Next]: ../03Sync/Tutorial.md "Вперёд"
[Readme]: ./Readme.md "К Readme"
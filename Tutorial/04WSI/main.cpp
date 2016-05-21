/* Теперь настало время привязки окна к Vulkan.
 * Если вы заметили ошибку, прошу об этом сообщить на форум или лично мне.
 * Были добавлены два специальных класса: Application и Window.
 * Классы сильно упрощены, используются только в качестве примера.
 * В этом уроке мы не будем сильно углубляться в WSI, а также не будем рисовать в окно,
 * тем не менее, здесь расскажеться о привязке самого окна, и о том, что нам понадобиться в будущем.
 * Все интересующие вас подробности смотрите здесь:
 * https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf
 * © Серый Волк.
*/

#include "common.h"
#include "Render.h"
#include "Logger.h"
#include "Application.h"
#include "Window.h"

#include <ctime>

int main()
{
	setlocale(LC_ALL, "Russian");
	Application app("Vulkan Tutorian. WSI. (copyleft) GrWolf.");
	if (!app.InitApplication()) //Инициализация приложения
		return -1;
	Window *w = app.CreateAppWindow(512, 512); //создание окна
	Render r; //объект, отвечающий за отрисовку
	Logger logger; //логгер, отвечает за вывод на экран и в файл сообщения Vulkan
	r.EnableDebug(true); //включение отладки
	//добавление проверочных слоёв
	r.AddInstanceLayer("VK_LAYER_LUNARG_standard_validation");
	r.AddDeviceLayer("VK_LAYER_LUNARG_standard_validation");

	/* Первым делом, что нам нужно: это получить плоскость (surface), в которую мы будем помещать изображение.
	 * Подробности смотрите внутри функции.
	*/
	if (!r.EnableSurface(true))
	{
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1; //
	}
	r.CreateInstance(app.GetAppName()); //создание экземпляра
	logger.Init(&r); //инициализация (получение Vulkan функций)
	logger.AttachLogger(&r); //присоединение callback
	/* Хорошо, мы создали Instance с нужными расширениями и подготовили его к работе. Теперь можно получить нашу плоскость.
	 * Но всё это будет немного скрыто в рамки класса, чтобы можно было сделать код максимально портитивный под разные платформы.
	*/
	if (!r.CreateSurface(&app, w))
	{
		logger.DetachLogger(&r);
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
	/* Всё, теперь эту плоскость можно использовать в SwapChain'ах. Но перед этим нужно добавить специальные расширения
	 * и подкотовить устройство, а также проверить его на совместимость с нашей оконной системой.
	 * Для начала подготовим и найдём подходящее устройство:
	*/
	r.PrepareGPU(); //поиск и подготовка GPU для VkDevice
	//Затем подключим необходимые расширения, подготвим swapchain и создадим устройство.
	logger.UserLog("SC", "Enabling.");
	bool e_sc, p_sc, dev;
	e_sc = r.EnableSwapchains(true);
	logger.UserLog("SC", "Preparing.");
	p_sc = r.PrepareSwapchain();
	logger.UserLog("Device", "Creating.");
	dev = r.CreateDevice();
	if (!(e_sc && p_sc && dev))
	{
		logger.DetachLogger(&r);
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
	logger.UserLog("Render", "Done.");
	/* После того, как мы создали устройство, теперь можно и создать для него swapchain'ы. Конечно же, их может быть несколько,
	 * если вы собираетесь рисовать в различные окна с одного устройства (Vulkan даже поможет в этом деле своими командными
	 * буферами, так как можно рисовать не один за другим, а несколько сразу, главное — правильно всё настроить).
	 * Но с цепочкой переключений всё немного сложнее: как только плоскость, которую она использует, приходит в негодность,
	 * swapchain необходимо пересоздать (если такое возможно и необходмо). В противном случае отображение нового кадра
	 * приведёт к ошибке.
	*/
	if (!r.CreateSwapchain(w))
	{
		r.DestroyDevice();
		logger.DetachLogger(&r);
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
	logger.UserLog("SC", "Created.");
	/* Теперь надо бы проверить работу нашего приложения, пусть это будет обычный Clear Screen.
	 * Но, тут наступают некоторые проблемы с синхронизаций и выбором кадра. У нас есть две функции:
	 * vkAcquireNextImageKHR и vkQueuePresentKHR. Первая запрашивает из изображений  swapchain свободное,
	 * и возвращает индекс изображения, в которое мы можем поместить информацию.
	 * Вторая отправляет уже отрисованное изображение в очередь. И вроде бы ничего сложного, но
	 * тут нам предстоит выбрать, как же узнать, в командном буфере, какое именно изображение нам нужно
	 * отрисовать? Ибо командным буферам будет уже всё равно на переменные в вашем коде, ведь командный буфер
	 * записывается и потом читается в устрйостве. Тем временем, функции выполняются на хосте и
	 * движком показа кадров мы тоже управляем с хоста. Как же решить такую проблему?
	 * Вариант первый: при каждом новом кадре перезаписывать командный буфер. Решение, мягко говоря, не очень.
	 * Поэтому стоит сразу его выбросить.
	 * Вариант второй: записать несколько командных буферов заранее, и отправлять каждый по мере необходимости.
	 * Звучит явно гораздо лучше, но тут наступают проблемы с памятью, так что такие буферы лучше записывать
	 * отдельно от основных, где происходит прорисовка кадров, т.е. мы будем просто потом копировать
	 * изображения из FrameBuffer в наше изображение.
	 * Сейчас воспользуемся вторым. Для начала создадим наш пул.
	*/
	VkCommandPool pool = r.CreateCommandPool(false, false);
	/* Затем воспользуемся функцией vkGetSwapchainImagesKHR, чтобы получить кол-во изображений и
	 * сами изображения. К слову говоря, в отличии от других изображений, имплементация уже позаботилась
	 * о памяти изображений, и давать им память вручную совершенно не нужно.
	*/

	uint32_t image_count = 0;
	if (vkGetSwapchainImagesKHR(r.GetDevice(), r.GetSwapchain(), &image_count, nullptr) != VK_SUCCESS)
	{
        logger.UserError("IMG", "Can't get count of swapchain images.");
		r.DestroyDevice();
		logger.DetachLogger(&r);
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
	std::vector<VkImage> images(image_count);
	//также, подготовим место под командные буферы
	std::vector<VkCommandBuffer> cmd_buffers(image_count);
	if (vkGetSwapchainImagesKHR(r.GetDevice(), r.GetSwapchain(), &image_count, images.data()) != VK_SUCCESS)
	{
		logger.UserError("IMG", "Can't get swapchain images.");
		r.DestroyDevice();
		logger.DetachLogger(&r);
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
	/* Теперь нам нужно настроить синхронизацию между двумя процессами: показ кадра и отрисовка кадра.
	 * для этого нам нужны будут два семафора.
	*/
	VkSemaphore sem_render_done, sem_image_available;
	VkSemaphoreCreateInfo sem_info;
	ZM(sem_info);
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	logger.UserLog("SEMAPHORE", "Creating semaphores.");
	VkResult rsem1, rsem2;
	rsem1 = vkCreateSemaphore(r.GetDevice(), &sem_info, nullptr, &sem_render_done);
	rsem2 = vkCreateSemaphore(r.GetDevice(), &sem_info, nullptr, &sem_image_available);
	if (rsem1 != VK_SUCCESS || rsem2 != VK_SUCCESS)
	{
		logger.UserError("SEMAPHORE", "Failed to create.");
		vkDestroySemaphore(r.GetDevice(), sem_image_available, nullptr);
		vkDestroySemaphore(r.GetDevice(), sem_render_done, nullptr);
		r.DestroyDevice();
		logger.DetachLogger(&r);
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}

	/* Заходя вперёд: первый будет сигналить о том, что мы завершили процесс рисования и можно отправилять
	 * изображение на экран. Второй подскажет, когда можно рендерить изображение, если оно доступно.
	 * Теперь необходимо записать буферы. Но мы также будем использовать барьеры.
	*/
	VkCommandBufferBeginInfo cmd_buffer_begin_info;
	ZM(cmd_buffer_begin_info);
	cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	/* Так как велик шанс отправить несколько одинаковых буферов подряд,
	 * нам нужен флаг SIMULTANEOUS_USE, тем не менее, они не будут запускаться в одно и тоже время.
	*/
	cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	//Теперь, выберем цвет "заливки". RGBA. Подробнее об этом будет в следующих уроках.
	VkClearColorValue clear_color = {
		{ 0.0f, 1.0f, 1.0f, 0.5f }
	};
	/* Далее, мы заполним информацию о том, какую часть изображения мы должны защитить барьером.
	*/
	VkImageSubresourceRange image_subresource_range;
	ZM(image_subresource_range);
	//Маска — обычный цвет.
	image_subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	//1 слой изображения (у нас всего один)
	image_subresource_range.layerCount = 1;
	//1 mip уровень (у нас он тоже всего один)
	image_subresource_range.levelCount = 1;
	/* baseMipLevel & baseArrayLayer = 0. Не сложно догадаться, что у нас их и так по одной штуке,
	 * и ставить индекс выше возможного не имеет смысла - защищать дальше уже нечего.
	 * Теперь, создадим буферы для последующей записи.
	*/
	VkCommandBufferAllocateInfo cmd_buffer_allocate_info;
	ZM(cmd_buffer_allocate_info);
	cmd_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmd_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmd_buffer_allocate_info.commandBufferCount = image_count;
	cmd_buffer_allocate_info.commandPool = pool;
	if (vkAllocateCommandBuffers(r.GetDevice(), &cmd_buffer_allocate_info, cmd_buffers.data()) != VK_SUCCESS)
	{
		logger.UserError("CMD", "Failed to allocate command buffers.");
		vkDestroySemaphore(r.GetDevice(), sem_image_available, nullptr);
		vkDestroySemaphore(r.GetDevice(), sem_render_done, nullptr);
		r.DestroyDevice();
		logger.DetachLogger(&r);
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
	/* Зписываем буферы и не забываем ставить барьеры.
	*/
	VkResult result;
	for( uint32_t i = 0; i < image_count; i++ )
	{
		/* Первый барьер будет переносить изображение из состояния только что отображённого на плоскости
		 * в состояние принять информацию.
		*/
		VkImageMemoryBarrier barrier_from_present_to_clear;
		ZM(barrier_from_present_to_clear);
		barrier_from_present_to_clear.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		/* Мы "не знаем" ничего о том, что происходило с изображением раньше,
		 * поэтому маска предыдущего доступа — 0.
		*/
		barrier_from_present_to_clear.srcAccessMask = 0;
		/* Следуящая стадия — стадия записи. Но так как запись будет не обычной, а переносом данных
		 * из одной области в другую (т.е. эта память не обязательно хранится на устройстве),
		 * то это не VK_ACCESS_MEMORY_WRITE_BIT, а VK_ACCESS_TRANSFER_WRITE_BIT.
		*/
		barrier_from_present_to_clear.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		/* Старый "макет", layout, или то, как мы планировали использовать изображение — undefined
		 * в данном случае, так как мы можем и не знать, как оно использовалось.
		*/
		barrier_from_present_to_clear.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		/* Так как мы будем копировать/записывать в эту память (т.е. наше изображение — принимающее),
		 * нам нужно поставить следующий макет:
		*/
		barrier_from_present_to_clear.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		/* Далее, нам нужно указать, какое семейство использовало и в последствии будет
		 * использовать изображение. Но если владелец не меняется, допустимо поставить такие флаги:
		*/
		barrier_from_present_to_clear.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier_from_present_to_clear.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		//указываем наше изображение
		barrier_from_present_to_clear.image = images[i];
		//указываем часть изображения, на которую будет распространятся барьер
		barrier_from_present_to_clear.subresourceRange = image_subresource_range;

		/* Далее, заполним информацию о барьере, который будет переносить изображение из режима принимать
		 * информацию в режим показывать её на экран.
		*/
		VkImageMemoryBarrier barrier_from_clear_to_present;
		//скопируем данные общие данные
		barrier_from_clear_to_present = barrier_from_present_to_clear;
		//укажем старый способ доступа и макет
		barrier_from_clear_to_present.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier_from_clear_to_present.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		/* Наше изображение будет читаться, причём не для копирования.
		*/
		barrier_from_clear_to_present.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		/* Указываем специальный макет, который будет служить нам для отображения изображения на экран.
		*/
		barrier_from_clear_to_present.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		/* Что же, теперь мы можем наконец-то записать наши командные буферы!
		*/
		vkBeginCommandBuffer(cmd_buffers[i], &cmd_buffer_begin_info);
		//укажем барьер
		vkCmdPipelineBarrier(cmd_buffers[i],
							/* Предыдущих стадий у нас нет — это единственный командный буфер, который
							 * будет исполнятся. Поэтому Top of Pipe.
							 * Если спросите, а если не исполнился такой же командный буфер?
							 * Но, нет. В данном случае семафоры и другие вещи просто не дадут этому случится.
							*/
							VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
							/* Следующая стадия - стадия копирования, так как в Vulkan очищение изображения —
							 * это копирование.
							*/
							VK_PIPELINE_STAGE_TRANSFER_BIT,
							0, //так как очищаем мы всё изображние, не ставим никаких флагов.
							0, nullptr, //не используем глобальные барьеры памяти
							0, nullptr, //не используем барьеры буферов
							//указываем барьер изображения — из показа на очистку
							1, &barrier_from_present_to_clear);
		/* Очистим изображние нашим цветом.
		 * Также, нужно указать текущий макет изображения (он может быть либо General, либо Transfer Dst.
		 * И укажем, какие части изображния нужно очистить.
		*/
		vkCmdClearColorImage(cmd_buffers[i], images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color,
							1, &image_subresource_range);
		vkCmdPipelineBarrier(cmd_buffers[i],
							//предыдущия стадия — копирование
							VK_PIPELINE_STAGE_TRANSFER_BIT,
							/* Копирование — единственное что происходит здесь.
							 * Другие стадии можно не задерживать — bottom of pipe, т.е. конец конвейера.
							*/
							VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
							0,
							0, nullptr,
							0, nullptr,
							//указываем барьер изображения — после очистки на показ
							1, &barrier_from_clear_to_present);
		//командный буфер записан
		if (vkEndCommandBuffer(cmd_buffers[i]) != VK_SUCCESS)
		{
			logger.UserError("CMD", "Error happen in writing command buffer.");
			return -1;
		}
	}
	/* Теперь заполним часть информации о том, как мы будем отправлять командные буферы в очередь.
	 * Укажем, какая стадия будет ждать сигнала семафора. У нас есть только стадия копирования.
	 * Так что между Top of Pipe и Transfer — нет никакой разницы. Но если есть и дргуие стадии,
	 * то разница уже будет.
	*/
	VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submit_info;
	ZM(submit_info);
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1; //кол-во семафоров, которые мы ждём
	submit_info.pWaitDstStageMask = &wait_dst_stage_mask; //стадии, которые ждут семафор
	submit_info.pWaitSemaphores = &sem_image_available; //семафоры
	submit_info.commandBufferCount = 1; //кол-во командных буферов
	submit_info.signalSemaphoreCount = 1; //кол-во семафоров (будут просигналены по завершении работы буферов)
	submit_info.pSignalSemaphores = &sem_render_done; //семафоры

	/* Теперь, также частично заполним информацию о том, как изображение выведется на плоскость.
	*/
	VkPresentInfoKHR present_info;
	ZM(present_info);
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1; //кол-во семафоров, которые будем ждать, прежде чем показывать
	present_info.pWaitSemaphores = &sem_render_done; //семафоры
	present_info.swapchainCount = 1; //кол-во цепочек переключений
	VkSwapchainKHR sw = r.GetSwapchain();
	present_info.pSwapchains = &sw; //цепочки переключений

	//Также, я подготовил простой счётчик кадров в секунду.
	time_t now, last_check;
	now = last_check = clock();
	float fps = 0;
	while (!app.Loop())
	{
		/* Теперь самое главное. Функция запроса изображения возвращает нам в хост, сюда индекс изображения.
		 * Хост будет ждать, пока не будет доступно хотя бы одно изображение, в которое можно записать
		 * информацию. Так или иначе, если функция может прекратить ждать до того, как изображние станет
		 * дейтсвительно доступным для записи, но вернёт верный индекс. Почему? Чтобы мы могли отправить
		 * команды уже до того, как всё станет доступным. Поэтому тут предусмотрели две вещи: семафор
		 * и забор. Первый на случай операций в устрйостве, второй на случай операций в хосте.
		 * Можно указать оба, но как минимум одно средство синхронизции должно быть указано.
		*/
		uint32_t image_index;
		result = vkAcquireNextImageKHR(r.GetDevice(),
			r.GetSwapchain(),
			UINT64_MAX, //максимально время, которые мы можем ждать индекс изображения
			sem_image_available, //семафор, который будет просигнален (только один)
			VK_NULL_HANDLE, //забор, который будет просигнален (тоже, макисмум - один)
			&image_index ); //получаем индекс изображения
		bool must_stop = false;
		if (result != VK_SUCCESS)
			logger.UserLog("Lol", "Smth");
		switch (result)
		{
		case VK_SUBOPTIMAL_KHR: //если наша плоскость изменилась - самое время пересоздать swapchain
		case VK_ERROR_OUT_OF_DATE_KHR:
			r.CreateSwapchain(w);
			sw = r.GetSwapchain();
			present_info.pSwapchains = &sw;
			logger.UserLog("SC", "Recreating");
			break;
		case VK_SUCCESS:
			break;
		case VK_TIMEOUT:
			continue;
		default:
			must_stop = true;
			break;
		}
		if (must_stop)
		{
			logger.UserError("LOOP", "Loop stops");
			break;
		}
		//Теперь укажем текущий командный буфер...
		submit_info.pCommandBuffers = &cmd_buffers[image_index];
		//... и индекс изображения
		present_info.pImageIndices = &image_index;
		//отправляем команды в очередь, без ожидания их выполнения на хосте
		result = vkQueueSubmit(r.GetQueue(r.GetFamIndex(true)), 1, &submit_info, VK_NULL_HANDLE);
		if (result != VK_SUCCESS)
			logger.UserLog("Lol", "Smth");
		/*if (must_stop)
		{
			logger.UserError("LOOP", "Loop stops");
			break;
		}*/
		//отправляем команду показа изображения.
		result = vkQueuePresentKHR(r.GetQueue(r.GetFamIndex(true)), &present_info);
		switch (result)
		{
		case VK_SUBOPTIMAL_KHR:
		case VK_ERROR_OUT_OF_DATE_KHR:
			r.CreateSwapchain(w);
			logger.UserLog("SC", "Recreating");
			sw = r.GetSwapchain();
			present_info.pSwapchains = &sw;
			break;
		case VK_SUCCESS:
			break;
		default:
			must_stop = true;
			break;
		}
		if (must_stop)
		{
			logger.UserError("LOOP", "Loop stops");
			break;
		}
		/* Далее выполняем простой подсчёт кадров в секунду: хотя это значение будет не точным, так как
		 * подсчёт идёт на хосте, а реальное отображение может быть немного другим. Отмечу, что только немного:
		 * Небольшое время также ждёт и хост, чтобы отправить команды.
		*/
		now = clock();
		fps += 1;
		if (now > last_check + CLOCKS_PER_SEC)
		{
			last_check = now;
			w->SetFPS(fps);
			fps = 0;
		}
	}
	//ждём, пока завершаться команды...
	vkDeviceWaitIdle(r.GetDevice());
	//... и закрываем дело
	vkDestroySemaphore(r.GetDevice(), sem_image_available, nullptr);
	vkDestroySemaphore(r.GetDevice(), sem_render_done, nullptr);
	r.DestroyCommandPool(pool);
	r.DestroySwapchain();
	r.DestroyDevice();
	logger.DetachLogger(&r);
	r.DestroySurface();
	r.DestroyInstance();
	app.DestroyAppWindow(&w);
	app.DeinitApplication();
	return 0;
}

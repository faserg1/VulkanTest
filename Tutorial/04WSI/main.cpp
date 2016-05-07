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
	logger.Init(r.GetInstance()); //инициализация (получение Vulkan функций)
	logger.AttachLogger(r.GetInstance()); //присоединение callback
	/* Хорошо, мы создали Instance с нужными расширениями и подготовили его к работе. Теперь можно получить нашу плоскость.
	 * Но всё это будет немного скрыто в рамки класса, чтобы можно было сделать код максимально портитивный под разные платформы.
	*/
	if (!r.CreateSurface(&app, w))
	{
		logger.DetachLogger(r.GetInstance());
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
	if (!(r.EnableSwapchains(true) && r.PrepareSwapchain() && r.CreateDevice()))
	{
		logger.DetachLogger(r.GetInstance());
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
	/* После того, как мы создали устройство, теперь можно и создать для него swapchain'ы. Конечно же, их может быть несколько,
	 * если вы собираетесь рисовать в различные окна с одного устройства (Vulkan даже поможет в этом деле своими командными
	 * буферами, так как можно рисовать не один за другим, а несколько сразу, главное — правильно всё настроить).
	 * Но с цепочкой переключений всё немного сложнее: как только плоскость, которую она использует, приходит в негодность,
	 * swapchain необходимо пересоздать (если такое возможно и необходмо). В противном случае отображение нового кадра
	 * приведёт к ошибке.
	*/
	if (!r.CreateSwapchain())
	{
		r.DestroyDevice();
		logger.DetachLogger(r.GetInstance());
		r.DestroySurface();
		r.DestroyInstance();
		app.DestroyAppWindow(&w);
		app.DeinitApplication();
		return -1;
	}
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
	uint32_t images_count = 0;
	if (vkGetSwapchainImagesKHR(r.GetDevice(), r.GetSwapchain(), &images_count, nullptr) != VK_SUCCESS)
		return -1;
	std::vector<VkImage> images(images_count);
	if (vkGetSwapchainImagesKHR(r.GetDevice(), r.GetSwapchain(), &images_count, images.data()) != VK_SUCCESS)
		return -1;
	/* Теперь нам нужно настроить синхронизацию между двумя процессами: показ кадра и отрисовка кадра.
	 * для этого нам нужны будут два семафора.
	*/
	VkSemaphore sem_render_done, sem_present;
	VkSemaphoreCreateInfo sem_info;
	ZM(sem_info);
	sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkCreateSemaphore(r.GetDevice(), &sem_info, nullptr, &sem_render_done);
	vkCreateSemaphore(r.GetDevice(), &sem_info, nullptr, &sem_present);
	while (!app.Loop())
	{

	}
	r.DestroyCommandPool(pool);
	r.DestroySwapchain();
	r.DestroyDevice();
	logger.DetachLogger(r.GetInstance());
	r.DestroySurface();
	r.DestroyInstance();
	app.DestroyAppWindow(&w);
	app.DeinitApplication();
	return 0;
}

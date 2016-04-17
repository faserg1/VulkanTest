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
	/* Теперь бы самое время сделать простой Clear Screen... Но отложим это на один из следующих уроков.
	 * А пока, наслаждаемся тем, что есть.
	*/ 
	while (!app.Loop())
	{
		
	}
	
	r.DestroySwapchain();
	r.DestroyDevice();
	logger.DetachLogger(r.GetInstance());
	r.DestroySurface();
	r.DestroyInstance();
	app.DestroyAppWindow(&w);
	app.DeinitApplication();
	return 0;
}
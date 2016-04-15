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
		return -1; //
	}
	r.CreateInstance(app.GetAppName()); //создание экземпляра
	logger.Init(r.GetInstance()); //инициализация (получение Vulkan функций)
	logger.AttachLogger(r.GetInstance()); //присоединение callback
	r.FindGPU(); //поиск GPU
	r.CreateDevice(); //создание устройства
	
	while (!app.Loop())
	{
		
	}
	
	r.DestroyDevice();
	logger.DetachLogger(r.GetInstance());
	r.DestroyInstance();
	app.DestroyAppWindow(&w);
	app.DeinitApplication();
	return 0;
}
/* Теперь настало время привязки окна к Vulkan.
 * Если вы заметили ошибку, прошу об этом сообщить на форум или лично мне.
 * Были добавлены два специальных класса: Application и Window.
 * Классы сильно упрощены, используются только в качестве примера.
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
	if (!app.InitApplication())
		return -1;
	Window *w = app.CreateAppWindow(512, 512);
	while (!app.Loop())
	{
		
	}
	app.DestroyAppWindow(&w);
	app.DeinitApplication();
	return 0;
}
# Vulkan Tutorial
![VulkanLogo](https://www.khronos.org/assets/khronos_20/css/images/Vulkan_500px_Mar15.png "Логотип Vulkan API")

## Описание

**Vulkan Tutorial** — уроки по Vulkan API, созданные на основе уроков Niko Kauppi: [YouTube][NikoYouTube], [GitHub][NikoGitHub], переведённые на русский. А также добавлена дополнительная информация из [спецификации Vulkan API + WSI][VulkanSpecWSI]. Также, мне помогли уроки от [Intel][IntelVulkanTutorial] ([GameTechDev][GameTechDevTutorial]).
Уроки представленны в виде наиболее упрощённого, последовательного кода с комментариями-объяснениями. Надеюсь, скучно там не будет!

***Если есть вопросы, вы можете задать их [здесь][GameDevForum] или [здесь][CyberForum].***


## Оглавление
### 00Device
Урок посвящён созданию экземпляра (instance) Vulkan и логического устройста (device). Урок является основами Vulkan и не посвящён созданию окон, отрисовки и так далее. 
### 01LayersAndExtensions
Урок посвящён тематике слоёв и расширений. Здесь поясняется, зачем они нужны и как их использовать. Также, урок демонстрирует, как можно отслеживать ошибки в Vulkan.

#### Дополнительная информация:
Чтобы Vulkan Loader смог найти слои, для этого в реестре Windows есть следующиее ключи:
HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Vulkan\ExplicitLayers (для явных)
HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\Vulkan\ImplicitLayers (для неявных)
Для того, чтобы добавить слой в поиск, нужно создать параметр, название которого будет создержать путь до .json файла (информация о слое), а значение должно быть 0 с типом DWORD.

### 02Commands
Урок посвящён очередям, в небольшом счёте конвейеру Vulkan, командным буферам и самим командам. Урок теоретический, и в нём не будет рассказываться о каждой команде или о какой-либо команде вообще — о каждой команде подробно можно будет ознакомиться в последующих уроках.
### 03Sync
Урок посвящён синхронизации между CPU и GPU, а также синхронизации внутри самой GPU. Будет расказно подробно о заборах, семафорах, барьерах и событиях, а также подробнее раскроются обычные функции синхронизации.
### 04WSI
Урок будет посвящён расширению VK_KHR_surface, привязкой поверхности к окну, а также swapchain. Урок не полный, и это только его первая часть.
### 05Allocation (Work In Progress)
Урок посвящён настройке произвольному выделению памяти. Будут обновления этого урока.
### 06ResourceCreation (Coming soon)
Урок рассказывает о создании различных ресурсов Vulkan: Bufffer & Image.

## Дополнительная информация:
В реестре Windows таже хранится информация об установленных Vulkan SDK. Информация находится в ключе HKEY_LOCAL_MACHINE\SOFTWARE\LunarG\VulkanSDK,
где есть параметр VK_SDK_PATHs со строковым типом REG_EXPAND_SZ, где через точку с запяток перечисленны все пути к установленным SDK.


Вы можете поддержать меня
======================================
С помощью [Я.Денег](https://money.yandex.ru/to/410012557544062).
Группа [DARTeam][DARTeamVK].


[VulkanSpecWSI]: https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/pdf/vkspec.pdf "Vulkan API + WSI Specification"
[NikoGitHub]: https://github.com/Niko40/Vulkan-API-Tutorials "Репозиторий уроков Niko Kauppi на GitHub"
[NikoYouTube]: https://www.youtube.com/playlist?list=PLUXvZMiAqNbK8jd7s52BIDtCbZnKNGp0P "Плейлист уроков Niko Kauppi на YouTube"
[GameDevForum]: http://www.gamedev.ru/code/forum/?id=212896 "Тема на форуме GameDev.ru"
[CyberForum]: http://www.cyberforum.ru/graphics/thread1705765.html "Тема на CyberForum"
[IntelVulkanTutorial]: https://software.intel.com/en-us/api-without-secrets-introduction-to-vulkan-part-1 "Уроки Vulkan от Intel"
[GameTechDevTutorial]: https://github.com/GameTechDev/IntroductionToVulkan "Уроки Vulkan от Intel — GitHub"
[DARTeamVK]: http://vk.com/dev.ani.resu.team "Группа в ВКонтакте"

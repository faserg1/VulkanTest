#ifndef RENDER_H
#define RENDER_H

#include <vulkan/vulkan.h> // Vulkan API
#include <vector>


class Render
{
	VkInstance instance; //используемый экземпляр
	VkDevice device; //устройство
	
	uint32_t family_index; //семейство, которое будет использоваться
	
	std::vector<const char *> instance_layers; //слои экземпляра, что будут использваться
	std::vector<const char *> instance_extensions; //расширения экземпляра
	std::vector<const char *> device_layers; //слои устройства
	std::vector<const char *> device_extensions; //расширения устройства
public:
	Render(); //простой констуркутор (подготовка)
	~Render(); //деструктор.
	
	//добавление слоёв и расширений
	void AddInstanceLayer(const char *name);
	void AddInstanceExtension(const char *name);
	void AddDeviceLayer(const char *name);
	void AddDeviceExtension(const char *name);
	
	//удаление слоёв и расширений из списка (на уже существующие экземпляры/устройства не повлияют)
	bool RemoveInstanceLayer(const char *name);
	bool RemoveInstanceExtension(const char *name);
	bool RemoveDeviceLayer(const char *name);
	bool RemoveDeviceExtension(const char *name);
	
	bool CreateInstance(); //создание экземпляра
	bool CreateDevice(); //создание устройства
	void DestroyInstance(); //уничтожение устройства
	void DestroyDevice(); //уничтожение экземпляра
	
	
	void EnableDebug(bool enable); //включение отладки (добавление/удаление расширения)
	
	const VkInstance GetInstance() const; //получение экземпляра
	const VkDevice GetDevice() const; //получение устройства
	
	const VkQueue GetQueue(uint32_t index) const; //получение очереди
	
	//Создание и уничтожение графического командного пула
	VkCommandPool CreateCommandPool(bool reset, bool transient);
	void DestroyCommandPool(VkCommandPool pool);
};

#endif // RENDER_H

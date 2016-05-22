#include "renderer.h"
#include "nativehandle.h"
#include <cstring>
#include <QDebug>

Renderer::Renderer() : vk_inst(nullptr), vk_dev(nullptr), gpu_count(0), native_handle(nullptr)
{
    this->native_handle = new NativeHandle;
}

Renderer::~Renderer()
{
    if (this->native_handle)
        delete this->native_handle;
    if (vk_dev)
    {
        vkDestroyDevice(vk_dev, nullptr);
    }
    if (vk_inst)
    {
        vkDestroyInstance(this->vk_inst, nullptr);
    }
}

void Renderer::load()
{
	if (!initVulkan())
        qCritical() << "Cannot create Vulkan Instance.\n";
    else
        qInfo() << "Created Vulkan!\n";
    if (!createDevice())
        qCritical() << "Cannot create Vulkan Device.\n";
    else
        qInfo() << "Vulkan have his own device...\n";
}

bool Renderer::initVulkan()
{
    VkApplicationInfo app_info;
    memset(&app_info, 0, sizeof(app_info));
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 11);
    app_info.pApplicationName = "VulkanTest.Sample001";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 3);

    VkInstanceCreateInfo inst_info;
    memset(&inst_info, 0, sizeof(inst_info));
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pApplicationInfo = &app_info;
    //inst_info.

    auto result = vkCreateInstance(&inst_info, nullptr, &this->vk_inst);
    if (result != VK_SUCCESS)
        return false;
    return true;
}

bool Renderer::createDevice()
{
    if (vkEnumeratePhysicalDevices(vk_inst, &gpu_count, nullptr) != VK_SUCCESS)
        return false;
    gpu_list.resize(gpu_count);
    if (vkEnumeratePhysicalDevices(vk_inst, &gpu_count, gpu_list.data()) != VK_SUCCESS)
        return false;

    VkDeviceCreateInfo dev_info;
    memset(&dev_info, 0, sizeof(dev_info));

    VkDeviceQueueCreateInfo dev_q_info;
    memset(&dev_q_info, 0, sizeof(dev_q_info));

    dev_q_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

    uint32_t fam_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu_list[0], &fam_count, nullptr);
    if (fam_count < 1)
        return false;
    std::vector<VkQueueFamilyProperties> prop_list;
    prop_list.resize(fam_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu_list[0], &fam_count, prop_list.data());

    uint32_t fq_index = (uint32_t) -1; // -1 as not found

    for (uint32_t i = 0; i < prop_list.size(); i++)
    {
        if (prop_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            fq_index = i;
        }
    }

    if (fq_index == (uint32_t) -1) //still not found graphics bit
    {
        qCritical() << "You have not a graphic bit in your family!\n";
        return false;
    }
    else
        qInfo() << "I found a graphic bit in a " << fq_index << " family index.\n";

    float dev_q_pri[] = {1.0f};

    dev_q_info.queueCount = 1;
    dev_q_info.queueFamilyIndex = fq_index;
    dev_q_info.pQueuePriorities = dev_q_pri;

    dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_info.queueCreateInfoCount = 1;
    dev_info.pQueueCreateInfos = &dev_q_info;

    if (vkCreateDevice(gpu_list[0], &dev_info, nullptr, &vk_dev) != VK_SUCCESS)
        return false;

    return true;
}

void Renderer::set_window(NativeHandle &nhandle)
{
    *(this->native_handle) = nhandle;
}

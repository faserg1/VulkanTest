#include "renderer.h"
#include <cstring>
#include <iostream>
#include <QWindow>

Renderer::Renderer() : vk_inst(nullptr), vk_dev(nullptr), gpu_count(0)
{
    if (!initVulkan())
        std::cerr << "Cannot create Vulkan Instance.\n";
    else
        std::cout << "Created Pukan!\n";
    if (!createDevice())
        std::cerr << "Cannot create Vulkan Device. Fuck!\n";
    else
        std::cout << "Pukan have his own device... WUT?\n";
}

Renderer::~Renderer()
{
    if (vk_dev)
    {
        vkDestroyDevice(vk_dev, nullptr);
    }
    if (vk_inst)
    {
        vkDestroyInstance(this->vk_inst, nullptr);
    }
}

bool Renderer::initVulkan()
{
    VkApplicationInfo app_info;
    memset(&app_info, 0, sizeof(app_info));
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.apiVersion = VK_API_VERSION;
    app_info.pApplicationName = "VulkanTest.Sample1";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 2);

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
    std::cout << "I have find " << gpu_count << " physical devices.\nAre you happy?\n";
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
        //VkQueueFamilyProperties::queueFlags
        if (prop_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            fq_index = i;
        }
    }

    if (fq_index == (uint32_t) -1) //still not found graphics bit
    {
        std::cerr << "You have not a graphic bit in your family!\n";
        return false; //fuck this shit;
    }
    else
        std::cout << "I found a graphic bit in a " << fq_index << " family index. Are you still happy?\n";

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

void Renderer::set_window(QWindow &window)
{

}

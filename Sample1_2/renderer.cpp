#include "renderer.h"
#include <cstring>
#include <iostream>
#include <QX11Info>
#include <QWindow>

Renderer::Renderer() : vk_inst(nullptr)
{
    if (!initVulkan())
    {
        std::cerr << "Cannot create Vulkan Instance.\n";
    }
    else
        std::cout << "Created Pukan!\n";
}

Renderer::~Renderer()
{
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
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);

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
    //WId id;
}

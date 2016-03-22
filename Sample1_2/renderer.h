#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <QWindow>

class Renderer
{
    VkInstance vk_inst;
    VkDevice vk_dev;

    uint32_t gpu_count;
    std::vector<VkPhysicalDevice> gpu_list;
protected:
    bool initVulkan();
    bool createDevice();
public:
    Renderer();
    ~Renderer();

    void set_window(QWindow &window);
};

#endif // RENDERER_H

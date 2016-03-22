#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>

class Renderer
{
    VkInstance vk_inst;
protected:
    bool initVulkan();
    bool createDevice();
public:
    Renderer();
    ~Renderer();
};

#endif // RENDERER_H

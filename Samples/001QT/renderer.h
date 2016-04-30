#ifndef RENDERER_H
#define RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>

struct NativeHandle;

class Renderer
{
    VkInstance vk_inst;
    VkDevice vk_dev;

    uint32_t gpu_count;
    std::vector<VkPhysicalDevice> gpu_list;
    NativeHandle *native_handle; //using for window handles
protected:
    bool initVulkan();
    bool createDevice();
public:
    Renderer();
    ~Renderer();

	void load();
    void set_window(NativeHandle &nhandle);
};

#endif // RENDERER_H

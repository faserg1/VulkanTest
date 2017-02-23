#ifndef RENDERER_H
#define RENDERER_H

#include <vector>

struct NativeHandle;
class RendererImp;

class Renderer
{
	RendererImp *imp;
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

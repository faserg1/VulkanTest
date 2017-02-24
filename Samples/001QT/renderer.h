#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include <QObject>

struct NativeHandle;
class RendererImp;
class QThread;

class Renderer : public QObject
{
	Q_OBJECT

	RendererImp *imp;
    NativeHandle *native_handle; //using for window handles

	QThread *renderThread;
protected:
	bool rendering;
private slots:
	void drawFrame();
public:
    Renderer();
    ~Renderer();

	void load();
    void set_window(NativeHandle &nhandle);
	void startRender();
	void stopRender();
};

#endif // RENDERER_H

#include "mainwindow.h"
#include "renderer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Renderer robj;

    MainWindow w;
    w.show();
	NativeHandle h = w.GetWindowHandle();
	robj.set_window(h);
	robj.load();

    return a.exec();
}

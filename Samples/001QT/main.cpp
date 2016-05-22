#include "mainwindow.h"
#include "renderer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Renderer robj;
	robj.load();
    MainWindow w;
    w.show();

    return a.exec();
}

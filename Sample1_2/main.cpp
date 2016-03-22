#include "mainwindow.h"
#include "renderer.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    Renderer rend;


    return a.exec();
}

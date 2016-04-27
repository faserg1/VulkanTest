#if defined(_WIN32)
#include <windows.h>
#endif
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

NativeHandle MainWindow::GetWindowHandle()
{
    NativeHandle handle_data;
    #if defined(_WIN32)
    handle_data.hWnd = (HWND) this->winId();
    handle_data.hInst = (HINSTANCE) GetWindowLongW(handle_data.hWnd, GWLP_HINSTANCE);
    #endif
    return handle_data;
}

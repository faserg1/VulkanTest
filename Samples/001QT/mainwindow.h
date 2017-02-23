#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "nativehandle.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
	NativeHandle GetWindowHandle(); //Получение родных хэндлов окна
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

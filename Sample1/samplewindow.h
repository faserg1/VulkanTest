#ifndef SAMPLEWINDOW_H
#define SAMPLEWINDOW_H

#include <QWidget>

namespace Ui {
class SampleWindow;
}

class SampleWindow : public QWidget
{
    Q_OBJECT

public:
    explicit SampleWindow(QWidget *parent = 0);
    ~SampleWindow();

private:
    Ui::SampleWindow *ui;
};

#endif // SAMPLEWINDOW_H

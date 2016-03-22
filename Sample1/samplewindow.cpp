#include "samplewindow.h"
#include "ui_samplewindow.h"

SampleWindow::SampleWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SampleWindow)
{
    ui->setupUi(this);
}

SampleWindow::~SampleWindow()
{
    delete ui;
}

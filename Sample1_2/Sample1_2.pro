#-------------------------------------------------
#
# Project created by QtCreator 2016-03-22T14:21:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sample1_2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    renderer.cpp

HEADERS  += mainwindow.h \
    renderer.h

FORMS    += mainwindow.ui

unix:!macx: LIBS += -L$$PWD/../../../VulkanSDK/1.0.3.1/x86_64/lib/ -lvulkan

INCLUDEPATH += $$PWD/../../../VulkanSDK/1.0.3.1/x86_64/include
DEPENDPATH += $$PWD/../../../VulkanSDK/1.0.3.1/x86_64/include

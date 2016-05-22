#-------------------------------------------------
#
# Project created by QtCreator 2016-04-18T04:52:24
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 001QT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    renderer.cpp

HEADERS  += mainwindow.h \
    nativehandle.h \
    renderer.h

FORMS    += mainwindow.ui

INCLUDEPATH += $$PWD/../VulkanLib/include
contains(QMAKE_HOST.arch, x86_64) {
    LIBS += -L$$PWD/../VulkanLib/lib
} else {
    LIBS += -L$$PWD/../VulkanLib/lib32
}
win32 {
    LIBS += -lUser32 -lKernel32
}
LIBS += -lvulkan-1

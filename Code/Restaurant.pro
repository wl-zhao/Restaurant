#-------------------------------------------------
#
# Project created by QtCreator 2017-08-01T07:43:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Restaurant
TEMPLATE = app
MOC_DIR = temp/moc
RCC_DIR = temp/rcc
UI_DIR = temp/ui
OBJECTS_DIR = temp/obj
DESTDIR = bin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QT += sql
QT += axcontainer
SOURCES += \
        main.cpp \
    database.cpp \
    exportexcel.cpp \
    signwidget.cpp \
    dish.cpp \
    diningtable.cpp \
    beautify.cpp \
    AdministratorWidget.cpp \
    CookWidget.cpp \
    ManagerWidget.cpp \
    WaiterWidget.cpp \
    CustomerWidget.cpp \
    WinMessage.cpp \
    user.cpp

HEADERS += \
    database.h \
    exportexcel.h \
    dish.h \
    diningtable.h \
    beautify.h \
    stable.h \
    AdministratorWidget.h \
    CookWidget.h \
    CustomerWidget.h \
    WaiterWidget.h \
    ManagerWidget.h \
    SignWidget.h \
    WinMessage.h \
    user.h

CONFIG += mobility
CONFIG += axcontainer
MOBILITY = 

RESOURCES += \
    style.qrc \
    picture.qrc

DISTFILES +=
RC_ICONS=icon.ico

PRECOMPILED_HEADER =stable.h

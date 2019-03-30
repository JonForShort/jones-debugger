QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = jones-debugger

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

SOURCES += \
        source/main.cc \
        source/main_window.cc \
        source/command_prompt.cc \
        source/console_window.cpp

HEADERS += \
        source/include/main_window.hh \
        source/include/command_prompt.hh \
        source/include/console_window.h

FORMS += \
        qt/forms/mainwindow.ui

INCLUDEPATH += \
        $$PWD/source/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

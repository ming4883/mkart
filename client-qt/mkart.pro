#-------------------------------------------------
#
# Project created by QtCreator 2017-09-14T13:24:47
#
#-------------------------------------------------

QT += core gui opengl sensors network

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android-sources

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MKartClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += -std=c++11

# Main
SOURCES += \
    main.cpp \
    timing.cpp \
    kart_application.cpp \
    kart_glwindow.cpp \
    tcp.cpp \
    remote_service.cpp \
    sensor_service.cpp \
    fpv_service.cpp \
    ImGuiRenderer.cpp \
    QtImGui.cpp \
    3rdparty/imgui/imgui_draw.cpp \
    3rdparty/imgui/imgui.cpp

HEADERS += \
    timing.h \
    kart_application.h \
    kart_glwindow.h \
    tcp.h \
    remote_service.h \
    connection_view.h \
    control_view.h \
    imgui_utils.h \
    sensor_service.h \
    fpv_service.h \
    fpv_view.h \ 
    ImGuiRenderer.h \
    QtImGui.h


OTHER_FILES += android-sources/AndroidManifest.xml

INCLUDEPATH += \
    3rdparty/glm \
    3rdparty/imgui

#INCLUDEPATH += libvpx/x86/include
#win32:LIBS += $$PWD/libvpx/x86/lib/libvpx.a

# Common Sources

SOURCES += \
    common/kart_commands.cpp \

HEADERS += \
    common/kart_commands.h \
    common/kart_archive.h \

INCLUDEPATH += common


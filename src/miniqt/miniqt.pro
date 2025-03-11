QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += debug
DESTDIR = $$PWD/bin

CONFIG += c++11
QMAKE_CXXFLAGS += -Wno-deprecated-copy

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DESTDIR=$$(MINIQT_TARGET_DIR)/                  #指定生成的应用程序放置的目录
OBJECTS_DIR=$$(MINIQT_TARGET_DIR)/object/
UI_DIR=$$(MINIQT_TARGET_DIR)/ui/               	#指定临时生成的ui_开头文件路径，最后的/不能去掉，在这里使用，不在Makefile.MK中，为了保证ui改变时，能重新编译
MOC_DIR =$$(MINIQT_TARGET_DIR)/moc/          	#指定临时生成的moc_开头文件路径

include($$PWD/BreakpadHandler/BreakpadHandler.pri)
include($$PWD/Keyboard/Keyboard.pri)

SOURCES += \
    BottomBar.cpp \
    CameraInfo.cpp \
    CameraInfoManagement.cpp \
    CameraManagement.cpp \
    CameraModel.cpp \
    CameraView.cpp \
    MsApplication.cpp \
    VapiLayout.cpp \
    VideoLayout.cpp \
    VideoWidget.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    BottomBar.h \
    CameraInfo.h \
    CameraInfoManagement.h \
    CameraManagement.h \
    CameraModel.h \
    CameraView.h \
    MainWindow.h \
    MsApplication.h \
    VapiLayout.h \
    VideoLayout.h \
    VideoWidget.h

FORMS += \
    BottomBar.ui \
    CameraManagement.ui \
    MainWindow.ui \
    VideoLayout.ui \
    VideoWidget.ui

# Default rules for deployment.
target.path = /opt/app/bin
INSTALLS += target

PLAT_FORM = $$(PLATFORM_TYPE)

contains(PLAT_FORM, hi3536g){
DEFINES += _HI3536_ _HI3536G_
}
contains(PLAT_FORM, hi3536a){
DEFINES += _HI3536A_
LIBS += -lsecurec
}
contains(PLAT_FORM, nt98633){
DEFINES += _NT98633_
}

###第三方库依赖，按顺序编译###
# faad2-2.7
# zlib-1.2.11
# openssl-1.1.1k
# x264-snapshot-20151029-2245
# x265_1.7

###项目库依赖，按顺序编译###
# make msstd
# make libvapi
# make osa
# make libmsrtsp
# make librecortsp
# make libmstde

SRCDIR = $$PWD/..

INCLUDEPATH += \
    $$SRCDIR/public/include

LIBS += -lm -ldl -lpthread

# vpai
INCLUDEPATH += \
    $$SRCDIR/public/include/vapi \
    $$SRCDIR/public/include/vapi/$$(PLATFORM_TYPE) \
    $$SRCDIR/libs/vapi/$$(PLATFORM_TYPE)

LIBS += \
    -L$$(PUBLIC_LIB_DIR) -lmsstd -lfaad -lvapi
contains(PLAT_FORM, nt98633){
LIBS += -lhdal -lvendor_media -lsamplerate
}

# rtsp
INCLUDEPATH += \
    $$SRCDIR/public/include/rtsp \

LIBS += \
    -L$$(STD_USRLIB_DIR) -lssl -lcrypto -lx264 -lx265 \
    -L$$(PUBLIC_LIB_DIR) -losa -lrecortsp

# tde
LIBS += \
    -L$$(PUBLIC_LIB_DIR) -lmstde

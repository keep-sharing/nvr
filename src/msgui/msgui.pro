#-------------------------------------------------
#
# Project created by QtCreator 2018-10-31T13:58:41
#
#-------------------------------------------------

QT       += core gui network testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mscore
TEMPLATE = app

PLAT_FORM = $$(PLATFORM_TYPE)

QMAKE_CFLAGS   += -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-parameter -Wno-write-strings

#所有警告当成错误
QMAKE_CFLAGS   += -Werror
QMAKE_CXXFLAGS += -Werror

#部分警告无法解决，先屏蔽，要区分平台
contains(PLAT_FORM, hi3536a) {
QMAKE_CFLAGS   += -Wno-error=deprecated-declarations -Wno-error=format-truncation= -Wno-error=restrict
QMAKE_CXXFLAGS += -Wno-error=format-truncation=
#qt库问题
QMAKE_CXXFLAGS += -Wno-deprecated-copy -Wno-error=class-memaccess
}
contains(PLAT_FORM, hi3536g) {
QMAKE_CFLAGS   += -Wno-error=deprecated-declarations -Wno-error=ignored-qualifiers
}
contains(PLAT_FORM, hi3536c) {
QMAKE_CFLAGS   += -Wno-error=deprecated-declarations -Wno-error=ignored-qualifiers
}
contains(PLAT_FORM, hi3798) {
QMAKE_CFLAGS   += -Wno-error=deprecated-declarations -Wno-error=ignored-qualifiers
}
contains(PLAT_FORM, nt98323) {
QMAKE_CFLAGS   += -Wno-error=deprecated-declarations -Wno-error=ignored-qualifiers
}
contains(PLAT_FORM, nt98633) {
QMAKE_CFLAGS   += -Wno-error=deprecated-declarations -Wno-error=ignored-qualifiers
}

#栈保护
QMAKE_CFLAGS   += -fstack-protector -fstack-protector-all
QMAKE_CXXFLAGS += -fstack-protector -fstack-protector-all

#
QMAKE_CFLAGS   += -rdynamic -funwind-tables -g
QMAKE_CXXFLAGS += -rdynamic -funwind-tables -g

#函数级编译链接
QMAKE_CFLAGS   += -ffunction-sections
QMAKE_CXXFLAGS += -ffunction-sections
QMAKE_LFLAGS   += -Wl,--gc-sections

# asan
# QMAKE_LFLAGS   += -fsanitize=address -fno-omit-frame-pointer -g

target.path = /opt/app/bin
INSTALLS += target

DEFINES += MANUAL_RESOURCE
DEFINES += THREAD_CORE

contains(PLAT_FORM, hi3536g) {
DEFINES += MS_FISHEYE_SOFT_DEWARP
}

DESTDIR=$$(GUI_TARGET_DIR)                    	#指定生成的应用程序放置的目录
OBJECTS_DIR=$$(GUI_TARGET_DIR)/object/
UI_DIR=$$(GUI_TARGET_DIR)/ui/               	#指定临时生成的ui_开头文件路径，最后的/不能去掉，在这里使用，不在Makefile.MK中，为了保证ui改变时，能重新编译
MOC_DIR =$$(GUI_TARGET_DIR)/moc/          	#指定临时生成的moc_开头文件路径

include($$PWD/DynamicDisplay/DynamicDisplay.pri)
include($$PWD/PosDisplay/PosDisplay.pri)
include($$PWD/TrackDisplay/TrackDisplay.pri)

include($$PWD/Action/Action.pri)
include($$PWD/EffectiveTime/EffectiveTime.pri)

include($$PWD/GlobalControl/GlobalControl.pri)
include($$PWD/MsDevice/MsDevice.pri)
include($$PWD/MsLog/MsLog.pri)
include($$PWD/MsMessage/MsMessage.pri)
include($$PWD/MyDebug/MyDebug.pri)
include($$PWD/MyUtils/MyUtils.pri)
include($$PWD/Script/Script.pri)

include($$PWD/SettingContainer/SettingContainer.pri)
include($$PWD/ModuleEvent/ModuleEvent.pri)
include($$PWD/ModuleSettings/ModuleSettings.pri)
include($$PWD/ModuleCamera/ModuleCamera.pri)
include($$PWD/ModuleRetrieve/ModuleRetrieve.pri)
include($$PWD/ModuleSmartanalysis/ModuleSmartanalysis.pri)
include($$PWD/ModuleStatus/ModuleStatus.pri)
include($$PWD/ModuleStorage/ModuleStorage.pri)

include($$PWD/MsQt/MsQt.pri)

include($$PWD/ContentLiveView/ContentLiveView.pri)
include($$PWD/AutoLogout/AutoLogout.pri)
include($$PWD/CustomLayout/CustomLayout.pri)
include($$PWD/Download/Download.pri)
include($$PWD/EventPopup/EventPopup.pri)
include($$PWD/FileDialog/FileDialog.pri)
include($$PWD/GraphicsDraw/GraphicsDraw.pri)
include($$PWD/Keyboard/Keyboard.pri)
include($$PWD/MsLanguage/MsLanguage.pri)
include($$PWD/LiveView/LiveView.pri)
include($$PWD/LiveviewTarget/LiveviewTarget.pri)
include($$PWD/LiveviewOccupancy/LiveviewOccupancy.pri)
include($$PWD/LiveviewSub/LiveviewSub.pri)
include($$PWD/MsSchedule/MsSchedule.pri)
include($$PWD/MsQtDefines/MsQtDefines.pri)
include($$PWD/MsUser/MsUser.pri)
include($$PWD/NetworkCommond/NetworkCommond.pri)
include($$PWD/Playback/Playback.pri)
include($$PWD/Ptz/Ptz.pri)
include($$PWD/Ptz3D/Ptz3D.pri)
include($$PWD/qjson/qjson.pri)
include($$PWD/Runnable/Runnable.pri)
include($$PWD/Splash/Splash.pri)
include($$PWD/Test/Test.pri)
include($$PWD/TestHardware/TestHardware.pri)
include($$PWD/Wizard/Wizard.pri)

if(contains(DEFINES, MS_FISHEYE_SOFT_DEWARP)){
include($$PWD/FisheyeDewarp/FisheyeDewarp.pri)
}else{
include($$PWD/Fisheye/Fisheye.pri)
}

include($$PWD/BreakpadHandler/BreakpadHandler.pri)

INCLUDEPATH += $$(PUBLIC_INC_DIR)
INCLUDEPATH += $$(PUBLIC_INC_DIR)/avilib
INCLUDEPATH += $$(PUBLIC_INC_DIR)/cellular
INCLUDEPATH += $$(PUBLIC_INC_DIR)/disk
INCLUDEPATH += $$(PUBLIC_INC_DIR)/gpio
INCLUDEPATH += $$(PUBLIC_INC_DIR)/gpio/$$(PLATFORM_TYPE)
INCLUDEPATH += $$(PUBLIC_INC_DIR)/log
INCLUDEPATH += $$(PUBLIC_INC_DIR)/msdb
INCLUDEPATH += $$(PUBLIC_INC_DIR)/msfs
INCLUDEPATH += $$(PUBLIC_INC_DIR)/msptz
INCLUDEPATH += $$(PUBLIC_INC_DIR)/mssocket
INCLUDEPATH += $$(PUBLIC_INC_DIR)/msverify
INCLUDEPATH += $$(PUBLIC_INC_DIR)/osa
INCLUDEPATH += $$(PUBLIC_INC_DIR)/p2p
INCLUDEPATH += $$(PUBLIC_INC_DIR)/poe
INCLUDEPATH += $$(PUBLIC_INC_DIR)/rtsp
INCLUDEPATH += $$(PUBLIC_INC_DIR)/sdkp2p
INCLUDEPATH += $$(PUBLIC_INC_DIR)/vapi
INCLUDEPATH += $$(PUBLIC_INC_DIR)/jpeg
INCLUDEPATH += $$(PUBLIC_INC_DIR)/tbox
INCLUDEPATH += $$(PUBLIC_INC_DIR)/msini
INCLUDEPATH += $$(PUBLIC_INC_DIR)/msmov
INCLUDEPATH += $$(PUBLIC_INC_DIR)/vapi/$$(PLATFORM_TYPE)
INCLUDEPATH += $$(PUBLIC_INC_DIR)/nghttp2
INCLUDEPATH += $$PWD/../boards/$$(PLATFORM_TYPE)

contains(PLAT_FORM, hi3536|hi3536g) {
INCLUDEPATH += $$(PUBLIC_INC_DIR)/fisheye
#qmake里面已经有定义flags
#QMAKE_CFLAGS += $$(PLATFORM_FLAGS)
#QMAKE_CXXFLAGS += $$(PLATFORM_FLAGS)
QMAKE_CFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -D_HI3536_ -DEGL_API_FB -DEGL_FBDEV
QMAKE_CXXFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -D_HI3536_ -DEGL_API_FB -DEGL_FBDEV
LIBS += -L$$PWD
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp -lsds
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, hi3536g){
LIBS += -lfisheye  -lGLESv2  -lmali
QMAKE_CFLAGS += -D_HI3536G_
QMAKE_CXXFLAGS += -D_HI3536G_
}

contains(PLAT_FORM, hi3536c) {
#QMAKE_CFLAGS += $$(PLATFORM_FLAGS)
#QMAKE_CXXFLAGS += $$(PLATFORM_FLAGS)
QMAKE_CFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -D_HI3536C_
QMAKE_CXXFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -D_HI3536C_
LIBS += -L$$PWD
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp -lsds
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, hi3798) {
#QMAKE_CFLAGS += $$(PLATFORM_FLAGS)
#QMAKE_CXXFLAGS += $$(PLATFORM_FLAGS)
QMAKE_CFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -DUSE_AEC -D_XOPEN_SOURCE=600 -D_HI3798_
QMAKE_CXXFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -DUSE_AEC -D_XOPEN_SOURCE=600 -D_HI3798_
LIBS += -L$$PWD
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp -lsds
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt -lhi_msp -lhi_common -lhi_jpeg -lasound -lspeexdsp
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, nt98323) {
#QMAKE_CFLAGS += $$(PLATFORM_FLAGS)
#QMAKE_CXXFLAGS += $$(PLATFORM_FLAGS)
QMAKE_CFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -DUSE_AEC -D_XOPEN_SOURCE=600 -D_NT98323_
QMAKE_CXXFLAGS += -D_UCLIBC_ -D_GNU_SOURCE -D__USE_XOPEN -DUSE_AEC -D_XOPEN_SOURCE=600 -D_NT98323_
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp -lsds
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt -lhdal -lgm2d -lvendor_media -lsamplerate -lspeexdsp -lnvtaudlib_anr
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, hi3536a) {
#INCLUDEPATH += $$(PUBLIC_INC_DIR)/fisheye
DEFINES += _UCLIBC _GNU_SOURCE __USE_XOPEN _HI3536A_ _LARGEFILE64_SOURCE
# yuyu 鱼眼软解先屏蔽
#LIBS += -lfisheye -lGLESv2 -lmali
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt
LIBS += -lm -ldl -lpthread -lsecurec
}

contains(PLAT_FORM, nt98633) {
DEFINES += _UCLIBC_ _GNU_SOURCE __USE_XOPEN USE_AEC _XOPEN_SOURCE=600 _NT98633_
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp -lsds
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt -lhdal -lvendor_media -lsamplerate
LIBS += -lm -ldl -lpthread
}
SOURCES += main.cpp\
    mainwindow.cpp

HEADERS  += \
    mainwindow.h

FORMS    += \
    mainwindow.ui

!contains(DEFINES, MANUAL_RESOURCE){
RESOURCES += \
    resource/resource.qrc
}

RESOURCES += \
    ../public/include/resource.qrc

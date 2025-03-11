CONFIG -= qt

INCLUDEPATH += $$PWD
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
INCLUDEPATH += $$(PUBLIC_INC_DIR)/ovfcli
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

INCLUDEPATH += $$PWD/msawsiot
INCLUDEPATH += $$PWD/msawsiot/include
INCLUDEPATH += $$PWD/msawsiot/external_libs/jsmn
INCLUDEPATH += $$PWD/msawsiot/platform/linux/mbedtls
INCLUDEPATH += $$PWD/msawsiot/platform/linux/common

PLAT_FORM = $$(PLATFORM_TYPE)

contains(PLAT_FORM, hi3536|hi3536g) {
INCLUDEPATH += $$(PUBLIC_INC_DIR)/fisheye
DEFINES += _UCLIBC_ _GNU_SOURCE __USE_XOPEN _HI3536_ EGL_API_FB EGL_FBDEV
LIBS += -L$$PWD -lkey_verify
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lovfcli -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, hi3536g){
LIBS += -lfisheye  -lGLESv2  -lmali
DEFINES += _HI3536G_
}

contains(PLAT_FORM, hi3536c) {
DEFINES += _UCLIBC_ _GNU_SOURCE __USE_XOPEN _HI3536C_
LIBS += -L$$PWD -lkey_verify
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lovfcli -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, hi3798) {
DEFINES += _UCLIBC_ _GNU_SOURCE __USE_XOPEN USE_AEC _XOPEN_SOURCE=600 _HI3798_
LIBS += -L$$PWD -lkey_verify
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lovfcli -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt -lhi_msp -lhi_common -lhi_jpeg -lasound -lspeexdsp
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, nt98323) {
DEFINES += _UCLIBC_ _GNU_SOURCE __USE_XOPEN USE_AEC _XOPEN_SOURCE=600 _NT98323_
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lovfcli -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt -lhdal -lgm2d -lvendor_media -lsamplerate -lspeexdsp -lnvtaudlib_anr
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, nt98332) {
DEFINES += _UCLIBC_ _GNU_SOURCE __USE_XOPEN USE_AEC _XOPEN_SOURCE=600 _NT98332_
LIBS += -L$$(PUBLIC_STATIC_LIB_DIR) -lpcap -lAVAPIs -lIOTCAPIs -lTUTKGlobalAPIs -lcurl_tutk -lssl_tutk -lcrypto_tutk -ljson-c_tutk -lupnp -lminiupnpc -lmsverify -lixml -lturbojpeg -ljpeg -lmbedtls -lmbedcrypto -lmbedx509 -ltbox
LIBS += -L$$(PUBLIC_LIB_DIR) -losa -lmsdb -lmsptz -lmssocket -lmsstd -llog -lvapi -lfaad -lencrypt -lpoe -lchipinfo -lmstde -lovfcli -lgpio -lqrencode -lmsfs -lavilib -luv -lsdkp2p -lmsini -lnghttp2 -lmov -lrecortsp
LIBS += -L$$(STD_USRLIB_DIR) -lx264 -lx265 -lcurl -luuid -lmscrypt -lsqlite3 -lssl -lcrypto -lrt -lhdal -lvendor_media -lsamplerate
LIBS += -lm -ldl -lpthread
}

contains(PLAT_FORM, hi3536a) {
DEFINES += _HI3536A_
}


include(backup.pri)
include(boards.pri)
include(daemon.pri)
include(dbtools.pri)
include(demo.pri)
include(fio.pri)
include(mscli.pri)
include(msjs.pri)
include(mskb.pri)
include(msmail.pri)
include(mssp.pri)
include(sysconf.pri)
include(update.pri)
include(web.pri)

include(avilib.pri)
include(chipinfo.pri)
include(log.pri)
include(msdb.pri)
include(msfs.pri)
include(msptz.pri)
include(msrtsp.pri)
include(mssocket.pri)
include(msstd.pri)
include(ovfcli.pri)
include(poe.pri)
include(recortsp.pri)
include(sdkp2p.pri)
include(vapi.pri)

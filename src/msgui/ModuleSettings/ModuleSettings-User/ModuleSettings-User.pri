INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/AddUserDialog.ui \
    $$PWD/EditUserDialog.ui \
    $$PWD/PageUserSettingOperator.ui \
    $$PWD/PageUserSettings.ui \
    $$PWD/UserLimitWidget.ui \
    $$PWD/edituserlimitsdialog.ui \
    $$PWD/setunlockpattern.ui

HEADERS += \
    $$PWD/AddUserDialog.h \
    $$PWD/EditUserDialog.h \
    $$PWD/PageUserSettingOperator.h \
    $$PWD/PageUserSettings.h \
    $$PWD/PermissionContainer.h \
    $$PWD/UserLimitWidget.h \
    $$PWD/edituserlimitsdialog.h \
    $$PWD/setunlockpattern.h

SOURCES += \
    $$PWD/AddUserDialog.cpp \
    $$PWD/EditUserDialog.cpp \
    $$PWD/PageUserSettingOperator.cpp \
    $$PWD/PageUserSettings.cpp \
    $$PWD/PermissionContainer.cpp \
    $$PWD/UserLimitWidget.cpp \
    $$PWD/edituserlimitsdialog.cpp \
    $$PWD/setunlockpattern.cpp

QT += widgets

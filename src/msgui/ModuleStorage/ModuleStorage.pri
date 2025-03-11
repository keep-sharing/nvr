include($$PWD/ModuleStorage-AutoBackup/ModuleStorage-AutoBackup.pri)
include($$PWD/ModuleStorage-Disk/ModuleStorage-Disk.pri)
include($$PWD/ModuleStorage-Raid/ModuleStorage-Raid.pri)
include($$PWD/ModuleStorage-Snapshot/ModuleStorage-Snapshot.pri)
include($$PWD/ModuleStorage-StorageMode/ModuleStorage-StorageMode.pri)
include($$PWD/ModuleStorage-VideoRecord/ModuleStorage-VideoRecord.pri)

INCLUDEPATH += $$PWD

FORMS += \
    $$PWD/storage.ui

HEADERS += \
    $$PWD/storage.h

SOURCES += \
    $$PWD/storage.cpp

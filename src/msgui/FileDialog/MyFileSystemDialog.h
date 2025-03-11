#ifndef MYFILESYSTEMDIALOG_H
#define MYFILESYSTEMDIALOG_H

#include "BaseShadowDialog.h"
#include "NewFolderDialog.h"
#include <QEventLoop>
#include <QFileInfo>
#include <QMap>

extern "C" {
#include "msg.h"
}

class QListWidgetItem;
class QPropertyAnimation;
class DeviceInfoWidget;

class FormatDialog;
class MessageReceive;
class MsWaitting;

enum ExportPeopleCountingFileFormat {
    PeopleCounting_CSV,
    PeopleCounting_PDF,
    PeopleCounting_PNG
};

enum SmartAnalysisBackupResult {
    SMARTANALYSIS_BACKUP_UNKOWN,
    SMARTANALYSIS_BACKUP_SUCCESS,
    SMARTANALYSIS_BACKUP_FAILED,
    SMARTANALYSIS_BACKUP_SOMESUCCESS
};

struct ExportPeopleCountingInfo {
    QString path;
    ExportPeopleCountingFileFormat fileFormat = PeopleCounting_CSV;

    bool isValid()
    {
        return !path.isEmpty();
    }
};

namespace Ui {
class MyFileSystemDialog;
}

class MyFileSystemDialog : public BaseShadowDialog {
    Q_OBJECT

public:
    enum OpenMode {
        ModeGetFile, //获取选中文件
        ModeGetDirectory, //获取选中目录
        ModeNormalExport,
        ModeNormalExportWithAnimate,
        ModeExportVideo,
        ModeExportVideoWithAnimate,
        ModeExportAnpr,
        ModeExportPos,
        ModeNone
    };

    explicit MyFileSystemDialog(QWidget *parent = nullptr);
    ~MyFileSystemDialog();

    static MyFileSystemDialog *instance();
    static QString bytesString(qint64 bytes);

    void initializeData();

    QString getOpenFileInfo();
    QString getOpenDirectory();
    QString exportFile();
    QString exportVideo();
    QString exportVideoWithAnimate();
    QString exportAnpr();
    QString exportPos();
    QString exportFace();
    QString exportPicture();
    QString exportPictureWithAnimate();
    ExportPeopleCountingInfo exportPeopleCounting();
    void showDialog();

    QString currentDeviceName();
    qint64 currentDeviceFreeBytes();
    int currentDeviceFreeMegaBytes();
    int currentDevicePort();
    int anprExportType() const;
    int anprExportBackType() const;
    int streamType() const;
    int fileType() const;
    int pictureResolution() const;

    QPoint exportButtonPos();

    //anpr
    bool isExportAnprPlate() const;
    bool isExportAnprVideo() const;
    bool isExportAnprPicture() const;
    void clearAnprSelected();

    //pos
    int exportPosFileType() const;

    void dealMessage(MessageReceive *message);
    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_EXPORT_DISK(MessageReceive *message);
    void ON_RESPONSE_FLAG_UDISK_OFFLINE(MessageReceive *message);

    bool isDrawShadow() override;
    void hideEvent(QHideEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void updateDeviceList();

private slots:
    void onLanguageChanged() override;

    void onPathChanged(const QString &path);
    void onCurrentFileInfo(const QFileInfo &fileInfo);

    void onNewFolder(const QString &name);

    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton_export_clicked();

    void on_listWidget_device_itemClicked(QListWidgetItem *item);

    void on_pushButton_newFolder_clicked();
    void on_pushButton_format_clicked();
    void on_pushButton_refresh_clicked();

private:
    static MyFileSystemDialog *s_fileSystemDialog;

    Ui::MyFileSystemDialog *ui;
    MsWaitting *m_watting = nullptr;
    QPropertyAnimation *m_animation = nullptr;
    QPixmap m_pixmap;

    OpenMode m_mode = ModeNone;

    QMap<int, resp_usb_info> m_usbInfoMap;
    const DeviceInfoWidget *m_currentDevice = nullptr;

    NewFolderDialog *m_newFolder = nullptr;
    FormatDialog *m_formatDialog = nullptr;

    QEventLoop m_eventLoop;
    QPoint m_exportPos;
};

#endif // MYFILESYSTEMDIALOG_H

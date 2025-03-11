#ifndef CAMERAEDIT_H
#define CAMERAEDIT_H

#include "BaseShadowDialog.h"

extern "C" {
#include "msg.h"
}

class AbstractSettingTab;
class MessageReceive;

namespace Ui {
class CameraEdit;
}

class CameraEdit : public BaseShadowDialog
{
    Q_OBJECT

    enum CameraTab
    {
        TabSettings,
        TabParameters,
        TabParametersOld,
    };

public:
    explicit CameraEdit(QWidget *parent = nullptr);
    ~CameraEdit();

    void showEdit(int channel);
    void showEdit(int channel, const resq_get_ipcdev &ipcdev);

    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_FRAME_RESOLUTION(MessageReceive *message);

private slots:
    void onLanguageChanged() override;

    void onTabBarClicked(int index);

private:
    Ui::CameraEdit *ui;

    QEventLoop m_eventLoop;

    int m_currentTab;
    QMap<int, AbstractSettingTab *> m_tabsMap;

    int m_channel;
    resq_get_ipcdev m_ipcdev;

    QByteArray m_jsonData;

    friend class CameraEditTabSettings;
    friend class CameraEditTabParameters;
    friend class CameraEditTabParametersOld;
};

#endif // CAMERAEDIT_H

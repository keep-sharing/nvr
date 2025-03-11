#ifndef TABFACEADVANCED_H
#define TABFACEADVANCED_H

#include "AbstractSettingTab.h"
#include <QEventLoop>

extern "C" {
#include "msg.h"
}

class FaceAttributeRecognitionSettings;

namespace Ui {
class TabFaceAdvanced;
}

class TabFaceAdvanced : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit TabFaceAdvanced(QWidget *parent = nullptr);
    ~TabFaceAdvanced();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_FACE_SUPPORT(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_FACE_CONFIG(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_FACE_CONFIG(MessageReceive *message);
    void hideMessage();
    void showNotConnectedMessage();
    void showNotSupportMessage();

    void clearSetting();
    void saveData();

     void updateEnableState();

private slots:
    void onLanguageChanged() override;
    void onChannelButtonClicked(int index);

    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

    void on_pushButtonEdit_clicked();
    void on_comboBoxFacePrivacy_currentIndexChanged(int index);

    void on_toolButtonTip_clicked(bool checked);

private:
    Ui::TabFaceAdvanced *ui;

    int m_currentChannel = 0;
    bool m_isConnected = false;
    bool m_isSupported = -1;
    MsFaceConfig m_faceConfig;
    FaceAttributeRecognitionSettings *m_faceSetting = nullptr;

    QEventLoop m_eventLoop;

    //提示语只在保存的时候不改变状态
    bool m_toolButtonClicked = false;
    bool m_hasApply = false;
};

#endif // TABFACEADVANCED_H

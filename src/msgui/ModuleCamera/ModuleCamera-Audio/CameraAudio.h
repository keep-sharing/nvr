#ifndef CAMERAAUDIO_H
#define CAMERAAUDIO_H

#include "abstractcamerapage.h"
#include <QEventLoop>
#include <QWidget>

extern "C" {
#include "msg.h"
}

namespace Ui {
class CameraAudio;
}

class CameraAudio : public AbstractCameraPage {
    Q_OBJECT

public:
    explicit CameraAudio(QWidget *parent = nullptr);
    ~CameraAudio();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_IPC_AUDIO_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_AUDIO_INFO(MessageReceive *message);

    void setSettingEnable(bool enable);
    void clearSetting();
    void setAudioOutputVisible(bool visible);

    void showEncodingComboBox();
    void showSampleRateComboBox();
    void showAudioBitRateComboBox();

    void saveAudioSettings(const QList<int> &channels);

private slots:
    void onChannelGroupClicked(int channel);

    void on_comboBoxEnable_indexSet(int index);
    void on_comboBoxEncoding_indexSet(int index);
    void on_comboBoxSampleRate_indexSet(int index);

    void on_pushButtonCopy_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::CameraAudio *ui;

    QEventLoop m_eventLoop;
    int m_currentChannel = 0;
    ipc_audio_info m_audio_info;
};

#endif // CAMERAAUDIO_H

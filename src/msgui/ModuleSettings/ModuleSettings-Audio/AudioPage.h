#ifndef AUDIOPAGE_H
#define AUDIOPAGE_H

#include "AddAudioFile.h"
#include "AbstractSettingPage.h"

class AddAudioFile;

namespace Ui {
class AudioPage;
}

class AudioPage : public AbstractSettingPage {
    Q_OBJECT

    enum AudioPageColumn {
        ColumnCheck,
        ColumnAudioFileNo,
        ColumnAudioFileName,
        ColumnPlay,
        ColumnEdit,
        ColumnDelete
    };

public:
    explicit AudioPage(QWidget *parent = nullptr);
    ~AudioPage();
    void initializeData() override;

    void processMessage(MessageReceive *message) override;
    void filterMessage(MessageReceive *message) override;

private:
    void showTable();
    void ON_RESPONSE_FLAG_GET_AUDIOFILE_INFO(MessageReceive *message);
    void ON_RESPONSE_FLAG_DEL_AUDIOFILE(MessageReceive *message);
    void ON_RESPONSE_FLAG_GET_AUDIOFILE_PALY_STATUS(MessageReceive *message);
    void ON_RESPONSE_FLAG_PLAY_AUDIOFILE(MessageReceive *message);
private slots:
    void onLanguageChanged() override;
    void AudioTableClicked(int row, int column);
    void on_pushButtonDelete_clicked();

    void on_pushButtonBack_clicked();

private:
    Ui::AudioPage *ui;
    QMap<int, AUDIO_FILE> m_AudioMap;
    AddAudioFile *m_addAudioFile = nullptr;
    bool m_hasRowChecked;
    bool m_startAudio = true;
};

#endif // AUDIOPAGE_H

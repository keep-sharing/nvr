#ifndef ADDAUDIOFILE_H
#define ADDAUDIOFILE_H

#include "BaseShadowDialog.h"
#include "msdb.h"
#include "LogWrite.h"
#include "msuser.h"
namespace Ui {
class AddAudioFile;
}

class AddAudioFile : public BaseShadowDialog
{
    Q_OBJECT

public:
    enum AddAudioFileMode {
           ItemAddAudioMode,
           ItemEditAudioMode
       };
    explicit AddAudioFile(QWidget *parent = nullptr);
    ~AddAudioFile();

    void initializeData(QMap<int, AUDIO_FILE> *audioMap, int row, AddAudioFileMode addMode);
    void initializeData(QMap<int, AUDIO_FILE> *audioMap, int row);
    void processMessage(MessageReceive *message) override;

private slots:
    void onLanguageChanged();

    void on_pushButtonBrowse_clicked();
    void on_pushButtonOK_clicked();
    void on_pushButtonCancel_clicked();
private:
    void ON_RESPONSE_FLAG_ADD_AUDIOFILE(MessageReceive *message);
    void ON_RESPONSE_FLAG_EDIT_AUDIOFILE(MessageReceive *message);
private:
    Ui::AddAudioFile *ui;
    QMap<int, AUDIO_FILE> *m_AudioMap = nullptr;
    AddAudioFileMode m_AddAudioFileMode;
    int m_row;
};

#endif // ADDAUDIOFILE_H

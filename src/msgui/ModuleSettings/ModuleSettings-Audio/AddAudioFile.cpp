#include "AddAudioFile.h"
#include "ui_AddAudioFile.h"
#include "MyFileSystemDialog.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include <QDebug>
#include "centralmessage.h"

AddAudioFile::AddAudioFile(QWidget *parent)
    :  BaseShadowDialog(parent)
    , ui(new Ui::AddAudioFile)
{
    ui->setupUi(this);

    ui->lineEditAudioFile->setCheckMode(MyLineEdit::AudioCheck);
    ui->lineEditAudioFile->setReadOnly(true);
    ui->lineEditAudioFileName->setCheckMode(MyLineEdit::EmptyCheck);

    onLanguageChanged();
}

AddAudioFile::~AddAudioFile()
{
    delete ui;
}

void AddAudioFile::initializeData(QMap<int, AUDIO_FILE> *audioMap, int row, AddAudioFile::AddAudioFileMode addMode)
{
    m_AudioMap = audioMap;
    m_AddAudioFileMode = addMode;
    m_row = row;
    ui->label_title->setText(GET_TEXT("AUDIOFILE/117013", "Audio File Add"));

    ui->lineEditAudioFile->clear();
    ui->lineEditAudioFileName->clear();
    ui->labelAudioFile->setVisible(true);
    ui->widgetAudioFIle->setVisible(true);
    ui->labelNote->setVisible(true);
}

void AddAudioFile::initializeData(QMap<int, AUDIO_FILE> *audioMap, int row)
{
    m_AudioMap = audioMap;
    m_AddAudioFileMode = ItemEditAudioMode;
    m_row = row;
    ui->label_title->setText(GET_TEXT("AUDIOFILE/117014", "Audio File Edit"));
    ui->labelAudioFile->setVisible(false);
    ui->widgetAudioFIle->setVisible(false);
    ui->labelNote->setVisible(false);
    //base64转码
    const AUDIO_FILE &content = m_AudioMap->value(row);
    char name[AUDIO_FILE_NAME_SIZE] = {0};
    base64DecodeUrl((MS_U8 *)name, sizeof(name), (MS_U8 *)content.name, strlen(content.name));
    ui->lineEditAudioFileName->setText(name);

}

void AddAudioFile::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_ADD_AUDIOFILE:
        ON_RESPONSE_FLAG_ADD_AUDIOFILE(message);
        break;
    case RESPONSE_FLAG_EDIT_AUDIOFILE:
        ON_RESPONSE_FLAG_EDIT_AUDIOFILE(message);
        break;
    default:
        break;
    }
}

void AddAudioFile::onLanguageChanged()
{
    ui->pushButtonBrowse->setText(GET_TEXT("CAMERAMAINTENANCE/38004", "Browse"));
    ui->pushButtonOK->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButtonCancel->setText(GET_TEXT("COMMON/1004", "Cancel"));
    ui->labelNote->setText(GET_TEXT("AUDIOFILE/117006", "Note: Only support '.wav' audio files with codec type PCM/PCMU/PCMA, 64kbps or 128kbps bitrate and no more than 500k!"));
    ui->labelAudioFile->setText(GET_TEXT("AUDIOFILE/117012", "Audio File"));
    ui->labelAudioFileName->setText(GET_TEXT("AUDIOFILE/117002", "Audio File Name"));
}

void AddAudioFile::on_pushButtonBrowse_clicked()
{
    ui->pushButtonBrowse->clearUnderMouse();
    ui->pushButtonBrowse->clearFocus();
    ui->lineEditAudioFileName->clear();

    QString filePath = MyFileSystemDialog::instance()->getOpenFileInfo();
    ui->lineEditAudioFile->setText(filePath);
}

void AddAudioFile::on_pushButtonOK_clicked()
{
    char name[AUDIO_FILE_NAME_SIZE] = {0};
    char pname[AUDIO_FILE_NAME_SIZE] = {0};

    switch (m_AddAudioFileMode) {
    case ItemAddAudioMode: {
        bool valid = ui->lineEditAudioFile->checkValid();
        if (valid) {
            QFileInfo path(ui->lineEditAudioFile->text());
            QString filesuffix = path.suffix();
            if((filesuffix != "wav" && filesuffix != "WAV") || path.size() > MAX_LEN_500K) {
                ui->lineEditAudioFile->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
                valid = false;
            }
        }
        valid = ui->lineEditAudioFileName->checkValid() && valid;

        snprintf(name, sizeof(name), "%s", ui->lineEditAudioFileName->text().toStdString().c_str());
        int ret = base64EncodeUrl((MS_U8 *)pname, sizeof(pname), (MS_U8 *)name, strlen(name));
        if (!ui->lineEditAudioFileName->text().isEmpty()) {
            if (ret <= 0) {
                ui->lineEditAudioFileName->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
                valid = false;
            }
            for (auto iter = m_AudioMap->constBegin(); iter != m_AudioMap->constEnd(); ++iter) {
                const AUDIO_FILE &content = iter.value();
                QString fileName = content.name;
                if (fileName == pname) {
                    ui->lineEditAudioFileName->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already Existed."));
                    valid = false;
                    break;
                }
            }
        }

        QString copyNmae = "/tmp/" + QString(pname);
        if (valid && !QFile::copy(ui->lineEditAudioFile->text(), copyNmae)){
            ui->lineEditAudioFileName->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already Existed."));
            valid = false;
        }

        if (!valid) {
            return;
        }
        break;
    }
    case ItemEditAudioMode:{
        if(!ui->lineEditAudioFileName->checkValid()) {
            return;
        }
        snprintf(name, sizeof(name), "%s", ui->lineEditAudioFileName->text().toStdString().c_str());
        int ret = base64EncodeUrl((MS_U8 *)pname, sizeof(pname), (MS_U8 *)name, strlen(name));
        if (ret <= 0) {
             ui->lineEditAudioFileName->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
            return;
        }
        for (auto iter = m_AudioMap->constBegin(); iter != m_AudioMap->constEnd(); ++iter) {
            const AUDIO_FILE &content = iter.value();
            QString fileName = content.name;
            if (fileName == pname && content.id != m_AudioMap->value(m_row).id) {
                ui->lineEditAudioFileName->setCustomValid(false, GET_TEXT("MYLINETIP/112011", "Already existed."));
                return;
            }
        }
        break;
    }
    default:
        break;
    }
    AUDIO_FILE audioInfo;
    audioInfo.id = m_row;
    audioInfo.enable = 1;
    memcpy(audioInfo.name, pname, sizeof(pname));
    m_AudioMap->remove(m_row);
    m_AudioMap->insert(m_row,audioInfo);

    switch (m_AddAudioFileMode) {
    case ItemAddAudioMode: {
        sendMessage(REQUEST_FLAG_ADD_AUDIOFILE, audioInfo.name, sizeof(audioInfo.name));
        break;
    }
    case ItemEditAudioMode:{
        sendMessage(REQUEST_FLAG_EDIT_AUDIOFILE, &audioInfo, sizeof(audioFile));
        break;
    }
    default:
        break;
    }
}

void AddAudioFile::on_pushButtonCancel_clicked()
{
    reject();
}

void AddAudioFile::ON_RESPONSE_FLAG_ADD_AUDIOFILE(MessageReceive *message)
{
    struct log_data logData;
    int res = *(int *)message->data;
    if (res == 0) {
        const int result = MessageBox::information(this, GET_TEXT("AUDIOFILE/117010", "Added successfully."));
        if (result == MessageBox::OK) {
            memset(&logData, 0, sizeof(logData));
            logData.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
            logData.log_data_info.parameter_type = SUB_PARAM_AUDIO_FILE;
            snprintf(logData.log_data_info.user, sizeof(logData.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
            MsWriteLog(logData);
            sync();
            accept();
        }
    } else {
        m_AudioMap->remove(m_row);
        ui->lineEditAudioFile->setCustomValid(false, GET_TEXT("MYLINETIP/112001", "Invalid."));
    }
}

void AddAudioFile::ON_RESPONSE_FLAG_EDIT_AUDIOFILE(MessageReceive *message)
{
    struct log_data logData;
    int res = *(int *)message->data;
    if (res == 0) {
        memset(&logData, 0, sizeof(logData));
        logData.log_data_info.subType = SUB_OP_CONFIG_LOCAL;
        logData.log_data_info.parameter_type = SUB_PARAM_AUDIO_FILE;
        snprintf(logData.log_data_info.user, sizeof(logData.log_data_info.user), "%s", gMsUser.userName().toStdString().c_str());
        MsWriteLog(logData);
        sync();
    }

    accept();
}

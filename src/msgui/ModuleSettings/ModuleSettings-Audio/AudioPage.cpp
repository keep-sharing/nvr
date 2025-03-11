#include "AudioPage.h"
#include "ui_AudioPage.h"
#include "AddAudioFile.h"
#include "centralmessage.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyCheckBox.h"
#include <QDebug>
#include "MessageFilter.h"

AudioPage::AudioPage(QWidget *parent)
    : AbstractSettingPage(parent)
    , ui(new Ui::AudioPage)
{
    ui->setupUi(this);

    m_addAudioFile = new AddAudioFile(this);

    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("AUDIOFILE/117001", "Audio File No.");
    headerList << GET_TEXT("AUDIOFILE/117002", "Audio File Name");
    headerList << GET_TEXT("COMMONBACKUP/100045", "Play");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);
    ui->tableView->setColumnCount(headerList.size());
    ui->tableView->setSortingEnabled(false);

    ui->tableView->setItemDelegateForColumn(ColumnPlay, new ItemCheckedButtonDelegate(ItemCheckedButtonDelegate::ButtonPlay, this));
    ui->tableView->setItemDelegateForColumn(ColumnEdit, new ItemButtonDelegate(QPixmap(":/common/common/edit.png"), this));
    ui->tableView->setItemDelegateForColumn(ColumnDelete, new ItemButtonDelegate(QPixmap(":/common/common/delete.png"), this));
    connect(ui->tableView, SIGNAL(itemClicked(int, int)), this, SLOT(AudioTableClicked(int, int)));

    ui->tableView->setColumnWidth(ColumnCheck, 50);
    ui->tableView->setColumnWidth(ColumnAudioFileNo, 150);
    ui->tableView->setColumnWidth(ColumnAudioFileName, 500);
    ui->tableView->setColumnWidth(ColumnPlay, 250);
    ui->tableView->setColumnWidth(ColumnEdit, 250);
    ui->tableView->setColumnWidth(ColumnDelete, 250);

    ui->tableView->setHasAddButton(true);
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonDelete->setText(GET_TEXT("AUDIOFILE/117005", "Delete"));

    gMessageFilter.installMessageFilter(RESPONSE_FLAG_GET_AUDIOFILE_PALY_STATUS, this);

    onLanguageChanged();
}

AudioPage::~AudioPage()
{
    delete ui;
}

void AudioPage::initializeData()
{
    //showWait();
    sendMessage(REQUEST_FLAG_GET_AUDIOFILE_INFO, &m_hasRowChecked, sizeof(bool));

    m_startAudio = true;
    sendMessage(REQUEST_FLAG_GET_AUDIOFILE_PALY_STATUS, &m_hasRowChecked, sizeof(bool));
    //closeWait();
}

void AudioPage::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_AUDIOFILE_INFO:
        ON_RESPONSE_FLAG_GET_AUDIOFILE_INFO(message);
        break;
    case RESPONSE_FLAG_DEL_AUDIOFILE:
        ON_RESPONSE_FLAG_DEL_AUDIOFILE(message);
        break;
    case RESPONSE_FLAG_PLAY_AUDIOFILE:
        ON_RESPONSE_FLAG_PLAY_AUDIOFILE(message);
        break;
    default:
        break;
    }
}

void AudioPage::filterMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_AUDIOFILE_PALY_STATUS:
        ON_RESPONSE_FLAG_GET_AUDIOFILE_PALY_STATUS(message);
        break;
    default:
        break;
    }
}

void AudioPage::AudioTableClicked(int row, int column)
{
    int rowCount = ui->tableView->rowCount() - 1;

    qDebug() << "====Audio::onTableClicked====";
    qDebug() << "----row:" << row << ", column:" << column << ", rowCount:" << rowCount;

    switch (column) {
    case ColumnCheck: {
        if (row == rowCount && rowCount < MAX_AUDIO_FILE) {
            ui->tableView->setItemChecked(row, false);
            int addNo = 0;
            for (int i = 1; i <= MAX_AUDIO_FILE; ++i) {
                if (!m_AudioMap.value(i).enable) {
                    addNo = i;
                    break;
                }
            }
            if (addNo == 0) {
                addNo = rowCount + 1;
            }
            m_addAudioFile->initializeData(&m_AudioMap, addNo, AddAudioFile::ItemAddAudioMode);
            int result = m_addAudioFile->exec();
            if (result == AddAudioFile::Accepted) {
                showTable();
            }
        }
        break;
    }
    case ColumnEdit: {
        if (row < rowCount) {
            int audioFileNo = ui->tableView->itemIntValue(row, ColumnAudioFileNo);
            m_addAudioFile->initializeData(&m_AudioMap, audioFileNo);
            int result = m_addAudioFile->exec();
            if (result == AddAudioFile::Accepted) {
                showTable();
            }
        }
        break;
    }
    case ColumnDelete: {
        if (row < rowCount) {
            const int result = MessageBox::question(this, GET_TEXT("AUDIOFILE/117011", "Sure to delete the selected audio file?"), GET_TEXT("PROFILE/76016", "Yes"), GET_TEXT("COMMON/1056", "No"));
            if (result == MessageBox::Yes) {
                int audioFileNo = ui->tableView->itemIntValue(row, ColumnAudioFileNo);
                m_AudioMap.remove(audioFileNo);
                int idMask = 0;
                idMask = idMask | (1 << (audioFileNo - 1));
                sendMessage(REQUEST_FLAG_DEL_AUDIOFILE, &idMask, sizeof(int));
            }
        }
        break;
    }
    case ColumnPlay: {
    if (row < rowCount) {
        int audioFileNo = ui->tableView->itemIntValue(row, ColumnAudioFileNo);

        bool isPlay = ui->tableView->isItemChecked(row, ColumnPlay);
        if (isPlay) {
            audioFileNo = 0;
        } else {
            ui->tableView->setItemData(row, ColumnPlay, 1, ItemCheckedRole);
        }
        Q_UNUSED(audioFileNo)
    }
    break;
    }
    default:
        break;
    }
}
void AudioPage::showTable()
{
    ui->tableView->clearContent();
    ui->tableView->setRowCount(1);
    int row = 0;
    for (auto iter = m_AudioMap.constBegin(); iter != m_AudioMap.constEnd(); ++iter) {
        const AUDIO_FILE &content = iter.value();
        if (content.enable) {
            ui->tableView->setItemIntValue(row, ColumnAudioFileNo, iter.key());
            //base64转码
            char name[AUDIO_FILE_NAME_SIZE] = {0};
            base64DecodeUrl((MS_U8 *)name, sizeof(name), (MS_U8 *)content.name, strlen(content.name));
            ui->tableView->setItemText(row, ColumnAudioFileName, name);
            row++;
            ui->tableView->setRowCount(row + 1);
        }
    }
    ui->tableView->setItemText(row, ColumnAudioFileNo, "-");
    ui->tableView->setItemText(row, ColumnAudioFileName, "-");
    ui->tableView->setItemText(row, ColumnPlay, "-");
    ui->tableView->setItemText(row, ColumnEdit, "-");
    ui->tableView->setItemText(row, ColumnDelete, "-");
    if (row == MAX_AUDIO_FILE) {
        ui->tableView->setItemEnable(row, ColumnCheck, false);
        ui->tableView->setItemPixmap(row, ColumnCheck, QPixmap (":/common/common/add-disable.png"));
    } else {
        ui->tableView->setItemPixmap(row, ColumnCheck, QPixmap (":/ptz/ptz/add.png"));
    }
}

void AudioPage::ON_RESPONSE_FLAG_GET_AUDIOFILE_INFO(MessageReceive *message)
{
    m_AudioMap.clear();
    AUDIO_FILE *param = (AUDIO_FILE *)message->data;
    for (int i = 0; i < MAX_AUDIO_FILE_NUM ; ++i) {
        m_AudioMap.insert(param[i].id, param[i]);
    }
    showTable();
}

void AudioPage::ON_RESPONSE_FLAG_DEL_AUDIOFILE(MessageReceive *message)
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

    showTable();
}

void AudioPage::ON_RESPONSE_FLAG_GET_AUDIOFILE_PALY_STATUS(MessageReceive *message)
{
    int audioId = *(int *)message->data;
    for (int i = 0 ;i < ui->tableView->rowCount(); i++) {
        int index = ui->tableView->itemIntValue(i, ColumnAudioFileNo);
        if (index == audioId) {
            if (m_startAudio) {
                ui->tableView->setItemData(i, ColumnPlay, 1, ItemCheckedRole);
                m_startAudio = false;
            } else {
                ui->tableView->setItemData(i, ColumnPlay, 0, ItemCheckedRole);
            }
            break;
        }
    }
}

void AudioPage::ON_RESPONSE_FLAG_PLAY_AUDIOFILE(MessageReceive *message)
{
    Q_UNUSED(message);
}


void AudioPage::onLanguageChanged()
{
    QStringList headerList;
    headerList << "";
    headerList << GET_TEXT("AUDIOFILE/117001", "Audio File No.");
    headerList << GET_TEXT("AUDIOFILE/117002", "Audio File Name");
    headerList << GET_TEXT("COMMONBACKUP/100045", "Play");
    headerList << GET_TEXT("COMMON/1019", "Edit");
    headerList << GET_TEXT("CHANNELMANAGE/30023", "Delete");
    ui->tableView->setHorizontalHeaderLabels(headerList);

    //
    ui->pushButtonBack->setText(GET_TEXT("COMMON/1002", "Back"));
    ui->pushButtonDelete->setText(GET_TEXT("AUDIOFILE/117005", "Delete"));
}

void AudioPage::on_pushButtonDelete_clicked()
{
    if (ui->tableView->rowCount() <= 1) {
        return ;
    }
    m_hasRowChecked = false;
    for (int row = ui->tableView->rowCount() - 1; row > 0; --row) {
        if (ui->tableView->isItemChecked(row)) {
            m_hasRowChecked = true;
            break;
        }
    }
    if (!m_hasRowChecked) {
        ShowMessageBox(GET_TEXT("AUDIOFILE/117007", "Please select at least one audio file."));
        return;
    } else {
        const int result = MessageBox::question(this, GET_TEXT("AUDIOFILE/117011", "Sure to delete the selected audio file?"), GET_TEXT("PROFILE/76016", "Yes"), GET_TEXT("COMMON/1056", "No"));
        if (result == MessageBox::Yes) {
            int idMask = 0;
            for (int row = ui->tableView->rowCount(); row >= 0; --row) { //选择视图表的一行
                if (ui->tableView->isItemChecked(row)) {
                    int audioFileNo = ui->tableView->itemIntValue(row, ColumnAudioFileNo);
                    m_AudioMap.remove(audioFileNo);
                    idMask = idMask | (1 << (audioFileNo - 1));
                }
            }
            sendMessage(REQUEST_FLAG_DEL_AUDIOFILE, &idMask, sizeof(int));
            ui->tableView->setHeaderChecked(false);
        }
    }
}

void AudioPage::on_pushButtonBack_clicked()
{
    emit sig_back();
}

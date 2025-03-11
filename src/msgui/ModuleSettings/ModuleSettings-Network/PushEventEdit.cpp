#include "PushEventEdit.h"
#include "ui_PushEventEdit.h"
#include "channelcopydialog.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include <QtDebug>

PushEventEdit::PushEventEdit(QWidget *parent)
    : BaseShadowDialog(parent)
    , ui(new Ui::PushEventEdit)
{
    ui->setupUi(this);

    ui->tabBar->addTab(GET_TEXT("EVENTSTATUS/63002", "Camera Event"));
    ui->tabBar->addTab(GET_TEXT("EVENTSTATUS/63005", "NVR Event"));
    connect(ui->tabBar, SIGNAL(tabClicked(int)), this, SLOT(onTabBarClicked(int)));

    int maxChannel = qMsNvr->maxChannel();
    ui->comboBox_channel->clear();
    for (int i = 0; i < maxChannel; ++i) {
        ui->comboBox_channel->addItem(QString("%1").arg(i + 1), i);
    }

    m_checkBoxMap.insert(0x01, ui->checkBox_motion);
    m_checkBoxMap.insert(0x01 << 1, ui->checkBox_videoLoss);
    m_checkBoxMap.insert(0x01 << 2, ui->checkBox_regionEntrance);
    m_checkBoxMap.insert(0x01 << 3, ui->checkBox_regionExiting);
    m_checkBoxMap.insert(0x01 << 4, ui->checkBox_adcancedMotion);
    m_checkBoxMap.insert(0x01 << 5, ui->checkBox_tamper);
    m_checkBoxMap.insert(0x01 << 6, ui->checkBox_lineCrossing);
    m_checkBoxMap.insert(0x01 << 7, ui->checkBox_loitering);
    m_checkBoxMap.insert(0x01 << 8, ui->checkBox_human);
    m_checkBoxMap.insert(0x01 << 9, ui->checkBox_objectLeftRemoved);
    m_checkBoxMap.insert(0x01 << 10, ui->checkBox_alarmIn_1);
    m_checkBoxMap.insert(0x01 << 11, ui->checkBox_alarmIn_2);
    m_checkBoxMap.insert(0x01 << 12, ui->checkBox_black);
    m_checkBoxMap.insert(0x01 << 13, ui->checkBox_white);
    m_checkBoxMap.insert(0x01 << 14, ui->checkBox_visitor);
    if (qMsNvr->isSupportFaceDetection()) {
        m_checkBoxMap.insert(0x01 << 15, ui->checkBoxFaceDetection);
    } else {
        ui->checkBoxFaceDetection->hide();
    }
    m_checkBoxMap.insert(0x01 << 16, ui->checkBoxAlarmIn3);
    m_checkBoxMap.insert(0x01 << 17, ui->checkBoxAlarmIn4);
    m_checkBoxMap.insert(0x01 << 18, ui->checkBoxAudioAlarm);
    m_checkBoxMap.insert(0x01 << 19, ui->checkBoxPeopleCounting);
    m_checkBoxMap.insert(0x01 << 20, ui->checkBoxRegionalPeopleCounting);

    QMap<int, QCheckBox *>::iterator iter = m_checkBoxMap.begin();
    for (; iter != m_checkBoxMap.end(); ++iter) {
        QCheckBox *checkBox = iter.value();
        connect(checkBox, SIGNAL(clicked(bool)), this, SLOT(onCheckBoxTypeClicked(bool)));
    }

    onTabBarClicked(0);
    ui->tabBar->setCurrentTab(0);
    onLanguageChanged();

    int alarmInput = qMsNvr->maxAlarmInput();
    ui->widgetAlarmInput->setVisible(alarmInput);
    ui->widgetAlarmGroup->setVisible(alarmInput);
    if (alarmInput) {
        ui->widget_alarm_group->setCount(alarmInput);
        if (alarmInput > 7) {
            ui->widget_spacer->hide();
        }
    }
    ui->widget_anpr->setVisible(!qMsNvr->is3536c());

    ui->checkBoxGroupPos->setCount(MAX_POS_CLIENT);
}

PushEventEdit::~PushEventEdit()
{
    delete ui;
}

QMap<int, int> PushEventEdit::getIPCPushType()
{
    return IPCPushType;
}

int PushEventEdit::getNVRAlarmInType()
{
    return NVRAlarmInType;
}

void PushEventEdit::setType(QMap<int, int> ipcType, int nvrType)
{
    IPCPushType = ipcType;
    NVRAlarmInType = nvrType;
    qDebug() << QString("NVRAlarmInType:[%1], IPCPushType1.size:[%2], IPCPushType1:[%3], IPCPushType2:[%4]").arg(NVRAlarmInType).arg(IPCPushType.size()).arg(IPCPushType.value(0, -1)).arg(IPCPushType.value(1, -1));
    on_comboBox_channel_activated(0);
    ui->widget_alarm_group->setCheckedFromInt(NVRAlarmInType);
}

void PushEventEdit::setNvrEventPos(PushMsgNvrEvent *event)
{
    m_nvrEventPos = event;
    ui->checkBoxGroupPos->setCheckedFromString(event->pos);
}

void PushEventEdit::showPushType(int event)
{
    QMap<int, QCheckBox *>::iterator iter = m_checkBoxMap.begin();
    for (; iter != m_checkBoxMap.end(); ++iter) {
        int key = iter.key();
        QCheckBox *checkBox = iter.value();
        checkBox->setChecked(event & key);
    }

    onCheckBoxTypeClicked(true);
}

int PushEventEdit::pushType()
{
    int type = 0;
    QMap<int, QCheckBox *>::iterator iter = m_checkBoxMap.begin();
    for (; iter != m_checkBoxMap.end(); ++iter) {
        int key = iter.key();
        QCheckBox *checkBox = iter.value();
        if (checkBox->isChecked()) {
            type |= key;
        }
    }
    return type;
}

void PushEventEdit::onLanguageChanged()
{
    ui->label_title->setText(GET_TEXT("SYSTEMNETWORK/71016", "Push Message Settings"));
    ui->checkBox_all->setText(GET_TEXT("SYSTEMGENERAL/164000", "All"));
    ui->checkBox_motion->setText(GET_TEXT("MOTION/51000", "Motion Detection"));
    ui->checkBox_videoLoss->setText(GET_TEXT("VIDEOLOSS/50001", "Video Loss"));
    ui->checkBox_regionEntrance->setText(GET_TEXT("SMARTEVENT/55001", "Region Entrance"));
    ui->checkBox_regionExiting->setText(GET_TEXT("SMARTEVENT/55002", "Region Exiting"));
    ui->checkBox_adcancedMotion->setText(GET_TEXT("SMARTEVENT/55003", "Advanced Motion Detection"));
    ui->checkBox_tamper->setText(GET_TEXT("SMARTEVENT/55004", "Tamper Detection"));
    ui->checkBox_lineCrossing->setText(GET_TEXT("SMARTEVENT/55005", "Line Crossing"));
    ui->checkBox_loitering->setText(GET_TEXT("SMARTEVENT/55006", "Loitering"));
    ui->checkBox_human->setText(GET_TEXT("SMARTEVENT/55007", "Human Detection"));
    ui->checkBox_objectLeftRemoved->setText(GET_TEXT("SMARTEVENT/55055", "Object Left/Removed"));
    ui->checkBox_alarmIn->setText(GET_TEXT("ALARMIN/52001", "Alarm Input"));
    ui->checkBox_anpr->setText(GET_TEXT("ANPR/103005", "ANPR"));
    ui->checkBox_black->setText(GET_TEXT("ANPR/103096", "Black List"));
    ui->checkBox_white->setText(GET_TEXT("ANPR/103097", "White List"));
    ui->checkBox_visitor->setText(GET_TEXT("ANPR/103098", "Visitor List"));
    ui->checkBoxFaceDetection->setText(GET_TEXT("FACE/141000", "Face Detection"));

    ui->pushButton_copy->setText(GET_TEXT("COMMON/1005", "Copy"));
    ui->pushButton_ok->setText(GET_TEXT("COMMON/1001", "OK"));
    ui->pushButton_cancel->setText(GET_TEXT("COMMON/1004", "Cancel"));

    ui->label_type->setText(GET_TEXT("SYSTEMNETWORK/71001", "Push Event Type"));
    ui->label_channel->setText(GET_TEXT("CHANNELMANAGE/30008", "Channel"));
    ui->label_alarm_input->setText(GET_TEXT("ALARMIN/52001", "Alarm Input"));

    ui->checkBoxAudioAlarm->setText(GET_TEXT("AUDIO_ALARM/159000", "Audio Alarm"));

    ui->widget_alarm_group->onLanguageChanged();

    ui->checkBoxPeopleCounting->setText(GET_TEXT("SMARTEVENT/55008", "People Counting"));
    ui->checkBoxRegionalPeopleCounting->setText(GET_TEXT("REGIONAL_PEOPLECOUNTING/103313", "Regional People Counting"));
}

void PushEventEdit::on_comboBox_channel_activated(int index)
{
    int event;
    if (m_channel != index) {
        event = pushType();
        IPCPushType.insert(m_channel, event);
    }
    m_channel = index;
    showPushType(IPCPushType.value(m_channel, 0));
}

void PushEventEdit::onCheckBoxTypeClicked(bool checked)
{
    Q_UNUSED(checked)

    if (ui->checkBox_alarmIn_1->isChecked() && ui->checkBox_alarmIn_2->isChecked() && ui->checkBoxAlarmIn3->isChecked() && ui->checkBoxAlarmIn4->isChecked()) {
        ui->checkBox_alarmIn->setCheckState(Qt::Checked);
    } else if (ui->checkBox_alarmIn_1->isChecked() || ui->checkBox_alarmIn_2->isChecked() || ui->checkBoxAlarmIn3->isChecked() || ui->checkBoxAlarmIn4->isChecked()) {
        ui->checkBox_alarmIn->setCheckState(Qt::PartiallyChecked);
    } else {
        ui->checkBox_alarmIn->setCheckState(Qt::Unchecked);
    }

    if ((ui->checkBox_black->isChecked() && ui->checkBox_white->isChecked() && ui->checkBox_visitor->isChecked())) {
        ui->checkBox_anpr->setCheckState(Qt::Checked);
    } else if ((ui->checkBox_black->isChecked() || ui->checkBox_white->isChecked() || ui->checkBox_visitor->isChecked())) {
        ui->checkBox_anpr->setCheckState(Qt::PartiallyChecked);
    } else {
        ui->checkBox_anpr->setCheckState(Qt::Unchecked);
    }

    int checkedCount = 0;
    QMap<int, QCheckBox *>::iterator iter = m_checkBoxMap.begin();
    for (; iter != m_checkBoxMap.end(); ++iter) {
        QCheckBox *checkBox = iter.value();
        if (checkBox->isChecked()) {
            checkedCount++;
        }
    }
    if (checkedCount == 0) {
        ui->checkBox_all->setCheckState(Qt::Unchecked);
    } else if (checkedCount == m_checkBoxMap.size()) {
        ui->checkBox_all->setCheckState(Qt::Checked);
    } else {
        ui->checkBox_all->setCheckState(Qt::PartiallyChecked);
    }
}

void PushEventEdit::on_checkBox_all_clicked(bool checked)
{
    QMap<int, QCheckBox *>::iterator iter = m_checkBoxMap.begin();
    for (; iter != m_checkBoxMap.end(); ++iter) {
        QCheckBox *checkBox = iter.value();
        checkBox->setChecked(checked);
    }
    auto status = checked ? Qt::Checked : Qt::Unchecked;
    ui->checkBox_alarmIn->setCheckState(status);
    ui->checkBox_anpr->setCheckState(status);
    ui->checkBox_all->setCheckState(status);
}

void PushEventEdit::on_checkBox_alarmIn_clicked(bool checked)
{
    ui->checkBox_alarmIn_1->setChecked(checked);
    ui->checkBox_alarmIn_2->setChecked(checked);
    ui->checkBoxAlarmIn3->setChecked(checked);
    ui->checkBoxAlarmIn4->setChecked(checked);
    onCheckBoxTypeClicked(true);
}

void PushEventEdit::on_checkBox_anpr_clicked(bool checked)
{
    ui->checkBox_black->setChecked(checked);
    ui->checkBox_white->setChecked(checked);
    ui->checkBox_visitor->setChecked(checked);
    onCheckBoxTypeClicked(true);
}

void PushEventEdit::on_pushButton_copy_clicked()
{
    int event = pushType();
    IPCPushType.insert(m_channel, event);
    ChannelCopyDialog copy(this);
    copy.setCurrentChannel(m_channel);
    int result = copy.exec();
    if (result == BaseShadowDialog::Accepted) {
        QList<int> channelList = copy.checkedList();
        for (int i = 0; i < channelList.size(); ++i) {
            IPCPushType.insert(channelList.at(i), event);
        }
    }
}

void PushEventEdit::on_pushButton_ok_clicked()
{
    NVRAlarmInType = 0;
    QList<int> NvrAlarmInList = ui->widget_alarm_group->checkedList();
    for (int i = 0; i < NvrAlarmInList.size(); ++i) {
        NVRAlarmInType |= (1 << NvrAlarmInList.at(i));
    }

    int event = pushType();
    IPCPushType.insert(m_channel, event);

    QString nvrEventCheckedString = ui->checkBoxGroupPos->checkedMask();
    snprintf(m_nvrEventPos->pos, sizeof(m_nvrEventPos->pos), "%s", nvrEventCheckedString.toStdString().c_str());

    accept();
}

void PushEventEdit::on_pushButton_cancel_clicked()
{
    reject();
}

void PushEventEdit::onTabBarClicked(int index)
{
    int event = pushType();
    IPCPushType.insert(m_channel, event);
    ui->stackedWidget->setCurrentIndex(index);
    ui->pushButton_copy->setVisible(!index);
    if (!index) {
        on_comboBox_channel_activated(m_channel);
    }
}

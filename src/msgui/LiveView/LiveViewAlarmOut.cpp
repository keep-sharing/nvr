#include "LiveViewAlarmOut.h"
#include "ui_LiveViewAlarmOut.h"
#include "centralmessage.h"
#include "MsCameraVersion.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MsWaitting.h"
#include "SubControl.h"
#include "MessageBox.h"
#include <QMouseEvent>
#include <QtDebug>

LiveViewAlarmOut::LiveViewAlarmOut(QWidget *parent) :
    BaseDialog (parent),
    ui(new Ui::LiveViewAlarmOut)
{
    ui->setupUi(this);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

LiveViewAlarmOut::~LiveViewAlarmOut()
{
    delete ui;
}

void LiveViewAlarmOut::initializeData(int channel)
{
    m_status = true;
    m_currentChannel = channel;
    sendMessage(REQUEST_FLAG_GET_IPC_ALARM_OUTPUT, &m_currentChannel, sizeof(int));
}

void LiveViewAlarmOut::moveAlarmOut(const QRect &videoGeometry)
{
    QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
    QPoint p(videoGeometry.left() - ui->widgetAlarmOut->width(), videoGeometry.bottom() - ui->widgetAlarmOut->height());
    if (p.x() < screenRect.left()) {
        p.setX(videoGeometry.right());
    }
    if (p.y() < screenRect.top()) {
        p.setY(screenRect.top());
    }
    if (p.x() >= screenRect.right()) {
        p.setX(videoGeometry.x());
    }
    if (p.y() >= screenRect.bottom()) {
        p.setY(videoGeometry.y());
    }
    ui->widgetAlarmOut->move(p);

    //
}

bool LiveViewAlarmOut::isSupport()
{
    return m_isSupport;
}

void LiveViewAlarmOut::processMessage(MessageReceive *message)
{
    switch (message->type()) {
    case RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT:
        ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(message);
        break;
    case RESPONSE_FLAG_SET_IPC_ALARMOUT_STATE:
        ON_RESPONSE_FLAG_SET_IPC_ALARMOUT_STATE(message);
        break;
    }
}

void LiveViewAlarmOut::ON_RESPONSE_FLAG_GET_IPC_ALARM_OUTPUT(MessageReceive *message)
{
    m_isSupport = true;
    memset(&m_ipc_alarm_out, 0, sizeof(ms_ipc_alarm_out));
    ms_ipc_alarm_out *ipc_alarm_out = (ms_ipc_alarm_out *)message->data;
    if (!ipc_alarm_out) {
        m_isSupport = false;
        showAlarm(m_currentChannel);
        return;
    }
    memcpy(&m_ipc_alarm_out, ipc_alarm_out, sizeof(ms_ipc_alarm_out));

    if (ipc_alarm_out->alarmCnt > 0) {
        ui->labelAlarm2->setVisible(ipc_alarm_out->alarmCnt > 1);
        ui->comboBoxAlarm2->setVisible(ipc_alarm_out->alarmCnt > 1);

        readAlarmoutName();

        int strlen1 = strlen(m_alarm_name_array[Alarmoutput1].name);
        int strlen2 = strlen(m_alarm_name_array[Alarmoutput2].name);
        if (strlen1 != 0) {
            ui->labelAlarm1->setText(changeText(QString(m_alarm_name_array[Alarmoutput1].name)));
            ui->labelAlarm1->setToolTip(QString(m_alarm_name_array[Alarmoutput1].name));
        } else {
            ui->labelAlarm1->setText(GET_TEXT("ALARMOUT/53012", "Alarm Output  %1").arg(1));
            ui->labelAlarm1->setToolTip(GET_TEXT("ALARMOUT/53012", "Alarm Output  %1").arg(1));
        }
        if (strlen2 != 0) {
            ui->labelAlarm2->setText(changeText(QString(m_alarm_name_array[Alarmoutput2].name)));
            ui->labelAlarm2->setToolTip(QString(m_alarm_name_array[Alarmoutput2].name));
        } else {
            ui->labelAlarm2->setText(GET_TEXT("ALARMOUT/53012", "Alarm Output  %1").arg(2));
            ui->labelAlarm2->setToolTip(GET_TEXT("ALARMOUT/53012", "Alarm Output  %1").arg(2));

        }


        ui->comboBoxAlarm1->setCurrentIndexFromData(ipc_alarm_out->alarmStatus[Alarmoutput1]);
        ui->comboBoxAlarm2->setCurrentIndexFromData(ipc_alarm_out->alarmStatus[Alarmoutput2]);

    } else {
        m_isSupport = false;
    }
    showAlarm(m_currentChannel);
}

void LiveViewAlarmOut::ON_RESPONSE_FLAG_SET_IPC_ALARMOUT_STATE(MessageReceive *message)
{
    Q_UNUSED(message);
}

void LiveViewAlarmOut::showEvent(QShowEvent *event)
{
    showMaximized();
    QWidget::showEvent(event);
}

void LiveViewAlarmOut::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
}

void LiveViewAlarmOut::mousePressEvent(QMouseEvent *event)
{
    if (!ui->widgetAlarmOut->geometry().contains(event->pos())) {
       on_pushButtonClose_clicked();
    }
    QWidget::mousePressEvent(event);
}

void LiveViewAlarmOut::onLanguageChanged()
{
    ui->comboBoxAlarm1->beginEdit();
    ui->comboBoxAlarm1->clear();
    ui->comboBoxAlarm1->addItem(GET_TEXT("COMMON/1012", "On"), 0);
    ui->comboBoxAlarm1->addItem(GET_TEXT("COMMON/1013", "Off"), 1);
    ui->comboBoxAlarm1->endEdit();

    ui->comboBoxAlarm2->beginEdit();
    ui->comboBoxAlarm2->clear();
    ui->comboBoxAlarm2->addItem(GET_TEXT("COMMON/1012", "On"), 0);
    ui->comboBoxAlarm2->addItem(GET_TEXT("COMMON/1013", "Off"), 1);
    ui->comboBoxAlarm2->endEdit();
    //
    ui->pushButtonClose->setText(GET_TEXT("PTZDIALOG/21005", "Close"));
    ui->label_title->setText(GET_TEXT("ALARMOUT/53006", "Camera Alarm Output"));
}

void LiveViewAlarmOut::showAlarm(int channel)
{
    if (m_currentChannel == channel && m_status) {
        m_status = false;
        if (isSupport()) {
            this->show();
            ui->pushButtonClose->clearFocus();
        } else {
            ShowMessageBox(this, GET_TEXT("PTZCONFIG/36013", "This channel does not support this function."));
        }
    }
}

void LiveViewAlarmOut::saveAlarmOut(AlarmIndex index)
{
    ms_ipc_alarm_out_state ipc_alarm_out;
    ipc_alarm_out.alarmid = m_alarm_name_array[index].alarmid;
    ipc_alarm_out.chanid = m_alarm_name_array[index].chnid;
    ipc_alarm_out.state = m_ipc_alarm_out.alarmStatus[index];

    if (ipc_alarm_out.state == 1) {
        ipc_alarm_out.state = 0;
    } else {
        ipc_alarm_out.state = 1;
    }
    sendMessage(REQUEST_FLAG_SET_IPC_ALARMOUT_STATE, &ipc_alarm_out, sizeof(ms_ipc_alarm_out_state));
}

QString LiveViewAlarmOut::changeText(QString text)
{
    int wWidth = QFontMetrics(this->font()).width("WWWWWWWWWW");
    int pointWidth = QFontMetrics(this->font()).width("...");
    int strlen = text.size();
    bool isShorten = false;
    int i = 0;
    for (i = 1; i < strlen; i++) {
        int tempWidth =  QFontMetrics(this->font()).width(text.left(i));
        if (tempWidth > wWidth) {
            isShorten = true;
            while(tempWidth + pointWidth > wWidth) {
                i--;
                tempWidth =  QFontMetrics(this->font()).width(text.left(i));
            }
            break;
        }
    }
    QString temp = text.left(i);
    if (isShorten) {
        temp = temp + "...";
    }
    return temp;
}

void LiveViewAlarmOut::readAlarmoutName()
{
    for (int i = 0; i < m_ipc_alarm_out.alarmCnt; ++i) {
        alarm_chn_out_name &alarm_name = m_alarm_name_array[i];
        memset(&alarm_name, 0, sizeof(alarm_chn_out_name));

        alarm_chn alarm_channel;
        alarm_channel.chnid = m_currentChannel;
        alarm_channel.alarmid = i;
        read_alarm_chnOut_event_name(SQLITE_FILE_NAME, &alarm_name, &alarm_channel);
    }
}

void LiveViewAlarmOut::on_pushButtonClose_clicked()
{
    close();
}

void LiveViewAlarmOut::on_comboBoxAlarm1_currentIndexChanged(int index)
{
    m_ipc_alarm_out.alarmStatus[Alarmoutput1] = index;
    saveAlarmOut(Alarmoutput1);
}

void LiveViewAlarmOut::on_comboBoxAlarm2_currentIndexChanged(int index)
{
    m_ipc_alarm_out.alarmStatus[Alarmoutput2] = index;
    saveAlarmOut(Alarmoutput2);
}

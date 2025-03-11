#include "ActionPageHTTPNotification.h"
#include "ui_ActionPageHTTPNotification.h"
#include "ActionAbstract.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "MyButtonGroup.h"
#include "channelcheckbox.h"
#include <QByteArray>

ActionPageHTTPNotification::ActionPageHTTPNotification(QWidget *parent)
    : ActionPageAbstract(parent)
    , ui(new Ui::ActionPageHTTPNotification)
{
    ui->setupUi(this);
    m_group = new MyButtonGroup(this);
    m_group->addButton(ui->pushButtonHTTP, m_action->scheduleType());
    m_group->addButton(ui->pushButtonErase, NONE);
    connect(m_group, SIGNAL(buttonClicked(int)), this, SLOT(onScheduleTypeClicked(int)));

    ui->scheduleWidget->setSingleEditType(m_action->scheduleType());
    if (m_action->eventType() == PEOPLE_COUNT) {
        ui->scheduleWidget->setTypeColor(m_action->scheduleType(), QColor("#FFE793"));
    }

    ui->comboBoxTriggerInterval->clear();
    ui->comboBoxTriggerInterval->addItem("10s", 10);
    ui->comboBoxTriggerInterval->addItem("20s", 20);
    ui->comboBoxTriggerInterval->addItem("40s", 40);
    ui->comboBoxTriggerInterval->addItem("60s", 60);
    ui->comboBoxTriggerInterval->addItem("100s", 100);
    ui->comboBoxTriggerInterval->addItem("5min", 300);
    ui->comboBoxTriggerInterval->addItem("15min", 900);
    ui->comboBoxTriggerInterval->addItem("30min", 1800);
    ui->comboBoxTriggerInterval->addItem("1h", 3600);
    ui->comboBoxTriggerInterval->addItem("8h", 28800);
    ui->comboBoxTriggerInterval->addItem("12h", 43200);
    ui->comboBoxTriggerInterval->addItem("24h", 86400);
}

ActionPageHTTPNotification::~ActionPageHTTPNotification()
{
    delete ui;
}

void ActionPageHTTPNotification::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

int ActionPageHTTPNotification::loadData()
{
    if (hasCache()) {
    } else {
        m_group->setCurrentButton(ui->pushButtonHTTP);
        ui->scheduleWidget->setSchedule(m_action->httpNotificationSchedule());
        ui->comboBoxTriggerInterval->setCurrentIndexFromData(*m_action->httpNotificationTriggerInterval());
        ui->textEditURL->setText(m_action->httpNotificationParams()->url);
        ui->lineEditUserName->setText(m_action->httpNotificationParams()->username);
        ui->lineEditPassword->setText(m_action->httpNotificationParams()->password);
        ui->labelHTTP->setColor(m_action->actionColor());

        //
        setCached();
    }
    return 0;
}

int ActionPageHTTPNotification::saveData()
{
    ui->scheduleWidget->getSchedule(m_action->httpNotificationSchedule());
    //
    int *triggerInterval = m_action->httpNotificationTriggerInterval();
    *triggerInterval = ui->comboBoxTriggerInterval->currentData().toInt();
    //
    HttpNotificationParams *httpNotificationParams = m_action->httpNotificationParams();
    strcpy(httpNotificationParams->url, ui->textEditURL->toPlainText().toStdString().c_str());
    strcpy(httpNotificationParams->username, ui->lineEditUserName->text().toStdString().c_str());
    strcpy(httpNotificationParams->password, ui->lineEditPassword->text().toStdString().c_str());
    return 0;
}

void ActionPageHTTPNotification::onLanguageChanged()
{
    ui->pushButtonHTTP->setText(GET_TEXT("ACTION/153000", "HTTP Notification"));
    ui->pushButtonErase->setText(GET_TEXT("RECORDMODE/90012", "Erase"));
    ui->labelTriggerInterval->setText(GET_TEXT("VIDEOLOSS/50006", "Triggered Interval"));
    ui->groupBoxHTTP->setTitle(GET_TEXT("ACTION/153000", "HTTP Notification"));
    ui->labelURL->setText(GET_TEXT("ACTION/153001", "URL"));
    ui->labelUserName->setText(GET_TEXT("COMMON/1007", "User Name"));
    ui->labelPassword->setText(GET_TEXT("COMMON/1008", "Password"));
}

void ActionPageHTTPNotification::onScheduleTypeClicked(int id)
{
    ui->scheduleWidget->setCurrentType(id);
}

void ActionPageHTTPNotification::on_textEditURL_textChanged()
{
    QString textContent = ui->textEditURL->toPlainText();
    int length = textContent.count();
    int maxLength = MAX_LEN_512;

    if (length > maxLength) {
        int position = ui->textEditURL->textCursor().position();
        QTextCursor textCursor = ui->textEditURL->textCursor();
        textContent.remove(position - (length - maxLength), length - maxLength);
        ui->textEditURL->setText(textContent);
        textCursor.setPosition(position - (length - maxLength));
        ui->textEditURL->setTextCursor(textCursor);
    }
}

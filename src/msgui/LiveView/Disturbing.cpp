#include "Disturbing.h"
#include "ui_Disturbing.h"
#include "centralmessage.h"
#include "LiveviewBottomBar.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include <QtDebug>

Disturbing::Disturbing(QWidget *parent)
    : BasePopup(parent)
    , ui(new Ui::Disturbing)
{
    ui->setupUi(this);

    ui->comboBox_audible->clear();
    ui->comboBox_audible->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_audible->addTranslatableItem("COMMON/1012", "On", State::On);
    ui->comboBox_email->clear();
    ui->comboBox_email->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_email->addTranslatableItem("COMMON/1012", "On", State::On);
    ui->comboBox_event->clear();
    ui->comboBox_event->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_event->addTranslatableItem("COMMON/1012", "On", State::On);
    ui->comboBox_ptz->clear();
    ui->comboBox_ptz->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_ptz->addTranslatableItem("COMMON/1012", "On", State::On);
    ui->comboBox_alarm->clear();
    ui->comboBox_alarm->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_alarm->addTranslatableItem("COMMON/1012", "On", State::On);
    ui->comboBox_white->clear();
    ui->comboBox_white->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBox_white->addTranslatableItem("COMMON/1012", "On", State::On);
    ui->comboBoxHTTP->clear();
    ui->comboBoxHTTP->addTranslatableItem("COMMON/1013", "Off", State::Off);
    ui->comboBoxHTTP->addTranslatableItem("COMMON/1012", "On", State::On);

    QTimer::singleShot(0, this, SLOT(updateState()));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

Disturbing::~Disturbing()
{
    delete ui;
}

void Disturbing::setPos(const QPoint &p)
{
    m_pos = p;
}

QPoint Disturbing::calculatePos()
{
    return m_pos;
}

void Disturbing::closePopup(BasePopup::CloseType type)
{
    Q_UNUSED(type)
}

void Disturbing::showEvent(QShowEvent *event)
{
    updateState();

    BasePopup::showEvent(event);
}

void Disturbing::saveSetting()
{
    quint32 state = 0x7FFFFFFF;
    if (ui->comboBox_audible->currentData().toInt() == State::Off) {
        state &= ~(0x01 << AUDIBLE_WARNING);
    }
    if (ui->comboBox_email->currentData().toInt() == State::Off) {
        state &= ~(0x01 << EMAIL_LINKAGE);
    }
    if (ui->comboBox_event->currentData().toInt() == State::Off) {
        state &= ~(0x01 << EVENT_POPUP);
    }
    if (ui->comboBox_ptz->currentData().toInt() == State::Off) {
        state &= ~(0x01 << PTZ_ACTION);
    }
    if (ui->comboBox_alarm->currentData().toInt() == State::Off) {
        state &= ~(0x01 << ALARM_OUTPUT);
    }
    if (ui->comboBox_white->currentData().toInt() == State::Off) {
        state &= ~(0x01 << WHITE_LED);
    }
    if (ui->comboBoxHTTP->currentData().toInt() == State::Off) {
        state &= ~(0x01 << HTTP_NOTIFICATION);
    }
    qDebug() << QString("set PARAM_ACTION_ENABLE: %1").arg(state, 0, 2);
    set_param_int(SQLITE_FILE_NAME, PARAM_ACTION_ENABLE, state);
    sendMessageOnly(REQUEST_FLAG_UPDATE_ACTION_ENABLE, &state, sizeof(state));

    LiveviewBottomBar::instance()->updateDisturbState(state != 0x7FFFFFFF);
}

void Disturbing::onLanguageChanged()
{
    ui->label_audible->setText(GET_TEXT("VIDEOLOSS/50009", "Audible Warning"));
    ui->label_email->setText(GET_TEXT("VIDEOLOSS/50010", "Email Linkage"));
    ui->label_event->setText(GET_TEXT("SYSTEMGENERAL/70041", "Event Popup"));
    ui->label_ptz->setText(GET_TEXT("VIDEOLOSS/50014", "PTZ Action"));
    ui->label_alarm->setText(GET_TEXT("VIDEOLOSS/50019", "Alarm Output"));
    ui->label_white->setText(GET_TEXT("WHITELED/105000", "White LED"));
    ui->labelHTTP->setText(GET_TEXT("ACTION/153000", "HTTP Notification"));

    ui->comboBox_audible->retranslate();
    ui->comboBox_email->retranslate();
    ui->comboBox_event->retranslate();
    ui->comboBox_ptz->retranslate();
    ui->comboBox_alarm->retranslate();
    ui->comboBox_white->retranslate();
    ui->comboBoxHTTP->retranslate();
}

void Disturbing::updateState()
{
    quint32 state = get_param_int(SQLITE_FILE_NAME, PARAM_ACTION_ENABLE, 0);
    qDebug() << QString("get PARAM_ACTION_ENABLE: %1").arg(state, 0, 2);
    ui->comboBox_audible->setCurrentIndexFromData((state >> AUDIBLE_WARNING) & 0x01);
    ui->comboBox_email->setCurrentIndexFromData((state >> EMAIL_LINKAGE) & 0x01);
    ui->comboBox_event->setCurrentIndexFromData((state >> EVENT_POPUP) & 0x01);
    ui->comboBox_ptz->setCurrentIndexFromData((state >> PTZ_ACTION) & 0x01);
    ui->comboBox_alarm->setCurrentIndexFromData((state >> ALARM_OUTPUT) & 0x01);
    ui->comboBox_white->setCurrentIndexFromData((state >> WHITE_LED) & 0x01);
    ui->comboBoxHTTP->setCurrentIndexFromData((state >> HTTP_NOTIFICATION) & 0x01);

    LiveviewBottomBar::instance()->updateDisturbState(state != 0x7FFFFFFF);
}

void Disturbing::on_comboBox_audible_activated(int index)
{
    Q_UNUSED(index)

    saveSetting();
}

void Disturbing::on_comboBox_email_activated(int index)
{
    Q_UNUSED(index)

    saveSetting();
}

void Disturbing::on_comboBox_event_activated(int index)
{
    Q_UNUSED(index)

    saveSetting();
}

void Disturbing::on_comboBox_ptz_activated(int index)
{
    Q_UNUSED(index)

    saveSetting();
}

void Disturbing::on_comboBox_alarm_activated(int index)
{
    Q_UNUSED(index)

    saveSetting();
}

void Disturbing::on_comboBox_white_activated(int index)
{
    Q_UNUSED(index)

    saveSetting();
}

void Disturbing::on_comboBoxHTTP_activated(int index)
{
    Q_UNUSED(index)

    saveSetting();
}

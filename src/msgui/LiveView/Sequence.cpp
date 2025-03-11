#include "Sequence.h"
#include "ui_Sequence.h"
#include "LiveView.h"
#include "LiveViewTargetPlay.h"
#include "LiveViewZoom.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "SubControl.h"
#include "ptz3dcontrol.h"
#include "MyDebug.h"

Sequence *Sequence::s_sequence = nullptr;

Sequence::Sequence(QWidget *parent)
    : BasePopup(parent)
    , ui(new Ui::Sequence)
{
    ui->setupUi(this);

    s_sequence = this;

    ui->comboBox_interval->addItem(QString("No Sequence"), 0);
#if 0
    ui->comboBox_interval->addItem(QString("%1s").arg(1), 1);
    ui->comboBox_interval->addItem(QString("%1s").arg(2), 2);
    ui->comboBox_interval->addItem(QString("%1s").arg(3), 3);
    ui->comboBox_interval->addItem(QString("%1s").arg(4), 4);
#endif
    ui->comboBox_interval->addItem(QString("%1s").arg(5), 5);
    ui->comboBox_interval->addItem(QString("%1s").arg(10), 10);
    ui->comboBox_interval->addItem(QString("%1s").arg(20), 20);
    ui->comboBox_interval->addItem(QString("%1s").arg(30), 30);
    ui->comboBox_interval->addItem(QString("%1s").arg(60), 60);
    ui->comboBox_interval->addItem(QString("%1s").arg(120), 120);
    ui->comboBox_interval->addItem(QString("%1s").arg(300), 300);

    m_timerMain = new QTimer(this);
    connect(m_timerMain, SIGNAL(timeout()), this, SLOT(onTimeoutMain()));

    m_timerSub = new QTimer(this);
    connect(m_timerSub, SIGNAL(timeout()), this, SLOT(onTimeoutSub()));

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

Sequence::~Sequence()
{
    s_sequence = nullptr;
    delete ui;
}

Sequence *Sequence::instance()
{
    return s_sequence;
}

void Sequence::initializeData()
{
    struct display display_info = qMsNvr->displayInfo();

    if (display_info.main_seq_interval <= 0) {
        m_timerMain->stop();
        emit sequenceStateChanged(SCREEN_MAIN, false);
    } else {
        m_timerMain->setInterval(1000 * display_info.main_seq_interval);
        m_timerMain->start();
        emit sequenceStateChanged(SCREEN_MAIN, true);
    }
    if (display_info.sub_seq_interval <= 0) {
        m_timerSub->stop();
        emit sequenceStateChanged(SCREEN_SUB, false);
    } else {
        m_timerSub->setInterval(1000 * display_info.sub_seq_interval);
        m_timerSub->start();
        emit sequenceStateChanged(SCREEN_SUB, true);
    }

    const SCREEN_E screen = SubControl::instance()->currentScreen();
    switch (screen) {
    case SCREEN_MAIN:
        ui->comboBox_interval->setCurrentIndexFromData(display_info.main_seq_interval);
        break;
    case SCREEN_SUB:
        ui->comboBox_interval->setCurrentIndexFromData(display_info.sub_seq_interval);
        break;
    default:
        break;
    }
}

void Sequence::updateDisplayInfo()
{
    struct display display_info = qMsNvr->displayInfo();
    const SCREEN_E screen = SubControl::instance()->currentScreen();
    switch (screen) {
    case SCREEN_MAIN:
        ui->comboBox_interval->setCurrentIndexFromData(display_info.main_seq_interval);
        break;
    case SCREEN_SUB:
        ui->comboBox_interval->setCurrentIndexFromData(display_info.sub_seq_interval);
        break;
    default:
        break;
    }
}

void Sequence::stopMainSequence()
{
    m_timerMain->stop();
    emit sequenceStateChanged(SCREEN_MAIN, false);
}

void Sequence::stopSubSequence()
{
    m_timerSub->stop();
    emit sequenceStateChanged(SCREEN_SUB, false);
}

bool Sequence::isSequencing(int screen)
{
    bool isOn = false;
    switch (screen) {
    case SCREEN_MAIN:
        isOn = m_timerMain->isActive();
        break;
    case SCREEN_SUB:
        isOn = m_timerSub->isActive();
        break;
    default:
        break;
    }
    return isOn;
}

void Sequence::networkSequence()
{
    qDebug() << "Sequence::networkSequence";
    struct display display_info = qMsNvr->displayInfo();
    const SCREEN_E screen = SubControl::instance()->currentScreen();
    switch (screen) {
    case SCREEN_MAIN:
        if (display_info.main_seq_interval <= 0) {
            display_info.main_seq_interval = 5;
            m_timerMain->setInterval(1000 * 5);
            m_timerMain->start();
            emit sequenceStateChanged(SCREEN_MAIN, true);
        } else {
            display_info.main_seq_interval = 0;
            m_timerMain->stop();
            emit sequenceStateChanged(SCREEN_MAIN, false);
        }
        break;
    case SCREEN_SUB:
        if (display_info.sub_seq_interval <= 0) {
            display_info.sub_seq_interval = 5;
            m_timerSub->setInterval(1000 * 5);
            m_timerSub->start();
            emit sequenceStateChanged(SCREEN_SUB, true);
        } else {
            display_info.sub_seq_interval = 0;
            m_timerSub->stop();
            emit sequenceStateChanged(SCREEN_SUB, false);
        }
        break;
    default:
        break;
    }
    qMsNvr->writeDisplayInfo(&display_info);
}

void Sequence::setPos(const QPoint &p)
{
    m_pos = p;
}

QPoint Sequence::calculatePos()
{
    return m_pos;
}

void Sequence::closePopup(BasePopup::CloseType type)
{
    Q_UNUSED(type)
}

void Sequence::mousePressEvent(QMouseEvent *event)
{
    BasePopup::mousePressEvent(event);
}

void Sequence::showEvent(QShowEvent *event)
{
    BasePopup::showEvent(event);
}

void Sequence::hideEvent(QHideEvent *event)
{
    BasePopup::hideEvent(event);
}

void Sequence::onLanguageChanged()
{
    ui->label->setText(GET_TEXT("LIVEPARAMETER/41005", "Sequence Interval"));
    ui->comboBox_interval->setItemText(0, GET_TEXT("LIVEPARAMETER/41008", "No Sequence"));
}

void Sequence::onTimeoutMain()
{
    //及时回放时暂停轮巡
    if (LiveViewPlayback::instance()) {
        if (LiveViewPlayback::instance()->isVisible()) {
            return;
        }
    }
    //
    if (LiveViewTargetPlay::instance()) {
        if (LiveViewTargetPlay::instance()->isVisible()) {
            return;
        }
    }
    //
    if (PTZ3DControl::instance() && PTZ3DControl::instance()->isVisible()) {
        return;
    }
    if (LiveViewZoom::instance() && LiveViewZoom::instance()->isVisible()) {
        return;
    }
    if (LiveView::instance()->isFisheyeMode()) {
        return;
    }
    if (LiveView::instance()->isManualChangeStream()) {
        return;
    }
    //
    qMsDebug() << "nextMainPage";
    LiveView::instance()->nextMainPage();
}

void Sequence::onTimeoutSub()
{
    //
    if (!SubControl::instance()->isSubEnable()) {
        return;
    }
    //及时回放时暂停轮巡
    if (LiveViewPlayback::instance()) {
        if (LiveViewPlayback::instance()->isVisible()) {
            return;
        }
    }
    //
    if (LiveViewTargetPlay::instance()) {
        if (LiveViewTargetPlay::instance()->isVisible()) {
            return;
        }
    }
    //
    if (PTZ3DControl::instance() && PTZ3DControl::instance()->isVisible()) {
        return;
    }
    if (LiveViewZoom::instance() && LiveViewZoom::instance()->isVisible()) {
        return;
    }
    if (LiveView::instance()->isFisheyeMode()) {
        return;
    }
    if (LiveView::instance()->isManualChangeStream()) {
        return;
    }
    //
    qMsDebug() << "nextSubPage";
    LiveView::instance()->nextSubPage();
}

void Sequence::on_comboBox_interval_activated(int index)
{
    int seconds = ui->comboBox_interval->itemData(index).toInt();

    struct display display_info = qMsNvr->displayInfo();
    const SCREEN_E screen = SubControl::instance()->currentScreen();
    switch (screen) {
    case SCREEN_MAIN:
        display_info.main_seq_interval = seconds;
        if (seconds <= 0) {
            m_timerMain->stop();
            emit sequenceStateChanged(SCREEN_MAIN, false);
        } else {
            m_timerMain->setInterval(1000 * seconds);
            m_timerMain->start();
            emit sequenceStateChanged(SCREEN_MAIN, true);
        }
        break;
    case SCREEN_SUB:
        display_info.sub_seq_interval = seconds;
        if (seconds <= 0) {
            m_timerSub->stop();
            emit sequenceStateChanged(SCREEN_SUB, false);
        } else {
            m_timerSub->setInterval(1000 * seconds);
            m_timerSub->start();
            emit sequenceStateChanged(SCREEN_SUB, true);
        }
        break;
    default:
        break;
    }
    qMsNvr->writeDisplayInfo(&display_info);
}

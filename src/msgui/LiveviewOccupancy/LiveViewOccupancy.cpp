#include "LiveViewOccupancy.h"
#include "ui_LiveViewOccupancy.h"
#include "LiveViewOccupancyManager.h"
#include "LiveViewOccupancySetting.h"
#include "MessageBox.h"
#include "MsDevice.h"
#include "MsGlobal.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include "PeopleCountingData.h"
#include "centralmessage.h"
#include <QMouseEvent>

LiveViewOccupancy::LiveViewOccupancy(QWidget *parent)
    : MsWidget(parent)
    , ui(new Ui::LiveViewOccupancy)
{
    ui->setupUi(this);

    m_mapWidgetRect.insert(ui->rcWidgetLogo, ui->rcWidgetLogo->geometry());
    m_mapWidgetRect.insert(ui->rcWidgetGroup, ui->rcWidgetGroup->geometry());
    m_mapWidgetRect.insert(ui->rcWidgetSetting, ui->rcWidgetSetting->geometry());
    m_mapWidgetRect.insert(ui->rcWidgetIcon, ui->rcWidgetIcon->geometry());
    m_mapWidgetRect.insert(ui->labelMessage, ui->labelMessage->geometry());
    m_mapWidgetRect.insert(ui->rcWidgetRefresh, ui->rcWidgetRefresh->geometry());
    m_mapWidgetRect.insert(ui->rcWidgetQuit, ui->rcWidgetQuit->geometry());

    if (qMsNvr->isOEM()) {
        ui->rcWidgetLogo->hide();
    }

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();
}

LiveViewOccupancy::~LiveViewOccupancy()
{
    delete ui;
}

void LiveViewOccupancy::show()
{
    QWidget::show();
}

void LiveViewOccupancy::close()
{
    QWidget::close();
}

void LiveViewOccupancy::setGroup(int group)
{
    m_group = group;

    ui->labelGroup->setElidedText(groupName(m_group));
    ui->labelGroup->setToolTip(groupName(m_group));

    clearState(m_group);

    //
    updateDisplaySetting();
    updateData();
}

int LiveViewOccupancy::group() const
{
    return m_group;
}

void LiveViewOccupancy::updateData()
{
    do {
        if (m_group < 0) {
            break;
        }
        people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
        const PEOPLECNT_SETTING &setting = people_info->sets[m_group];
        const PEOPLECNT_CURRENT &current = people_info->cur[m_group];
        if (!setting.enable) {
            break;
        }
        if (!PeopleCountingData::hasChannel(setting.tri_channels)) {
            break;
        }
        //int
        int in = current.pcntin;
        ui->labelInValue->setText(QString("%1").arg(in));
        //out
        int out = current.pcntout;
        ui->labelOutValue->setText(QString("%1").arg(out));
        //stays
        int stays = in - out;
        if (stays < 0) {
            stays = 0;
        }
        ui->labelStayValue->setText(QString("%1").arg(stays));
        //available
        int available = setting.stays - stays;
        if (available < 0) {
            available = 0;
        }
        ui->labelAvailableValue->setText(QString("%1").arg(available));
        //icon
        int iconState = 0;
        if (m_group != -1) {
            if (available > 0) {
                iconState = 1;
            } else {
                iconState = 2;
            }
        }
        if (iconState != m_states.value(m_group).iconState) {
            m_states[m_group].iconState = iconState;
            switch (iconState) {
            case 0: //灰色
                ui->labelGreen->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/gray2.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->labelRed->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/gray_1.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->labelMessage->clear();
                break;
            case 1: //绿灯
                ui->labelGreen->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/green_1.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->labelRed->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/gray_1.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->labelMessage->setText(setting.liveview_green_tips);
                break;
            case 2: //红灯
                ui->labelGreen->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/gray2.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->labelRed->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/red_1.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                ui->labelMessage->setText(setting.liveview_red_tips);
                break;
            }
        }
        //message text
        switch (m_states.value(m_group).iconState) {
        case 0:
            ui->labelMessage->clear();
            break;
        case 1:
            ui->labelMessage->setText(setting.liveview_green_tips);
            break;
        case 2:
            ui->labelMessage->setText(setting.liveview_red_tips);
            break;
        }
        //message style
        if (setting.liveview_font_size != m_states.value(m_group).fontSize) {
            m_states[m_group].fontSize = setting.liveview_font_size;
            switch (setting.liveview_font_size) {
            case 0:
                ui->labelMessage->setStyleSheet("color: #FFFFFF; font-size: 25pt");
                break;
            case 1:
                ui->labelMessage->setStyleSheet("color: #FFFFFF; font-size: 35pt");
                break;
            case 2:
                ui->labelMessage->setStyleSheet("color: #FFFFFF; font-size: 45pt");
                break;
            }
        }
        //
        ui->toolButtonReset->setEnabled(true);
        return;
    } while (0);

    //
    ui->labelInValue->setText(QString("%1").arg(0));
    ui->labelOutValue->setText(QString("%1").arg(0));
    ui->labelStayValue->setText(QString("%1").arg(0));
    ui->labelAvailableValue->setText(QString("%1").arg(0));
    ui->labelGreen->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/gray2.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelRed->setPixmap(QPixmap(":/liveview_occupancy/liveview_occupancy/gray_1.png").scaled(220, 220, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->labelMessage->clear();
    ui->toolButtonReset->setEnabled(false);
}

void LiveViewOccupancy::resizeEvent(QResizeEvent *)
{
    resizeAllWidget();
}

void LiveViewOccupancy::showEvent(QShowEvent *)
{
    resizeAllWidget();
}

void LiveViewOccupancy::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        m_isPressed = true;
        m_pressedPoint = event->pos();
    }
}

void LiveViewOccupancy::mouseReleaseEvent(QMouseEvent *event)
{

    if (event->button() == Qt::RightButton) {
        m_isPressed = false;
        QPoint p = event->pos() - m_pressedPoint;
        if (p.manhattanLength() < 5) {
            LiveViewOccupancyManager::instance()->closeOccupancy(LiveViewOccupancyManager::NormalReason);
        }
    }
}

void LiveViewOccupancy::resizeAllWidget()
{
    qreal rw = 1920.0 / width();
    qreal rh = 1080.0 / height();
    for (auto iter = m_mapWidgetRect.constBegin(); iter != m_mapWidgetRect.constEnd(); ++iter) {
        QWidget *widget = iter.key();
        QRect rc = iter.value();
        QRect rcNew(rc.left() / rw, rc.top() / rw, rc.width() / rw, rc.height() / rh);
        widget->setGeometry(rcNew);
    }
}

void LiveViewOccupancy::updateDisplaySetting()
{
    if (m_group < 0) {
        return;
    }

    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    const PEOPLECNT_SETTING &setting = people_info->sets[m_group];
    int display = setting.liveview_display;

    int count = 0;
    if (display & PEOPLECNT_DISPLAY_AVALIABLE) {
        ui->widgetAvailable->show();
        count++;
    } else {
        ui->widgetAvailable->hide();
    }
    if (display & PEOPLECNT_DISPLAY_STAY) {
        ui->widgetStay->show();
        count++;
    } else {
        ui->widgetStay->hide();
    }
    if (display & PEOPLECNT_DISPLAY_IN) {
        ui->widgetIn->show();
        count++;
    } else {
        ui->widgetIn->hide();
    }
    if (display & PEOPLECNT_DISPLAY_OUT) {
        ui->widgetOut->show();
        count++;
    } else {
        ui->widgetOut->hide();
    }
    if (count > 2) {
        ui->labelIconIn->show();
        ui->labelIconOut->show();
        if (display & PEOPLECNT_DISPLAY_AVALIABLE) {
            ui->labelIconStay->show();
            ui->horizontalLayoutStay->setContentsMargins(100, 9, 9, 9);
            ui->horizontalLayoutIn->setContentsMargins(100, 9, 9, 9);
            ui->horizontalLayoutOut->setContentsMargins(100, 9, 9, 9);
            ui->widgetAvailable->setStyleSheet("QLabel {color: #FFFFFF; font-size: 75pt; font-weight: bold;}");
            ui->widgetStay->setStyleSheet("QLabel {color: #FFFFFF; font-size: 48pt;}");
            ui->widgetIn->setStyleSheet("QLabel {color: #FFFFFF; font-size: 48pt;}");
            ui->widgetOut->setStyleSheet("QLabel {color: #FFFFFF; font-size: 48pt;}");
        } else {
            ui->labelIconStay->hide();
            ui->horizontalLayoutStay->setContentsMargins(9, 9, 9, 9);
            ui->horizontalLayoutIn->setContentsMargins(100, 9, 9, 9);
            ui->horizontalLayoutOut->setContentsMargins(100, 9, 9, 9);
            ui->widgetStay->setStyleSheet("QLabel {color: #FFFFFF; font-size: 75pt; font-weight: bold;}");
            ui->widgetIn->setStyleSheet("QLabel {color: #FFFFFF; font-size: 48pt;}");
            ui->widgetOut->setStyleSheet("QLabel {color: #FFFFFF; font-size: 48pt;}");
        }
    } else {
        ui->labelIconStay->hide();
        ui->labelIconIn->hide();
        ui->labelIconOut->hide();
        ui->horizontalLayoutStay->setContentsMargins(9, 9, 9, 9);
        ui->horizontalLayoutIn->setContentsMargins(9, 9, 9, 9);
        ui->horizontalLayoutOut->setContentsMargins(9, 9, 9, 9);
        ui->widgetAvailable->setStyleSheet("QLabel {color: #FFFFFF; font-size: 75pt; font-weight: bold;}");
        ui->widgetStay->setStyleSheet("QLabel {color: #FFFFFF; font-size: 75pt; font-weight: bold;}");
        ui->widgetIn->setStyleSheet("QLabel {color: #FFFFFF; font-size: 75pt; font-weight: bold;}");
        ui->widgetOut->setStyleSheet("QLabel {color: #FFFFFF; font-size: 75pt; font-weight: bold;}");
    }
}

QString LiveViewOccupancy::groupName(int group)
{
    if (group < 0) {
        return QString();
    }
    people_cnt_info *people_info = gPeopleCountingData.peopleInfo();
    const peoplecnt_setting &setting = people_info->sets[group];
    QString groupName(setting.name);
    if (groupName.isEmpty()) {
        groupName = QString("Group%1").arg(group + 1);
    }

    return groupName;
}

void LiveViewOccupancy::clearState(int group)
{
    if (group < 0) {
        return;
    }
    m_states[group].clear();
}

void LiveViewOccupancy::onLanguageChanged()
{
    ui->labelAvailable->setText(GET_TEXT("OCCUPANCY/74248", "Available:"));
    ui->labelStay->setText(GET_TEXT("OCCUPANCY/74247", "Stay:"));
    ui->labelIn->setText(GET_TEXT("OCCUPANCY/74245", "In:"));
    ui->labelOut->setText(GET_TEXT("OCCUPANCY/74246", "Out:"));
    ui->toolButtonSetting->setToolTip(GET_TEXT("OCCUPANCY/74220", "Settings"));
    ui->toolButtonReset->setToolTip(GET_TEXT("OCCUPANCY/74221", "Reset"));
    ui->toolButtonQuit->setToolTip(GET_TEXT("OCCUPANCY/74222", "Exit"));
}

void LiveViewOccupancy::on_toolButtonReset_clicked()
{
    ui->toolButtonReset->clearUnderMouse();
    ui->toolButtonReset->update();
    int result = ExecQuestionBox(GET_TEXT("OCCUPANCY/74235", "Do you really want to reset the group counting data in live view?"));
    if (result == MessageBox::Yes) {
        sendMessage(REQUEST_FLAG_SET_PEOPLECNT_LIVEVIEW_RESET, &m_group, sizeof(m_group));
    }
}

void LiveViewOccupancy::on_toolButtonSetting_clicked()
{
    ui->toolButtonSetting->clearUnderMouse();
    ui->toolButtonSetting->update();
    if (!m_setting) {
        m_setting = new LiveViewOccupancySetting(this);
    }
    m_setting->show();
    int result = m_setting->exec();
    if (result == LiveViewOccupancySetting::Accepted) {
        setGroup(m_setting->currentGroup());
    }
}

void LiveViewOccupancy::on_toolButtonQuit_clicked()
{
    ui->toolButtonQuit->clearUnderMouse();
    ui->toolButtonQuit->update();
    //
    LiveViewOccupancyManager::instance()->closeOccupancy(LiveViewOccupancyManager::NormalReason);
}

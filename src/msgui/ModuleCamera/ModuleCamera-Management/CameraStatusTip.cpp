#include "CameraStatusTip.h"
#include "ui_CameraStatusTip.h"
#include "MyDebug.h"
#include "MsLanguage.h"
#include "settingcontent.h"
#include <QPainter>

extern "C" {
#include "recortsp.h"
}

CameraStatusTip *CameraStatusTip::self = nullptr;

CameraStatusTip::CameraStatusTip(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CameraStatusTip)
{
    ui->setupUi(this);
    setWindowFlags(Qt::ToolTip);
    setAttribute(Qt::WA_TranslucentBackground);
}

CameraStatusTip::~CameraStatusTip()
{
    delete ui;
}

CameraStatusTip *CameraStatusTip::instance()
{
    if (!self) {
        self = new CameraStatusTip(SettingContent::instance());
    }
    return self;
}

void CameraStatusTip::setState(int state, const QString &text)
{
    QFontMetrics fm(font());
    if (RTSP_CLIENT_CONNECT == state) {
        ui->labelState->setText(GET_TEXT("CHANNELMANAGE/30076", "Connected"));
        ui->labelText->hide();
        m_height = fm.height() + 10;
        m_width = fm.width(ui->labelState->text()) + 10;
    } else {
        ui->labelState->setText(GET_TEXT("CHANNELMANAGE/30077", "Disconnected:"));
        ui->labelText->setText(text);
        ui->labelText->show();
        m_height = fm.height() * 2 + 10;
        m_width = qMax(fm.width(ui->labelState->text()), fm.width(text)) + 10;
    }
    //BUG 这里resize之后有时会不是自己设置的大小，有些奇怪
    resize(m_width, m_height);
}

void CameraStatusTip::paintEvent(QPaintEvent *)
{
    QRect rc(0, 0, m_width - 2, m_height - 2);
    rc.moveCenter(rect().center());

    QPainter painter(this);
    painter.setPen(QPen(QColor("#010200"), 1));
    painter.setBrush(QColor("#FEFEDA"));
    painter.drawRect(rc);
}

#include "Keyboard.h"
#include "ui_Keyboard.h"
#include "KeyboardData.h"
#include "KeyboardPolski.h"
#include "KeyboardWidget.h"
#include "MsLanguage.h"
#include <QMouseEvent>
#include <QPainter>

Keyboard::Keyboard(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Keyboard)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setWindowModality(Qt::WindowModal);

    connect(MsLanguage::instance(), SIGNAL(languageChanged()), this, SLOT(onLanguageChanged()));
    onLanguageChanged();

    installEventFilter(this);
}

Keyboard::~Keyboard()
{
    delete ui;
}

bool Keyboard::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                m_dragPosition = mouseEvent->globalPos() - pos();
                m_dragThreshold.restart();
            }
            return true;
        }
        if (event->type() == QEvent::MouseButtonRelease) {
            m_dragThreshold.invalidate();
            return true;
        }
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if ((mouseEvent->buttons() & Qt::LeftButton) && m_dragThreshold.isValid() && m_dragThreshold.elapsed() > 200)
                move(mouseEvent->globalPos() - m_dragPosition);
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void Keyboard::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(185, 185, 185), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect());
}

void Keyboard::onLanguageChanged()
{
    if (m_keyboard) {
        ui->gridLayout->removeWidget(m_keyboard);
        m_keyboard->deleteLater();
    }

    int id = MsLanguage::instance()->currentLanguage();
    gKeyboardData.initialize(id);
    switch (id) {
    case MsLanguage::LAN_PL:
        m_keyboard = new KeyboardPolski(this);
        connect(m_keyboard, SIGNAL(sigClose()), this, SLOT(onKeyboardClose()));
        break;
    default:
        m_keyboard = new KeyboardWidget(this);
        connect(m_keyboard, SIGNAL(sigClose()), this, SLOT(onKeyboardClose()));
        break;
    }

    ui->gridLayout->addWidget(m_keyboard, 0, 0);
}

void Keyboard::onKeyboardClose()
{
    QWidget *focusWidget = QApplication::focusWidget();
    if (focusWidget) {
        focusWidget->clearFocus();
    }
    hide();
}

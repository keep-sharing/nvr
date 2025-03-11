#include "MsWaitting.h"
#include "MyDebug.h"
#include "mainwindow.h"
#include "SubControl.h"
#include "qglobal.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QtDebug>

MsWaitting *MsWaitting::s_msWaitting = nullptr;
QHash<MsWaitting *, bool> MsWaitting::s_waittingMap;

MsWaitting::MsWaitting(QWidget *parent)
    : QDialog(parent)
    , m_parent(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    //BUG: 加了这个会导致CameraEdit无法输入，奇怪
    //setAttribute(Qt::WA_ShowWithoutActivating);
    setWindowModality(Qt::ApplicationModal);

    m_brushColor = QColor(6, 127, 172);
    m_angle = 0;

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(45);

    resize(100, 100);
}

MsWaitting::~MsWaitting()
{
    s_waittingMap.remove(this);
}

MsWaitting *MsWaitting::instance()
{
    if (!s_msWaitting) {
        s_msWaitting = new MsWaitting(MainWindow::instance());
        s_msWaitting->setCustomPos(true);
    }
    return s_msWaitting;
}

int MsWaitting::globalRef()
{
    return instance()->ref();
}

int MsWaitting::globalDeref()
{
    return instance()->deref();
}

void MsWaitting::showGlobalWait(QWidget *parent)
{
    Q_UNUSED(parent)
    // if (parent) {
    //     QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, instance()->size(), QRect(parent->mapToGlobal(QPoint(0, 0)), parent->size()));
    //     instance()->move(rc.topLeft());
    // } else {
    //     QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
    //     QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, instance()->size(), screenRc);
    //     instance()->move(rc.topLeft());
    // }
    //instance()->//showWait();
}

void MsWaitting::execGlobalWait(QWidget *parent)
{
    Q_UNUSED(parent)
    // if (parent) {
    //     QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, instance()->size(), QRect(parent->mapToGlobal(QPoint(0, 0)), parent->size()));
    //     instance()->move(rc.topLeft());
    // } else {
    //     QRect screenRc = SubControl::instance()->logicalMainScreenGeometry();
    //     QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, instance()->size(), screenRc);
    //     instance()->move(rc.topLeft());
    // }
    // instance()->execWait();
}

void MsWaitting::closeGlobalWait()
{
    //instance()->//closeWait();
}

bool MsWaitting::hasWaitting()
{
    return !s_waittingMap.isEmpty();
}

int MsWaitting::userResult()
{
    return m_userResult;
}

void MsWaitting::setUserResult(int result)
{
    m_userResult = result;
}

void MsWaitting::setCustomPos(bool enable)
{
    m_isCustomPos = enable;
}

void MsWaitting::moveToCenter(const QRect &rc)
{
    move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), rc).topLeft());
}

void MsWaitting::showWait()
{
    //showWait(nullptr, QString());
}

int MsWaitting::execWait()
{
    return 0;
}

void MsWaitting::exitWait(int value)
{
    Q_UNUSED(value)
    //done(value);
}

void MsWaitting::showWait(const QString &message)
{
    Q_UNUSED(message)
    //showWait(nullptr, message);
}

int MsWaitting::execWait(const QString &message)
{
    Q_UNUSED(message)
    return 0;
}

void MsWaitting::showWait(QWidget *parent, const QString &message)
{
    Q_UNUSED(parent)
    Q_UNUSED(message)
    // if (parent) {
    //     QRect parentRect(parentWidget()->mapToGlobal(QPoint(0, 0)), parent->size());
    //     QRect rc = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), parentRect);
    //     move(rc.topLeft());
    // }

    // //
    // if (!message.isEmpty()) {
    //     qDebug() << "MsWaitting, show:" << message;
    // }
    // show();
}

int MsWaitting::execWait(QWidget *parent, const QString &message)
{
    Q_UNUSED(parent)
    Q_UNUSED(message)
    // if (parent) {
    //     QRect parentRect(parentWidget()->mapToGlobal(QPoint(0, 0)), parent->size());
    //     move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), parentRect).topLeft());
    // }

    // //
    // if (!message.isEmpty()) {
    //     qDebug() << "MsWaitting, exec:" << message;
    // }
    return 0;
}

void MsWaitting::closeWait()
{
    //closeWait(QString());
}

void MsWaitting::closeWait(const QString &message)
{
    Q_UNUSED(message)
    // if (!message.isEmpty()) {
    //     qDebug() << "MsWaitting, closed:" << message;
    // }
    // close();
}

int MsWaitting::ref()
{
    return 0;
}

int MsWaitting::deref()
{
    return 0;
}

void MsWaitting::showEvent(QShowEvent *event)
{
    Q_UNUSED(event)
    // if (!m_isCustomPos) {
    //     QWidget *parent = parentWidget();
    //     if (parent) {
    //         QRect parentRect(parent->mapToGlobal(QPoint(0, 0)), parent->size());
    //         move(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), parentRect).topLeft());
    //     }
    // }

    // m_timer->start();

    // s_waittingMap[this] = true;
    // QDialog::showEvent(event);
}

void MsWaitting::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event)
    // m_timer->stop();
    // m_clickCount = 0;

    // s_waittingMap.remove(this);
    // QDialog::hideEvent(event);
}

void MsWaitting::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    // m_timer->stop();
    // m_clickCount = 0;

    // s_waittingMap.remove(this);
    // QDialog::closeEvent(event);
}

void MsWaitting::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int width = qMin(this->width(), this->height());

    int outerRadius = (width - 1) * 0.3;
    int innerRadius = (width - 1) * 0.3 * 0.38;

    int capsuleHeight = (outerRadius - innerRadius) / 2;
    int capsuleWidth = capsuleHeight;
    int capsuleRadius = capsuleWidth / 2;

    for (int i = 0; i < 6; i++) {
        m_brushColor.setAlphaF(1.0f - (i / 12.0f));
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_brushColor);
        painter.save();
        painter.translate(rect().center());
        painter.rotate(m_angle - i * 30.0f);
        painter.drawRoundedRect(-capsuleWidth * 0.5 + 15, -(innerRadius + capsuleHeight) - 5, capsuleWidth, capsuleHeight, capsuleRadius, capsuleRadius);
        painter.restore();
    }
}

void MsWaitting::mousePressEvent(QMouseEvent *event)
{
#if 0
    if (event->button() == Qt::LeftButton)
    {
        m_clickCount++;
        qMsDebug() << "left button press, count:" << m_clickCount;
        if (m_clickCount > 5)
        {
            m_clickCount = 0;
            close();
            qMsDebug() << "manual close";
        }
    }
#endif
    QDialog::mousePressEvent(event);
}

void MsWaitting::onTimeout()
{
    m_angle = (m_angle + 30) % 360;
    update();
}

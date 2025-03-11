#include "AnimateToast.h"
#include "ui_AnimateToast.h"
#include <QDesktopWidget>
#include "MsLanguage.h"
#include "screencontroller.h"

AnimateToast *AnimateToast::s_animateToast = nullptr;

AnimateToast::AnimateToast(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AnimateToast)
{
    ui->setupUi(this);

    s_animateToast = this;

    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowModality(Qt::WindowModal);
    setFocusPolicy(Qt::StrongFocus);

    m_animation = new QPropertyAnimation(this, "geometry");
    m_animation->setDuration(1000);
    connect(m_animation, SIGNAL(finished()), this, SLOT(close()));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setSingleShot(true);

    m_timerAutoHide = new QTimer(this);
    m_timerAutoHide->setSingleShot(true);
    connect(m_timerAutoHide, SIGNAL(timeout()), this, SLOT(close()));
}

AnimateToast::~AnimateToast()
{
    s_animateToast = nullptr;
    delete ui;
}

void AnimateToast::setDestWidget(QWidget *widget)
{
    m_destWidget = widget;
}

void AnimateToast::showToast(const QString &str)
{
    ui->label->setMinimumSize(0, 50);
    ui->label->setText(str);

    //
    QRect screenRect = qApp->desktop()->screenGeometry();
    QSize toastSize = QSize(450, screenRect.height());
    QRect centerRect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, toastSize, screenRect);
    resize(450, screenRect.height());
    move(centerRect.topLeft());
    show();
}

void AnimateToast::showToast(const QString &str, int msec)
{
    showToast(str);
    m_timerAutoHide->start(msec);
}

void AnimateToast::hideToast()
{
    close();
}

void AnimateToast::hideToastLater(int msec)
{
    m_timerAutoHide->start(msec);
}

void AnimateToast::setToastText(const QString &str)
{
    ui->label->setText(str);
}

void AnimateToast::startAnimationLater(int msec, QWidget *dest/* = nullptr*/)
{
    //位置有点偏差，暂时先这样修正一下。
    QRect rc;
    if (dest)
    {
        rc = QRect(dest->mapToGlobal(QPoint(0, 0)), dest->size());
    }
    else
    {
        rc = QRect(m_destWidget->mapToGlobal(QPoint(0, 0)), m_destWidget->size());
    }

    m_animation->setStartValue(geometry());
    m_animation->setEndValue(rc);
    m_animation->setEasingCurve(QEasingCurve::OutBounce);
    m_timer->start(msec);
}

void AnimateToast::addItem(const QString &strChannel, const QString &strResult)
{
    if (m_itemList.isEmpty())
    {
        AnimateToastItem *itemHeader = new AnimateToastItem(this);
        itemHeader->setBackgroundColor(QColor("#2C9BC6"));
        itemHeader->setItemText(GET_TEXT("CHANNELMANAGE/30008", "Channel"), GET_TEXT("PLAYBACK/80113", "Result"));
        itemHeader->setItemTextColor(QColor("#FFFFFF"));

        ui->verticalLayout_item->addWidget(itemHeader);
        m_itemList.append(itemHeader);
    }

    //
    AnimateToastItem *itemHeader = new AnimateToastItem(this);
    if (m_itemList.size() % 2 == 0)
    {
        itemHeader->setBackgroundColor(QColor("#FFFFFF"));
    }
    else
    {
        itemHeader->setBackgroundColor(QColor("#E1E1E1"));
    }
    itemHeader->setItemText(strChannel, strResult);

    ui->verticalLayout_item->addWidget(itemHeader);
    m_itemList.append(itemHeader);

    //
    show();
}

void AnimateToast::addItemMap(const QMap<int, QString> &map)
{
    for (auto iter = map.constBegin(); iter != map.constEnd(); ++iter)
    {
        const int &channel = iter.key();
        const QString &text = iter.value();
        addItem(QString::number(channel + 1), text);
    }
}

void AnimateToast::setCount(int count)
{
    m_totalCount = count;
    m_index = 0;
    m_successCount = 0;
    m_failedCount = 0;
}

void AnimateToast::successAdded()
{
    m_successCount++;
}

void AnimateToast::failAdded()
{
    m_failedCount++;
}

void AnimateToast::indexAdded()
{
    m_index++;
}

bool AnimateToast::isEnd()
{
    return m_index >= m_totalCount;
}

int AnimateToast::successCount()
{
    return m_successCount;
}

void AnimateToast::closeEvent(QCloseEvent *)
{
    clearItem();
    ScreenController::instance()->speedDown();
}

void AnimateToast::clearItem()
{
    for (int i = 0; i < m_itemList.size(); ++i)
    {
        AnimateToastItem *item = m_itemList.at(i);
        ui->verticalLayout_item->removeWidget(item);
        delete item;
    }
    m_itemList.clear();
}

void AnimateToast::onTimeout()
{
    ui->label->setMinimumSize(0, 0);
    ui->label->clear();
    clearItem();
    m_animation->start();
    ScreenController::instance()->speedUp();
}

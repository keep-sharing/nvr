#include "toast.h"
#include "ui_toast.h"
#include <QDesktopWidget>
#include <QtDebug>
#include "SubControl.h"

Toast::Toast(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Toast)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    m_animation = new QPropertyAnimation(this, "pos");

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_timer->setInterval(2000);
}

Toast::~Toast()
{
    delete ui;
}

void Toast::setText(const QString &text)
{
    ui->label->setText(text);
}

void Toast::showToast(QWidget *parent, const QString &text)
{
    Toast *toast = new Toast(parent);
    toast->setText(text);
    toast->show();
}

void Toast::showEvent(QShowEvent *event)
{
    QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
    m_animation->setStartValue(QPoint(screenRect.left() + screenRect.width() / 2 - width() / 2, screenRect.top() - height()));
    m_animation->setEndValue(QPoint(screenRect.left() + screenRect.width() / 2 - width() / 2, screenRect.top()));
    m_animation->setDuration(200);
    m_animation->start();
    m_timer->start();
    QDialog::showEvent(event);
}

void Toast::hideEvent(QHideEvent *event)
{
    QDialog::hideEvent(event);
}

void Toast::onTimeout()
{
    QRect screenRect = SubControl::instance()->logicalMainScreenGeometry();
    connect(m_animation, SIGNAL(finished()), this, SLOT(deleteLater()));
    m_animation->setStartValue(QPoint(screenRect.left() + screenRect.width() / 2 - width() / 2, screenRect.top()));
    m_animation->setEndValue(QPoint(screenRect.left() + screenRect.width() / 2 - width() / 2, screenRect.top() - height()));
    m_animation->setDuration(200);
    m_animation->start();
}

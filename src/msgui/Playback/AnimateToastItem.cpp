#include "AnimateToastItem.h"
#include "ui_AnimateToastItem.h"
#include <QPainter>

AnimateToastItem::AnimateToastItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnimateToastItem)
{
    ui->setupUi(this);

    ui->label_key->clear();
    ui->label_value->clear();

    m_backgroundColor = QColor("#FFFFFF");
}

AnimateToastItem::~AnimateToastItem()
{
    delete ui;
}

void AnimateToastItem::setBackgroundColor(const QColor &color)
{
    m_backgroundColor = color;
    update();
}

void AnimateToastItem::setItemText(const QString &key, const QString &value)
{
    ui->label_key->setText(key);
    ui->label_value->setText(value);
}

void AnimateToastItem::setItemTextColor(const QColor &color)
{
    ui->label_key->setStyleSheet(QString("color: %1; background-color: transparent").arg(color.name()));
    ui->label_value->setStyleSheet(QString("color: %1; background-color: transparent").arg(color.name()));
}

void AnimateToastItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(m_backgroundColor));
    painter.drawRect(rect());
}

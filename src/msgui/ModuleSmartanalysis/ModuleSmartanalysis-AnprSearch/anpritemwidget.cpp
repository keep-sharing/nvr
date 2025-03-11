#include "anpritemwidget.h"
#include "ui_anpritemwidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>

AnprItemWidget::AnprItemWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AnprItemWidget)
{
    ui->setupUi(this);
}

AnprItemWidget::~AnprItemWidget()
{
    delete ui;
}

int AnprItemWidget::index() const
{
    return m_index;
}

void AnprItemWidget::setIndex(int index)
{
    m_index = index;
}

void AnprItemWidget::setItemInfo(int channel, const QDateTime &dateTime, const QImage &image)
{
    ui->checkBox_info->setText(QString("CH%1  %2").arg(channel + 1).arg(dateTime.toString("yyyy-MM-dd HH:mm:ss")));
    //ui->label_time->setText(QString("CH%1  %2").arg(channel + 1).arg(dateTime.toString("HH:mm:ss")));
    if (m_isFace) {
        ui->label_image->setRatio(1);
    }
    ui->label_image->setImage(image);
}

QImage AnprItemWidget::image() const
{
    return ui->label_image->image();
}

bool AnprItemWidget::isSelected() const
{
    return m_isSelected;
}

void AnprItemWidget::setSelected(bool selected)
{
    m_isSelected = selected;
    update();
}

bool AnprItemWidget::isInfoVisible() const
{
    return m_infoVisible;
}

void AnprItemWidget::setInfoVisible(bool visible)
{
    m_infoVisible = visible;
    ui->label_image->setVisible(visible);
    ui->checkBox_info->setVisible(visible);
    ui->label_time->setVisible(false);
    update();
}

bool AnprItemWidget::isChecked() const
{
    return ui->checkBox_info->isChecked();
}

void AnprItemWidget::setChecked(bool checked)
{
    ui->checkBox_info->setChecked(checked);
}

void AnprItemWidget::resizeEvent(QResizeEvent *event)
{
    qreal ratio = 16.0 / 9;

    QRect imageRect;
    qreal hight = event->size().width() / ratio;
    if (m_isFace) {
        imageRect.setLeft((event->size().width() - hight) / 2);
        imageRect.setTop(m_margin);
        imageRect.setRight((event->size().width() - hight) / 2 + m_margin + (event->size().width() / ratio));
        imageRect.setBottom(m_margin + event->size().width() / ratio);
    } else {
        imageRect.setLeft(m_margin);
        imageRect.setTop(m_margin);
        imageRect.setRight(m_margin + (event->size().width() - m_margin * 2));
        imageRect.setBottom(m_margin + event->size().width() / ratio);
    }
    ui->label_image->setGeometry(imageRect);

    QRect textRect = imageRect;
    textRect.setTop(imageRect.bottom());
    textRect.setBottom(imageRect.bottom() + 30);
    if (m_isFace) {
        textRect.setLeft(m_margin);
        textRect.setRight(m_margin + (event->size().width() - m_margin * 2));
    }
    ui->label_time->setGeometry(textRect);

    QRect checkBoxRect = textRect;
    ui->checkBox_info->setGeometry(checkBoxRect);

    m_selectedRect = rect();
    m_selectedRect.setLeft(m_selectedRect.left() + 1);
    m_selectedRect.setTop(m_selectedRect.top() + 1);
    m_selectedRect.setRight(m_selectedRect.right() - 1);
    m_selectedRect.setBottom(checkBoxRect.bottom());
}

void AnprItemWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (m_isSelected && m_infoVisible) {
        painter.save();
        painter.setPen(QPen(QColor("#09A8E2"), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(m_selectedRect);
        painter.restore();
    }
}

void AnprItemWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (isInfoVisible()) {
            emit itemClicked(m_index);
        }
    } else {
        return QWidget::mousePressEvent(event);
    }
}

void AnprItemWidget::on_checkBox_info_clicked(bool checked)
{
    emit itemChecked(m_index, checked);
}

void AnprItemWidget::setIsFace(bool value)
{
    m_isFace = value;
}

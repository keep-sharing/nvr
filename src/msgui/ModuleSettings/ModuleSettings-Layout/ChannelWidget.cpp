#include "ChannelWidget.h"
#include "ui_ChannelWidget.h"
#include "LayoutSettings.h"
#include "MsDevice.h"
#include <QPainter>

ChannelWidget::ChannelWidget(const CustomLayoutKey &key, int index, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChannelWidget)
    , m_key(key)
    , m_index(index)
{
    ui->setupUi(this);

    m_backgroundColor = QColor("#959595");
    m_hoverBorderColor = QColor("#71D6FC");
    m_selectedBorderColor = QColor("#0AA9E3");

    ui->label_index->setText(QString::number(index + 1));

    int channel = LayoutSettings::instance()->channelFromGlobalIndex(key, m_index);
    setChannel(channel);

    if (index >= qMsNvr->maxChannel()) {
        ui->label_channel->hide();
        ui->label_index->hide();
        ui->label_name->hide();
        ui->toolButton_clear->hide();
    }
}

ChannelWidget::~ChannelWidget()
{
    delete ui;
}

int ChannelWidget::channel() const
{
    return m_channel;
}

void ChannelWidget::setChannel(int channel)
{
    m_channel = channel;
    if (channel < 0) {
        ui->label_channel->setText("-");
        ui->label_name->setText("-");
    } else {
        QString text = qMsNvr->channelName(channel);
        ui->label_channel->setText(QString::number(channel + 1));
        ui->label_name->setElidedText(text);
    }
}

void ChannelWidget::updateChannel()
{
    int channel = LayoutSettings::instance()->channelFromGlobalIndex(m_key, m_index);
    setChannel(channel);
}

int ChannelWidget::index() const
{
    return m_index;
}

void ChannelWidget::setChecked(bool checked)
{
    if (checked) {
        m_state = StateSelected;
    } else {
        m_state = StateNone;
    }
    update();
}

void ChannelWidget::resizeEvent(QResizeEvent *)
{
    adjustAllWidgets();
}

void ChannelWidget::showEvent(QShowEvent *)
{
    adjustAllWidgets();
}

void ChannelWidget::enterEvent(QEvent *event)
{
    if (m_state != StateSelected) {
        m_state = StateHover;
    }
    update();
    QWidget::enterEvent(event);
}

void ChannelWidget::leaveEvent(QEvent *event)
{
    if (m_state != StateSelected) {
        m_state = StateNone;
    }
    update();
    QWidget::leaveEvent(event);
}

void ChannelWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.save();
    switch (m_state) {
    case StateNone:
        painter.setPen(Qt::NoPen);
        break;
    case StateHover:
        painter.setPen(QPen(m_hoverBorderColor, 4));
        break;
    case StateSelected:
        painter.setPen(QPen(m_selectedBorderColor, 4));
        break;
    }
    if (m_index >= qMsNvr->maxChannel()) {
        painter.setPen(Qt::NoPen);
    }
    painter.setBrush(m_backgroundColor);
    QRect rc = rect();
    painter.drawRect(rc);
    painter.restore();
}

void ChannelWidget::mousePressEvent(QMouseEvent *)
{
    emit clicked(m_index, m_channel);
}

void ChannelWidget::adjustAllWidgets()
{
    ui->label_index->adjustSize();
    ui->label_index->move(10, 10);
    ui->toolButton_clear->move(width() - ui->toolButton_clear->width() - 10, 10);
    ui->widget->setGeometry(rect());
}

void ChannelWidget::on_toolButton_clear_clicked()
{
    setChannel(-1);
    LayoutSettings::instance()->setChannelFromGlobalIndex(m_key, m_index, -1);
}

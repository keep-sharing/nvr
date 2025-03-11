#include "PageLayout.h"
#include "ui_PageLayout.h"
#include "LayoutSettings.h"
#include <QPainter>
#include <qmath.h>

PageLayout::PageLayout(Mode mode, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PageLayout)
    , m_mode(mode)
{
    ui->setupUi(this);
}

PageLayout::~PageLayout()
{
    delete ui;
}

void PageLayout::initializeData(const CustomLayoutInfo &info, int page)
{
    m_key = info.key();
    m_page = page;

    int rowCount = info.baseRow();
    int columnCount = info.baseColumn();
    for (int row = 0; row < rowCount; ++row) {
        ui->gridLayout->setRowStretch(row, 1);
    }
    for (int column = 0; column < columnCount; ++column) {
        ui->gridLayout->setColumnStretch(column, 1);
    }

    const auto &positions = info.positions();
    int index = 0;
    for (auto iter = positions.constBegin(); iter != positions.constEnd(); ++iter) {
        int channelIndex = positions.count() * page + index;
        const VideoPosition &p = iter.key();
        switch (m_mode) {
        case ModePreview: {
            ChannelWidgetPreview *widget = new ChannelWidgetPreview(m_key, channelIndex, this);
            ui->gridLayout->addWidget(widget, p.row, p.column, p.rowSpan, p.columnSpan);
            widget->show();
            m_channelWidgetPreviewList.append(widget);
            break;
        }
        case ModeDetail: {
            ChannelWidget *widget = new ChannelWidget(m_key, channelIndex, this);
            connect(widget, SIGNAL(clicked(int, int)), this, SLOT(onChannelWidgetClicked(int, int)));
            ui->gridLayout->addWidget(widget, p.row, p.column, p.rowSpan, p.columnSpan);
            widget->show();
            m_mapChannelWidget.insert(channelIndex, widget);
            break;
        }
        default:
            break;
        }
        index++;
    }

    //默认选中第一个
    if (m_mode == ModeDetail) {
        auto iter = m_mapChannelWidget.constBegin();
        onChannelWidgetClicked(iter.key(), iter.value()->channel());
    }
}

int PageLayout::page() const
{
    return m_page;
}

int PageLayout::layoutMode() const
{
    return m_layoutMode;
}

void PageLayout::setChecked(bool checked)
{
    m_isSelected = checked;
    update();
}

void PageLayout::updateChannelPreview()
{
    for (int i = 0; i < m_channelWidgetPreviewList.size(); ++i) {
        ChannelWidgetPreview *channelWidget = m_channelWidgetPreviewList.at(i);
        channelWidget->updateChannel();
    }
}

void PageLayout::updateChannel()
{
    for (auto iter = m_mapChannelWidget.begin(); iter != m_mapChannelWidget.end(); ++iter) {
        ChannelWidget *channelWidget = iter.value();
        channelWidget->updateChannel();
    }
}

void PageLayout::setChannel(int channel)
{
    ChannelWidget *widget = m_mapChannelWidget.value(m_currentIndex, nullptr);
    if (widget) {
        widget->setChannel(channel);
    }
    //
    LayoutSettings::instance()->setChannelFromGlobalIndex(m_key, m_currentIndex, channel);
}

void PageLayout::enterEvent(QEvent *)
{
    m_isHover = true;
    update();
}

void PageLayout::leaveEvent(QEvent *)
{
    m_isHover = false;
    update();
}

void PageLayout::paintEvent(QPaintEvent *)
{
    if (m_mode != ModePreview) {
        return;
    }

    QPainter painter(this);
    if (m_isSelected) {
        painter.setPen(QPen(QColor(10, 169, 227), 4));
    } else if (m_isHover) {
        painter.setPen(QPen(QColor(113, 214, 252), 4));
    } else {
        return;
    }
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(rect());
}

void PageLayout::mousePressEvent(QMouseEvent *)
{
    emit clicked(m_key, m_page);
}

void PageLayout::onChannelWidgetClicked(int index, int channel)
{
    Q_UNUSED(channel)

    m_currentIndex = index;
    for (auto iter = m_mapChannelWidget.begin(); iter != m_mapChannelWidget.end(); ++iter) {
        ChannelWidget *widget = iter.value();
        if (index == widget->index()) {
            widget->setChecked(true);
        } else {
            widget->setChecked(false);
        }
    }
}

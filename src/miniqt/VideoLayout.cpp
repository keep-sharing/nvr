#include "VideoLayout.h"
#include "ui_VideoLayout.h"

VideoLayout::VideoLayout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoLayout)
{
    ui->setupUi(this);
}

VideoLayout::~VideoLayout()
{
    delete ui;
}

void VideoLayout::setVideoLayout(int row, int column)
{
    for (auto iter = m_mapVideoWidget.constBegin(); iter != m_mapVideoWidget.constEnd(); ++iter) {
        auto *widget = iter.value();
        ui->gridLayout->removeWidget(widget);
        widget->hide();
    }

    for (int i = 0; i < row; ++i) {
        for (int j = 0; j < column; ++j) {
            int index = i * column + j;
            VideoWidget *widget = m_mapVideoWidget.value(index);
            if (!widget) {
                widget = new VideoWidget(this);
                widget->setChannel(index);
                m_mapVideoWidget.insert(index, widget);
            }
            ui->gridLayout->addWidget(widget, i, j);
            widget->show();
        }
    }
}

#include "HistogramScene.h"
#include "MyDebug.h"
#include "LegendItem.h"
#include "MsDevice.h"
#include "MsLanguage.h"
#include "SubHistogramItem.h"
#include "TotalHistogramItem.h"
#include <QPainter>
#include <qmath.h>

HistogramScene::HistogramScene(QObject *parent)
    : BaseChartScene(parent)
{
    connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(onSceneRectChanged(QRectF)));
}

void HistogramScene::showHistogram(const QList<int> &channels, int groupFilter)
{
    clear();

    m_mode = ModeHistogram;
    m_channels = channels;
    m_groupFilter = groupFilter;

    showLegends(channels);

    m_gridData = gPeopleCountingData.peopleHistogramGridData(m_groupFilter);
    const PeopleHistogramData &histogramData = gPeopleCountingData.peopleHistogramData(m_groupFilter);

    QRectF gridRc = gridRect();
    qreal rectWidth = gridRc.width() / m_gridData.xMaxValue / 2;
    for (auto xIter = histogramData.histogramData.constBegin(); xIter != histogramData.histogramData.constEnd(); ++xIter) {
        qreal xValue = xIter.key();
        const QMap<HistogramKey, HistogramValue> &valueMap = xIter.value();
        //
        QRectF rc;
        for (auto valueIter = valueMap.constBegin(); valueIter != valueMap.constEnd(); ++valueIter) {
            const HistogramKey &key = valueIter.key();
            if (!isChannelVisible(key.id)) {
                continue;
            }
            const HistogramValue &value = valueIter.value();
            if (value.value == 0) {
                continue;
            }
            qreal yValue = value.value;
            qreal left = xValue / m_gridData.xMaxValue * gridRc.width() + gridRc.left() + rectWidth / 2;
            qreal right = left + rectWidth;
            qreal top = gridRc.bottom() - yValue / m_gridData.yMaxValue * gridRc.height();
            qreal bottom = gridRc.bottom();
            rc.setLeft(left);
            rc.setRight(right);
            rc.setTop(top);
            rc.setBottom(bottom);

            if (key.id == TOTAL_LINE_INDEX || channels.count() == 1) {
                TotalHistogramItem *item = new TotalHistogramItem(value.color, value.value);
                addItem(item);
                item->setRect(rc);
            } else {
                SubHistogramItem *item = new SubHistogramItem(value.color, value.value);
                addItem(item);
                item->setRect(rc);
            }
            //超过10个通道时仅显示total, 11 = 10 channel + total
            if (m_channels.size() > 11) {
                break;
            }
        }
    }

    update();
}

void HistogramScene::showHistogram2(const QList<int> &channels, int groupFilter)
{
    clear();

    m_mode = ModeHistogram2;
    m_channels = channels;
    m_groupFilter = groupFilter;

    showLegends(channels);

    m_gridData = gPeopleCountingData.peopleHistogramGridData2(m_groupFilter);
    const PeopleHistogramData &histogramData = gPeopleCountingData.peopleHistogramData(m_groupFilter);

    QRectF gridRc = gridRect();
    qreal rectWidth = gridRc.width() / m_gridData.xMaxValue / 2;
    int x = 0;
    QRectF rc;
    for (auto lineIter = histogramData.histogramData2.constBegin(); lineIter != histogramData.histogramData2.constEnd(); ++lineIter) {
        int line = lineIter.key();
        if (!isChannelVisible(line)) {
            x++;
            continue;
        }
        const HistogramValue &value = lineIter.value();

        qreal xValue = x;
        qreal yValue = value.value;
        qreal left = xValue / m_gridData.xMaxValue * gridRc.width() + gridRc.left() + rectWidth / 2;
        qreal right = left + rectWidth;
        qreal top = gridRc.bottom() - yValue / m_gridData.yMaxValue * gridRc.height();
        qreal bottom = gridRc.bottom();
        rc.setLeft(left);
        rc.setRight(right);
        rc.setTop(top);
        rc.setBottom(bottom);

        TotalHistogramItem *item = new TotalHistogramItem(value.color, value.value);
        addItem(item);
        item->setRect(rc);

        //
        x++;

        //超过10个通道时仅显示total, 11 = 10 channel + total
        if (m_channels.size() > 11) {
            break;
        }
    }

    update();
}

void HistogramScene::setChannelVisible(int channel, bool visible)
{
    if (m_currentGroup < 0) {
        return;
    }
    m_visibleChannels[m_currentGroup][channel] = visible;
    switch (m_mode) {
    case ModeHistogram:
        showHistogram(m_channels, m_groupFilter);
        break;
    case ModeHistogram2:
        showHistogram2(m_channels, m_groupFilter);
        break;
    }
}

void HistogramScene::drawXAxis(QPainter *painter, const QRectF &rect)
{
    ReportType report = gPeopleCountingData.reportType();
    qreal xStep = (rect.width() - marginLeft() - marginRight()) / (m_gridData.xLineCount - 1);
    qreal x = rect.left() + marginLeft();
    for (int i = 0; i < m_gridData.xLineCount; ++i) {
        painter->setPen(QPen(QColor(139, 139, 139), 1));
        painter->drawLine(QPointF(x, rect.top() + marginTop()), QPointF(x, rect.bottom() - marginBottom()));

        //
        if (!m_gridData.xAxisStrings.isEmpty()) {
            QString text = m_gridData.xAxisStrings.at(i);
            QRectF textRect;
            switch (report) {
            case DailyReport:
                textRect.setLeft(x - xStep);
                textRect.setRight(x + xStep);
                textRect.setTop(rect.bottom() - marginBottom());
                textRect.setBottom(rect.bottom() - marginBottom() + 30);
                break;
            case WeeklyReport:
            case MonthlyReport:
            case YearlyReport:
                textRect.setLeft(x + xStep / 2 - xStep);
                textRect.setRight(x + xStep / 2 + xStep);
                textRect.setTop(rect.bottom() - marginBottom());
                textRect.setBottom(rect.bottom() - marginBottom() + 30);
                break;
            }
            if (textRect.center().x() - (rect.right() - marginRight()) < 1) {
                painter->setPen(QPen(QColor(87, 87, 87)));
                painter->drawText(textRect, Qt::AlignCenter, text);
            }
        }

        //
        x += xStep;
    }
}

void HistogramScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)

    //MsDebug() << "scene rect:" << sceneRect() << ", update rect:" << rect;

    //title
    drawTitle(painter);

    //
    drawYAxis(painter, sceneRect());
    drawXAxis(painter, sceneRect());
}

#ifndef DRAWITEMTIMEHEATMAP_H
#define DRAWITEMTIMEHEATMAP_H

#include <QGraphicsRectItem>
#include <QDateTime>

class DrawItemTimeHeatMap : public QGraphicsRectItem
{
public:
    enum ReportType
    {
        DailyReport,
        WeeklyReport,
        MonthlyReport,
        AnnualReport
    };

    enum yScaleBase
    {
        yDay,
        yHour,
        yMinute,
        ySecond
    };

    DrawItemTimeHeatMap(QGraphicsItem *parent = nullptr);

    void showHeatMap(const QString &text, int reportType, const QDateTime &dateTime);
    bool saveTimeHeatMap(const QString &filePath);
    bool saveTimeHeatMap(const QString &text, int reportType, const QDateTime &dateTime, const QString &fileName);
    bool saveTimeHeatMap(const QList<int> &valuesList, int reportType, const QDateTime &dateTime, const QString &fileName);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    void drawGrid(QPainter *painter);
    void drawLine(QPainter *painter);
    QString intoWeek(int day);

private:
    int m_reportType = 0;
    QDateTime m_dateTime;
    int m_max = 0;
    int m_min = 0;
    QList<int> m_values;
    QList<QRectF> m_valueRects;

    yScaleBase m_yScaleBase = ySecond;

    int m_left = 0;
    int m_right = 0;
    int m_top = 0;
    int m_bottom = 0;

    int m_yMax = 0;
    qreal m_xStep = 0;

    int m_hoverIndex = -1;
    qreal m_hoverValue = 0;
};

#endif // DRAWITEMTIMEHEATMAP_H

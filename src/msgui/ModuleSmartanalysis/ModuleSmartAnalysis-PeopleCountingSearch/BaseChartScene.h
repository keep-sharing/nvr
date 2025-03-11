#ifndef BASECHARTSCENE_H
#define BASECHARTSCENE_H

/******************************************************************
* @brief    人数统计绘图基类
* @author   LiuHuanyu
* @date     2021-07-16
******************************************************************/

#include <QGraphicsScene>
#include "PeopleCountingData.h"

class ToolTipItem;

class BaseChartScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit BaseChartScene(QObject *parent = 0);

    void setMainTitle(const QString &text);
    void setSubTitle(const QString &text);

    void setCurrentGroup(int group);
    void clearChannelVisible();
    virtual void setChannelVisible(int channel, bool visible);

protected:
    bool isChannelVisible(int channel);

    //图例
    virtual QString legendName(int channel);
    void showLegends(const QList<int> &channels);

    virtual void drawBackgroundColor(QPainter *painter);

    virtual void drawTitle(QPainter *painter);

    virtual void drawYAxis(QPainter *painter, const QRectF &rect);
    virtual int yLineValue(int index) const;

    virtual void drawXAxis(QPainter *painter, const QRectF &rect);
    virtual int xLineValue(int index) const;

    QRectF gridRect() const;
    QRectF legendRect() const;

    virtual int marginLeft() const;
    virtual int marginRight() const;
    virtual int marginTop() const;
    virtual int marginBottom() const;

signals:

protected slots:
    void onSceneRectChanged(const QRectF &rc);

protected:
    QString m_mainTitle;
    QString m_subTitle;

    PeopleGridData m_gridData;

    int m_currentGroup = -1;
    //key1: group, key2: channel
    QMap<int, QMap<int, bool>> m_visibleChannels;

    QList<int> m_channels;
    int m_groupFilter;

    ToolTipItem *m_toolTip = nullptr;
};

#endif // BASECHARTSCENE_H

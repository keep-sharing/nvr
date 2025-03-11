#ifndef GRAPHICSDRAWLINECROSSING_H
#define GRAPHICSDRAWLINECROSSING_H

#include "MsGraphicsObject.h"
#include "GraphicsDrawLineCrossingItem.h"

extern "C" {
#include "msg.h"
}

class GraphicsDrawLineCrossing : public MsGraphicsObject
{
    Q_OBJECT

public:
    explicit GraphicsDrawLineCrossing(QGraphicsItem *parent = nullptr);
    ~GraphicsDrawLineCrossing() override;

    void setLineCrossInfo(ms_linecrossing_info *info);
    void getLineCrossInfo(ms_linecrossing_info *info);

    void setLineCrossInfo(ms_linecrossing_info2 *info);
    void getLineCrossInfo(ms_linecrossing_info2 *info);

    void setLineCrossInfo(ms_linecrossing_info2 *info, int index);
    void getLineCrossInfo(ms_linecrossing_info2 *info, int index);

    void setPeopleCountInfo(ms_smart_event_people_cnt *info);
    void getPeopleCountInfo(ms_smart_event_people_cnt *info);

    void setCurrentLine(int index);

    void setLineDirection(int direction);
    int lineDirection(int index);

    void clearLine(int index);
    void clearAllLine();

    void setEnabled(bool enabled);

    void setIsShowLineInfo(bool enabled, int index);
    void setLineCntOsdType(int osdType);
    void setLineCntData(MS_PEOPLECNT_DATA *info);

    void setRect(const QRectF &rect);

    void setLineCrossState(int state, int line);

private:
    int m_currentLineIndex = 0;
    QMap<int, GraphicsDrawLineCrossingItem *> m_lineMap;
    ms_smart_event_people_cnt m_info ;
};

#endif // GRAPHICSDRAWLINECROSSING_H

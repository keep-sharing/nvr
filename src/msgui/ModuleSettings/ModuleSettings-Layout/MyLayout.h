#ifndef MYLAYOUT_H
#define MYLAYOUT_H

#include <QMap>
#include <QObject>

struct ChannelPosition {
    int index = -1;
    int channel = -1;
    int row = 0;
    int column = 0;
    int rowSpan = 1;
    int columnSpan = 1;

    ChannelPosition()
    {
    }
    ChannelPosition(int r, int c, int rSpan = 1, int cSpan = 1)
    {
        row = r;
        column = c;
        rowSpan = rSpan;
        columnSpan = cSpan;
    }
};
typedef QList<ChannelPosition> ChannelPositionList;

class MyLayout : public QObject {
    Q_OBJECT
public:
    explicit MyLayout(QObject *parent = nullptr);

    static QMap<int, ChannelPositionList> layoutPageMap(const struct mosaic &layout);

private:
    static QMap<int, ChannelPositionList> makeNormalLayout(int row, int column, const mosaic &layoutData);
    static QMap<int, ChannelPositionList> makeSpecificLayout(const QList<ChannelPosition> &list, const mosaic &layoutData);

signals:

public slots:
};

#endif // MYLAYOUT_H

#ifndef POSDISPLAYDATA_H
#define POSDISPLAYDATA_H

#include <QColor>
#include <QObject>

#define gPosDisplayData PosDisplayData::instance()

namespace MsPos {
struct ColorInfo {
    int index;
    QColor color;
    QString name;
    ColorInfo()
    {
    }
    ColorInfo(int _index, QString _name, QColor _color)
        : index(_index)
        , color(_color)
        , name(_name)
    {
    }
};
}

class PosDisplayData : public QObject {
    Q_OBJECT

public:
    explicit PosDisplayData(QObject *parent = nullptr);

    static PosDisplayData &instance();

    const QList<MsPos::ColorInfo> &colorList() const;
    void setColorList(const QList<MsPos::ColorInfo> &newColorList);
    QColor colorFromIndex(int index);

    int fontSizeFromIndex(int index);

    QString protocolName(int protocol);
    int protocolValue(const QString &name);

    QString characterEncodingName(int encoding);
    int characterEncodingValue(const QString &name);

signals:

private slots:
    void onLanguageChanged();

private:
    QList<MsPos::ColorInfo> m_colorList;
};

#endif // POSDISPLAYDATA_H

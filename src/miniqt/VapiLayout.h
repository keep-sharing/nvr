#ifndef VAPILAYOUT_H
#define VAPILAYOUT_H

#include <QMap>
#include <QObject>
#include "vapi.h"

#define gVapiLayout VapiLayout::instance()

struct LayoutInfo {
    int id = -1;
    bool enable = false;
};

class VapiLayout : public QObject {
    Q_OBJECT
  public:
    explicit VapiLayout(QObject *parent = nullptr);

    static VapiLayout &instance();

    void vapiSetLayout(int row, int column);
    void updateLayout(int id, bool enable);

  signals:

  private:
    int m_layoutWidth = 0;
    int m_layoutHeight = 0;
    LAYOUT_S m_stLayout;
    QMap< int, LayoutInfo> m_mapLayoutInfo;
};

#endif // VAPILAYOUT_H

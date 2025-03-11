#ifndef EVENTPOPUPDATA_H
#define EVENTPOPUPDATA_H

#include <QObject>

#define gEventPopupData EventPopupData::instance()

struct EventPopupInfo {
    int screen;
    int layout;
    QList<int> channels;
};

class EventPopupData : public QObject
{
    Q_OBJECT
public:
    explicit EventPopupData(QObject *parent = nullptr);

    static EventPopupData &instance();

signals:

};

#endif // EVENTPOPUPDATA_H

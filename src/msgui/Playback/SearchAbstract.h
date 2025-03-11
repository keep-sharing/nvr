#ifndef SEARCHABSTRACT_H
#define SEARCHABSTRACT_H

#include "MsObject.h"

class SearchAbstract : public MsObject {
    Q_OBJECT

public:
    enum State {
        StateNone,
        StateSearching,
        StateFinished
    };

    explicit SearchAbstract(QObject *parent = nullptr);

    int channel() const;

    bool isSearching() const;
    bool isSearchFinished() const;

signals:
    void finished(int channel);

protected:
    State m_state = StateNone;
    int m_channel = -1;
};

#endif // SEARCHABSTRACT_H

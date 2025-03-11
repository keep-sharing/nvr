#ifndef PUSHBUTTONEDITSTATE_H
#define PUSHBUTTONEDITSTATE_H

#include "mypushbutton.h"

class PushButtonEditState : public MyPushButton {
    Q_OBJECT

public:
    enum State {
        StateFinished,
        StateEditing
    };

public:
    explicit PushButtonEditState(QWidget *parent = nullptr);

    void editButtonState(State state);
    void setButtonState(State state);
    State buttonState() const;

signals:
    void stateSet(PushButtonEditState::State state);

private:
    State m_state = StateFinished;
};

#endif // PUSHBUTTONEDITSTATE_H

#include "pushbuttoneditstate.h"
#include "MsLanguage.h"

PushButtonEditState::PushButtonEditState(QWidget *parent)
    : MyPushButton(parent)
{
}

void PushButtonEditState::editButtonState(PushButtonEditState::State state)
{
    m_state = state;
    switch (m_state) {
    case StateFinished:
        setTranslatableText("COMMON/1019", "Edit");
        break;
    case StateEditing:
        setTranslatableText("SMARTEVENT/55021", "Finish");
        break;
    }
}

void PushButtonEditState::setButtonState(State state)
{
    editButtonState(state);

    emit stateSet(state);
}

PushButtonEditState::State PushButtonEditState::buttonState() const
{
    return m_state;
}

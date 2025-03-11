#include "LineEditAlarmOutName.h"
#include "MsLanguage.h"

LineEditAlarmOutName::LineEditAlarmOutName(QWidget *parent)
    : LineEdit(parent)
{
}

bool LineEditAlarmOutName::check()
{
    QRegExp rx(R"([&/\\:\*\?'"<>\|%])");
    if (rx.indexIn(text()) != -1) {
        return false;
    }
    return true;
}

QString LineEditAlarmOutName::tipString()
{
    return GET_TEXT("MYLINETIP/112004", "Invalid characters: &/\\:*?'\"<>|%.");
}

#include "LineEditRtsp.h"
#include "MsLanguage.h"

LineEditRtsp::LineEditRtsp(QWidget *parent)
    : LineEdit(parent)
{
    clear();
}

RtspInfo LineEditRtsp::rtspInfo() const
{
    return RtspInfo(text());
}

void LineEditRtsp::clear()
{
    setText("rtsp://");
}

bool LineEditRtsp::isEmpty() const
{
    return text().isEmpty() || text() == QString("rtsp://");
}

bool LineEditRtsp::check()
{
    RtspInfo info(text());
    return info.isValid();
}

QString LineEditRtsp::tipString()
{
    if (text().isEmpty()) {
        return GET_TEXT("MYLINETIP/112000", "Cannot be empty.");
    } else {
        return GET_TEXT("MYLINETIP/112002", "Invalid RTSP url.");
    }
}

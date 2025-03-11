#include "TargetInfoFace.h"
#include "ToolButtonFace.h"
#include "MsLanguage.h"

TargetInfoFace::TargetInfoFace(int channel, QString time, const MS_FACE_IMAGE *face)
    : TargetInfo()
    , m_channel(channel)
    , m_time(time)
{
    m_type = TARGET_FACE;
    memcpy(&m_info, face, sizeof(MS_FACE_PRIVATE));
    m_smallImageData = QByteArray((char *)face->img.data, face->img.size);
}

TargetInfoFace::~TargetInfoFace()
{
}

int TargetInfoFace::channel() const
{
    return m_channel;
}

QString TargetInfoFace::timeString() const
{
    return m_time;
}

FACE_GENDE TargetInfoFace::male() const
{
    return static_cast<FACE_GENDE>(m_info.gender);
}

void TargetInfoFace::updateMaleInfo(ToolButtonFace *button)
{
    switch (m_info.gender) {
    case FACE_GENDE_NONE:
        button->setIcon(QIcon(":/face/face/unrecognized.png"));
        button->setElidedText(GET_TEXT("TARGETMODE/103205", "N/A"));
        break;
    case FACE_GENDE_MALE:
        button->setIcon(QIcon(":/face/face/male.png"));
        button->setElidedText(GET_TEXT("FACE/141043", "Male"));
        break;
    case FACE_GENDE_FEMALE:
        button->setIcon(QIcon(":/face/face/female.png"));
        button->setElidedText(GET_TEXT("FACE/141044", "Female"));
        break;
    }
    button->setToolTip(button->wholeText());
}

FACE_AGE TargetInfoFace::age() const
{
    return static_cast<FACE_AGE>(m_info.age);
}

void TargetInfoFace::updateAgeInfo(ToolButtonFace *button)
{
    switch (m_info.age) {
    case FACE_AGE_NONE:
        button->setIcon(QIcon(":/face/face/unrecognized.png"));
        button->setElidedText(GET_TEXT("TARGETMODE/103205", "N/A"));
        break;
    case FACE_AGE_CHILD:
        button->setIcon(QIcon(":/face/face/child.png"));
        button->setElidedText(GET_TEXT("FACE/141040", "Child"));
        break;
    case FACE_AGE_ADULT:
        button->setIcon(QIcon(":/face/face/adult.png"));
        button->setElidedText(GET_TEXT("FACE/141041", "Adult"));
        break;
    case FACE_AGE_ELDERLY:
        button->setIcon(QIcon(":/face/face/elderly.png"));
        button->setElidedText(GET_TEXT("FACE/141042", "Elderly"));
        break;
    default:
        break;
    }
    button->setToolTip(button->wholeText());
}

FACE_GLASSES TargetInfoFace::glasses() const
{
    return static_cast<FACE_GLASSES>(m_info.glasses);
}

void TargetInfoFace::updateGlassesInfo(ToolButtonFace *button)
{
    switch (m_info.glasses) {
    case FACE_GLASSES_NONE:
        button->setIcon(QIcon(":/face/face/unrecognized.png"));
        button->setElidedText(GET_TEXT("TARGETMODE/103205", "N/A"));
        break;
    case FACE_GLASSES_NO:
        button->setIcon(QIcon(":/face/face/no-glasses.png"));
        button->setElidedText(GET_TEXT("FACE/141045", "No"));
        break;
    case FACE_GLASSES_YES:
        button->setIcon(QIcon(":/face/face/glasses.png"));
        button->setElidedText(GET_TEXT("FACE/141048", "Yes"));
        break;
    }
    button->setToolTip(button->wholeText());
}

FACE_MASK TargetInfoFace::mask() const
{
    return static_cast<FACE_MASK>(m_info.mask);
}

void TargetInfoFace::updateMaskInfo(ToolButtonFace *button)
{
    switch (m_info.mask) {
    case FACE_MASK_NONE:
        button->setIcon(QIcon(":/face/face/unrecognized.png"));
        button->setElidedText(GET_TEXT("TARGETMODE/103205", "N/A"));
        break;
    case FACE_MASK_NO:
        button->setIcon(QIcon(":/face/face/no-mask.png"));
        button->setElidedText(GET_TEXT("FACE/141045", "No"));
        break;
    case FACE_MASK_YES:
        button->setIcon(QIcon(":/face/face/mask.png"));
        button->setElidedText(GET_TEXT("FACE/141048", "Yes"));
        break;
    }
    button->setToolTip(button->wholeText());
}

FACE_CAP TargetInfoFace::cap() const
{
    return static_cast<FACE_CAP>(m_info.cap);
}

void TargetInfoFace::updateCapInfo(ToolButtonFace *button)
{
    switch (m_info.cap) {
    case FACE_CAP_NONE:
        button->setIcon(QIcon(":/face/face/unrecognized.png"));
        button->setElidedText(GET_TEXT("TARGETMODE/103205", "N/A"));
        break;
    case FACE_CAP_NO:
        button->setIcon(QIcon(":/face/face/no-hats.png"));
        button->setElidedText(GET_TEXT("FACE/141045", "No"));
        break;
    case FACE_CAP_YES:
        button->setIcon(QIcon(":/face/face/hats.png"));
        button->setElidedText(GET_TEXT("FACE/141048", "Yes"));
        break;
    }
    button->setToolTip(button->wholeText());
}

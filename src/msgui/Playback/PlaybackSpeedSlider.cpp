#include "PlaybackSpeedSlider.h"

PlaybackSpeedSlider::PlaybackSpeedSlider(QWidget *parent)
    : BaseSlider(parent)
{
    setRange(VALUE_1X, VALUE_128X);
}

PLAY_SPEED PlaybackSpeedSlider::speedValue() const
{
    int v = value();
    return static_cast<PLAY_SPEED>(v);
}

void PlaybackSpeedSlider::setSpeedValue(PLAY_SPEED value)
{
    setValue(value);
}

QString PlaybackSpeedSlider::tipText()
{
    int value = valueUnderMouse(m_mouseMovePos);
    QString text;
    switch (value) {
    case VALUE_1X:
        text = "1X";
        break;
    case VALUE_2X:
        text = "2X";
        break;
    case VALUE_4X:
        text = "4X";
        break;
    case VALUE_8X:
        text = "8X";
        break;
    case VALUE_16X:
        text = "16X";
        break;
    case VALUE_32X:
        text = "32X";
        break;
    case VALUE_64X:
        text = "64X";
        break;
    case VALUE_128X:
        text = "128X";
        break;
    }
    return text;
}

QString PlaybackSpeedSlider::valueText()
{
    QString text;
    switch (m_value) {
    case VALUE_1X:
        text = "1X";
        break;
    case VALUE_2X:
        text = "2X";
        break;
    case VALUE_4X:
        text = "4X";
        break;
    case VALUE_8X:
        text = "8X";
        break;
    case VALUE_16X:
        text = "16X";
        break;
    case VALUE_32X:
        text = "32X";
        break;
    case VALUE_64X:
        text = "64X";
        break;
    case VALUE_128X:
        text = "128X";
        break;
    }
    return text;
}

QColor PlaybackSpeedSlider::tipNormalColor()
{
    return QColor("#FFFFFF");
}

QColor PlaybackSpeedSlider::tipDisableColor()
{
    return QColor("#969696");
}

QColor PlaybackSpeedSlider::valueNormalColor()
{
    return QColor("#FFFFFF");
}

QColor PlaybackSpeedSlider::valueDisableColor()
{
    return QColor("#969696");
}

int PlaybackSpeedSlider::rightMarginWithText() const
{
    return 50;
}

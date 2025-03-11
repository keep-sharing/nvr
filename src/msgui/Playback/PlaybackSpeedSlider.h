#ifndef PLAYBACKSPEEDSLIDER_H
#define PLAYBACKSPEEDSLIDER_H

#include "baseslider.h"

extern "C" {
#include "msdefs.h"
}

class PlaybackSpeedSlider : public BaseSlider {
    Q_OBJECT

    enum VALUE {
        VALUE_1X,
        VALUE_2X,
        VALUE_4X,
        VALUE_8X,
        VALUE_16X,
        VALUE_32X,
        VALUE_64X,
        VALUE_128X
    };

public:
    explicit PlaybackSpeedSlider(QWidget *parent = nullptr);

    PLAY_SPEED speedValue() const;
    void setSpeedValue(PLAY_SPEED value);

protected:
    QString tipText() override;
    QString valueText() override;
    QColor tipNormalColor() override;
    QColor tipDisableColor() override;
    QColor valueNormalColor() override;
    QColor valueDisableColor() override;

    int rightMarginWithText() const override;

private:
signals:

private:
};

#endif // PLAYBACKSPEEDSLIDER_H

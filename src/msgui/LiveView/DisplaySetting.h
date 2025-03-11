#ifndef DISPLAYSETTING_H
#define DISPLAYSETTING_H

#include "BasePopup.h"

namespace Ui {
class DisplaySetting;
}

class DisplaySetting : public BasePopup {
    Q_OBJECT

    enum State {
        Off,
        On,
        Auto
    };

public:
    enum EventFlags {
        EventNone = 0,
        EventRegionEntrance         = 0x00000001,
        EventRegionExiting          = 0x00000002,
        EventMotionDetection        = 0x00000004,
        EventTamperDefocus          = 0x00000008,
        EventLineCrossing           = 0x00000010,
        EventLoitering              = 0x00000020,
        EventHumanDetection         = 0x00000040,
        EventPeopleCounting         = 0x00000080,
        EventObject                 = 0x00000100,
        EventRegionalPeopleCounting = 0x00000200
    };

public:
    explicit DisplaySetting(QWidget *parent = 0);
    ~DisplaySetting();

    static DisplaySetting *instance();

    void initializeData();
    void initializePlayMode();

    static QColor s_displayColor;

    //
    void setPos(const QPoint &p);
    QPoint calculatePos() override;
    void closePopup(CloseType type) override;

signals:
    void displayStateChanged(bool open);
    void borderStateChanged(bool visible, const QColor &color);
    void timeInfoModeChanged(int mode);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onLanguageChanged();
    void onDisplayInfoChanged();

    void on_comboBox_playMode_activated(int index);
    void on_comboBox_timeInfo_activated(int index);
    void on_comboBoxEventDetectionRegion_activated(int index);

    void showNoPermission();

private:
    void saveData();

private:
    static DisplaySetting *s_displaySetting;

    Ui::DisplaySetting *ui;

    QPoint m_pos;

    QList<QColor> m_colorList;
    QList<QString> m_colorNameList;
};

#endif // DISPLAYSETTING_H

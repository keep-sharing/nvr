#ifndef LIVEVIEWOCCUPANCY_H
#define LIVEVIEWOCCUPANCY_H

#include "MsWidget.h"
#include <QMap>

class LiveViewOccupancySetting;

namespace Ui {
class LiveViewOccupancy;
}

class LiveViewOccupancy : public MsWidget {
    Q_OBJECT

    struct State {
        int iconState = -1;
        int fontSize = -1;

        void clear()
        {
            iconState = -1;
            fontSize = -1;
        }
    };

public:
    explicit LiveViewOccupancy(QWidget *parent = 0);
    ~LiveViewOccupancy();

    void show();
    void close();

    void setGroup(int group);
    int group() const;
    void updateData();

protected:
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;

private:
    void resizeAllWidget();
    void updateDisplaySetting();

    QString groupName(int group);
    void clearState(int group);

private slots:
    void onLanguageChanged();

    void on_toolButtonReset_clicked();
    void on_toolButtonSetting_clicked();
    void on_toolButtonQuit_clicked();

private:
    Ui::LiveViewOccupancy *ui;

    QMap<QWidget *, QRect> m_mapWidgetRect;

    LiveViewOccupancySetting *m_setting = nullptr;

    int m_group = 0;

    bool m_isPressed = false;
    QPoint m_pressedPoint;

    //key: group
    QMap<int, State> m_states;
};

#endif // LIVEVIEWOCCUPANCY_H

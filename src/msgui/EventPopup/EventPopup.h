#ifndef EVENTPOPUP_H
#define EVENTPOPUP_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class EventPopup;
}

class EventPopup : public QWidget
{
    Q_OBJECT

public:
    explicit EventPopup(QWidget *parent = nullptr);
    ~EventPopup();

    static EventPopup *instance();

    void setPopupInfo(int screen, int seconds);
    void setPopupChannel(quint64 state, int layout, const QList<int> &channels);
    int popupScreen();

    void closePopup();

protected:
    void hideEvent(QHideEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private slots:
    void onTimeout();

private:
    static EventPopup *s_eventPopup;

    Ui::EventPopup *ui;

    int m_popupScreen = 0;
    int m_popupInterval = 0;

    QTimer *m_timer = nullptr;
};

#endif // EVENTPOPUP_H

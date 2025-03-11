#ifndef EVENTPOPUPSUB_H
#define EVENTPOPUPSUB_H

#include <QWidget>

namespace Ui {
class EventPopupSub;
}

class EventPopupSub : public QWidget
{
    Q_OBJECT

public:
    explicit EventPopupSub(QWidget *parent = nullptr);
    ~EventPopupSub();

    static EventPopupSub *instance();

    void showSubPopup();
    void closeSubPopup();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    static EventPopupSub *s_eventpopupSub;

    Ui::EventPopupSub *ui;
};

#endif // EVENTPOPUPSUB_H

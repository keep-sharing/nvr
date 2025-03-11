#ifndef EVENTNOTIFICATION_H
#define EVENTNOTIFICATION_H

#include "BasePopup.h"
#include <QTimer>
#include <QModelIndex>

extern "C" {
#include "msdb.h"
}

class MessageReceive;

namespace Ui {
class EventNotification;
}

class EventNotification : public BasePopup {
    Q_OBJECT

public:
    explicit EventNotification(QWidget *parent = 0);
    ~EventNotification();

    void setPos(const QPoint &p);
    QPoint calculatePos() override;
    void closePopup(CloseType type) override;

    void setExceptionMap(const QMap<QString, QString> &exceptionMap);
    void processMessage(MessageReceive *message);

protected:
    void ON_RESPONSE_FLAG_GET_EXCEPTION(MessageReceive *message);

    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    bool isNotificationChecked(int value);
    void makeNotificationValue(int &value, bool checked);

private slots:
    void onLanguageChanged();
    void getException();
    void on_pushButton_setting_clicked();
    void on_pushButton_ok_clicked();
    void on_treeView_setting_clicked(const QModelIndex &index);

private:
    Ui::EventNotification *ui;

    QPoint m_pos;

    trigger_alarms m_alarms;

    QTimer *m_timer = nullptr;
};

#endif // EVENTNOTIFICATION_H

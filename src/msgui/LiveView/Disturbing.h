#ifndef DISTURBING_H
#define DISTURBING_H

#include "BasePopup.h"

namespace Ui {
class Disturbing;
}

class Disturbing : public BasePopup
{
    Q_OBJECT

    enum State
    {
        Off,
        On
    };

public:
    explicit Disturbing(QWidget *parent = nullptr);
    ~Disturbing();

    void setPos(const QPoint &p);

    QPoint calculatePos() override;
    void closePopup(CloseType type) override;

protected:
    void showEvent(QShowEvent *event) override;

private:
    void saveSetting();

private slots:
    void onLanguageChanged();
    void updateState();

    void on_comboBox_audible_activated(int index);
    void on_comboBox_email_activated(int index);
    void on_comboBox_event_activated(int index);
    void on_comboBox_ptz_activated(int index);
    void on_comboBox_alarm_activated(int index);
    void on_comboBox_white_activated(int index);
    void on_comboBoxHTTP_activated(int index);

private:
    Ui::Disturbing *ui;

    QPoint m_pos;
};

#endif // DISTURBING_H

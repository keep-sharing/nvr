#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "BasePopup.h"
#include <QTimer>

namespace Ui {
class Sequence;
}

class Sequence : public BasePopup {
    Q_OBJECT

public:
    explicit Sequence(QWidget *parent = 0);
    ~Sequence();

    static Sequence *instance();

    void initializeData();
    void updateDisplayInfo();

    void stopMainSequence();
    void stopSubSequence();

    bool isSequencing(int screen);

    void networkSequence();

    //
    void setPos(const QPoint &p);
    QPoint calculatePos() override;
    void closePopup(CloseType type) override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void sequenceStateChanged(int screen, bool open);

private slots:
    void onLanguageChanged();
    void onTimeoutMain();
    void onTimeoutSub();

    void on_comboBox_interval_activated(int index);

private:
    static Sequence *s_sequence;
    Ui::Sequence *ui;

    QPoint m_pos;

    QTimer *m_timerMain;
    QTimer *m_timerSub;
};

#endif // SEQUENCE_H

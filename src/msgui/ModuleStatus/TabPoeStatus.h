#ifndef TABPOESTATUS_H
#define TABPOESTATUS_H

#include <QTimer>
#include "AbstractSettingTab.h"

class MessageReceive;
class MsWaitting;

namespace Ui {
class PagePoeStatus;
}

class TabPoeStatus : public AbstractSettingTab {
    Q_OBJECT

public:
    enum PoeColumn {
        ColumnPort,
        ColumnIP,
        ColumnPower,
        ColumnStatus
    };

    explicit TabPoeStatus(QWidget *parent = 0);
    ~TabPoeStatus();

    void initializeData();
    void processMessage(MessageReceive *message);

protected:
    void ON_RESPONSE_FLAG_GET_POE_STATE(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onLanguageChanged();

    void onTimerout();

private:
    Ui::PagePoeStatus *ui;

    int m_poeCount = 0;
    qreal m_totalPower = 0;

    QTimer *m_timer = nullptr;

    MsWaitting *m_waitting = nullptr;
};

#endif // TABPOESTATUS_H

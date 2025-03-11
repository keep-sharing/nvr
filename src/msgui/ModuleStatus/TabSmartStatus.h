#ifndef TABSMARTSTATUS_H
#define TABSMARTSTATUS_H

#include "AbstractSettingTab.h"
#include <QTimer>
#include <QWidget>

class MsWaitting;

namespace Ui {
class PageSmartStatus;
}

class TabSmartStatus : public AbstractSettingTab {
    Q_OBJECT

public:
    enum SmartColumn {
        ColumnID,
        ColumnName,
        ColumnValue,
        ColumnWorst,
        ColumnThreshold,
        ColumnType,
        ColumnStatus
    };

    explicit TabSmartStatus(QWidget *parent = 0);
    ~TabSmartStatus();

    static TabSmartStatus *instance();

    void initializeData(QList<int> portList);

    void processMessage(MessageReceive *message) override;

protected:
    void ON_RESPONSE_FLAG_GET_SMART_ATTR(MessageReceive *message);
    void ON_RESPONSE_FLAG_RET_SMART_TEST_START(MessageReceive *message);

    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onLanguageChanged();
    void onProgressChanged(int port, int progress);

    void on_comboBox_port_activated(int index);
    void on_pushButton_test_clicked();

private:
    static TabSmartStatus *s_smartStatus;
    Ui::PageSmartStatus *ui;

    MsWaitting *m_waitting;
    QTimer *m_processTimer;
};

#endif // TABSMARTSTATUS_H

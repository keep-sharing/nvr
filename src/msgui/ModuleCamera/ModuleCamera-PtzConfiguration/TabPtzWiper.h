#ifndef TABPTZWIPER_H
#define TABPTZWIPER_H

#include "ptzbasepage.h"
#include <QEventLoop>

namespace Ui {
class TabPtzWiper;
}

class TabPtzWiper : public PtzBasePage {
    Q_OBJECT

public:
    explicit TabPtzWiper(QWidget *parent = nullptr);
    ~TabPtzWiper();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;
    void ON_RESPONSE_FLAG_GET_IPC_PTZ_WIPER(MessageReceive *message);
    void ON_RESPONSE_FLAG_SET_IPC_PTZ_WIPER(MessageReceive *message);

    void setWpierEnable(bool enable);

private slots:
    void onLanguageChanged();
    void on_comboBoxChannel_activated(int index);
    void on_pushButtonCopy_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::TabPtzWiper *ui;
    int m_channel;

    QList<int> m_copyList;
    QEventLoop m_eventLoop;
};

#endif // TABPTZWIPER_H

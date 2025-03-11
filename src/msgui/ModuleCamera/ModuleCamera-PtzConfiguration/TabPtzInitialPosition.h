#ifndef TABPTZINITIALPOSITION_H
#define TABPTZINITIALPOSITION_H

#include "ptzbasepage.h"
extern "C" {
#include "msg.h"

}

namespace Ui {
class TabPtzInitialPosition;
}

class TabPtzInitialPosition : public PtzBasePage {
    Q_OBJECT

public:
    explicit TabPtzInitialPosition(QWidget *parent = nullptr);
    ~TabPtzInitialPosition();
    void initializeData() override;
    void processMessage(MessageReceive *message) override;
  private:
    void setSettingEnable(bool enable);
    void setInitialPosition(int action);

private slots:
    void onLanguageChanged();
    void onChannelGroupClicked(int channel);
    void on_pushButtonBack_clicked();

    void on_pushButtonSet_clicked();
    void on_pushButtonClear_clicked();
    void on_pushButtonCall_clicked();

  private:
    Ui::TabPtzInitialPosition *ui;
    int m_channel;
};

#endif // TABPTZINITIALPOSITION_H

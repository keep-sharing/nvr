#ifndef TABPTZCONFIGCLEAR_H
#define TABPTZCONFIGCLEAR_H

#include "ptzbasepage.h"
extern "C" {
#include "msg.h"

}
namespace Ui {
class TabPtzConfigClear;
}

class TabPtzConfigClear : public PtzBasePage {
    Q_OBJECT

public:
    explicit TabPtzConfigClear(QWidget *parent = nullptr);
    ~TabPtzConfigClear();

    void initializeData() override;
    void processMessage(MessageReceive *message) override;

  private:
    void setSettingEnable(bool enable);

  private slots:
    void onLanguageChanged();
    void on_comboBoxChannel_activated(int index);
    void on_pushButtonCopy_clicked();
    void on_pushButtonApply_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::TabPtzConfigClear *ui;
    int m_channel;

    QList<int> m_copyList;
};

#endif // TABPTZCONFIGCLEAR_H

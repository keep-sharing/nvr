#ifndef TESTHARDWARE_H
#define TESTHARDWARE_H

#include "BaseShadowDialog.h"
#include "TestWizard.h"

class AbstractTestItem;
class QProcess;

namespace Ui {
class TestHardware;
}

class TestHardware : public BaseShadowDialog
{
    Q_OBJECT
public:
    explicit TestHardware(QWidget *parent = nullptr);
    ~TestHardware() override;

    static TestHardware *instance();

    void initialize();

    void filterMessage(MessageReceive *message) override;
    void processMessage(MessageReceive *message) override;
    void setMainSubResolution(int main_resolution, int sub_resolution, int spot_resolution);
    void readConfig();
    QString updateMonitorInfo(QString message);

  private:
    void ON_RESPONSE_FLAG_GET_SYSINFO(MessageReceive *message);

private slots:
    void onShowWizardMessage(const QString &text);
    void onShowMessage(QColor color, const QString &text);
    void updateResolutionInfo();
    void onScreenSwitched();

    void onItemStarted(AbstractTestItem *item);
    void onItemChecked(bool checked);

    void onWizardNext();
    void onWizardCancel();

    void on_pushButtonStart_clicked();
    void on_pushButtonClose_clicked();

    void on_checkBoxAll_clicked(bool checked);
    void on_comboBoxOutput_activated(int index);
    void on_comboBoxHDMI2_activated(int index);
    void on_pushButtonApply_clicked();
    void on_pushButtonMonitorUpdate_clicked();

  private:
    Ui::TestHardware *ui;
    static TestHardware *s_hardWare;

    QList<AbstractTestItem *> m_testItems;
    TestWizard *m_testWizard = nullptr;
    int m_currentTestIndex = 0;
    int m_homologous;

    struct display *pDisplayDb = nullptr;
};

#endif // TESTHARDWARE_H

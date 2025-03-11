#ifndef TESTITEMCOMMON_H
#define TESTITEMCOMMON_H

#include "AbstractTestItem.h"
#include "TestHardwareData.h"

namespace Ui {
class TestItemCommon;
}

class TestItemCommon : public AbstractTestItem
{
    Q_OBJECT

public:
    explicit TestItemCommon(QWidget *parent = nullptr);
    ~TestItemCommon();

    void setModuleName(const QString &text) override;
    void startTest() override;

    void setChecked(bool newChecked) override;

private slots:
    void on_checkBox_clicked(bool checked);
    void on_toolButtonStart_clicked();

protected:
    Ui::TestItemCommon *ui;
};

#endif // TESTITEMCOMMON_H

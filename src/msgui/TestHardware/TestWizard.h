#ifndef TESTWIZARD_H
#define TESTWIZARD_H

#include "BaseShadowDialog.h"

class AbstractTestItem;

namespace Ui {
class TestWizard;
}

class TestWizard : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit TestWizard(QWidget *parent = nullptr);
    ~TestWizard();

    void startTest(AbstractTestItem *item);
    void showMessage(const QString &text);

signals:
    void testStop();
    void testNext();
    void testCancel();

private slots:
    void onTimeout();

    void on_pushButtonPass_clicked();
    void on_pushButtonNotPass_clicked();

    void on_pushButtonCancel_clicked();

private:
    Ui::TestWizard *ui;

    AbstractTestItem *m_currentItem = nullptr;
    QTimer *m_timer = nullptr;
};

#endif // TESTWIZARD_H

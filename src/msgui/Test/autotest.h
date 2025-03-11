#ifndef AUTOTEST_H
#define AUTOTEST_H

#include "BaseShadowDialog.h"

#include <QTimer>

namespace Ui {
class AutoTest;
}

class AutoTest : public BaseShadowDialog {
    Q_OBJECT

public:
    explicit AutoTest(QWidget *parent = nullptr);
    ~AutoTest();

    static AutoTest *instance();
    void stopAllTest();

private slots:
    void onTimerLayoutChange();
    void onTimerMenuChange();

    void on_pushButton_layoutChange_clicked();
    void on_pushButton_menuChange_clicked();

private:
    static AutoTest *self;

    Ui::AutoTest *ui;

    QTimer *m_timerLayoutChange = nullptr;
    quint64 m_testLayoutChangeCount = 0;
    QTimer *m_timerMenuChange = nullptr;
    quint64 m_testMenuChangeCount = 0;
    QList<QWidget *> m_itemButtonList;
};

#endif // AUTOTEST_H

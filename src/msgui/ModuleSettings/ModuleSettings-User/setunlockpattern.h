#ifndef SETUNLOCKPATTERN_H
#define SETUNLOCKPATTERN_H

#include "BaseShadowDialog.h"
#include "gusturelock.h"

namespace Ui {
class SetUnlockPattern;
}

class SetUnlockPattern : public BaseShadowDialog
{
    Q_OBJECT

public:
    explicit SetUnlockPattern(QWidget *parent = nullptr);
    ~SetUnlockPattern();

    static SetUnlockPattern *instance();
    void setText();
    QString getText() const;

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancel_clicked();
    void onDrawFinished(QString text);
    void onDrawStart();

private:
    Ui::SetUnlockPattern *ui;
    static SetUnlockPattern *s_setUnlockPattern;
    QString m_pattern_text;
    int step = 1;
};

#endif // SETUNLOCKPATTERN_H

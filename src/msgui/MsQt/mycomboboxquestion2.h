#ifndef MYCOMBOBOXQUESTION2_H
#define MYCOMBOBOXQUESTION2_H

#include <QWidget>
#include "mycomboboxquestion.h"

class MyComboBoxQuestion2 : public MyComboBoxQuestion
{
    Q_OBJECT
public:
    explicit MyComboBoxQuestion2(QWidget *parent = nullptr);

    bool isValid() const;
    void setValid(bool newValid);

    bool checkValid();

    void setTipString(const QString &str);

protected:
    virtual QString lineEditStyleSheet() const override;
    void hideEvent(QHideEvent *) override;

signals:
    void validChanged();

public slots:
    void showWarningTip();
    void hideWarningTip();

private slots:
    void onEditFinished();
    void onTimeout();
private:
    bool m_valid = true;
    QTimer *m_timer = nullptr;
    MyLineEditTip *m_invalidTip = nullptr;
};

#endif // MYCOMBOBOXQUESTION2_H

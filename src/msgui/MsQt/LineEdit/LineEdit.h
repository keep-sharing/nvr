#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class MyLineEditTip;

class LineEdit : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid WRITE setValid NOTIFY validChanged)

public:
    explicit LineEdit(QWidget *parent = nullptr);

    bool isValid() const;
    void setValid(bool newValid);
    void clearWarning();

    bool checkValid();
    void showCustomTip();

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void changeEvent(QEvent *) override;

    virtual int tipHeight() const;
    //
    virtual bool check() = 0;
    virtual QString tipString() = 0;

protected slots:
    void showWarningTip();
    void hideWarningTip();
    void onTipTimeout();

signals:
    void validChanged();

private:
    bool m_valid = true;

    QTimer *m_timerTip = nullptr;
    MyLineEditTip *m_invalidTip = nullptr;
};

#endif // LINEEDIT_H

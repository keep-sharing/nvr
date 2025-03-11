#ifndef CHECKABLELINEEDIT_H
#define CHECKABLELINEEDIT_H

#include <QWidget>

class MyLineEditTip;

namespace Ui {
class CheckableLineEdit;
}

class CheckableLineEdit : public QWidget {
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid WRITE setValid NOTIFY validChanged)

public:
    explicit CheckableLineEdit(QWidget *parent = nullptr);
    ~CheckableLineEdit();

    void setRange(int min, int max);

    void setChecked(bool checked);
    bool isChecked() const;

    void setValue(int value);
    int value() const;

    void setEnabled(bool enable);

    bool isValid() const;
    void setValid(bool newValid);
    void clearCheck();

    bool checkValid();

    void setTipString(const QString &str);

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void changeEvent(QEvent *) override;
    void paintEvent(QPaintEvent *) override;

signals:
    void validChanged();

public slots:
    void showWarningTip();
    void hideWarningTip();

private slots:
    void onEditFinished();
    void onCheckBoxStateSet(int state);
    void onTimeout();

private:
    Ui::CheckableLineEdit *ui;

    bool m_valid = true;
    QTimer *m_timer = nullptr;
    MyLineEditTip *m_invalidTip = nullptr;

    int m_minValue = 0;
    int m_maxValue = 0;
    int m_value = 0;
};

#endif // CHECKABLELINEEDIT_H

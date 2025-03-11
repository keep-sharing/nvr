#ifndef VCASIZELINEEDITBOX_H
#define VCASIZELINEEDITBOX_H

#include <QWidget>

class MyLineEditTip;

namespace Ui {
class VCASizeLineEditBox;
}

class VCASizeLineEditBox : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid WRITE setValid NOTIFY validChanged)

public:
    explicit VCASizeLineEditBox(QWidget *parent = nullptr);
    ~VCASizeLineEditBox();

    void setHeightRange(int min, int max);
    void setWidthRange(int min, int max);

    void setWidthValue(int width);
    int widthValue() ;

    void setHeightValue(int height);
    int heightValue() ;

    void setEnabled(bool enable);

    bool isValid() const;
    void setValid(bool newValid);

    bool checkValid();

    void setTipString(const QString &str);

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void changeEvent(QEvent *) override;
    void paintEvent(QPaintEvent *) override;

signals:
    void validChanged();
    void widthChange();
    void heightChange();

public slots:
    void showWarningTip();
    void hideWarningTip();

private slots:
    void onTimeout();

    void onEditFinished();
    void on_lineEditWidth_editingFinished();
    void on_lineEditHeight_editingFinished();

private:
    Ui::VCASizeLineEditBox *ui;

    bool m_valid = true;
    QTimer *m_timer = nullptr;
    MyLineEditTip *m_invalidTip = nullptr;

    int m_minWidthValue = 0;
    int m_maxWidthValue = 0;
    int m_widthValue = 0;

    int m_minHeightValue = 0;
    int m_maxHeightValue = 0;
    int m_heightValue = 0;
};

#endif // VCASIZELINEEDITBOX_H

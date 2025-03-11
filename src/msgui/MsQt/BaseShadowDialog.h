#ifndef BASESHADOWDIALOG_H
#define BASESHADOWDIALOG_H

#include "BaseDialog.h"

class BaseShadowDialog : public BaseDialog
{
    Q_OBJECT
public:
    explicit BaseShadowDialog(QWidget *parent = 0);

    void setTitleWidget(QWidget *title);

protected:
    virtual bool isDrawShadow();
    bool eventFilter(QObject *obj, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

signals:

public slots:
    virtual void onLanguageChanged();

private:
    QWidget *m_titleWidget = nullptr;
    int m_shadowWidth = 11;
    QPoint m_pressPoint;
    bool m_bPressed = false;
};

#endif // BASESHADOWDIALOG_H

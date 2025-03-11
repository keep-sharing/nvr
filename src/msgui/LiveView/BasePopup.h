#ifndef BASEPOPUP_H
#define BASEPOPUP_H

#include "BaseDialog.h"

class BasePopup : public BaseDialog
{
    Q_OBJECT
public:
    enum CloseType
    {
        CloseNormal,
        CloseWithRightButton,
        CloseWithLeftButton
    };

    explicit BasePopup(QWidget *parent = nullptr);

    void setTitleWidget(QWidget *title);

    virtual QPoint calculatePos() = 0;
    virtual void closePopup(CloseType type) = 0;

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

    NetworkResult dealRockerNvr(const RockerDirection &direction) override;

signals:

public slots:

private:
    QWidget *m_titleWidget = nullptr;
    int m_shadowWidth = 11;
    QPoint m_pressPoint;
    bool m_bPressed = false;
};

#endif // BASEPOPUP_H

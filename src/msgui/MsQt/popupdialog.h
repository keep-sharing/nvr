#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include "BaseDialog.h"

class PopupDialog : public BaseDialog
{
    Q_OBJECT
public:
    explicit PopupDialog(QWidget *parent = 0);

    void move(const QPoint &p);
    void move(int x, int y);

    void setMainWidget(QWidget *widget);

    static bool hasPopupWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

    bool isMoveToCenter() override;
    bool isAddToVisibleList() override;

    NetworkResult dealRockerNvr(const RockerDirection &direction) override;
    void escapePressed() override;

signals:

public slots:

protected:
    static int popupWindowCount;

private:
    QWidget *m_mainWidget = nullptr;
};

#endif // POPUPDIALOG_H

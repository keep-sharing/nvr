#ifndef MYDEBUGDIALOG_H
#define MYDEBUGDIALOG_H

#include "BaseShadowDialog.h"

namespace Ui {
class MyDebugDialog;
}

class MyDebugDialog : public BaseShadowDialog
{
    Q_OBJECT

    enum ResizeMode
    {
        ResizeNone,
        ResizeBottomRight
    };

public:
    explicit MyDebugDialog(QWidget *parent = nullptr);
    ~MyDebugDialog();

    static MyDebugDialog &instance();

protected:
    void closeEvent(QCloseEvent *) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:

signals:

private slots:
    void onTabBarClicked(int index);
    void onShowDebugInfo(const QString &str);

    void on_toolButton_close_clicked();
    void on_pushButton_clear_clicked();

    void on_pushButton_add_clicked();
    void on_pushButton_delete_clicked();

private:
    Ui::MyDebugDialog *ui;

    ResizeMode m_resizeMode = ResizeNone;
    QRect m_tempGeometry;
    QPoint m_pressDistance;

    bool m_showMousePos = false;
};

#endif // MYDEBUGDIALOG_H

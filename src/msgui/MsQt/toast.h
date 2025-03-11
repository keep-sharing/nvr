#ifndef TOAST_H
#define TOAST_H

#include <QDialog>
#include <QPropertyAnimation>
#include <QTimer>

namespace Ui {
class Toast;
}

class Toast : public QDialog
{
    Q_OBJECT

public:
    explicit Toast(QWidget *parent = 0);
    ~Toast();

    void setText(const QString &text);

    static void showToast(QWidget *parent, const QString &text);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void onTimeout();

private:
    Ui::Toast *ui;

    QPropertyAnimation *m_animation;
    QTimer *m_timer;
};

#endif // TOAST_H

#ifndef SIMPLETOAST_H
#define SIMPLETOAST_H

#include <QWidget>

namespace Ui {
class SimpleToast;
}

class SimpleToast : public QWidget
{
    Q_OBJECT

public:
    explicit SimpleToast(QWidget *parent = nullptr);
    ~SimpleToast();

    static void showToast(const QString &text, const QRect &globalRc);

private:
    void setText(const QString &text);

private slots:
    void onTimeout();

private:
    Ui::SimpleToast *ui;

    QTimer *m_timer;
};

#endif // SIMPLETOAST_H

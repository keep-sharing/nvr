#ifndef SPLASHDIALOG_H
#define SPLASHDIALOG_H

#include <QDialog>

namespace Ui {
class SplashDialog;
}

class SplashDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SplashDialog(QWidget *parent = 0);
    ~SplashDialog();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::SplashDialog *ui;

    QPixmap m_background;
};

#endif // SPLASHDIALOG_H

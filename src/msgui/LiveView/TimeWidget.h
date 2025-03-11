#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class TimeWidget;
}

class TimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TimeWidget(QWidget *parent = nullptr);
    ~TimeWidget();

    void setMode(int mode);
    int mode() const;

    void setBackgroundVisible(bool visible);

protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void onTimeout();

private:
    Ui::TimeWidget *ui;

    QTimer *m_timer = nullptr;
    int m_mode = 0;

    bool m_isBackgroundVisible = true;
};

#endif // TIMEWIDGET_H

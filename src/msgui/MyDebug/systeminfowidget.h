#ifndef SYSTEMINFOWIDGET_H
#define SYSTEMINFOWIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
class SystemInfoWidget;
}

class SystemInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SystemInfoWidget(QWidget *parent = nullptr);
    ~SystemInfoWidget();

protected:
    void mouseDoubleClickEvent(QMouseEvent *) override;

signals:

public slots:
    void onTimerSystem();

private slots:
    void onLanguageChanged();

private:
    Ui::SystemInfoWidget *ui;
    QTimer *m_timerSystem = nullptr;
};

#endif // SYSTEMINFOWIDGET_H

#ifndef WIDGETBACKGROUND_H
#define WIDGETBACKGROUND_H

#include <QWidget>

class WidgetBackground : public QWidget
{
    Q_OBJECT
public:
    explicit WidgetBackground(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;

signals:

};

#endif // WIDGETBACKGROUND_H

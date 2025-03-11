#ifndef WIDGETMESSAGE_H
#define WIDGETMESSAGE_H

#include <QWidget>

namespace Ui {
class WidgetMessage;
}

class WidgetMessage : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetMessage(QWidget *parent = nullptr);
    ~WidgetMessage();

    void showMessage(const QString &text);
    void hideMessage();

private:
    Ui::WidgetMessage *ui;
};

#endif // WIDGETMESSAGE_H

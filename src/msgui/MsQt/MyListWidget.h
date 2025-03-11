#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H

#include <QListWidget>
#include <QWidget>

namespace Ui {
class MyListWidget;
}

class MyListWidget : public QWidget {
    Q_OBJECT

public:
    explicit MyListWidget(QWidget *parent = nullptr);
    ~MyListWidget() override;
    void setCurrentIndex(int index);
    void addItem(QString text, int index);
    void clear();
    int currentInt();

protected:
    void hideEvent(QHideEvent *) override;

signals:
    void itemClicked(const QString &name);

private slots:
    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::MyListWidget *ui;
};

#endif // MYLISTWIDGET_H

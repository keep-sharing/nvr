#ifndef MYTIMEEDIT_H
#define MYTIMEEDIT_H

#include <QTimeEdit>

class MyTimeEdit : public QTimeEdit
{
    Q_OBJECT
public:
    explicit MyTimeEdit(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:

public slots:
};

#endif // MYTIMEEDIT_H

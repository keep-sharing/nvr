#ifndef ITEMBUTTONWIDGET_H
#define ITEMBUTTONWIDGET_H

#include <QWidget>

class ItemButtonWidget : public QWidget
{
    Q_OBJECT
public:
    enum ButtonType
    {
        ButtonEdit,
        ButtonDelete,
        ButtonPlay,
        ButtonLock
    };

    explicit ItemButtonWidget(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent = 0);

signals:
    void clicked(int row, int column, ItemButtonWidget::ButtonType type);

public slots:

protected:
    int m_row;
    int m_column;
    ButtonType m_type;
};

#endif // ITEMBUTTONWIDGET_H

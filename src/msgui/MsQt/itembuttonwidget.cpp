#include "itembuttonwidget.h"

ItemButtonWidget::ItemButtonWidget(int row, int column, ItemButtonWidget::ButtonType type, QWidget *parent) :
    QWidget(parent),
    m_row(row),
    m_column(column),
    m_type(type)
{

}


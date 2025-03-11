#ifndef MYLINEEDITPAGE_H
#define MYLINEEDITPAGE_H

#include "mylineedit.h"

class MyLineEditPage : public QLineEdit
{
    Q_OBJECT
public:
    explicit MyLineEditPage(QWidget *parent = nullptr);

    void setMaxPage(int page);

signals:

private slots:
    void onTextEdited(const QString &text);
    void onEditingFinished();

private:
    int m_maxPage = 0;
};

#endif // MYLINEEDITPAGE_H

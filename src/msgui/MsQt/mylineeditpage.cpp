#include "mylineeditpage.h"
#include <QRegExpValidator>
#include "QDebug"

MyLineEditPage::MyLineEditPage(QWidget *parent)
    : QLineEdit(parent)
{
    QRegExp rx("\\d*");
    QRegExpValidator *validator = new QRegExpValidator(rx, this);
    setValidator(validator);
    setContextMenuPolicy(Qt::NoContextMenu);

    connect(this, SIGNAL(textEdited(QString)), this, SLOT(onTextEdited(QString)));
    connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
}

void MyLineEditPage::setMaxPage(int page)
{
    m_maxPage = page;
}

void MyLineEditPage::onTextEdited(const QString &text)
{
    int page = text.toInt();
    if (page > m_maxPage) {
        setText(QString::number(m_maxPage));
    }
}

void MyLineEditPage::onEditingFinished()
{
    int page = text().toInt();
    if (page < 1) {
        setText(QString::number(1));
    }
}

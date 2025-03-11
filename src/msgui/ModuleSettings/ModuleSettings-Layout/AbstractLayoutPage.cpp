#include "AbstractLayoutPage.h"
#include "MsWaitting.h"

AbstractLayoutPage::AbstractLayoutPage(QWidget *parent)
    : QWidget(parent)
{
    m_waitting = new MsWaitting(this);
    m_waitting->setWindowModality(Qt::ApplicationModal);
    m_waitting->hide();

    setFocusPolicy(Qt::StrongFocus);
}

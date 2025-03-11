#include "maindialog.h"
#include "centralmessage.h"

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);
}

MainDialog::~MainDialog()
{

}

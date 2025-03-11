#include "BaseDialog.h"
#include "MyDebug.h"
#include "SubControl.h"
#include <QKeyEvent>

BaseDialog::BaseDialog(QWidget *parent)
    : BaseWidget(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void BaseDialog::showEvent(QShowEvent *event)
{
    BaseWidget::showEvent(event);

    if (isMoveToCenter()) {
        QPoint centerPos = SubControl::instance()->logicalMainScreenGeometry().center();
        QPoint pos(centerPos.x() - width() / 2, centerPos.y() - height() / 2);
        move(pos);

        qMsCDebug("qt_mydialog") << this << pos;
    }
}

bool BaseDialog::isMoveToCenter()
{
    return true;
}

int BaseDialog::exec()
{
    show();
    return m_dialogEventLoop.exec();
}

bool BaseDialog::close()
{
    if (m_dialogEventLoop.isRunning()) {
        m_dialogEventLoop.exit();
    }
    emit closed();
    return BaseWidget::close();
}

void BaseDialog::done(int r)
{
    if (m_dialogEventLoop.isRunning()) {
        m_dialogEventLoop.exit(r);
    }
    close();

    emit finished(r);
    if (r == Accepted) {
        emit accepted();
    } else if (r == Rejected) {
        emit rejected();
    }
}

void BaseDialog::accept()
{
    done(Accepted);
}

void BaseDialog::reject()
{
    done(Rejected);
}

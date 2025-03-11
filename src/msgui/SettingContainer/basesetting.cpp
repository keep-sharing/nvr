#include "basesetting.h"
#include <QMetaEnum>
#include <QtDebug>

BaseSetting::BaseSetting(SettingType type, QWidget *parent)
    : BaseWidget(parent)
    , m_type(type)
{
    setFocusPolicy(Qt::StrongFocus);
}

BaseSetting::~BaseSetting()
{
}

BaseSetting::SettingType BaseSetting::settingType() const
{
    return m_type;
}

void BaseSetting::initializeSetting()
{
    int index = staticMetaObject.indexOfEnumerator("SettingType");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    qDebug() << QString("Enter Setting, %1.").arg(metaEnum.valueToKey(m_type));
}

void BaseSetting::closeSetting()
{
    int index = staticMetaObject.indexOfEnumerator("SettingType");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    qDebug() << QString("Leave Setting, %1.").arg(metaEnum.valueToKey(m_type));
}

bool BaseSetting::isCloseable()
{
    return true;
}

void BaseSetting::closeCurrentPage()
{

}

bool BaseSetting::isChangeable()
{
    return true;
}

bool BaseSetting::canAutoLogout()
{
    return true;
}

void BaseSetting::dealMessage(MessageReceive *message)
{
    Q_UNUSED(message)
}

void BaseSetting::closeEvent(QCloseEvent *)
{
    emit sig_close();
}

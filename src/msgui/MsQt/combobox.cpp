#include "combobox.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include <QFile>
#include <QKeyEvent>
#include <QListView>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QtDebug>
#include "msuser.h"
ComboBox::ComboBox(QWidget *parent)
    : QComboBox(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    QListView *view = new QListView(this);
    view->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    view->setFrameShape(QFrame::NoFrame);

    setView(view);

    connect(this, SIGNAL(activated(int)), this, SLOT(onActivated(int)));
}

void ComboBox::addTranslatableItem(const QString &key, const QString &defaultValue, const QVariant &userData)
{
    int index = count();
    if (!key.isEmpty()) {
        m_languageMap.insert(index, qMakePair(key, defaultValue));
    }

    const QString &value = MsLanguage::instance()->value(key, defaultValue);
    addItem(value, userData);
}

void ComboBox::retranslate()
{
    for (int i = 0; i < count(); ++i) {
        if (m_languageMap.contains(i)) {
            QPair<QString, QString> language = m_languageMap.value(i);
            const QString &key = language.first;
            const QString &value = language.second;
            const QString &text = MsLanguage::instance()->value(key, value);
            setItemText(i, text);
        }
    }
}

void ComboBox::setCurrentIndexFromData(const QVariant &data, int role)
{
    int index = findData(data, role);
    setCurrentIndex(index);
}

QVariant ComboBox::currentData(int role)
{
    int index = currentIndex();
    return itemData(index, role);
}

int ComboBox::currentIntData(int role)
{
    int index = currentIndex();
    if (index < 0) {
        return -1;
    } else {
        return itemData(index, role).toInt();
    }
}

void ComboBox::setCurrentIndex(int index)
{
    QComboBox::setCurrentIndex(index);
    //
    if (!m_isEditing) {
        emit indexSet(index);
    }
}

void ComboBox::beginEdit()
{
    m_isEditing = true;
}

void ComboBox::endEdit()
{
    m_isEditing = false;
}

void ComboBox::editCurrentIndexFromData(const QVariant &data, int role)
{
    beginEdit();
    setCurrentIndexFromData(data, role);
    endEdit();
}

void ComboBox::removeItem(int index)
{
    m_languageMap.remove(index);
    QComboBox::removeItem(index);
}

void ComboBox::removeItemFromData(const QVariant &data, int role)
{
    int index = findData(data, role);
    m_languageMap.remove(index);
    removeItem(index);
}

void ComboBox::clear()
{
    m_languageMap.clear();

    QComboBox::clear();
}

void ComboBox::reActivateIndex()
{
    int index = currentIndex();
    emit activated(index);
}

void ComboBox::reSetIndex()
{
    int index = currentIndex();
    emit indexSet(index);
}

void ComboBox::setPermission(int mode, int permission)
{
    m_checkPermission = true;
    m_mode = mode;
    m_permission = permission;
}

void ComboBox::keyPressEvent(QKeyEvent *e)
{
    int key = e->key();
    if (key == Qt::Key_Return) {
        m_isEnterPressed = !m_isEnterPressed;
    } else if (key == Qt::Key_Up || key == Qt::Key_Down) {
        //抛给父窗口去处理，要不然当焦点在myQcomboBox上，父窗口上下按键不能移动焦点。
        e->ignore();
    } else {
        QComboBox::keyPressEvent(e);
    }
    if (m_isEnterPressed) {
        QKeyEvent myEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        QComboBox::keyPressEvent(&myEvent);
        m_isEnterPressed = false;
    }
}

void ComboBox::mousePressEvent(QMouseEvent *e)
{
    if (m_checkPermission && e->button() == Qt::LeftButton) {
        if (!gMsUser.checkBasicPermission(m_mode, m_permission)) {
            emit showNoPermission();
            return;
        }
    }
    QComboBox::mousePressEvent(e);
}

void ComboBox::onActivated(int index)
{
    if (m_isEditing) {
        return;
    }
    emit indexSet(index);
}

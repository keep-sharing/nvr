#include "KeyboardPolski.h"
#include "ui_KeyboardPolski.h"
#include "KeyboardData.h"
#include "MyDebug.h"
#include <QWSServer>

KeyboardPolski::KeyboardPolski(QWidget *parent)
    : AbstractKeyboard(parent)
{
    ui = new Ui::KeyboardPolski;
    ui->setupUi(this);

    auto keys = findChildren<KeyButton *>();
    for (int i = 0; i < keys.size(); ++i) {
        auto *key = keys.at(i);
        connect(key, SIGNAL(keyClicked(QString)), this, SLOT(onKeyButtonClicked(QString)));
        QString name = key->initialize(this);
        m_keyMap.insert(name, key);
    }
}

KeyboardPolski::~KeyboardPolski()
{
    delete ui;
}

QString KeyboardPolski::buttonText(const QString &name) const
{
    const auto &key = gKeyboardData.key(name);

    if (key.hasGlobalText()) {
        return key.gtext;
    }

    //
    QString text;
    const auto &value = key.currentValue();
    if (value.v != 0) {
        switch (value.v) {
        default:
            text = QString(QChar(value.v));
            break;
        }
    } else if (value.c != KeyboardData::C_None) {
        text = value.ctext;
        if (text.isEmpty()) {
            qMsWarning() << "error ctext:" << name;
        }
    } else if (value.k != Qt::Key_unknown) {
        switch (value.k) {
        default:
            qMsWarning() << "error key value:" << value.k;
            break;
        }
    } else {
    }
    return text;
}

void KeyboardPolski::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
}

void KeyboardPolski::hideEvent(QHideEvent *event)
{
    gKeyboardData.removeAllControls();
    QWidget::hideEvent(event);
}

void KeyboardPolski::dealControls(const KeyboardData::Controls &cs)
{
    switch (cs) {
    case KeyboardData::C_Shift_QuoteLeft:
    case KeyboardData::C_Control_2:
    case KeyboardData::C_Control_3:
    case KeyboardData::C_Control_4:
    case KeyboardData::C_Control_5:
    case KeyboardData::C_Control_6:
    case KeyboardData::C_Control_8:
    case KeyboardData::C_Control_9:
    case KeyboardData::C_Control_0:
    case KeyboardData::C_Control_Minus:
    case KeyboardData::C_Control_Equal:
        gKeyboardData.removeControls(KeyboardData::C_Shift);
        gKeyboardData.removeControls(KeyboardData::C_Control);
        gKeyboardData.removeControls(KeyboardData::C_AltGr);
        break;
    default:
        break;
    }

    if (gKeyboardData.currentControls() & cs) {
        gKeyboardData.removeControls(cs);
    } else {
        gKeyboardData.addControls(cs);
        //ESc按下会清除Caps Lock
        if (cs & KeyboardData::C_Esc) {
            gKeyboardData.removeControls(KeyboardData::C_CapsLock);
        }
    }
    gKeyboardData.updateAllKeys();
}

void KeyboardPolski::dealKeyEvent(const Qt::Key &k)
{
    QWSServer::sendKeyEvent(k, k, Qt::NoModifier, true, false);
}

void KeyboardPolski::dealKeyValue(const ushort &s)
{
    QWSServer::sendKeyEvent(s, s, Qt::NoModifier, true, false);
}

void KeyboardPolski::onKeyButtonClicked(const QString &name)
{
    const auto &key = gKeyboardData.key(name);

    if (name == "Close") {
        emit sigClose();
        return;
    }

    //特殊按键处理
    if (key.hasGlobalControl()) {
        dealControls(key.gc);
        return;
    }
    if (key.hasGlobalKey()) {
        dealKeyEvent(key.gk);
        return;
    }
    if (key.hasGlobalValue()) {
        dealKeyValue(key.gv);
        return;
    }

    //按照配置文件处理
    const auto &value = key.currentValue();
    if (value.v != 0) {
        QWSServer::sendKeyEvent(value.v, value.v, Qt::NoModifier, true, false);
        if (gKeyboardData.hasControls()) {
            gKeyboardData.removeSingleControls();
            gKeyboardData.updateAllKeys();
        }
    } else if (value.c != KeyboardData::C_None) {
        dealControls(value.c);
    } else if (value.k != Qt::Key_unknown) {
        dealKeyEvent(value.k);
    } else {
        gKeyboardData.removeSingleControls();
        gKeyboardData.updateAllKeys();
    }
}

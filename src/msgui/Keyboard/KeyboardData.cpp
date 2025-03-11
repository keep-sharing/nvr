#include "KeyboardData.h"
#include "MsLanguage.h"
#include "MyDebug.h"
#include <QFile>
#include <QMetaEnum>
#include <QRegExp>
#include <QTextStream>

QDebug operator<<(QDebug dbg, const KeyboardData::Controls &cs)
{
    QString text;
    if (cs == KeyboardData::C_None) {
        text += "C_None";
    } else {
        if (cs & KeyboardData::C_Esc) {
            text += "C_Esc|";
        }
        if (cs & KeyboardData::C_QuoteLeft) {
            text += "C_QuoteLeft|";
        }
        if (cs & KeyboardData::C_CapsLock) {
            text += "C_CapsLock|";
        }
        if (cs & KeyboardData::C_Shift) {
            text += "C_Shift|";
        }
        if (cs & KeyboardData::C_Control) {
            text += "C_Control|";
        }
        if (cs & KeyboardData::C_Alt) {
            text += "C_Alt|";
        }
        if (cs & KeyboardData::C_AltGr) {
            text += "C_AltGr|";
        }
        if (cs & KeyboardData::C_Shift_QuoteLeft) {
            text += "C_Shift_QuoteLeft|";
        }
        if (cs & KeyboardData::C_Control_2) {
            text += "C_Control_2|";
        }
        if (cs & KeyboardData::C_Control_3) {
            text += "C_Control_3|";
        }
        if (cs & KeyboardData::C_Control_4) {
            text += "C_Control_4|";
        }
        if (cs & KeyboardData::C_Control_5) {
            text += "C_Control_5|";
        }
        if (cs & KeyboardData::C_Control_6) {
            text += "C_Control_6|";
        }
        if (cs & KeyboardData::C_Control_8) {
            text += "C_Control_8|";
        }
        if (cs & KeyboardData::C_Control_9) {
            text += "C_Control_9|";
        }
        if (cs & KeyboardData::C_Control_0) {
            text += "C_Control_0|";
        }
        if (cs & KeyboardData::C_Control_Minus) {
            text += "C_Control_Minus|";
        }
        if (cs & KeyboardData::C_Control_Equal) {
            text += "C_Control_Equal|";
        }
        text.chop(1);
    }
    dbg.nospace() << QString("KeyboardData::Controls(0x%1, %2)").arg(cs, 0, 16).arg(text).toLocal8Bit().data();

    return dbg.space();
}

KeyboardData::KeyboardData(QObject *parent)
    : QObject(parent)
{
}

KeyboardData &KeyboardData::instance()
{
    static KeyboardData self;
    return self;
}

void KeyboardData::initialize(int language_id)
{
    QString path;
    switch (language_id) {
    case MsLanguage::LAN_PL:
        path = ":/keyboard/keyboard/keyboard_polski.txt";
        break;
    default:
        return;
    }
    if (path.isEmpty()) {
        qMsCritical() << "keyboard file error.";
        return;
    }

    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qMsCritical() << "keyboard file error:" << file.errorString();
        return;
    }

    QString group;

    QTextStream stream(&file);
    QString line;
    int lineNum = 0;
    do {
        line = stream.readLine();
        lineNum++;
        if (line.startsWith("#")) {
            continue;
        }

        QRegExp rxGroup(R"(\[(.+)\])");
        if (rxGroup.indexIn(line) != -1) {
            group = rxGroup.cap(1);
        } else {
            QStringList texts = line.split("=", QString::SkipEmptyParts);
            if (texts.size() != 2) {
                continue;
            }

            //
            auto &key = m_keyDataMap[group];

            if (line.startsWith("GlobalControl")) {
                QStringList list = texts.at(1).split("+", QString::SkipEmptyParts);
                for (int i = 0; i < list.size(); ++i) {
                    key.gc |= stringToControl(list.at(i));
                }
            } else if (line.startsWith("GlobalKey")) {
                key.gk = stringToKey(texts.at(1));
            } else if (line.startsWith("GlobalValue")) {
                key.gv = stringToUnicode(texts.at(1));
            } else if (line.startsWith("GlobalText")) {
                key.gtext = texts.at(1);
            } else {
                //
                Controls cs;
                QStringList cList = texts.at(0).split("+", QString::SkipEmptyParts);
                for (int i = 0; i < cList.size(); ++i) {
                    QString str = cList.at(i);
                    cs |= stringToControl(str);
                }
                //
                Value vs;
                QStringList vList = texts.at(1).split("+", QString::SkipEmptyParts);
                for (int i = 0; i < vList.size(); ++i) {
                    QString str = vList.at(i);
                    if (str.startsWith(R"(C_)")) {
                        vs.c |= stringToControl(str);
                    } else if (str.startsWith(R"(T_)")) {
                        vs.ctext = stringToUnicodeString(str.remove("T_"));
                    } else if (str.startsWith(R"(Qt::Key)")) {
                        vs.k = stringToKey(str);
                    } else if (str.startsWith(R"(\u)")) {
                        vs.v = stringToUnicode(str);
                    } else {
                        qMsWarning() << "error text:" << str;
                    }
                }
                //
                key.valueMap.insert(cs, vs);
            }
        }
    } while (!line.isNull());
}

KeyboardData::KeyData KeyboardData::key(const QString &name) const
{
    return m_keyDataMap.value(name);
}

KeyboardData::Controls KeyboardData::currentControls()
{
    return m_currentControls;
}

bool KeyboardData::hasControls()
{
    return m_currentControls != C_None;
}

void KeyboardData::addControls(const Controls &cs)
{
    m_currentControls |= cs;

    qMsDebug() << m_currentControls;
}

void KeyboardData::removeControls(const Controls &cs)
{
    m_currentControls &= ~cs;

    qMsDebug() << m_currentControls;
}

void KeyboardData::removeAllControls()
{
    m_currentControls = C_None;

    qMsDebug() << m_currentControls;
}

void KeyboardData::removeSingleControls()
{
    bool caps = m_currentControls & C_CapsLock;
    bool esc  = m_currentControls & C_Esc;

    m_currentControls = C_None;

    if (caps) {
        m_currentControls |= C_CapsLock;
    }
    if (esc) {
        m_currentControls |= C_Esc;
    }

    qMsDebug() << m_currentControls;
}

void KeyboardData::updateAllKeys()
{
    emit sigUpdateAllKeys();
}

KeyboardData::Control KeyboardData::stringToControl(const QString &str)
{
    int index          = staticMetaObject.indexOfEnumerator("Control");
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    int value          = metaEnum.keyToValue(str.toStdString().c_str());
    if (value < 0) {
        qMsWarning() << "error str:" << str;
    }

    Control c = static_cast<Control>(value);
    return c;
}

Qt::Key KeyboardData::stringToKey(const QString &str)
{
    if (str == "Qt::Key_Backspace") {
        return Qt::Key_Backspace;
    }
    if (str == "Qt::Key_Tab") {
        return Qt::Key_Tab;
    }
    if (str == "Qt::Key_Return") {
        return Qt::Key_Return;
    }
    return Qt::Key_unknown;
}

ushort KeyboardData::stringToUnicode(const QString &str)
{
    QString text = str;
    text.remove(R"(\u)");
    bool ok;
    ushort s = text.toUShort(&ok, 16);
    if (!ok) {
        qMsWarning() << "error unicode:" << str;
    }
    return s;
}

QString KeyboardData::stringToUnicodeString(const QString &str)
{
    QChar c(stringToUnicode(str));
    return QString(c);
}

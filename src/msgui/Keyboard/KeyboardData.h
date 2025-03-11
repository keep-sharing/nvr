#ifndef KEYBOARDDATA_H
#define KEYBOARDDATA_H

#include <QMap>
#include <QObject>

#define gKeyboardData KeyboardData::instance()

class KeyboardData : public QObject {
    Q_OBJECT

public:
    enum Control {
        C_None            = 0x00000000,
        C_Esc             = 0x00000001,
        C_QuoteLeft       = 0x00000002,
        C_CapsLock        = 0x00000004,
        C_Shift           = 0x00000008,
        C_Control         = 0x00000010,
        C_Alt             = 0x00000020,
        C_AltGr           = 0x00000040,
        C_Shift_QuoteLeft = 0x00000080,
        C_Control_2       = 0x00000100,
        C_Control_3       = 0x00000200,
        C_Control_4       = 0x00000400,
        C_Control_5       = 0x00000800,
        C_Control_6       = 0x00001000,
        C_Control_8       = 0x00002000,
        C_Control_9       = 0x00004000,
        C_Control_0       = 0x00008000,
        C_Control_Minus   = 0x00010000,
        C_Control_Equal   = 0x00020000
    };
    Q_ENUMS(Control)
    Q_DECLARE_FLAGS(Controls, Control)

    enum ValueType {
        V_None,    //
        V_Control, //控制建，会改变键盘输入的
        V_Key,     //删除回车等，改变已输入的内容
        V_Value    //要输入的字符
    };
    struct Value {
        ValueType type = V_None;
        Controls c     = C_None;
        Qt::Key k      = Qt::Key_unknown;
        ushort v       = 0;
        QString ctext; //作为控制键时要显示的内容
    };

    struct KeyData {
        Controls gc = C_None;           //全局控制键，比如Ctrl，永远就是Ctrl，不会根据模式发生改变，避免配置文件麻烦
        Qt::Key gk  = Qt::Key_unknown;  //全局按键
        ushort gv   = 0;                //全局字符，如空格
        QString gtext;                  //全局文字，按键上显示的字永远不会变的
        QMap<Controls, Value> valueMap; //按键在不同模式下对应的Value

        bool hasGlobalControl() const
        {
            return gc != C_None;
        }
        bool hasGlobalKey() const
        {
            return gk != Qt::Key_unknown;
        }
        bool hasGlobalValue() const
        {
            return gv != 0;
        }
        bool hasGlobalText() const
        {
            return !gtext.isEmpty();
        }
        bool isControl() const
        {
            const auto &cs = gKeyboardData.currentControls();
            if (cs & gc) {
                return true;
            }
            const auto &value = valueMap.value(cs);
            if (cs & value.c) {
                return true;
            }
            return false;
        }
        Value currentValue() const
        {
            return valueMap.value(gKeyboardData.currentControls());
        }
    };

public:
    explicit KeyboardData(QObject *parent = nullptr);

    static KeyboardData &instance();

    void initialize(int language_id);

    KeyData key(const QString &name) const;

    Controls currentControls();
    bool hasControls();
    void addControls(const Controls &cs);
    void removeControls(const Controls &cs);
    void removeAllControls();
    void removeSingleControls();

    void updateAllKeys();

private:
    Control stringToControl(const QString &str);
    Qt::Key stringToKey(const QString &str);
    ushort stringToUnicode(const QString &str);
    QString stringToUnicodeString(const QString &str);

signals:
    void sigUpdateAllKeys();

private:
    QMap<QString, KeyData> m_keyDataMap;

    Controls m_currentControls = C_None;
};

QDebug operator<<(QDebug dbg, const KeyboardData::Controls &cs);

#endif // KEYBOARDDATA_H

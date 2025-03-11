#include "CustomLayoutKey.h"
#include <QRegExp>
#include <QtDebug>

extern "C" {
#include "msdb.h"
#include "vapi.h"
}

QDebug operator<<(QDebug debug, const CustomLayoutKey &c)
{
    debug.nospace() << QString("CustomLayoutKey(name: %1, screen: %2, type: %3)")
                           .arg(c.name())
                           .arg(c.screenString())
                           .arg(c.typeString());
    return debug.space();
}

CustomLayoutKey::CustomLayoutKey()
{
}

CustomLayoutKey::CustomLayoutKey(const QString &name, int screen, int type)
    : m_name(name)
    , m_screen(screen)
    , m_type(static_cast<CustomLayoutKey::LayoutType>(type))
{
}

CustomLayoutKey::CustomLayoutKey(const QString &name, int screen, LayoutType type)
    : m_name(name)
    , m_screen(screen)
    , m_type(type)
{
}

CustomLayoutKey::CustomLayoutKey(const layout_custom &layout)
    : m_name(layout.name)
    , m_screen(layout.screen)
    , m_type(static_cast<CustomLayoutKey::LayoutType>(layout.type))
{
}

QString CustomLayoutKey::name() const
{
    return m_name;
}

void CustomLayoutKey::setName(const QString &name)
{
    m_name = name;
}

int CustomLayoutKey::screen() const
{
    return m_screen;
}

void CustomLayoutKey::setScreen(int screen)
{
    m_screen = screen;
}

QString CustomLayoutKey::screenString() const
{
    QString text;
    switch (m_screen) {
    case SCREEN_MAIN:
        text = "SCREEN_MAIN";
        break;
    case SCREEN_SUB:
        text = "SCREEN_SUB";
        break;
    }
    return text;
}

CustomLayoutKey::LayoutType CustomLayoutKey::type() const
{
    return m_type;
}

QString CustomLayoutKey::typeString() const
{
    QString text;
    switch (m_type) {
    case DefaultType:
        text = "Layout_Default";
        break;
    case CustomType:
        text = "Layout_Custom";
        break;
    }
    return text;
}

bool CustomLayoutKey::isValid() const
{
    if (name().isEmpty()) {
        return false;
    }
    return true;
}

bool CustomLayoutKey::operator<(const CustomLayoutKey &other) const
{
    if (type() != other.type()) {
        return type() < other.type();
    }
    if (screen() != other.screen()) {
        return screen() < other.screen();
    }
    int id1 = 0;
    int id2 = 0;
    QRegExp rx(R"(Custom Layout (\d+))");
    if (rx.exactMatch(name())) {
        id1 = rx.cap(1).toInt();
    }
    if (rx.exactMatch(other.name())) {
        id2 = rx.cap(1).toInt();
    }
    if (id1 > 0 && id2 > 0) {
        return id1 < id2;
    }
    if (id1 > 0) {
        return true;
    }
    if (id2 > 0) {
        return false;
    }
    return name() < other.name();
}

bool CustomLayoutKey::operator==(const CustomLayoutKey &other) const
{
    return name() == other.name() && screen() == other.screen() && type() == other.type();
}

bool CustomLayoutKey::operator!=(const CustomLayoutKey &other) const
{
    return name() != other.name() || screen() != other.screen() || type() != other.type();
}

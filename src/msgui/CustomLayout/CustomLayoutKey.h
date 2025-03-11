#ifndef CUSTOMLAYOUTKEY_H
#define CUSTOMLAYOUTKEY_H

#include <QString>
#include <QMetaType>

class CustomLayoutKey;
class layout_custom;

QDebug operator<<(QDebug debug, const CustomLayoutKey &c);

class CustomLayoutKey
{
public:
    enum LayoutType
    {
        DefaultType,
        CustomType
    };

    explicit CustomLayoutKey();
    explicit CustomLayoutKey(const QString &name, int screen, int type = CustomType);
    explicit CustomLayoutKey(const QString &name, int screen, LayoutType type);
    explicit CustomLayoutKey(const layout_custom &layout);

    QString name() const;
    void setName(const QString &name);
    int screen() const;
    void setScreen(int screen);
    QString screenString() const;
    LayoutType type() const;
    QString typeString() const;

    bool isValid() const;

    bool operator <(const CustomLayoutKey &other) const;
    bool operator ==(const CustomLayoutKey &other) const;
    bool operator !=(const CustomLayoutKey &other) const;

private:
    QString m_name;
    int m_screen = 0;
    LayoutType m_type = CustomType;
};

Q_DECLARE_METATYPE(CustomLayoutKey)

#endif // CUSTOMLAYOUTKEY_H

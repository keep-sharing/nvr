#ifndef StreamKey_H
#define StreamKey_H

#include <QString>
#include <QMetaType>

class layout_custom;
class StreamKey;

QDebug operator<<(QDebug debug, const StreamKey &c);

class StreamKey
{
public:
    explicit StreamKey();
    explicit StreamKey(int channel, int screen);

    int currentChannel() const;
    void currentChannel(const int channel);
    int screen() const;
    void setScreen(int screen);
    QString screenString() const;

    bool operator <(const StreamKey &other) const;
    bool operator ==(const StreamKey &other) const;
    bool operator !=(const StreamKey &other) const;

private:
    int m_currentChannel = -1;
    int m_screen = 0;
};
Q_DECLARE_METATYPE(StreamKey)
#endif // StreamKey_H

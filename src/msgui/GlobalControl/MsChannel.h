#ifndef MSCHANNEL_H
#define MSCHANNEL_H


class MsChannel
{
public:
    explicit MsChannel();
    explicit MsChannel(int screen, int channel);

    bool valid() const;

    int screen() const;
    int channel() const;

private:
    int m_screen = -1;
    int m_channel = -1;
};

#endif // MSCHANNEL_H

#ifndef MYQT_H
#define MYQT_H

#include <QObject>
#include <QRect>

class MyQt : public QObject
{
    Q_OBJECT
public:
    explicit MyQt(QObject *parent = 0);

    static QRect marginsRect(const QRect &rect, int left, int top, int right, int bottom);

    static bool isValidIP(const QString &ip);
    static bool isValidDomain(const QString &domain);
    static bool isValidIPv6(const QString &ipv6);
    static bool isValidAddress(const QString &address);
    static bool isSubnetMask(const QString netmask);

    static int isFolderNameValid(const QString &name);
    static int isPasswordValid(const QString &password);

    //
    static void makeChannelMask(QList<int> list, char *mask, int size);
    static void makeChannelMask(int channel, char *mask, int size);

    //
    static QString weekString(int week);

signals:

public slots:
};

#endif // MYQT_H

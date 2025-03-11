#include "myqt.h"
#include "MsDevice.h"
#include <QRegExp>

MyQt::MyQt(QObject *parent)
    : QObject(parent)
{
}

QRect MyQt::marginsRect(const QRect &rect, int left, int top, int right, int bottom)
{
    QRect rc;
    rc.setLeft(rect.left() - left);
    rc.setTop(rect.top() - top);
    rc.setRight(rect.right() + right);
    rc.setBottom(rect.bottom() + bottom);
    return rc;
}

bool MyQt::isValidIP(const QString &ip)
{
    QRegExp rx("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    return rx.exactMatch(ip);
}

bool MyQt::isValidDomain(const QString &domain)
{
    QRegExp rx("^(\\w+(-\\w+)*)(\\.(\\w+(-\\w+)*))*(\\.[a-zA-Z]{2,6})$");
    return rx.exactMatch(domain);
}

bool MyQt::isValidIPv6(const QString &ipv6)
{
    return (net_is_validipv6(ipv6.toStdString().c_str()) == 0) ? (true) : (false);
}

bool MyQt::isValidAddress(const QString &address)
{
    if (isValidIP(address) || isValidDomain(address) || isValidIPv6(address)) {
        return true;
    } else {
        return false;
    }
}

bool MyQt::isSubnetMask(const QString netmask)
{
    if (isValidIP(netmask)) {
        unsigned int b = 0, i, n[4];
        sscanf(netmask.toStdString().c_str(), "%u.%u.%u.%u", &n[3], &n[2], &n[1], &n[0]);
        //qDebug()<<"n[3]="<<n[3]<< ", n[2]="<<n[2]<<",n[1]="<<n[1]<<", n[0]="<<n[0];
        for (i = 0; i < 4; ++i) //将子网掩码存入32位无符号整型
            b += n[i] << (i * 8);
        b = ~b + 1;
        if ((b & (b - 1)) == 0) //判断是否为2^n
            return true;
    }
    return false;
}

int MyQt::isFolderNameValid(const QString &name)
{
    static QRegExp rx("[\\\\/:\\*\\?\"<>|]");
    if (name.contains(rx)) {
        return -1;
    }

    if (!name.isEmpty()) {
        if (name.at(0) == QChar('.') || name.endsWith(".")) {
            return -2;
        }
    }

    return 0;
}

int MyQt::isPasswordValid(const QString &password)
{
    static QRegExp rx(QString("[\\+&:/'\\\\;\\?\" ]"));
    if (password.contains(rx)) {
        return -1;
    }

    return 0;
}

void MyQt::makeChannelMask(QList<int> list, char *mask, int size)
{
    memset(mask, 0, size);

    for (int i = 0; i < size; ++i) {
        if (list.contains(i)) {
            mask[i] = '1';
        } else {
            mask[i] = '0';
        }
    }
}

void MyQt::makeChannelMask(int channel, char *mask, int size)
{
    memset(mask, 0, size);

    for (int i = 0; i < size; ++i) {
        if (i == channel) {
            mask[i] = '1';
        } else {
            mask[i] = '0';
        }
    }
}

QString MyQt::weekString(int week)
{
    QString text;
    switch (week) {
    case 1:
        text = "Monday";
        break;
    case 2:
        text = "Tuesday";
        break;
    case 3:
        text = "Wednesday";
        break;
    case 4:
        text = "Thursday";
        break;
    case 5:
        text = "Friday";
        break;
    case 6:
        text = "Saturday";
        break;
    case 7:
        text = "Sunday";
        break;
    }
    return text;
}

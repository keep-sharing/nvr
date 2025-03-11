#ifndef MYLINEEDIT_H
#define MYLINEEDIT_H

#include "RtspInfo.h"
#include <QLineEdit>

class MyLineEditTip;
class QToolButton;
class QLabel;

class MyLineEdit : public QLineEdit {
    Q_OBJECT
    Q_PROPERTY(bool valid READ isValid WRITE setValid NOTIFY validChanged)

public:
    enum CheckMode {
        NoCheck,
        IPCheck, //ipv4 | ipv6
        IPv4Check,
        IPv6Check,
        RtspCheck,
        RangeCheck,
        RangeCanEmptyCheck,
        SpecialRangeCheck, //允许特定值的范围检测
        UserNameCheck, //特殊字符检测 & 为空检测
        EmptyCheck,
        PasswordCheck,
        IPv4CanEmptyCheck, //允许为空的IP检测
        IPv6CanEmptyCheck, //允许为空的IP检测
        SpecialStrCheck, //不能含有特殊字符的检测
        DDNSCheck,
        EmailCheck,
        EmailCanEmptyCheck,
        MACCheck,
        ServerCheck,
        SnmpCheck,
        SnmpPasswordCheck,
        FolderNameCheck,
        NasDirectoryCheck,
        WizardNetCheck,
        WizardNetCanEmptyCheck,
        PTZRangeCheck,
        AudioCheck //允许只读状态下也显示
    };

    explicit MyLineEdit(QWidget *parent = nullptr);
    ~MyLineEdit() override;

    CheckMode checkMode() const;
    void setCheckMode(CheckMode newCheckMode);
    void setCheckMode(CheckMode newCheckMode, int min, int max);
    void setCheckMode(CheckMode newCheckMode, int min, int max, int val);
    bool checkValid();
    void setCustomValid(bool valid, const QString &tip = QString());

    bool isValid() const;
    void setValid(bool newValid);

    void setTipString(const QString &str);
    void setTipFronSize(const int size);

    RtspInfo rtspInfo() const;

    bool checkMail(const char *pszEmail);
    bool checkSpecialStr(const QString &str);
    bool checkMACAddress(const QString &str);
    bool checkSmnp(const QString &str);
    bool checkTooLong();

protected:
    void hideEvent(QHideEvent *) override;
    void changeEvent(QEvent *event) override;
    void moveEvent(QMoveEvent *) override;

signals:
    void validChanged();

public slots:
    void showWarningTip();
    void hideWarningTip();

private slots:
    void onEditFinished();
    void onTimeout();

private:
    CheckMode m_checkMode = NoCheck;

    bool m_valid = true;

    RtspInfo m_rtspInfo;

    int m_minValue = 0;
    int m_maxValue = 0;
    int m_value = 0;

    QTimer *m_timer;
    MyLineEditTip *m_invalidTip = nullptr;
};

#endif // MYLINEEDIT_H

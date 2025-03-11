#ifndef ABSTRACTSETTINGPAGE_H
#define ABSTRACTSETTINGPAGE_H

#include "MsWidget.h"
#include <QMap>
#include "networkcommond.h"

class MessageReceive;
class AbstractSettingTab;

class AbstractSettingPage : public MsWidget
{
    Q_OBJECT
public:
    explicit AbstractSettingPage(QWidget *parent = 0);

    void back();

    void showWait(QWidget *parent = nullptr);
    void execWait(QWidget *parent = nullptr);
    void closeWait();

    virtual void initializeData();
    virtual void dealMessage(MessageReceive *message);

    virtual bool isCloseable();
    virtual void closePage();
    virtual bool isChangeable();
    virtual bool canAutoLogout();

    virtual NetworkResult dealNetworkCommond(const QString &commond);

signals:
    void sig_back();
    void updateVideoGeometry();

public slots:
    virtual void onLanguageChanged();

protected:
    int m_currentTab = 0;
    QMap<int, AbstractSettingTab *> m_tabMap;
};

#endif // ABSTRACTSETTINGPAGE_H

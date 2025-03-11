#ifndef ABSTRACTNETWORKPAGE_H
#define ABSTRACTNETWORKPAGE_H

#include "AbstractSettingTab.h"

class AbstractNetworkPage : public AbstractSettingTab {
    Q_OBJECT

public:
    explicit AbstractNetworkPage(QWidget *parent = nullptr);

protected:
    int get_mac_addr(char mac0[32], char mac1[32]);

public slots:

protected:
    char mac0[32], mac1[32];
};

#endif // ABSTRACTNETWORKPAGE_H

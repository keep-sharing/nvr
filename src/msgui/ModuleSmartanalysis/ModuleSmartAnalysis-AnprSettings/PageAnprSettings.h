#ifndef PAGEANPRSETTINGS_H
#define PAGEANPRSETTINGS_H

#include "AbstractSettingPage.h"
#include <QGraphicsItem>

class AnprBasePage;

namespace Ui {
class AnprSetting;
}

class PageAnprSettings : public AbstractSettingPage {
    Q_OBJECT

    enum AnprItem {
        ItemSetting,
        ItemListManagement,
        ItemBlackList,
        ItemWhiteList,
        ItemVisitor,
        ItemAutoBackup,
        ItemNone
    };

public:
    explicit PageAnprSettings(QWidget *parent = nullptr);
    ~PageAnprSettings();

    void setDrawWidget(QWidget *widget);
    void addGraphicsItem(QGraphicsItem *item);

    bool isChannelConnected(int channel);

    virtual void initializeData() override;
    virtual void processMessage(MessageReceive *message) override;

private:
    void ON_RESPONSE_FLAG_GET_LPR_SUPPORT(MessageReceive *message);

    void selectChannel(int channel);

private slots:
    void onLanguageChanged() override;

    void onTabClicked(int index);
    void onChannelClicked(int channel);

private:
    Ui::AnprSetting *ui;

    int m_currentChannel = -1;

    AnprItem m_currentItem = ItemNone;
    QMap<int, AnprBasePage *> m_itemMap;
};

#endif // PAGEANPRSETTINGS_H

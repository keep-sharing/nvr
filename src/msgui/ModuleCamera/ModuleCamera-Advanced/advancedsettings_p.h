#pragma once
#include <QObject>
#include <QPointer>
#include <QHash>

class AdvancedSettings;
class AbstractAdvancedSettingsPage;
class AdvancedSettingsPrivate : public QObject
{
    Q_OBJECT
public:
    enum ItemCategory
    {
	ItemNone = -1,
        ItemWatermark = 0,
	ItemHeatmap
    };
public:
    AdvancedSettingsPrivate(QObject *parent);
    bool isChannelConnected(int channel);
public slots:
    void onTabClicked(int index);
    void onChannelClicked(int channel);
public:
    bool isInitializing = false;
    int currentChannel = -1;
    ItemCategory currentItem = ItemNone;
    QHash<ItemCategory, AbstractAdvancedSettingsPage *> items;
    AdvancedSettings *q;
};
